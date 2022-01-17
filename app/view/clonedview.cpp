/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "clonedview.h"
#include "containmentinterface.h"
#include "visibilitymanager.h"
#include "../data/viewdata.h"
#include "../layouts/storage.h"

namespace Latte {

const int ClonedView::ERRORAPPLETID;

QStringList ClonedView::CONTAINMENTMANUALSYNCEDPROPERTIES = QStringList()
        << QString("appletOrder")
        << QString("lockedZoomApplets")
        << QString("userBlocksColorizingApplets");  

ClonedView::ClonedView(Plasma::Corona *corona, Latte::OriginalView *originalView, QScreen *targetScreen, bool byPassX11WM)
    : View(corona, targetScreen, byPassX11WM),
      m_originalView(originalView)
{
    m_originalView->addClone(this);
    initSync();
}

ClonedView::~ClonedView()
{
}

void ClonedView::initSync()
{
    connect(m_originalView, &View::containmentChanged, this, &View::groupIdChanged);

    //! Update Visibility From Original
    connect(m_originalView->visibility(), &Latte::ViewPart::VisibilityManager::modeChanged, this, [&]() {
        visibility()->setMode(m_originalView->visibility()->mode());
    });

    connect(m_originalView->visibility(), &Latte::ViewPart::VisibilityManager::raiseOnDesktopChanged, this, [&]() {
        visibility()->setRaiseOnDesktop(m_originalView->visibility()->raiseOnDesktop());
    });

    connect(m_originalView->visibility(), &Latte::ViewPart::VisibilityManager::raiseOnActivityChanged, this, [&]() {
        visibility()->setRaiseOnActivity(m_originalView->visibility()->raiseOnActivity());
    });

    connect(m_originalView->visibility(), &Latte::ViewPart::VisibilityManager::enableKWinEdgesChanged, this, [&]() {
        visibility()->setEnableKWinEdges(m_originalView->visibility()->enableKWinEdges());
    });

    connect(m_originalView->visibility(), &Latte::ViewPart::VisibilityManager::timerShowChanged, this, [&]() {
        visibility()->setTimerShow(m_originalView->visibility()->timerShow());
    });

    connect(m_originalView->visibility(), &Latte::ViewPart::VisibilityManager::timerHideChanged, this, [&]() {
        visibility()->setTimerHide(m_originalView->visibility()->timerHide());
    });


    //! Update Applets from Clone -> OriginalView
    connect(extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletConfigPropertyChanged, this, &ClonedView::updateOriginalAppletConfigProperty);
    connect(extendedInterface(), &Latte::ViewPart::ContainmentInterface::initializationCompleted, this, &ClonedView::updateAppletIdsHash);
    connect(extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletsOrderChanged, this, &ClonedView::updateAppletIdsHash);
    connect(extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletDataCreated, this, &ClonedView::updateAppletIdsHash);
    connect(extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletCreated, this, [&](const QString &pluginId) {
        m_originalView->addApplet(pluginId, containment()->id());
    });

    connect(extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletDropped, this, [&](QObject *data, int x, int y) {
        m_originalView->addApplet(data, x, y, containment()->id());
    });

    //! Update Applets and Containment from OrigalView -> Clone
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::containmentConfigPropertyChanged, this, &ClonedView::updateContainmentConfigProperty);
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletConfigPropertyChanged, this, &ClonedView::onOriginalAppletConfigPropertyChanged);
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletInScheduledDestructionChanged, this, &ClonedView::onOriginalAppletInScheduledDestructionChanged);
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletRemoved, this, &ClonedView::onOriginalAppletRemoved);
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletsOrderChanged, this, &ClonedView::onOriginalAppletsOrderChanged);
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletsInLockedZoomChanged, this, &ClonedView::onOriginalAppletsInLockedZoomChanged);
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletsDisabledColoringChanged, this, &ClonedView::onOriginalAppletsDisabledColoringChanged);
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletDataCreated, this, &ClonedView::updateAppletIdsHash);
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletCreated, this->extendedInterface(), [&](const QString &pluginId) {
        extendedInterface()->addApplet(pluginId);
    });
    connect(m_originalView->extendedInterface(), &Latte::ViewPart::ContainmentInterface::appletDropped, this->extendedInterface(), [&](QObject *data, int x, int y) {
        extendedInterface()->addApplet(data, x, y);
    });

    //! Indicator
    connect(m_originalView, &Latte::View::indicatorChanged, this, &ClonedView::indicatorChanged);
}

