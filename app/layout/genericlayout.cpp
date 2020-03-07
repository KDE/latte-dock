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
#include "storage.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/importer.h"
#include "../layouts/manager.h"
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
#include <KConfigGroup>

namespace Latte {
namespace Layout {

GenericLayout::GenericLayout(QObject *parent, QString layoutFile, QString assignedName)
    : AbstractLayout (parent, layoutFile, assignedName),
      m_storage(new Storage(this))
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

    QList<Plasma::Containment *> systrays;

    //!identify systrays and unload them first
    for (const auto containment : m_containments) {
        if (Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(containment->parent())) {
            systrays.append(containment);
        }
    }

    while (!systrays.isEmpty()) {
        Plasma::Containment *systray = systrays.at(0);
        m_unloadedContainmentsIds << QString::number(systray->id());
        systrays.removeFirst();
        m_containments.removeAll(systray);
        delete systray;
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
    disconnect(m_corona->activityConsumer(), &KActivities::Consumer::currentActivityChanged, this, &GenericLayout::updateLastUsedActivity);

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

bool GenericLayout::configViewIsShown() const
{
    for (const auto view : m_latteViews) {
        if (view && view->settingsWindowIsShown()) {
            return true;
        }
    }

    return false;
}

bool GenericLayout::isActive() const
{
    if (!m_corona) {
        return false;
    }

    GenericLayout *generic = m_corona->layoutsManager()->synchronizer()->layout(m_layoutName);

    if (generic) {
        return true;
    } else {
        return false;
    }
}

bool GenericLayout::isCurrent() const
{
    if (!m_corona) {
        return false;
    }

    return name() == m_corona->layoutsManager()->currentLayoutName();
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
        if (view->tasksPresent()) {
            result++;
        }
    }

    return result;
}

QStringList GenericLayout::unloadedContainmentsIds()
{
    return m_unloadedContainmentsIds;
}

Latte::Corona *GenericLayout::corona()
{
    return m_corona;
}

Types::ViewType GenericLayout::latteViewType(int containmentId) const
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
    if (!latteViews().contains(m_lastConfigViewFor)) {
        m_lastConfigViewFor = nullptr;
        return nullptr;
    }

    return m_lastConfigViewFor;
}

void GenericLayout::setLastConfigViewFor(Latte::View *view)
{
    if (m_lastConfigViewFor == view) {
        return;
    }

    m_lastConfigViewFor = view;
    emit lastConfigViewForChanged(view);
}

Latte::View *GenericLayout::viewForContainment(Plasma::Containment *containment)
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

QList<Latte::View *> GenericLayout::sortedLatteViews(QList<Latte::View *> views)
{
    QList<Latte::View *> sortedViews = views.isEmpty() ? latteViews() : views;

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

bool GenericLayout::viewDataAtLowerScreenPriority(const ViewData &test, const ViewData &base) const
{
    if (test.onPrimary && base.onPrimary) {
        return false;
    } else if (!base.onPrimary && test.onPrimary) {
        return false;
    } else if (base.onPrimary && !test.onPrimary) {
        return true;
    } else {
        return test.screenId <= base.screenId;
    }
}

bool GenericLayout::viewDataAtLowerStatePriority(const ViewData &test, const ViewData &base) const
{
    if (test.active == base.active) {
        return false;
    } else if (!base.active && test.active) {
        return false;
    } else if (base.active && !test.active) {
        return true;
    }

    return false;
}

bool GenericLayout::viewDataAtLowerEdgePriority(const ViewData &test, const ViewData &base) const
{
    QList<Plasma::Types::Location> edges{Plasma::Types::RightEdge, Plasma::Types::TopEdge,
                Plasma::Types::LeftEdge, Plasma::Types::BottomEdge};

    int testPriority = -1;
    int basePriority = -1;

    for (int i = 0; i < edges.count(); ++i) {
        if (edges[i] == base.location) {
            basePriority = i;
        }

        if (edges[i] == test.location) {
            testPriority = i;
        }
    }

    if (testPriority < basePriority) {
        return true;
    } else {
        return false;
    }
}

QList<ViewData> GenericLayout::sortedViewsData(const QList<ViewData> &viewsData)
{
    QList<ViewData> sortedData = viewsData;

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
                ViewData temp = sortedData[j + 1];
                sortedData[j + 1] = sortedData[j];
                sortedData[j] = temp;
            }
        }
    }

    return sortedData;
}


