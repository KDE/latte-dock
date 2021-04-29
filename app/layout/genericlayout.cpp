/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "genericlayout.h"

// local
#include "abstractlayout.h"
#include "../apptypes.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/importer.h"
#include "../layouts/manager.h"
#include "../layouts/storage.h"
#include "../layouts/synchronizer.h"
#include "../shortcuts/shortcutstracker.h"
#include "../view/view.h"
#include "../view/positioner.h"

// Qt
#include <QDebug>
#include <QScreen>

// Plasma
#include <Plasma>
#include <Plasma/Applet>
#include <Plasma/Containment>

// KDE
#include <KActionCollection>
#include <KConfigGroup>

namespace Latte {
namespace Layout {

GenericLayout::GenericLayout(QObject *parent, QString layoutFile, QString assignedName)
    : AbstractLayout (parent, layoutFile, assignedName)
{
}

GenericLayout::~GenericLayout()
{
}

Type GenericLayout::type() const
{
    return Type::Generic;
}

void GenericLayout::unloadContainments()
{
    if (!m_corona) {
        return;
    }

    qDebug() << "Layout - " + name() + " : [unloadContainments]"
             << "containments ::: " << m_containments.size()
             << " ,latteViews in memory ::: " << m_latteViews.size()
             << " ,hidden latteViews in memory :::  " << m_waitingLatteViews.size();

    for (const auto view : m_latteViews) {
        view->disconnectSensitiveSignals();
    }

    for (const auto view : m_waitingLatteViews) {
        view->disconnectSensitiveSignals();
    }

    m_unloadedContainmentsIds.clear();

    QList<Plasma::Containment *> subcontainments;

    //!identify subcontainments and unload them first
    for (const auto containment : m_containments) {
        if (Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(containment->parent())) {
            subcontainments.append(containment);
        }
    }

    while (!subcontainments.isEmpty()) {
        Plasma::Containment *sub = subcontainments.at(0);
        m_unloadedContainmentsIds << QString::number(sub->id());
        subcontainments.removeFirst();
        m_containments.removeAll(sub);
        delete sub;
    }

    while (!m_containments.isEmpty()) {
        Plasma::Containment *containment = m_containments.at(0);
        m_unloadedContainmentsIds << QString::number(containment->id());
        m_containments.removeFirst();
        delete containment;
    }
}

void GenericLayout::unloadLatteViews()
{
    if (!m_corona) {
        return;
    }

    qDebug() << "Layout - " + name() + " : [unloadLatteViews]"
             << "containments ::: " << m_containments.size()
             << " ,latteViews in memory ::: " << m_latteViews.size()
             << " ,hidden latteViews in memory :::  " << m_waitingLatteViews.size();

    //!disconnect signals in order to avoid crashes when the layout is unloading
    disconnect(this, &GenericLayout::viewsCountChanged, m_corona, &Plasma::Corona::availableScreenRectChanged);
    disconnect(this, &GenericLayout::viewsCountChanged, m_corona, &Plasma::Corona::availableScreenRegionChanged);
    disconnect(this, &GenericLayout::activitiesChanged, this, &GenericLayout::updateLastUsedActivity);
    disconnect(m_corona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, &GenericLayout::updateLastUsedActivity);

    for (const auto view : m_latteViews) {
        view->disconnectSensitiveSignals();
    }

    for (const auto view : m_waitingLatteViews) {
        view->disconnectSensitiveSignals();
    }

    qDeleteAll(m_latteViews);
    qDeleteAll(m_waitingLatteViews);
    m_latteViews.clear();
    m_waitingLatteViews.clear();
}

bool GenericLayout::blockAutomaticLatteViewCreation() const
{
    return m_blockAutomaticLatteViewCreation;
}

void GenericLayout::setBlockAutomaticLatteViewCreation(bool block)
{
    if (m_blockAutomaticLatteViewCreation == block) {
        return;
    }

    m_blockAutomaticLatteViewCreation = block;
}

bool GenericLayout::isActive() const
{
    return m_corona && (m_corona->layoutsManager()->synchronizer()->layout(m_layoutName) != nullptr);
}

bool GenericLayout::isCurrent()
{
    if (!m_corona) {
        return false;
    }

    return m_corona->layoutsManager()->currentLayoutsNames().contains(name());
}

QString GenericLayout::background() const
{
    QString colorsPath = m_corona->kPackage().path() + "../../shells/org.kde.latte.shell/contents/images/canvas/";

    if (backgroundStyle() == Layout::PatternBackgroundStyle) {
        if (customBackground().isEmpty()) {

            return colorsPath + "defaultcustomprint.jpg";
        } else {
            return AbstractLayout::customBackground();
        }
    }

    return colorsPath + AbstractLayout::color() + "print.jpg";
}

QString GenericLayout::textColor() const
{
    if (backgroundStyle() == Layout::PatternBackgroundStyle && customBackground().isEmpty() && customTextColor().isEmpty()) {
        return AbstractLayout::defaultCustomTextColor();
    }

    return AbstractLayout::textColor();
}

int GenericLayout::viewsCount(int screen) const
{
    if (!m_corona) {
        return 0;
    }

    QScreen *scr = m_corona->screenPool()->screenForId(screen);

    int views{0};

    for (const auto view : m_latteViews) {
        if (view && view->screen() == scr && !view->containment()->destroyed()) {
            ++views;
        }
    }

    return views;
}

int GenericLayout::viewsCount(QScreen *screen) const
{
    if (!m_corona) {
        return 0;
    }

    int views{0};

    for (const auto view : m_latteViews) {
        if (view && view->screen() == screen && !view->containment()->destroyed()) {
            ++views;
        }
    }

    return views;
}

int GenericLayout::viewsCount() const
{
    if (!m_corona) {
        return 0;
    }

    int views{0};

    for (const auto view : m_latteViews) {
        if (view && view->containment() && !view->containment()->destroyed()) {
            ++views;
        }
    }

    return views;
}

QList<int> GenericLayout::qmlFreeEdges(int screen) const
{
    if (!m_corona) {
        const QList<int> emptyEdges;
        return emptyEdges;
    }

    const auto edges = freeEdges(screen);
    QList<int> edgesInt;

    for (const Plasma::Types::Location &edge : edges) {
        edgesInt.append(static_cast<int>(edge));
    }

    return edgesInt;
}

QList<Plasma::Types::Location> GenericLayout::freeEdges(QScreen *scr) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    for (const auto view : m_latteViews) {
        if (view && view->positioner()->currentScreenName() == scr->name()) {
            edges.removeOne(view->location());
        }
    }