bool ClonedView::isSingle() const
{
    return false;
}

bool ClonedView::isOriginal() const
{
    return false;
}

bool ClonedView::isCloned() const
{
    return true;
}

bool ClonedView::isPreferredForShortcuts() const
{
    return false;
}

int ClonedView::groupId() const
{
    if (!m_originalView->containment()) {
        return -1;
    }

    return m_originalView->containment()->id();
}

Latte::Types::ScreensGroup ClonedView::screensGroup() const
{
    return Latte::Types::SingleScreenGroup;
}

ViewPart::Indicator *ClonedView::indicator() const
{
    return m_originalView->indicator();
}


bool ClonedView::hasOriginalAppletId(const int &clonedid)
{
    if (clonedid < 0) {
        return false;
    }

    QHash<int, int>::const_iterator i = m_currentAppletIds.constBegin();
    while (i != m_currentAppletIds.constEnd()) {
        if (i.value() == clonedid) {
            return true;
        }

        ++i;
    }

    return false;
}

int ClonedView::originalAppletId(const int &clonedid)
{
    if (clonedid < 0) {
        return -1;
    }

    QHash<int, int>::const_iterator i = m_currentAppletIds.constBegin();
    while (i != m_currentAppletIds.constEnd()) {
        if (i.value() == clonedid) {
            return i.key();
        }

        ++i;
    }

    return -1;
}


bool ClonedView::isTranslatableToClonesOrder(const QList<int> &originalOrder)
{
    for(int i=0; i<originalOrder.count(); ++i) {
        int oid = originalOrder[i];
        if (oid < 0 ) {
            continue;
        }

        if (!m_currentAppletIds.contains(oid)) {
            return false;
        }
    }

    return true;
}

Latte::Data::View ClonedView::data() const
{
    Latte::Data::View vdata = View::data();
    vdata.isClonedFrom = m_originalView->containment()->id();
    return vdata;
}

void ClonedView::updateAppletIdsHash()
{
    QList<int> originalids = m_originalView->extendedInterface()->appletsOrder();
    QList<int> clonedids = extendedInterface()->appletsOrder();

    for (int i=0; i<originalids.count(); ++i) {
        int oid = originalids[i];
        if (oid < 0 || (m_currentAppletIds.contains(oid) && m_currentAppletIds[oid] > 0)) {
            continue;
        }

        int oindex = m_originalView->extendedInterface()->indexOfApplet(oid);
        ViewPart::AppletInterfaceData originalapplet = m_originalView->extendedInterface()->appletDataForId(oid);
        ViewPart::AppletInterfaceData clonedapplet = extendedInterface()->appletDataAtIndex(oindex);

        bool registeredclonedid = (originalAppletId(clonedapplet.id) > 0);

        if (originalapplet.id>0 && clonedapplet.id>0 && originalapplet.plugin == clonedapplet.plugin && !registeredclonedid) {
            m_currentAppletIds[originalapplet.id] = clonedapplet.id;
        }
    }
}

QList<int> ClonedView::translateToClonesOrder(const QList<int> &originalIds)
{
    QList<int> ids;

    for (int i=0; i<originalIds.count(); ++i) {
        int originalid = originalIds[i];
        if (originalid < 0 ) {
            ids << originalid;
            continue;
        }

        if (m_currentAppletIds.contains(originalid)) {
            ids << m_currentAppletIds[originalid];
        } else {
            ids << ERRORAPPLETID; //error
        }
    }

    return ids;
}