const QList<Plasma::Containment *> *GenericLayout::containments()
{
    return &m_containments;
}

QList<Latte::View *> GenericLayout::viewsWithPlasmaShortcuts()
{
    QList<Latte::View *> views;

    if (!m_corona) {
        return views;
    }

    QList<int> appletsWithShortcuts = m_corona->globalShortcuts()->shortcutsTracker()->appletsWithPlasmaShortcuts();

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

    if (m_corona->layoutsManager()->memoryUsage() == Types::SingleLayout) {
        m_containments.append(containment);
        containmentInLayout = true;
    } else if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
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
    //! In Multiple Layout the orphaned systrays must be assigned to layouts
    //! when the user adds them
    KConfigGroup appletSettings = applet->containment()->config().group("Applets").group(QString::number(applet->id())).group("Configuration");

    int systrayId = appletSettings.readEntry("SystrayContainmentId", -1);

    if (systrayId != -1) {
        uint sId = (uint)systrayId;

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
    if (!m_corona || m_corona->layoutsManager()->memoryUsage() != Types::MultipleLayouts) {
        return;
    }

    if (m_layoutFile != Layouts::Importer::layoutFilePath(newName)) {
        setFile(Layouts::Importer::layoutFilePath(newName));
    }

    setName(newName);

    for (const auto containment : m_containments) {
        qDebug() << "Cont ID :: " << containment->id();
        containment->config().writeEntry("layoutId", m_layoutName);
    }
}

void GenericLayout::addNewView()
{
    if (!m_corona) {
        return;
    }

    m_corona->addViewForLayout(name());
    emit viewEdgeChanged();
}

void GenericLayout::addView(Plasma::Containment *containment, bool forceOnPrimary, int explicitScreen, Layout::ViewsMap *occupied)
{
    qDebug() << "Layout :::: " << m_layoutName << " ::: addView was called... m_containments :: " << m_containments.size();

    if (!containment || !m_corona || !containment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    qDebug() << "step 1...";

    if (!m_storage->isLatteContainment(containment))
        return;

    qDebug() << "step 2...";

    for (auto *dock : m_latteViews) {
        if (dock->containment() == containment)
            return;
    }

    qDebug() << "step 3...";

    QScreen *nextScreen{qGuiApp->primaryScreen()};

    bool onPrimary = containment->config().readEntry("onPrimary", true);
    int id = containment->screen();

    if (id == -1 && explicitScreen == -1) {
        id = containment->lastScreen();
    }

    if (onPrimary) {
        id = m_corona->screenPool()->primaryScreenId();
    } else if (explicitScreen > -1) {
        id = explicitScreen;
    }

    Plasma::Types::Location edge = containment->location();

    QString connector = m_corona->screenPool()->hasId(id) ? m_corona->screenPool()->connector(id) : "";

    qDebug() << "Adding view - containment id:" << containment->id() << " ,screen :" << id << " - " << connector
             << " ,onprimary:" << onPrimary << " - "  << " edge:" << edge << " ,screenName:" << qGuiApp->primaryScreen()->name() << " ,forceOnPrimary:" << forceOnPrimary;

    if (occupied && m_corona->screenPool()->hasId(id) && (*occupied).contains(connector) && (*occupied)[connector].contains(edge)) {
        qDebug() << "Rejected : adding view because the edge is already occupied by a higher priority view ! : " << (*occupied)[connector][edge];
        return;
    }

    if (id >= 0 && !onPrimary && !forceOnPrimary) {
        qDebug() << "Add view - connector : " << connector;
        bool found{false};

        if (m_corona->screenPool()->hasId(id)) {
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
        if (nextScreen == qGuiApp->primaryScreen() && primaryDockOccupyEdge(containment->location())) {
            qDebug() << "Rejected : adding explicit view, primary dock occupies edge at screen ! : " << connector;
            return;
        }
    }

    if (id >= 0 && onPrimary) {
        qDebug() << "add dock - connector : " << connector;

        for (const Plasma::Containment *testContainment : m_latteViews.keys()) {
            int testScreenId = testContainment->screen();

            if (testScreenId == -1) {
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

    latteView->init();
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

    for(const auto view : latteViews()) {
        if (view->positioner()->currentScreenName() == validScreenName && view->location() == edge) {
            view->visibility()->toggleHiddenState();
            return;
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
        if (m_corona->layoutsManager()->memoryUsage() == Types::SingleLayout) {
            addContainment(containment);
        } else if (m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
            QString layoutId = containment->config().readEntry("layoutId", QString());

            if (!layoutId.isEmpty() && (layoutId == m_layoutName)) {
                addContainment(containment);
            }
        }
    }

    qDebug() << "Layout ::::: " << name() << " added containments ::: " << m_containments.size();

    updateLastUsedActivity();

    //! signals
    connect(m_corona->activityConsumer(), &KActivities::Consumer::currentActivityChanged,
            this, &GenericLayout::updateLastUsedActivity);

    connect(m_corona, &Plasma::Corona::containmentAdded, this, &GenericLayout::addContainment);

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

    if (!m_lastUsedActivity.isEmpty() && !m_corona->layoutsManager()->synchronizer()->activities().contains(m_lastUsedActivity)) {
        clearLastUsedActivity();
    }

    QString currentId = m_corona->activitiesConsumer()->currentActivity();

    QStringList appliedActivitiesIds = appliedActivities();

    if (m_lastUsedActivity != currentId
            && (appliedActivitiesIds.contains(currentId)
                || m_corona->layoutsManager()->memoryUsage() == Types::SingleLayout)) {
        m_lastUsedActivity = currentId;

        emit lastUsedActivityChanged();
    }
}

void GenericLayout::assignToLayout(Latte::View *latteView, QList<Plasma::Containment *> containments)
{
    if (!m_corona) {
        return;
    }

    if (latteView) {
        m_latteViews[latteView->containment()] = latteView;
        m_containments << containments;

        for (const auto containment : containments) {
            containment->config().writeEntry("layoutId", name());

            if (latteView->containment() != containment) {
                //! assign signals only to systrays
                //! the View::setLayout() is responsible for the View::Containment signals
                connect(containment, &QObject::destroyed, this, &GenericLayout::containmentDestroyed);
                connect(containment, &Plasma::Applet::destroyedChanged, this, &GenericLayout::destroyedChanged);
                connect(containment, &Plasma::Containment::appletCreated, this, &GenericLayout::appletCreated);
            }
        }

        latteView->setLayout(this);

        emit viewsCountChanged();
    }

    //! sync the original layout file for integrity
    if (m_corona && m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
        m_storage->syncToLayoutFile(false);
    }
}

QList<Plasma::Containment *> GenericLayout::unassignFromLayout(Latte::View *latteView)
{
    QList<Plasma::Containment *> containments;

    if (!m_corona) {
        return containments;
    }

    containments << latteView->containment();

    for (const auto containment : m_containments) {
        Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(containment->parent());

        //! add systrays from that latteView
        if (parentApplet && parentApplet->containment() && parentApplet->containment() == latteView->containment()) {
            containments << containment;
            //! unassign signals only to systrays
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
        m_latteViews.remove(latteView->containment());
    }

    //! sync the original layout file for integrity
    if (m_corona && m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
        m_storage->syncToLayoutFile(false);
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
        if (m_storage->isLatteContainment(containment)) {
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
        if (m_storage->isLatteContainment(containment)) {
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
        if (m_storage->isLatteContainment(containment)) {
            int screenId = 0;

            //! valid screen id
            if (latteViewExists(containment)) {
                screenId = m_latteViews[containment]->positioner()->currentScreenId();
            } else {
                screenId = containment->screen();

                if (screenId == -1) {
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

            if (onPrimary && (!occupiedMap || !(*occupiedMap)[prmScreenName].contains(location))) {
                map[prmScreenName][location] << containment->id();
            }
        }
    }

    Layout::ViewsMap explicitMap;

    //! second step: explicit docks must be placed in their screens if the screen edge is free
    for (const auto containment : m_containments) {
        if (m_storage->isLatteContainment(containment)) {
            int screenId = 0;

            //! valid screen id
            if (latteViewExists(containment)) {
                screenId = m_latteViews[containment]->positioner()->currentScreenId();
            } else {
                screenId = containment->screen();

                if (screenId == -1) {
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

                if (m_corona->screenPool()->screenExists(screenId) && !map[expScreenName].contains(location)) {
                    explicitMap[expScreenName][location] << containment->id();
                }
            }
        }
    }

    for(const QString &expScreenName : explicitMap.keys()) {
        for(const Plasma::Types::Location &expLocation : explicitMap[expScreenName].keys()) {
            map[expScreenName][expLocation] << explicitMap[expScreenName][expLocation];
        }
    }

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

        if (screenId == -1) {
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

QList<int> GenericLayout::containmentSystrays(Plasma::Containment *containment) const
{
    QList<int> trays;

    if (m_storage->isLatteContainment(containment)) {
        auto applets = containment->config().group("Applets");

        for (const auto &applet : applets.groupList()) {
            KConfigGroup appletSettings = applets.group(applet).group("Configuration");
            int tSysId = appletSettings.readEntry("SystrayContainmentId", -1);

            if (tSysId != -1) {
                trays << tSysId;
            }
        }
    }

    return trays;
}


QString GenericLayout::reportHtml(const ScreenPool *screenPool)
{
    //qDebug() << "DBUS CALL ::: " << identifier << " - " << value;
    auto locationText = [this](const int &location) {
        switch (location) {
        case Plasma::Types::BottomEdge: return i18nc("bottom edge", "Bottom");
        case Plasma::Types::LeftEdge: return i18nc("left edge", "Left");
        case Plasma::Types::TopEdge: return i18nc("top edge", "Top");
        case Plasma::Types::RightEdge: return i18nc("right edge", "Right");
        }

        return QString();
    };

    auto idsLineStr = [this](const QList<int> list) {
        QString line;

        for(int i=0; i<list.count(); ++i) {
            if(i!=0) {
                line += ", ";
            }
            line += "["+QString::number(list[i]) + "]";
        }

        return line;
    };

    ///////!  main report layout code

    QString report;

    int activeViews = m_latteViews.count();

    report += "<table cellspacing='8'>";
    report += "<tr>";
    report += "<td><b>" + i18nc("active docks panels","Active Views:") +"</b></td>";
    if (activeViews == 0) {
        report += "<td><b> -- </b></td>";
    } else {
        report += "<td><b><font color='blue'>" + QString::number(activeViews) +"</font></b></td>";
    }
    report += "</tr>";

    //! latte containment ids, systrays
    QHash<int, QList<int>> systrays;
    QList<int> assignedSystrays;
    QList<int> orphanSystrays;

    if (isActive()) {
        //! organize systrays
        for (const auto containment : m_containments) {
            QList<int> trays = containmentSystrays(containment);
            if (trays.count() > 0) {
                systrays[containment->id()] = trays;
                assignedSystrays << trays;
            }
        }

        //! orphan systrays
        for (const auto containment : m_containments) {
            if (!m_storage->isLatteContainment(containment) && !assignedSystrays.contains(containment->id())) {
                orphanSystrays << containment->id();
            }
        }
    } else {
        m_storage->systraysInformation(systrays, assignedSystrays, orphanSystrays);
    }

    report += "<tr>";
    report += "<td><b>" + i18n("Orphan Systrays:") +"</b></td>";
    if (orphanSystrays.count() == 0) {
        report += "<td><b> -- </b></td>";
    } else {
        report += "<td><b><font color='red'>" + idsLineStr(orphanSystrays) +"</font></b></td>";
    }
    report += "</tr>";
    report += "</table>";

    report += "<table cellspacing='14'>";
    report += "<tr><td align='center'><b>" + i18nc("view id","ID") + "</b></td>" +
            "<td align='center'><b>" + i18n("Screen") + "</b></td>" +
            "<td align='center'><b>" + i18nc("screen edge","Edge") + "</b></td>" +
            "<td align='center'><b>" + i18nc("active dock/panel","Active") + "</b></td>" +
            "<td align='center'><b>" + i18n("Systrays") + "</b></td>";

    report += "<tr><td colspan='5'><hr></td></tr>";

    QList<ViewData> viewsData;

    if (isActive()) {
        //! collect viewData results
        for (const auto containment : m_containments) {
            if (m_storage->isLatteContainment(containment)) {
                ViewData vData;
                vData.id = containment->id();
                vData.active = latteViewExists(containment);
                vData.location = containment->location();

                //! onPrimary / Screen Id
                int screenId = -1;
                bool onPrimary = true;

                if (latteViewExists(containment)) {
                    screenId = m_latteViews[containment]->positioner()->currentScreenId();
                    onPrimary = m_latteViews[containment]->onPrimary();
                } else {
                    screenId = containment->screen();
                    onPrimary = containment->config().readEntry("onPrimary", true);

                    if (screenId == -1) {
                        screenId = containment->lastScreen();
                    }
                }

                vData.onPrimary = onPrimary;
                vData.screenId = screenId;
                vData.systrays = containmentSystrays(containment);

                viewsData << vData;
            }
        }
    } else {
        viewsData = m_storage->viewsData(systrays);
    }

    //! sort views data
    viewsData = sortedViewsData(viewsData);

    QStringList unknownScreens;

    //! print viewData results
    for (int i=0; i<viewsData.count(); ++i) {
        report += "<tr>";

        //! view id
        QString idStr = "[" + QString::number(viewsData[i].id) + "]";
        if(viewsData[i].active) {
            idStr = "<b>" + idStr + "</b>";
        }
        report += "<td align='center'>" + idStr + "</td>";

        //! screen
        QString screenStr = "[" + i18nc("primary screen","Primary") + "]";
        if (viewsData[i].active && viewsData[i].onPrimary) {
            screenStr = "<font color='green'>" + screenStr + "</font>";
        }
        if (!viewsData[i].onPrimary) {
            if (!screenPool->hasId(viewsData[i].screenId)) {
                screenStr = "<font color='red'><i>[" + QString::number(viewsData[i].screenId) + "]</i></font>";

                unknownScreens << QString("[" + QString::number(viewsData[i].screenId) + "]");
            } else {
                screenStr = screenPool->connector(viewsData[i].screenId);
            }
        }
        if(viewsData[i].active) {
            screenStr = "<b>" + screenStr + "</b>";
        }
        report += "<td align='center'>" + screenStr + "</td>";

        //! edge
        QString edgeStr = locationText(viewsData[i].location);
        if(viewsData[i].active) {
            edgeStr = "<b>" + edgeStr + "</b>";
        }
        report += "<td align='center'>" + edgeStr + "</td>" ;

        //! active
        QString activeStr = " -- ";
        if(viewsData[i].active) {
            activeStr = "<b>" + i18n("Yes") + "</b>";
        }
        report += "<td align='center'>" + activeStr + "</td>" ;

        //! systrays
        QString systraysStr = " -- ";
        if (viewsData[i].systrays.count() > 0) {
            systraysStr = idsLineStr(viewsData[i].systrays);
        }
        if(viewsData[i].active) {
            systraysStr = "<b>" + systraysStr + "</b>";
        }
        report += "<td align='center'>" + systraysStr + "</td>";

        report += "</tr>";
    }

    report += "</table>";

    report += "<br/><hr>";

    QStringList errorsList;
    bool broken = m_storage->layoutIsBroken(errorsList);

    if (!broken && unknownScreens.count() == 0) {
        report += "<font color='green'>" + i18n("No errors were identified for this layout...") + "</font><br/>";
    } else {
        report += "<font color='red'><b>" + i18n("Errors:") + "</b></font><br/>";
    }

    if (broken){
        for(int i=0; i<errorsList.count(); ++i) {
            report += "<font color='red'><b>[" + QString::number(i) + "] - " + errorsList[i] + "</b></font><br/>";
        }
    }

    if (unknownScreens.count() > 0) {
        report += "<font color='red'><b>" + i18n("Unknown screens: ") + unknownScreens.join(", ") + "</b></font><br/>";
    }

    return report;
}

QList<int> GenericLayout::viewsScreens()
{
    QList<int> screens;

    if (isActive()) {
        for (const auto containment : m_containments) {
            if (m_storage->isLatteContainment(containment)) {
                int screenId = -1;

                //! valid screen id
                if (latteViewExists(containment)) {
                    screenId = m_latteViews[containment]->positioner()->currentScreenId();
                } else {
                    screenId = containment->screen();

                    if (screenId == -1) {
                        screenId = containment->lastScreen();
                    }
                }

                if (screenId!=-1 &&!screens.contains(screenId)) {
                    screens << screenId;
                }
            }
        }

        return screens;
    } else {
        return m_storage->viewsScreens();
    }
}


//! STORAGE

bool GenericLayout::isWritable() const
{
    return m_storage->isWritable();
}

void GenericLayout::lock()
{
    m_storage->lock();
}

void GenericLayout::unlock()
{
    m_storage->unlock();
}

void GenericLayout::syncToLayoutFile(bool removeLayoutId)
{
    m_storage->syncToLayoutFile(removeLayoutId);
}

void GenericLayout::copyView(Plasma::Containment *containment)
{
    m_storage->copyView(containment);
    emit viewEdgeChanged();
}

void GenericLayout::importToCorona()
{
    m_storage->importToCorona();
}

bool GenericLayout::layoutIsBroken() const
{
    QStringList errors;
    return m_storage->layoutIsBroken(errors);
}

}
}