    return edges;
}

QList<Plasma::Types::Location> GenericLayout::freeEdges(int screen) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    QScreen *scr = m_corona->screenPool()->screenForId(screen);

    for (const auto view : m_latteViews) {
        if (view && scr && view->positioner()->currentScreenName() == scr->name()) {
            edges.removeOne(view->location());
        }
    }

    return edges;
}

int GenericLayout::viewsWithTasks() const
{
    if (!m_corona) {
        return 0;
    }

    int result = 0;

    for (const auto view : m_latteViews) {
        if (view->extendedInterface()->hasLatteTasks() || view->extendedInterface()->hasPlasmaTasks()) {
            result++;
        }
    }

    return result;
}

QStringList GenericLayout::unloadedContainmentsIds()
{
    return m_unloadedContainmentsIds;
}

Latte::Corona *GenericLayout::corona() const
{
    return m_corona;
}

Types::ViewType GenericLayout::latteViewType(uint containmentId) const
{
    for (const auto view : m_latteViews) {
        if (view->containment() && view->containment()->id() == containmentId) {
            return view->type();
        }
    }

    return Types::DockView;
}

Latte::View *GenericLayout::highestPriorityView()
{
    QList<Latte::View *> views = sortedLatteViews();

    return (views.count() > 0 ? views[0] : nullptr);
}

Latte::View *GenericLayout::lastConfigViewFor()
{
    return m_lastConfigViewFor;
}

void GenericLayout::setLastConfigViewFor(Latte::View *view)
{
    if (m_lastConfigViewFor == view) {
        return;
    }

    m_lastConfigViewFor = view;

    if (view) {
        emit lastConfigViewForChanged(view);
    }
}

void GenericLayout::onLastConfigViewChangedFrom(Latte::View *view)
{
    if (!m_latteViews.values().contains(view)) {
        setLastConfigViewFor(nullptr);
    }
}

Latte::View *GenericLayout::viewForContainment(uint id) const
{
    for(auto view : m_latteViews) {
        if (view && view->containment()->id() == id) {
            return view;
        }
    }

    return nullptr;
}

Plasma::Containment *GenericLayout::containmentForId(uint id) const
{
    for(auto containment : m_containments) {
        if (containment->id() == id) {
            return containment;
        }
    }

    return nullptr;
}

bool GenericLayout::contains(Plasma::Containment *containment) const
{
    return m_containments.contains(containment);
}

int GenericLayout::screenForContainment(Plasma::Containment *containment)
{
    if (!containment) {
        return -1;
    }

    //! there is a pending update
    QString containmentid = QString::number(containment->id());
    if (m_pendingContainmentUpdates.containsId(containmentid)) {
        if (m_corona && m_pendingContainmentUpdates[containmentid].onPrimary) {
            return m_corona->screenPool()->primaryScreenId();
        } else {
            return m_pendingContainmentUpdates[containmentid].screen;
        }
    }

    //! there is a view present
    Latte::View *view{nullptr};

    if (m_latteViews.contains(containment)) {
        view = m_latteViews[containment];
    } else if (m_waitingLatteViews.contains(containment)) {
        view = m_waitingLatteViews[containment];
    }

    if (view && view->screen()) {
        return m_corona->screenPool()->id(view->screen()->name());
    }

    //! fallback scenario
    return containment->lastScreen();
}

bool GenericLayout::containsView(const int &containmentId) const
{
    if (!isActive()) {
        return Layouts::Storage::self()->containsView(file(), containmentId);
    }

    for(auto containment : m_containments) {
        if ((int)containment->id() == containmentId && Layouts::Storage::self()->isLatteContainment(containment)) {
            return true;
        }
    }

    return false;
}

Latte::View *GenericLayout::viewForContainment(Plasma::Containment *containment) const
{
    if (m_containments.contains(containment) && m_latteViews.contains(containment)) {
        return m_latteViews[containment];
    }

    return nullptr;
}

QList<Latte::View *> GenericLayout::latteViews()
{
    return m_latteViews.values();
}

QList<Latte::View *> GenericLayout::sortedLatteViews()
{
    return sortedLatteViews(latteViews());
}

QList<Latte::View *> GenericLayout::sortedLatteViews(QList<Latte::View *> views)
{
    QList<Latte::View *> sortedViews = views;

    qDebug() << " -------- ";

    for (int i = 0; i < sortedViews.count(); ++i) {
        qDebug() << i << ". " << sortedViews[i]->screen()->name() << " - " << sortedViews[i]->location();
    }

    //! sort the views based on screens and edges priorities
    //! views on primary screen have higher priority and
    //! for views in the same screen the priority goes to
    //! Bottom,Left,Top,Right
    for (int i = 0; i < sortedViews.size(); ++i) {
        for (int j = 0; j < sortedViews.size() - i - 1; ++j) {
            if (viewAtLowerScreenPriority(sortedViews[j], sortedViews[j + 1])
                    || (sortedViews[j]->screen() == sortedViews[j + 1]->screen()
                        && viewAtLowerEdgePriority(sortedViews[j], sortedViews[j + 1]))) {
                Latte::View *temp = sortedViews[j + 1];
                sortedViews[j + 1] = sortedViews[j];
                sortedViews[j] = temp;
            }
        }
    }

    Latte::View *highestPriorityView{nullptr};

    for (int i = 0; i < sortedViews.size(); ++i) {
        if (sortedViews[i]->isPreferredForShortcuts()) {
            highestPriorityView = sortedViews[i];
            sortedViews.removeAt(i);
            break;
        }
    }

    if (highestPriorityView) {
        sortedViews.prepend(highestPriorityView);
    }

    qDebug() << " -------- sorted -----";

    for (int i = 0; i < sortedViews.count(); ++i) {
        qDebug() << i << ". " << sortedViews[i]->isPreferredForShortcuts() << " - " << sortedViews[i]->screen()->name() << " - " << sortedViews[i]->location();
    }

    return sortedViews;
}