void ClonedView::showConfigurationInterface(Plasma::Applet *applet)
{
    Plasma::Containment *c = qobject_cast<Plasma::Containment *>(applet);

    if (Layouts::Storage::self()->isLatteContainment(c)) {
        m_originalView->showSettingsWindow();
    } else {
        View::showConfigurationInterface(applet);
    }
}

void ClonedView::onOriginalAppletRemoved(const int &id)
{
    if (!m_currentAppletIds.contains(id)) {
        return;
    }

    extendedInterface()->removeApplet(m_currentAppletIds[id]);
    m_currentAppletIds.remove(id);
}

void ClonedView::onOriginalAppletConfigPropertyChanged(const int &id, const QString &key, const QVariant &value)
{
    if (!m_currentAppletIds.contains(id)) {
        return;
    }

    extendedInterface()->updateAppletConfigProperty(m_currentAppletIds[id], key, value);
}

void ClonedView::onOriginalAppletInScheduledDestructionChanged(const int &id, const bool &enabled)
{
    if (!m_currentAppletIds.contains(id)) {
        return;
    }

    extendedInterface()->setAppletInScheduledDestruction(m_currentAppletIds[id], enabled);
}

void ClonedView::updateContainmentConfigProperty(const QString &key, const QVariant &value)
{
    if (!CONTAINMENTMANUALSYNCEDPROPERTIES.contains(key)) {
        extendedInterface()->updateContainmentConfigProperty(key, value);
    } else {
        //qDebug() << "org.kde.sync :: containment config value syncing blocked :: " << key;
    }
}

void ClonedView::updateOriginalAppletConfigProperty(const int &clonedid, const QString &key, const QVariant &value)
{
    if (!hasOriginalAppletId(clonedid)) {
        return;
    }

    m_originalView->extendedInterface()->updateAppletConfigProperty(originalAppletId(clonedid), key, value);
}

void ClonedView::onOriginalAppletsOrderChanged()
{
    updateAppletIdsHash();
    QList<int> originalorder = m_originalView->extendedInterface()->appletsOrder();

    if (originalorder.count() != extendedInterface()->appletsOrder().count()) {
        //probably an applet was removed or added and clone has not been updated yet
        return;
    }

    if (!isTranslatableToClonesOrder(originalorder)) {
        qDebug() << "org.kde.sync ::: original applets order changed but unfortunately original order can not be translated to cloned ids...";
        return;
    }

    QList<int> newclonesorder = translateToClonesOrder(originalorder);

    if (newclonesorder.contains(ERRORAPPLETID)) {
        qDebug() << "org.kde.sync ::: original applets order changed but unfortunately original and clones order map can not be generated...";
        return;
    }

    extendedInterface()->setAppletsOrder(newclonesorder);
}

void ClonedView::onOriginalAppletsInLockedZoomChanged(const QList<int> &originalapplets)
{
    if (!isTranslatableToClonesOrder(originalapplets)) {
        qDebug() << "org.kde.sync ::: original applets order changed but unfortunately original order can not be translated to cloned ids...";
        return;
    }

    QList<int> newclonesorder = translateToClonesOrder(originalapplets);

    if (newclonesorder.contains(ERRORAPPLETID)) {
        qDebug() << "org.kde.sync ::: original applets order changed but unfortunately original and clones order map can not be generated...";
        return;
    }

    extendedInterface()->setAppletsInLockedZoom(newclonesorder);
}

void ClonedView::onOriginalAppletsDisabledColoringChanged(const QList<int> &originalapplets)
{
    if (!isTranslatableToClonesOrder(originalapplets)) {
        qDebug() << "org.kde.sync ::: original applets order changed but unfortunately original order can not be translated to cloned ids...";
        return;
    }

    QList<int> newclonesorder = translateToClonesOrder(originalapplets);

    if (newclonesorder.contains(ERRORAPPLETID)) {
        qDebug() << "org.kde.sync ::: original applets order changed but unfortunately original and clones order map can not be generated...";
        return;
    }

    extendedInterface()->setAppletsDisabledColoring(newclonesorder);
}


}
