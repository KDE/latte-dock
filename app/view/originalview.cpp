/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "originalview.h"
#include "clonedview.h"
#include "positioner.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/storage.h"

// KDE
#include <KLocalizedString>

namespace Latte {
OriginalView::OriginalView(Plasma::Corona *corona, QScreen *targetScreen, bool byPassX11WM)
    : View(corona, targetScreen, byPassX11WM)
{
    connect(this, &View::containmentChanged, this, [&]() {
        if (!this->containment()) {
            return;
        }

        connect(containment(), &Plasma::Applet::destroyedChanged, this, &OriginalView::syncClonesToScreens);
        restoreConfig();
    });

    connect(this, &View::layoutChanged, this, &OriginalView::syncClonesToScreens);
    connect(this, &OriginalView::screensGroupChanged, this, &OriginalView::syncClonesToScreens);
    connect(this, &OriginalView::screensGroupChanged, this, &OriginalView::saveConfig);
}

OriginalView::~OriginalView()
{
    cleanClones();
}

bool OriginalView::isSingle() const
{
    return m_screensGroup == Latte::Types::SingleScreenGroup;
}

bool OriginalView::isOriginal() const
{
    return true;
}

bool OriginalView::isCloned() const
{
    return !isOriginal();
}

int OriginalView::clonesCount() const
{
    return m_clones.count();
}

int OriginalView::expectedScreenIdFromScreenGroup(const Latte::Types::ScreensGroup &nextScreensGroup) const
{
    Data::View view = data();
    view.screensGroup = nextScreensGroup;
    return Latte::Layouts::Storage::self()->expectedViewScreenId(m_corona, view);
}

Latte::Types::ScreensGroup OriginalView::screensGroup() const
{
    return m_screensGroup;
}

void OriginalView::setScreensGroup(const Latte::Types::ScreensGroup &group)
{
    if (m_screensGroup == group) {
        return;
    }

    m_screensGroup = group;
    emit screensGroupChanged();
}

void OriginalView::addClone(Latte::ClonedView *view)
{
    if (m_clones.contains(view)) {
        return;
    }

    m_clones << view;
    m_waitingCreation.removeAll(view->positioner()->currentScreenId());
}

void OriginalView::removeClone(Latte::ClonedView *view)
{
    if (!m_clones.contains(view)) {
        return;
    }

    int idx = m_clones.indexOf(view);
    auto cloned = m_clones.takeAt(idx);

    if (!cloned->layout()) {
        return;
    }
    cloned->positioner()->slideOutDuringExit();
    cloned->layout()->removeView(cloned->data());
}

void OriginalView::createClone(int screenId)
{
    if (!layout() || !containment()) {
        return;
    }

    QString templateFile = layout()->storedView(containment()->id());

    if (templateFile.isEmpty()) {
        return;
    }

    Data::ViewsTable templateviews = Layouts::Storage::self()->views(templateFile);

    if (templateviews.rowCount() <= 0) {
        return;
    }

    Data::View nextdata = templateviews[0];
    nextdata.name = i18nc("clone of original dock panel, name","Clone of %1", name());
    nextdata.onPrimary = false;
    nextdata.screensGroup = Latte::Types::SingleScreenGroup;
    nextdata.isClonedFrom = containment()->id();
    nextdata.screen = screenId;

    nextdata.setState(Data::View::OriginFromViewTemplate, templateFile);

    if (!m_waitingCreation.contains(screenId)) {
        m_waitingCreation << screenId;
        layout()->newView(nextdata);
    }
}

void OriginalView::cleanClones()
{
    if (m_clones.count()==0) {
        return;
    }

    while(!m_clones.isEmpty()) {
        removeClone(m_clones[0]);
    }
}

void OriginalView::reconsiderScreen()
{
    View::reconsiderScreen();
    syncClonesToScreens();
}

void OriginalView::setNextLocationForClones(const QString layoutName, int edge, int alignment)
{
    if (m_clones.count()==0) {
        return;
    }

    for (const auto clone : m_clones) {
        clone->positioner()->setNextLocation(layoutName, Latte::Types::SingleScreenGroup, "", edge, alignment);
    }
}

void OriginalView::addApplet(const QString &pluginId, const int &excludecloneid)
{
    if (m_clones.count() == 0) {
        return;
    }

    // add applet in original view
    extendedInterface()->addApplet(pluginId);

    // add applet in clones and exclude the one that probably produced this triggering
    for(const auto clone: m_clones) {
        if (clone->containment()->id() == excludecloneid) {
            // this way we make sure that an applet will not be double added
            continue;
        }

        clone->extendedInterface()->addApplet(pluginId);
    }
}

void OriginalView::addApplet(QObject *mimedata, const int &x, const int &y, const int &excludecloneid)
{
    if (m_clones.count() == 0) {
        return;
    }

    // add applet in original view
    extendedInterface()->addApplet(mimedata, x, y);

    // add applet in clones and exclude the one that probably produced this triggering
    for(const auto clone: m_clones) {
        if (clone->containment()->id() == excludecloneid) {
            // this way we make sure that an applet will not be double added
            continue;
        }

        clone->extendedInterface()->addApplet(mimedata, x, y);
    }
}

void OriginalView::syncClonesToScreens()
{
    if (isSingle() || (containment() && containment()->destroyed())) {
        cleanClones();
        return;
    }

    QList<int> secondaryscreens = m_corona->screenPool()->secondaryScreenIds();

    for (const auto scrid : secondaryscreens) {
        if (m_waitingCreation.contains(scrid)) {
            secondaryscreens.removeAll(scrid);
        }
    }

    if (m_screensGroup == Latte::Types::AllSecondaryScreensGroup) {
        //! occuped screen from original view in "allsecondaryscreensgroup" must be ignored
        secondaryscreens.removeAll(expectedScreenIdFromScreenGroup(m_screensGroup));
    }

    QList<Latte::ClonedView *> removable;

    for (const auto clone : m_clones) {
        if (secondaryscreens.contains(clone->positioner()->currentScreenId())) {
            // do nothing valid clone
            secondaryscreens.removeAll(clone->positioner()->currentScreenId());
        } else {
            // must be removed the screen is not active
            removable << clone;
        }
    }

    for (const auto scrid : secondaryscreens) {
        if (removable.count() > 0) {
            //! move deprecated and available clone to valid secondary screen
            auto clone = removable.takeFirst();
            clone->positioner()->setScreenToFollow(m_corona->screenPool()->screenForId(scrid));
        } else {
            //! create a new clone
            createClone(scrid);
        }
    }

    for (auto removableclone : removable) {
        //! remove deprecated clones
        removeClone(removableclone);
    }
}

void OriginalView::saveConfig()
{

    if (!this->containment()) {
        return;
    }

    auto config = this->containment()->config();
    config.writeEntry("screensGroup", (int)m_screensGroup);
}

void OriginalView::restoreConfig()
{
    if (!this->containment()) {
        return;
    }

    auto config = this->containment()->config();
    m_screensGroup = static_cast<Latte::Types::ScreensGroup>(config.readEntry("screensGroup", (int)Latte::Types::SingleScreenGroup));

    //! Send changed signals at the end in order to be sure that saveConfig
    //! wont rewrite default/invalid values
    emit screensGroupChanged();
}

}