bool GenericLayout::viewAtLowerScreenPriority(Latte::View *test, Latte::View *base)
{
    if (!base || ! test) {
        return true;
    }

    if (base->screen() == test->screen()) {
        return false;
    } else if (base->screen() != qGuiApp->primaryScreen() && test->screen() == qGuiApp->primaryScreen()) {
        return false;
    } else if (base->screen() == qGuiApp->primaryScreen() && test->screen() != qGuiApp->primaryScreen()) {
        return true;
    } else {
        int basePriority = -1;
        int testPriority = -1;

        for (int i = 0; i < qGuiApp->screens().count(); ++i) {
            if (base->screen() == qGuiApp->screens()[i]) {
                basePriority = i;
            }

            if (test->screen() == qGuiApp->screens()[i]) {
                testPriority = i;
            }
        }

        if (testPriority <= basePriority) {
            return true;
        } else {
            return false;
        }

    }

    qDebug() << "viewAtLowerScreenPriority : shouldn't had reached here...";
    return false;
}

bool GenericLayout::viewAtLowerEdgePriority(Latte::View *test, Latte::View *base)
{
    if (!base || ! test) {
        return true;
    }

    QList<Plasma::Types::Location> edges{Plasma::Types::RightEdge, Plasma::Types::TopEdge,
                Plasma::Types::LeftEdge, Plasma::Types::BottomEdge};

    int testPriority = -1;
    int basePriority = -1;

    for (int i = 0; i < edges.count(); ++i) {
        if (edges[i] == base->location()) {
            basePriority = i;
        }

        if (edges[i] == test->location()) {
            testPriority = i;
        }
    }

    if (testPriority < basePriority) {
        return true;
    } else {
        return false;
    }
}

bool GenericLayout::viewDataAtLowerScreenPriority(const Latte::Data::View &test, const Latte::Data::View &base) const
{
    if (test.onPrimary && base.onPrimary) {
        return false;
    } else if (!base.onPrimary && test.onPrimary) {
        return false;
    } else if (base.onPrimary && !test.onPrimary) {
        return true;
    } else {
        return test.screen <= base.screen;
    }
}

bool GenericLayout::viewDataAtLowerStatePriority(const Latte::Data::View &test, const Latte::Data::View &base) const
{
    if (test.isActive == base.isActive) {
        return false;
    } else if (!base.isActive && test.isActive) {
        return false;
    } else if (base.isActive && !test.isActive) {
        return true;
    }

    return false;
}

bool GenericLayout::viewDataAtLowerEdgePriority(const Latte::Data::View &test, const Latte::Data::View &base) const
{
    QList<Plasma::Types::Location> edges{Plasma::Types::RightEdge, Plasma::Types::TopEdge,
                Plasma::Types::LeftEdge, Plasma::Types::BottomEdge};

    int testPriority = -1;
    int basePriority = -1;

    for (int i = 0; i < edges.count(); ++i) {
        if (edges[i] == base.edge) {
            basePriority = i;
        }

        if (edges[i] == test.edge) {
            testPriority = i;
        }
    }

    if (testPriority < basePriority) {
        return true;
    } else {
        return false;
    }
}

QList<Latte::Data::View> GenericLayout::sortedViewsData(const QList<Latte::Data::View> &viewsData)
{
    QList<Latte::Data::View> sortedData = viewsData;

    //! sort the views based on screens and edges priorities
    //! views on primary screen have higher priority and
    //! for views in the same screen the priority goes to
    //! Bottom,Left,Top,Right
    for (int i = 0; i < sortedData.size(); ++i) {
        for (int j = 0; j < sortedData.size() - i - 1; ++j) {
            if (viewDataAtLowerStatePriority(sortedData[j], sortedData[j + 1])
                    || viewDataAtLowerScreenPriority(sortedData[j], sortedData[j + 1])
                    || (!viewDataAtLowerScreenPriority(sortedData[j], sortedData[j + 1])
                        && viewDataAtLowerEdgePriority(sortedData[j], sortedData[j + 1])) ) {
                Latte::Data::View temp = sortedData[j + 1];
                sortedData[j + 1] = sortedData[j];
                sortedData[j] = temp;
            }
        }
    }

    return sortedData;
}


const QList<Plasma::Containment *> *GenericLayout::containments() const
{
    return &m_containments;
}

QList<Latte::View *> GenericLayout::viewsWithPlasmaShortcuts()
{
    QList<Latte::View *> views;

    if (!m_corona) {
        return views;
    }

    QList<uint> appletsWithShortcuts = m_corona->globalShortcuts()->shortcutsTracker()->appletsWithPlasmaShortcuts();

    for (const auto &appletId : appletsWithShortcuts) {
        for (const auto view : m_latteViews) {
            bool found{false};
            for (const auto applet : view->containment()->applets()) {
                if (appletId == applet->id()) {
                    if (!views.contains(view)) {
                        views.append(view);
                        found = true;
                        break;
                    }
                }
            }

            if (found) {
                break;
            }
        }
    }

    return views;
}


//! Containments Actions
void GenericLayout::addContainment(Plasma::Containment *containment)
{
    if (!containment || m_containments.contains(containment)) {
        return;
    }

    bool containmentInLayout{false};

    if (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout) {
        m_containments.append(containment);
        containmentInLayout = true;
    } else if (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
        QString layoutId = containment->config().readEntry("layoutId", QString());

        if (!layoutId.isEmpty() && (layoutId == m_layoutName)) {
            m_containments.append(containment);
            containmentInLayout = true;
        }
    }

    if (containmentInLayout) {
        if (!blockAutomaticLatteViewCreation()) {
            addView(containment);
        } else {
            qDebug() << "delaying LatteView creation for containment :: " << containment->id();
        }

        connect(containment, &QObject::destroyed, this, &GenericLayout::containmentDestroyed);
    }
}

void GenericLayout::appletCreated(Plasma::Applet *applet)
{
    //! In Multiple Layout the orphaned subcontainments must be assigned to layouts
    //! when the user adds them
    KConfigGroup appletSettings = applet->containment()->config().group("Applets").group(QString::number(applet->id()));

    int subId = Layouts::Storage::self()->subContainmentId(appletSettings);

    if (Layouts::Storage::isValid(subId)) {
        uint sId = (uint)subId;

        for (const auto containment : m_corona->containments()) {
            if (containment->id() == sId) {
                containment->config().writeEntry("layoutId", m_layoutName);
            }

            addContainment(containment);
        }
    }
}

void GenericLayout::containmentDestroyed(QObject *cont)
{
    if (!m_corona) {
        return;
    }

    Plasma::Containment *containment = static_cast<Plasma::Containment *>(cont);

    if (containment) {
        int containmentIndex = m_containments.indexOf(containment);

        if (containmentIndex >= 0) {
            m_containments.removeAt(containmentIndex);
        }

        qDebug() << "Layout " << name() << " :: containment destroyed!!!!";
        auto view = m_latteViews.take(containment);

        if (!view) {
            view = m_waitingLatteViews.take(containment);
        }

        if (view) {
            view->disconnectSensitiveSignals();
            view->positioner()->slideOutDuringExit(containment->location());
            view->deleteLater();

            emit viewEdgeChanged();
            emit viewsCountChanged();
        }
    }
}

void GenericLayout::destroyedChanged(bool destroyed)
{
    if (!m_corona) {
        return;
    }

    qDebug() << "dock containment destroyed changed!!!!";
    Plasma::Containment *sender = qobject_cast<Plasma::Containment *>(QObject::sender());

    if (!sender) {
        return;
    }

    if (destroyed) {
        m_waitingLatteViews[sender] = m_latteViews.take(static_cast<Plasma::Containment *>(sender));
    } else {
        m_latteViews[sender] = m_waitingLatteViews.take(static_cast<Plasma::Containment *>(sender));
    }

    emit viewEdgeChanged();
    emit viewsCountChanged();
}

void GenericLayout::renameLayout(QString newName)
{
    if (!m_corona || m_corona->layoutsManager()->memoryUsage() != MemoryUsage::MultipleLayouts) {
        return;
    }

    if (m_layoutFile != Layouts::Importer::layoutUserFilePath(newName)) {
        setFile(Layouts::Importer::layoutUserFilePath(newName));
    }

    setName(newName);

    for (const auto containment : m_containments) {
        qDebug() << "Cont ID :: " << containment->id();
        containment->config().writeEntry("layoutId", m_layoutName);
    }
}

void GenericLayout::addView(Plasma::Containment *containment, bool forceOnPrimary, int explicitScreen, Layout::ViewsMap *occupied)
{
    qDebug() << "Layout :::: " << m_layoutName << " ::: addView was called... m_containments :: " << m_containments.size();

    if (!containment || !m_corona || !containment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    qDebug() << "step 1...";

    if (!Layouts::Storage::self()->isLatteContainment(containment)) {
        return;
    }

    qDebug() << "step 2...";

    for (auto *dock : m_latteViews) {
        if (dock->containment() == containment)
            return;
    }

    qDebug() << "step 3...";

    QScreen *nextScreen{qGuiApp->primaryScreen()};

    bool onPrimary = containment->config().readEntry("onPrimary", true);
    int id = containment->screen();

    if (!Layouts::Storage::isValid(id) && !Layouts::Storage::isValid(explicitScreen)) {
        id = containment->lastScreen();
    }

    if (onPrimary) {
        id = m_corona->screenPool()->primaryScreenId();
    } else if (Layouts::Storage::isValid(explicitScreen)) {
        id = explicitScreen;
    }

    Plasma::Types::Location edge = containment->location();

    QString connector = m_corona->screenPool()->hasScreenId(id) ? m_corona->screenPool()->connector(id) : "";

    qDebug() << "Adding view - containment id:" << containment->id() << " ,screen :" << id << " - " << connector
             << " ,onprimary:" << onPrimary << " - "  << " edge:" << edge << " ,screenName:" << qGuiApp->primaryScreen()->name() << " ,forceOnPrimary:" << forceOnPrimary;

    if (occupied && m_corona->screenPool()->hasScreenId(id) && (*occupied).contains(connector) && (*occupied)[connector].contains(edge)) {
        qDebug() << "Rejected : adding view because the edge is already occupied by a higher priority view ! : " << (*occupied)[connector][edge];
        return;
    }

    if (Layouts::Storage::isValid(id) && !onPrimary && !forceOnPrimary) {
        qDebug() << "Add view - connector : " << connector;
        bool found{false};

        if (m_corona->screenPool()->hasScreenId(id)) {
            for (const auto scr : qGuiApp->screens()) {
                if (scr && scr->name() == connector) {
                    found = true;
                    nextScreen = scr;
                    break;
                }
            }
        }

        if (!found) {
            qDebug() << "Rejected : adding explicit view, screen not available ! : " << connector;
            return;
        }

        //! explicit dock can not be added at explicit screen when that screen is the same with
        //! primary screen and that edge is already occupied by a primary dock
        //! CAN BE REMOVED because we now accept primary and explicit views on ACTIVE SCREENS at ALL CASES
        //if (nextScreen == qGuiApp->primaryScreen() && primaryDockOccupyEdge(containment->location())) {
        //    qDebug() << "Rejected : adding explicit view, primary dock occupies edge at screen ! : " << connector;
        //    return;
        // }
    }

    if (Layouts::Storage::isValid(id) && onPrimary) {
        qDebug() << "add dock - connector : " << connector;

        for (const Plasma::Containment *testContainment : m_latteViews.keys()) {
            int testScreenId = testContainment->screen();

            if (!Layouts::Storage::isValid(testScreenId)) {
                testScreenId = testContainment->lastScreen();
            }

            bool testOnPrimary = testContainment->config().readEntry("onPrimary", true);
            Plasma::Types::Location testLocation = static_cast<Plasma::Types::Location>((int)testContainment->config().readEntry("location", (int)Plasma::Types::BottomEdge));

            if (!testOnPrimary && m_corona->screenPool()->primaryScreenId() == testScreenId && testLocation == containment->location()) {
                qDebug() << "Rejected explicit latteView and removing it in order add an onPrimary with higher priority at screen: " << connector;
                auto viewToDelete = m_latteViews.take(testContainment);
                viewToDelete->disconnectSensitiveSignals();
                viewToDelete->deleteLater();
            }
        }
    }

    qDebug() << "Adding view passed ALL checks" << " ,onPrimary:" << onPrimary << " ,screen:" << nextScreen->name() << " !!!";

    //! it is used to set the correct flag during the creation
    //! of the window... This of course is also used during
    //! recreations of the window between different visibility modes
    auto mode = static_cast<Types::Visibility>(containment->config().readEntry("visibility", static_cast<int>(Types::DodgeActive)));
    bool byPassWM{false};

    if (mode == Types::AlwaysVisible
            || mode == Types::WindowsGoBelow
            || mode == Types::WindowsCanCover
            || mode == Types::WindowsAlwaysCover) {
        byPassWM = false;
    } else {
        byPassWM = containment->config().readEntry("byPassWM", false);
    }

    auto latteView = new Latte::View(m_corona, nextScreen, byPassWM);

    latteView->init(containment);
    latteView->setContainment(containment);

    //! force this special dock case to become primary
    //! even though it isnt
    if (forceOnPrimary) {
        qDebug() << "Enforcing onPrimary:true as requested for LatteView...";
        latteView->setOnPrimary(true);
    }

    latteView->setLayout(this);


    //! Qt 5.9 creates a crash for this in wayland, that is why the check is used
    //! but on the other hand we need this for copy to work correctly and show
    //! the copied dock under X11
    //if (!KWindowSystem::isPlatformWayland()) {
    latteView->show();
    //}

    m_latteViews[containment] = latteView;

    emit viewsCountChanged();
}

void GenericLayout::toggleHiddenState(QString screenName, Plasma::Types::Location edge)
{
    if (!m_corona) {
        return;
    }

    QString validScreenName = qGuiApp->primaryScreen()->name();
    if (!screenName.isEmpty()) {
        validScreenName = screenName;
    }

    int viewsOnEdge{0};

    for(const auto view : latteViews()) {
        if (view->positioner()->currentScreenName() == validScreenName && view->location() == edge) {
            viewsOnEdge++;
        }
    }

    if (viewsOnEdge >= 1) {
        for(const auto view : latteViews()) {
            if (view->positioner()->currentScreenName() == validScreenName && view->location() == edge) {
                if (viewsOnEdge == 1 || (viewsOnEdge >1 && view->visibility() && view->visibility()->isSidebar())) {
                    view->visibility()->toggleHiddenState();
                    return;
                }
            }
        }
    }
}

bool GenericLayout::initToCorona(Latte::Corona *corona)
{
    if (m_corona) {
        return false;
    }

    m_corona = corona;

    for (const auto containment : m_corona->containments()) {
        if (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout) {
            addContainment(containment);
        } else if (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
            QString layoutId = containment->config().readEntry("layoutId", QString());

            if (!layoutId.isEmpty() && (layoutId == m_layoutName)) {
                addContainment(containment);
            }
        }
    }

    qDebug() << "Layout ::::: " << name() << " added containments ::: " << m_containments.size();

    updateLastUsedActivity();

    //! signals
    connect(this, &GenericLayout::activitiesChanged, this, &GenericLayout::updateLastUsedActivity);
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, &GenericLayout::updateLastUsedActivity);
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged, this, &GenericLayout::updateLastUsedActivity);

    connect(m_corona, &Plasma::Corona::containmentAdded, this, &GenericLayout::addContainment);

    connect(this, &GenericLayout::lastConfigViewForChanged, m_corona->layoutsManager(), &Layouts::Manager::lastConfigViewChangedFrom);
    connect(m_corona->layoutsManager(), &Layouts::Manager::lastConfigViewChangedFrom, this, &GenericLayout::onLastConfigViewChangedFrom);

    //!connect signals after adding the containment
    connect(this, &GenericLayout::viewsCountChanged, m_corona, &Plasma::Corona::availableScreenRectChanged);
    connect(this, &GenericLayout::viewsCountChanged, m_corona, &Plasma::Corona::availableScreenRegionChanged);

    emit viewsCountChanged();

    return true;
}

void GenericLayout::updateLastUsedActivity()
{
    if (!m_corona) {
        return;
    }

    QString currentId = m_corona->activitiesConsumer()->currentActivity();
    QStringList appliedActivitiesIds = appliedActivities();

    if (appliedActivitiesIds.contains(Data::Layout::ALLACTIVITIESID)
            || (m_lastUsedActivity != currentId && appliedActivitiesIds.contains(currentId))) {
        m_lastUsedActivity = currentId;
        emit lastUsedActivityChanged();
    }
}

void GenericLayout::assignToLayout(Latte::View *latteView, QList<Plasma::Containment *> containments)
{
    if (!m_corona || containments.isEmpty()) {
        return;
    }

    if (latteView) {
        m_latteViews[latteView->containment()] = latteView;
    }

    m_containments << containments;

    for (const auto containment : containments) {
        containment->config().writeEntry("layoutId", name());

        if (!latteView || (latteView && latteView->containment() != containment)) {
            //! assign signals only to subcontainments
            //! the View::setLayout() is responsible for the View::Containment signals
            connect(containment, &QObject::destroyed, this, &GenericLayout::containmentDestroyed);
            connect(containment, &Plasma::Applet::destroyedChanged, this, &GenericLayout::destroyedChanged);
            connect(containment, &Plasma::Containment::appletCreated, this, &GenericLayout::appletCreated);
        }
    }

    if (latteView) {
        latteView->setLayout(this);
    }

    emit viewsCountChanged();

    //! sync the original layout file for integrity
    if (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
        Layouts::Storage::self()->syncToLayoutFile(this, false);
    }
}

QList<Plasma::Containment *> GenericLayout::unassignFromLayout(Plasma::Containment *latteContainment)
{
    QList<Plasma::Containment *> containments;

    if (!m_corona || !latteContainment || !contains(latteContainment)) {
        return containments;
    }

    containments << latteContainment;

    for (const auto containment : m_containments) {
        Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(containment->parent());

        //! add subcontainments from that latteView
        if (parentApplet && parentApplet->containment() && parentApplet->containment() == latteContainment) {
            containments << containment;
            //! unassign signals only to subcontainments
            //! the View::setLayout() is responsible for the View::Containment signals
            disconnect(containment, &QObject::destroyed, this, &GenericLayout::containmentDestroyed);
            disconnect(containment, &Plasma::Applet::destroyedChanged, this, &GenericLayout::destroyedChanged);
            disconnect(containment, &Plasma::Containment::appletCreated, this, &GenericLayout::appletCreated);
        }
    }

    for (const auto containment : containments) {
        m_containments.removeAll(containment);
    }

    if (containments.size() > 0) {
        m_latteViews.remove(latteContainment);
    }

    //! sync the original layout file for integrity
    if (m_corona && m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
        Layouts::Storage::self()->syncToLayoutFile(this, false);
    }

    return containments;
}

void GenericLayout::recreateView(Plasma::Containment *containment, bool delayed)
{
    if (!m_corona || m_viewsToRecreate.contains(containment) || !containment || !m_latteViews.contains(containment)) {
        return;
    }

    int delay = delayed ? 350 : 0;
    m_viewsToRecreate << containment;

    //! give the time to config window to close itself first and then recreate the dock
    //! step:1 remove the latteview
    QTimer::singleShot(delay, [this, containment]() {
        auto view = m_latteViews[containment];
        view->disconnectSensitiveSignals();

        //! step:2 add the new latteview
        connect(view, &QObject::destroyed, this, [this, containment]() {
            auto view = m_latteViews.take(containment);
            QTimer::singleShot(250, this, [this, containment]() {
                if (!m_latteViews.contains(containment)) {
                    qDebug() << "recreate - step 2: adding dock for containment:" << containment->id();
                    addView(containment);
                    m_viewsToRecreate.removeAll(containment);
                }
            });
        });

        view->deleteLater();
    });
}


bool GenericLayout::latteViewExists(Plasma::Containment *containment)
{
    if (!m_corona) {
        return false;
    }

    return m_latteViews.keys().contains(containment);
}

QList<Plasma::Types::Location> GenericLayout::availableEdgesForView(QScreen *scr, Latte::View *forView) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    for (const auto view : m_latteViews) {
        //! make sure that availabe edges takes into account only views that should be excluded,
        //! this is why the forView should not be excluded
        if (view && view != forView && view->positioner()->currentScreenName() == scr->name()) {
            edges.removeOne(view->location());
        }
    }

    return edges;
}

bool GenericLayout::explicitDockOccupyEdge(int screen, Plasma::Types::Location location) const
{
    if (!m_corona) {
        return false;
    }

    for (const auto containment : m_containments) {
        if (Layouts::Storage::self()->isLatteContainment(containment)) {
            bool onPrimary = containment->config().readEntry("onPrimary", true);
            int id = containment->lastScreen();
            Plasma::Types::Location contLocation = containment->location();

            if (!onPrimary && id == screen && contLocation == location) {
                return true;
            }
        }
    }

    return false;
}

bool GenericLayout::primaryDockOccupyEdge(Plasma::Types::Location location) const
{
    if (!m_corona) {
        return false;
    }

    for (const auto containment : m_containments) {
        if (Layouts::Storage::self()->isLatteContainment(containment)) {
            bool onPrimary{false};

            if (m_latteViews.contains(containment)) {
                onPrimary = m_latteViews[containment]->onPrimary();
            } else {
                onPrimary = containment->config().readEntry("onPrimary", true);
            }

            Plasma::Types::Location contLocation = containment->location();

            if (onPrimary && contLocation == location) {
                return true;
            }
        }
    }

    return false;
}

bool GenericLayout::mapContainsId(const Layout::ViewsMap *map, uint viewId) const
{
    for(const auto &scr : map->keys()) {
        for(const auto &edge : (*map)[scr].keys()) {
            if ((*map)[scr][edge].contains(viewId)) {
                return true;
            }
        }
    }

    return false;
}

//! screen name, location, containmentId
Layout::ViewsMap GenericLayout::validViewsMap(Layout::ViewsMap *occupiedMap)
{
    //! Shared Views occupy the screen edge first
    //! Primary Views occupy the screen edge if Shared Views do not exist already on that screen edge
    //! Explicity Views occypy the screen edge if Shared Views and Primary Views do not exist already on that screen edge
    Layout::ViewsMap map;

    if (!m_corona) {
        return map;
    }

    if (occupiedMap != nullptr) {
        map = (*occupiedMap);
    }

    QString prmScreenName = qGuiApp->primaryScreen()->name();

    //! first step: primary docks must be placed in primary screen free edges
    for (const auto containment : m_containments) {
        if (Layouts::Storage::self()->isLatteContainment(containment)) {
            int screenId{Layouts::Storage::IDNULL};

            //! valid screen id
            if (latteViewExists(containment)) {
                screenId = m_latteViews[containment]->positioner()->currentScreenId();
            } else {
                screenId = containment->screen();

                if (!Layouts::Storage::isValid(screenId)) {
                    screenId = containment->lastScreen();
                }
            }

            bool onPrimary{true};

            //! valid onPrimary flag
            if (latteViewExists(containment)) {
                onPrimary = m_latteViews[containment]->onPrimary();
            } else {
                onPrimary = containment->config().readEntry("onPrimary", true);
            }

            //! valid location
            Plasma::Types::Location location = containment->location();

            if (onPrimary) {
                map[prmScreenName][location] << containment->id();
            } else {
                QString expScreenName = m_corona->screenPool()->connector(screenId);

                if (m_corona->screenPool()->isScreenActive(screenId)) {
                    map[expScreenName][location] << containment->id();
                }
            }
        }
    }

    /*
    //! CAN BE REMOVED because we now accept primary and explicit views on ACTIVE SCREENS at ALL CASES
    Layout::ViewsMap explicitMap;

    //! second step: explicit docks must be placed in their screens if the screen edge is free
    for (const auto containment : m_containments) {
        if (Layouts::Storage::self()->isLatteContainment(containment)) {
            int screenId{Layouts::Storage::IDNULL};

            //! valid screen id
            if (latteViewExists(containment)) {
                screenId = m_latteViews[containment]->positioner()->currentScreenId();
            } else {
                screenId = containment->screen();

                if (!Layouts::Storage::isValid(screenId)) {
                    screenId = containment->lastScreen();
                }
            }

            bool onPrimary{true};

            //! valid onPrimary flag
            if (latteViewExists(containment)) {
                onPrimary = m_latteViews[containment]->onPrimary();
            } else {
                onPrimary = containment->config().readEntry("onPrimary", true);
            }

            //! valid location
            Plasma::Types::Location location = containment->location();

            if (!onPrimary) {
                QString expScreenName = m_corona->screenPool()->connector(screenId);

                if (m_corona->screenPool()->isScreenActive(screenId) && !map[expScreenName].contains(location)) {
                    explicitMap[expScreenName][location] << containment->id();
                }
            }
        }
    }

    for(const QString &expScreenName : explicitMap.keys()) {
        for(const Plasma::Types::Location &expLocation : explicitMap[expScreenName].keys()) {
            map[expScreenName][expLocation] << explicitMap[expScreenName][expLocation];
        }
    }*/

    return map;
}


//! the central functions that updates loading/unloading latteviews
//! concerning screen changed (for multi-screen setups mainly)
void GenericLayout::syncLatteViewsToScreens(Layout::ViewsMap *occupiedMap)
{
    if (!m_corona) {
        return;
    }

    qDebug() << "START of SyncLatteViewsToScreens ....";
    qDebug() << "LAYOUT ::: " << name();
    qDebug() << "screen count changed -+-+ " << qGuiApp->screens().size();

    //! Clear up pendingContainmentUpdates when no-needed any more
    QStringList clearpendings;
    for(int i=0; i<m_pendingContainmentUpdates.rowCount(); ++i) {
        auto viewdata = m_pendingContainmentUpdates[i];
        auto containment = containmentForId(viewdata.id.toUInt());

        if (containment) {
            if ((viewdata.onPrimary && containment->lastScreen() == m_corona->screenPool()->primaryScreenId())
                    || (!viewdata.onPrimary && containment->lastScreen() == viewdata.screen)) {
                clearpendings << viewdata.id;
            }
        }
    }

    for(auto pendingid : clearpendings) {
        m_pendingContainmentUpdates.remove(pendingid);
    }

    if (m_pendingContainmentUpdates.rowCount() > 0) {
        qDebug () << "  Pending View updates still valid : ";
        m_pendingContainmentUpdates.print();
    }

    //! use valid views map based on active screens
    Layout::ViewsMap viewsMap = validViewsMap(occupiedMap);

    if (occupiedMap != nullptr) {
        qDebug() << "Occupied map used :: " << *occupiedMap;
    }

    QString prmScreenName = qGuiApp->primaryScreen()->name();

    qDebug() << "PRIMARY SCREEN :: " << prmScreenName;
    qDebug() << "LATTEVIEWS MAP :: " << viewsMap;

    //! add views
    for (const auto containment : m_containments) {
        int screenId = containment->screen();

        if (!Layouts::Storage::isValid(screenId)) {
            screenId = containment->lastScreen();
        }

        if (!latteViewExists(containment) && mapContainsId(&viewsMap, containment->id())) {
            qDebug() << "syncLatteViewsToScreens: view must be added... for containment:" << containment->id() << " at screen:" << m_corona->screenPool()->connector(screenId);
            addView(containment);
        }
    }

    //! remove views
    QList<Plasma::Containment *> viewsToDelete;

    for (auto view : m_latteViews) {
        auto containment = view->containment();
        if (containment && !mapContainsId(&viewsMap, containment->id())) {
            viewsToDelete << containment;
        }
    }

    while(!viewsToDelete.isEmpty()) {
        auto containment = viewsToDelete.takeFirst();
        auto view = m_latteViews.take(containment);
        qDebug() << "syncLatteViewsToScreens: view must be deleted... for containment:" << containment->id() << " at screen:" << view->positioner()->currentScreenName();
        view->disconnectSensitiveSignals();
        view->deleteLater();
    }

    //! reconsider views
    for (const auto view : m_latteViews) {
        if (view->containment() && mapContainsId(&viewsMap, view->containment()->id())) {
            //! if the dock will not be deleted its a very good point to reconsider
            //! if the screen in which is running is the correct one
            qDebug() << "syncLatteViewsToScreens: view must consider its screen... for containment:" << view->containment()->id() << " at screen:" << view->positioner()->currentScreenName();
            view->reconsiderScreen();
        }
    }

    qDebug() << "end of, syncLatteViewsToScreens ....";
}

QList<Plasma::Containment *> GenericLayout::subContainmentsOf(uint id) const
{
    QList<Plasma::Containment *> subs;

    auto containment = containmentForId(id);

    if (!containment || !Layouts::Storage::self()->isLatteContainment(containment)) {
        return subs;
    }

    auto applets = containment->config().group("Applets");

    for (const auto &applet : applets.groupList()) {
        int tSubId = Layouts::Storage::self()->subContainmentId(applets.group(applet));

        if (Layouts::Storage::isValid(tSubId)) {
            auto subcontainment = containmentForId(tSubId);

            if (subcontainment) {
                subs << subcontainment;
            }
        }
    }

    return subs;
}

QList<int> GenericLayout::subContainmentsOf(Plasma::Containment *containment) const
{
    QList<int> subs;

    if (Layouts::Storage::self()->isLatteContainment(containment)) {
        auto applets = containment->config().group("Applets");

        for (const auto &applet : applets.groupList()) {
            int tSubId = Layouts::Storage::self()->subContainmentId(applets.group(applet));

            if (Layouts::Storage::isValid(tSubId)) {
                subs << tSubId;
            }
        }
    }

    return subs;
}

QList<int> GenericLayout::viewsScreens()
{
    QList<int> screens;

    if (isActive()) {
        for (const auto containment : m_containments) {
            if (Layouts::Storage::self()->isLatteContainment(containment)) {
                int screenId{Layouts::Storage::IDNULL};

                //! valid screen id
                if (latteViewExists(containment)) {
                    screenId = m_latteViews[containment]->positioner()->currentScreenId();
                } else {
                    screenId = containment->screen();

                    if (!Layouts::Storage::isValid(screenId)) {
                        screenId = containment->lastScreen();
                    }
                }

                if (Layouts::Storage::isValid(screenId) && !screens.contains(screenId)) {
                    screens << screenId;
                }
            }
        }

        return screens;
    } else {
        return Layouts::Storage::self()->viewsScreens(file());
    }
}


//! STORAGE

bool GenericLayout::isWritable() const
{
    return Layouts::Storage::self()->isWritable(this);
}

void GenericLayout::lock()
{
    Layouts::Storage::self()->lock(this);
}

void GenericLayout::unlock()
{
    Layouts::Storage::self()->unlock(this);
}

void GenericLayout::syncToLayoutFile(bool removeLayoutId)
{
    syncSettings();
    Layouts::Storage::self()->syncToLayoutFile(this, removeLayoutId);
}

Data::View GenericLayout::newView(const Latte::Data::View &nextViewData)
{
    if (nextViewData.state() == Data::View::IsInvalid) {
        return Data::View();
    }

    Data::View result = Layouts::Storage::self()->newView(this, nextViewData);
    emit viewEdgeChanged();

    return result;
}

void GenericLayout::updateView(const Latte::Data::View &viewData)
{
    //! storage -> storage [view scenario]
    if (!isActive()) {
        Layouts::Storage::self()->updateView(this, viewData);
        return;
    }

    //! active -> active [view scenario]
    Latte::View *view = viewForContainment(viewData.id.toUInt());
    bool viewMustBeDeleted = (view && !viewData.onPrimary && !m_corona->screenPool()->isScreenActive(viewData.screen));

    QString nextactivelayoutname = (viewData.state() == Data::View::OriginFromLayout && !viewData.originLayout().isEmpty() ? viewData.originLayout() : QString());

    if (view) {
        if (!viewMustBeDeleted) {
            QString scrName = Latte::Data::Screen::ONPRIMARYNAME;

            if (!viewData.onPrimary) {
                if (m_corona->screenPool()->hasScreenId(viewData.screen)) {
                    scrName = m_corona->screenPool()->connector(viewData.screen);
                } else {
                    scrName = "";
                }
            }

            view->setName(viewData.name);
            view->positioner()->setNextLocation(nextactivelayoutname, scrName, viewData.edge, viewData.alignment);
            return;
        } else {
            //! viewMustBeDeleted
            m_latteViews.remove(view->containment());
            view->disconnectSensitiveSignals();
            delete view;
        }
    }

    //! inactiveinmemory -> active/inactiveinmemory [viewscenario]
    //! active -> inactiveinmemory                  [viewscenario]
    auto containment = containmentForId(viewData.id.toUInt());
    if (containment) {
        Layouts::Storage::self()->updateView(this, viewData);

        //! by using pendingContainmentUpdates we make sure that when containment->screen() will be
        //! called though reactToScreenChange() the proper screen will be returned
        if (!m_pendingContainmentUpdates.containsId(viewData.id)) {
            m_pendingContainmentUpdates << viewData;
        } else {
            m_pendingContainmentUpdates[viewData.id] = viewData;
        }
        containment->reactToScreenChange();
    }

    if (!nextactivelayoutname.isEmpty()) {
        m_corona->layoutsManager()->moveView(name(), viewData.id.toUInt(), nextactivelayoutname);
    }

    //! complete update circle and inform the others about the changes
    if (viewMustBeDeleted) {
        emit viewEdgeChanged();
        emit viewsCountChanged();
    }

    syncLatteViewsToScreens();
}

void GenericLayout::removeView(const Latte::Data::View &viewData)
{
    if (!containsView(viewData.id.toInt())) {
        return;
    }

    if (!isActive()) {
        Layouts::Storage::self()->removeView(file(), viewData);
        return;
    }

    Plasma::Containment *viewcontainment = containmentForId(viewData.id.toUInt());

    if (viewcontainment) {
        m_containments.removeAll(viewcontainment);
        viewcontainment->setImmutability(Plasma::Types::Mutable);
        viewcontainment->destroy();
    }
}

QString GenericLayout::storedView(const int &containmentId)
{
    return Layouts::Storage::self()->storedView(this, containmentId);
}

void GenericLayout::importToCorona()
{
    Layouts::Storage::self()->importToCorona(this);
}

bool GenericLayout::isBroken() const
{
    QStringList errors;
    return Layouts::Storage::self()->isBroken(this, errors);
}

Data::ErrorsList GenericLayout::errors() const
{
    return Layouts::Storage::self()->errors(this);
}

Data::WarningsList GenericLayout::warnings() const
{
    return Layouts::Storage::self()->warnings(this);
}

Latte::Data::ViewsTable GenericLayout::viewsTable() const
{
    return Layouts::Storage::self()->views(this);
}

}
}
