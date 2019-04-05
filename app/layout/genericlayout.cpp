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
#include "../importer.h"
#include "../lattecorona.h"
#include "../layoutmanager.h"
#include "../screenpool.h"
#include "../shortcuts/shortcutstracker.h"
#include "../view/view.h"
#include "../view/positioner.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
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
    : AbstractLayout (parent, layoutFile, assignedName)
{
}

GenericLayout::~GenericLayout()
{
}

void GenericLayout::unloadContainments()
{
    if (!m_corona) {
        return;
    }

    //!disconnect signals in order to avoid crashes when the layout is unloading
    disconnect(this, &GenericLayout::viewsCountChanged, m_corona, &Plasma::Corona::availableScreenRectChanged);
    disconnect(this, &GenericLayout::viewsCountChanged, m_corona, &Plasma::Corona::availableScreenRegionChanged);

    qDebug() << "Layout - " + name() + " unload: containments ... size ::: " << m_containments.size()
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

    qDebug() << "Layout - " + name() + " unload: latteViews ... size: " << m_latteViews.size();

    qDeleteAll(m_latteViews);
    qDeleteAll(m_waitingLatteViews);
    m_latteViews.clear();
    m_waitingLatteViews.clear();
}

bool GenericLayout::isLatteContainment(Plasma::Containment *containment)
{
    if (!containment) {
        return false;
    }

    if (containment->pluginMetaData().pluginId() == "org.kde.latte.containment") {
        return true;
    }

    return false;
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

    return views.count() > 0 ? views[0] : nullptr;
}

QHash<const Plasma::Containment *, Latte::View *> *GenericLayout::latteViews()
{
    return &m_latteViews;
}

QList<Latte::View *> GenericLayout::sortedLatteViews()
{
    QList<Latte::View *> sortedViews;

    //! create views list to be sorted out
    for (const auto view : m_latteViews) {
        sortedViews.append(view);
    }

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

    if (testPriority < basePriority)
        return true;
    else
        return false;
}

QList<Plasma::Containment *> *GenericLayout::containments()
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

    if (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout) {
        m_containments.append(containment);
        containmentInLayout = true;
    } else if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
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

    emit viewsCountChanged();
}

void GenericLayout::renameLayout(QString newName)
{
    if (m_layoutFile != Importer::layoutFilePath(newName)) {
        setFile(Importer::layoutFilePath(newName));
    }

    if (m_layoutName != newName) {
        setName(newName);
    }

    //! thus this is a linked file
    if (m_corona) {
        for (const auto containment : m_containments) {
            containment->config().writeEntry("layoutId", m_layoutName);
        }
    }
}

void GenericLayout::addNewView()
{
    if (!m_corona) {
        return;
    }

    m_corona->loadDefaultLayout();
}

void GenericLayout::addView(Plasma::Containment *containment, bool forceOnPrimary, int explicitScreen)
{
    qDebug() << "Layout :::: " << m_layoutName << " ::: addView was called... m_containments :: " << m_containments.size();

    if (!containment || !m_corona || !containment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    qDebug() << "step 1...";

    if (!isLatteContainment(containment))
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

    if (explicitScreen > -1) {
        id = explicitScreen;
    }

    qDebug() << "add dock - containment id: " << containment->id() << " ,screen : " << id << " - " << m_corona->screenPool()->connector(id)
             << " ,onprimary:" << onPrimary << " - " << qGuiApp->primaryScreen()->name() << " ,forceOnPrimary:" << forceOnPrimary;

    if (id >= 0 && !onPrimary && !forceOnPrimary) {
        QString connector = m_corona->screenPool()->connector(id);
        qDebug() << "add dock - connector : " << connector;
        bool found{false};

        for (const auto scr : qGuiApp->screens()) {
            if (scr && scr->name() == connector) {
                found = true;
                nextScreen = scr;
                break;
            }
        }

        if (!found) {
            qDebug() << "reject : adding explicit dock, screen not available ! : " << connector;
            return;
        }

        //! explicit dock can not be added at explicit screen when that screen is the same with
        //! primary screen and that edge is already occupied by a primary dock
        if (nextScreen == qGuiApp->primaryScreen() && primaryDockOccupyEdge(containment->location())) {
            qDebug() << "reject : adding explicit dock, primary dock occupies edge at screen ! : " << connector;
            return;
        }
    }

    if (id >= 0 && onPrimary) {
        QString connector = m_corona->screenPool()->connector(id);
        qDebug() << "add dock - connector : " << connector;

        for (const auto view : m_latteViews) {
            auto testContainment = view->containment();

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

    /* old behavior to not add primary docks on explicit one
        else if (onPrimary) {
        if (explicitDockOccupyEdge(m_corona->screenPool()->primaryScreenId(), containment->location())) {
            qDebug() << "CORONA ::: adding dock rejected, the edge is occupied by explicit dock ! : " <<  containment->location();
            //we must check that an onPrimary dock should never catch up the same edge on
            //the same screen with an explicit dock
            return;
        }
    }*/

    qDebug() << "Adding dock for container...";
    qDebug() << "onPrimary: " << onPrimary << "screen!!! :" << nextScreen->name();

    //! it is used to set the correct flag during the creation
    //! of the window... This of course is also used during
    //! recreations of the window between different visibility modes
    auto mode = static_cast<Types::Visibility>(containment->config().readEntry("visibility", static_cast<int>(Types::DodgeActive)));
    bool byPassWM{false};

    if (mode == Types::AlwaysVisible || mode == Types::WindowsGoBelow) {
        byPassWM = false;
    } else {
        byPassWM = containment->config().readEntry("byPassWM", false);
    }

    auto latteView = new Latte::View(m_corona, nextScreen, byPassWM);

    latteView->init();
    latteView->setContainment(containment);
    latteView->setManagedLayout(this);

    //! force this special dock case to become primary
    //! even though it isnt
    if (forceOnPrimary) {
        qDebug() << "Enforcing onPrimary:true as requested for LatteView...";
        latteView->setOnPrimary(true);
    }

    //  connect(containment, &QObject::destroyed, this, &GenericLayout::containmentDestroyed);
    connect(containment, &Plasma::Applet::destroyedChanged, this, &GenericLayout::destroyedChanged);
    connect(containment, &Plasma::Applet::locationChanged, m_corona, &Latte::Corona::viewLocationChanged);
    connect(containment, &Plasma::Containment::appletAlternativesRequested
            , m_corona, &Latte::Corona::showAlternativesForApplet, Qt::QueuedConnection);

    if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        connect(containment, &Plasma::Containment::appletCreated, this, &GenericLayout::appletCreated);
    }

    //! Qt 5.9 creates a crash for this in wayland, that is why the check is used
    //! but on the other hand we need this for copy to work correctly and show
    //! the copied dock under X11
    //if (!KWindowSystem::isPlatformWayland()) {
    latteView->show();
    //}

    m_latteViews[containment] = latteView;

    emit viewsCountChanged();
}


bool GenericLayout::initToCorona(Latte::Corona *corona)
{
    if (m_corona) {
        return false;
    }

    m_corona = corona;

    for (const auto containment : m_corona->containments()) {
        if (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout) {
            addContainment(containment);
        } else if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
            QString layoutId = containment->config().readEntry("layoutId", QString());

            if (!layoutId.isEmpty() && (layoutId == m_layoutName)) {
                addContainment(containment);
            }
        }
    }

    qDebug() << "Layout ::::: " << name() << " added containments ::: " << m_containments.size();

    connect(m_corona, &Plasma::Corona::containmentAdded, this, &GenericLayout::addContainment);

    //!connect signals after adding the containment
    connect(this, &GenericLayout::viewsCountChanged, m_corona, &Plasma::Corona::availableScreenRectChanged);
    connect(this, &GenericLayout::viewsCountChanged, m_corona, &Plasma::Corona::availableScreenRegionChanged);

    emit viewsCountChanged();

    return true;
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

            connect(containment, &QObject::destroyed, this, &GenericLayout::containmentDestroyed);
            connect(containment, &Plasma::Applet::destroyedChanged, this, &GenericLayout::destroyedChanged);
            connect(containment, &Plasma::Containment::appletCreated, this, &GenericLayout::appletCreated);
        }

        latteView->setManagedLayout(this);

        emit viewsCountChanged();
    }

    //! sync the original layout file for integrity
    if (m_corona && m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        syncToLayoutFile(false);
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
    if (m_corona && m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        syncToLayoutFile(false);
    }

    return containments;
}

void GenericLayout::recreateView(Plasma::Containment *containment)
{
    if (!m_corona) {
        return;
    }

    //! give the time to config window to close itself first and then recreate the dock
    //! step:1 remove the latteview
    QTimer::singleShot(350, [this, containment]() {
        auto view = m_latteViews.take(containment);

        if (view) {
            qDebug() << "recreate - step 1: removing dock for containment:" << containment->id();

            //! step:2 add the new latteview
            connect(view, &QObject::destroyed, this, [this, containment]() {
                QTimer::singleShot(250, this, [this, containment]() {
                    if (!m_latteViews.contains(containment)) {
                        qDebug() << "recreate - step 2: adding dock for containment:" << containment->id();
                        addView(containment);
                    }
                });
            });

            view->deleteLater();
        }
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
        if (isLatteContainment(containment)) {
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
        if (isLatteContainment(containment)) {
            bool onPrimary = containment->config().readEntry("onPrimary", true);
            Plasma::Types::Location contLocation = containment->location();

            if (onPrimary && contLocation == location) {
                return true;
            }
        }
    }

    return false;
}


//! the central functions that updates loading/unloading latteviews
//! concerning screen changed (for multi-screen setups mainly)
void GenericLayout::syncLatteViewsToScreens()
{
    if (!m_corona) {
        return;
    }

    qDebug() << "start of, syncLatteViewsToScreens ....";
    qDebug() << "LAYOUT ::: " << name();
    qDebug() << "screen count changed -+-+ " << qGuiApp->screens().size();

    QHash<QString, QList<Plasma::Types::Location>> futureDocksLocations;
    QList<uint> futureShownViews;
    QString prmScreenName = qGuiApp->primaryScreen()->name();

    //! first step: primary docks must be placed in primary screen free edges
    for (const auto containment : m_containments) {
        if (isLatteContainment(containment)) {
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

            if (onPrimary && !futureDocksLocations[prmScreenName].contains(location)) {
                futureDocksLocations[prmScreenName].append(location);
                futureShownViews.append(containment->id());
            }
        }
    }

    //! second step: explicit docks must be placed in their screens if the screen edge is free
    for (const auto containment : m_containments) {
        if (isLatteContainment(containment)) {
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

                if (m_corona->screenPool()->screenExists(screenId) && !futureDocksLocations[expScreenName].contains(location)) {
                    futureDocksLocations[expScreenName].append(location);
                    futureShownViews.append(containment->id());
                }
            }
        }
    }

    qDebug() << "PRIMARY SCREEN :: " << prmScreenName;
    qDebug() << "LATTEVIEWS MUST BE PRESENT AT :: " << futureDocksLocations;
    qDebug() << "FUTURESHOWNVIEWS MUST BE :: " << futureShownViews;

    //! add views
    for (const auto containment : m_containments) {
        int screenId = containment->screen();

        if (screenId == -1) {
            screenId = containment->lastScreen();
        }

        if (!latteViewExists(containment) && futureShownViews.contains(containment->id())) {
            qDebug() << "syncLatteViewsToScreens: view must be added... for containment:" << containment->id() << " at screen:" << m_corona->screenPool()->connector(screenId);
            addView(containment);
        }
    }

    //! remove views
    for (const auto view : m_latteViews) {
        if (view->containment() && !futureShownViews.contains(view->containment()->id())) {
            qDebug() << "syncLatteViewsToScreens: view must be deleted... for containment:" << view->containment()->id() << " at screen:" << view->positioner()->currentScreenName();
            auto viewToDelete = m_latteViews.take(view->containment());
            viewToDelete->disconnectSensitiveSignals();
            viewToDelete->deleteLater();
        }
    }

    //! reconsider views
    for (const auto view : m_latteViews) {
        if (view->containment() && futureShownViews.contains(view->containment()->id())) {
            //! if the dock will not be deleted its a very good point to reconsider
            //! if the screen in which is running is the correct one
            view->reconsiderScreen();
        }
    }

    qDebug() << "end of, syncLatteViewsToScreens ....";
}



//////////////////////////////////////////////STORAGE/////////////////////////
/// //////////////////////to be moved in its own CLASS

bool GenericLayout::isWritable() const
{
    QFileInfo layoutFileInfo(m_layoutFile);

    if (layoutFileInfo.exists() && !layoutFileInfo.isWritable()) {
        return false;
    } else {
        return true;
    }
}

void GenericLayout::lock()
{
    QFileInfo layoutFileInfo(m_layoutFile);

    if (layoutFileInfo.exists() && layoutFileInfo.isWritable()) {
        QFile(m_layoutFile).setPermissions(QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }
}

void GenericLayout::unlock()
{
    QFileInfo layoutFileInfo(m_layoutFile);

    if (layoutFileInfo.exists() && !layoutFileInfo.isWritable()) {
        QFile(m_layoutFile).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }
}

void GenericLayout::syncToLayoutFile(bool removeLayoutId)
{
    if (!m_corona || !isWritable()) {
        return;
    }

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(m_layoutFile);

    KConfigGroup oldContainments = KConfigGroup(filePtr, "Containments");
    oldContainments.deleteGroup();
    oldContainments.sync();

    qDebug() << " LAYOUT :: " << m_layoutName << " is syncing its original file.";

    for (const auto containment : m_containments) {
        if (removeLayoutId) {
            containment->config().writeEntry("layoutId", "");
        }

        KConfigGroup newGroup = oldContainments.group(QString::number(containment->id()));
        containment->config().copyTo(&newGroup);

        if (!removeLayoutId) {
            newGroup.writeEntry("layoutId", "");
            newGroup.sync();
        }
    }

    oldContainments.sync();
}

void GenericLayout::copyView(Plasma::Containment *containment)
{
    if (!containment || !m_corona)
        return;

    qDebug() << "copying containment layout";
    //! Setting mutable for create a containment
    m_corona->setImmutability(Plasma::Types::Mutable);

    QString temp1File = QDir::homePath() + "/.config/lattedock.copy1.bak";

    //! WE NEED A WAY TO COPY A CONTAINMENT!!!!
    QFile copyFile(temp1File);

    if (copyFile.exists())
        copyFile.remove();


    KSharedConfigPtr newFile = KSharedConfig::openConfig(temp1File);
    KConfigGroup copied_conts = KConfigGroup(newFile, "Containments");
    KConfigGroup copied_c1 = KConfigGroup(&copied_conts, QString::number(containment->id()));
    KConfigGroup copied_systray;

    // toCopyContainmentIds << QString::number(containment->id());
    //  toCopyAppletIds << containment->config().group("Applets").groupList();
    containment->config().copyTo(&copied_c1);

    //!investigate if there is a systray in the containment to copy also
    int systrayId = -1;
    QString systrayAppletId;
    auto applets = containment->config().group("Applets");

    for (const auto &applet : applets.groupList()) {
        KConfigGroup appletSettings = applets.group(applet).group("Configuration");

        int tSysId = appletSettings.readEntry("SystrayContainmentId", -1);

        if (tSysId != -1) {
            systrayId = tSysId;
            systrayAppletId = applet;
            qDebug() << "systray was found in the containment... ::: " << tSysId;
            break;
        }
    }

    if (systrayId != -1) {
        Plasma::Containment *systray{nullptr};

        for (const auto containment : m_corona->containments()) {
            if (containment->id() == systrayId) {
                systray = containment;
                break;
            }
        }

        if (systray) {
            copied_systray = KConfigGroup(&copied_conts, QString::number(systray->id()));
            //  toCopyContainmentIds << QString::number(systray->id());
            //  toCopyAppletIds << systray->config().group("Applets").groupList();
            systray->config().copyTo(&copied_systray);
        }
    }

    //! end of systray specific code

    //! update ids to unique ones
    QString temp2File = newUniqueIdsLayoutFromFile(temp1File);


    //! Don't create LatteView when the containment is created because we must update
    //! its screen settings first
    setBlockAutomaticLatteViewCreation(true);
    //! Finally import the configuration
    QList<Plasma::Containment *> importedDocks = importLayoutFile(temp2File);

    Plasma::Containment *newContainment{nullptr};

    if (importedDocks.size() == 1) {
        newContainment = importedDocks[0];
    }

    if (!newContainment || !newContainment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    auto config = newContainment->config();

    //in multi-screen environment the copied dock is moved to alternative screens first
    const auto screens = qGuiApp->screens();
    auto dock = m_latteViews[containment];

    bool setOnExplicitScreen = false;

    int dockScrId = -1;
    int copyScrId = -1;

    if (dock) {
        dockScrId = dock->positioner()->currentScreenId();
        qDebug() << "COPY DOCK SCREEN ::: " << dockScrId;

        if (dockScrId != -1 && screens.count() > 1) {
            for (const auto scr : screens) {
                copyScrId = m_corona->screenPool()->id(scr->name());

                //the screen must exist and not be the same with the original dock
                if (copyScrId > -1 && copyScrId != dockScrId) {
                    QList<Plasma::Types::Location> fEdges = freeEdges(copyScrId);

                    if (fEdges.contains((Plasma::Types::Location)containment->location())) {
                        ///set this containment to an explicit screen
                        config.writeEntry("onPrimary", false);
                        config.writeEntry("lastScreen", copyScrId);
                        newContainment->setLocation(containment->location());

                        qDebug() << "COPY DOCK SCREEN NEW SCREEN ::: " << copyScrId;

                        setOnExplicitScreen = true;
                        break;
                    }
                }
            }
        }
    }

    if (!setOnExplicitScreen) {
        QList<Plasma::Types::Location> edges = freeEdges(newContainment->screen());

        if (edges.count() > 0) {
            newContainment->setLocation(edges.at(0));
        } else {
            newContainment->setLocation(Plasma::Types::BottomEdge);
        }

        config.writeEntry("onPrimary", false);
        config.writeEntry("lastScreen", dockScrId);
    }

    newContainment->config().sync();

    if (setOnExplicitScreen && copyScrId > -1) {
        qDebug() << "Copy Dock in explicit screen ::: " << copyScrId;
        addView(newContainment, false, copyScrId);
        newContainment->reactToScreenChange();
    } else {
        qDebug() << "Copy Dock in current screen...";
        addView(newContainment, false, dockScrId);
    }

    setBlockAutomaticLatteViewCreation(false);
}

void GenericLayout::importToCorona()
{
    if (!m_corona) {
        return;
    }

    //! Setting mutable for create a containment
    m_corona->setImmutability(Plasma::Types::Mutable);

    QString temp1FilePath = QDir::homePath() + "/.config/lattedock.copy1.bak";
    //! we need to copy first the layout file because the kde cache
    //! may not have yet been updated (KSharedConfigPtr)
    //! this way we make sure at the latest changes stored in the layout file
    //! will be also available when changing to Multiple Layouts
    QString tempLayoutFilePath = QDir::homePath() + "/.config/lattedock.layout.bak";

    //! WE NEED A WAY TO COPY A CONTAINMENT!!!!
    QFile tempLayoutFile(tempLayoutFilePath);
    QFile copyFile(temp1FilePath);
    QFile layoutOriginalFile(m_layoutFile);

    if (tempLayoutFile.exists()) {
        tempLayoutFile.remove();
    }

    if (copyFile.exists())
        copyFile.remove();

    layoutOriginalFile.copy(tempLayoutFilePath);

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(tempLayoutFilePath);
    KSharedConfigPtr newFile = KSharedConfig::openConfig(temp1FilePath);
    KConfigGroup copyGroup = KConfigGroup(newFile, "Containments");
    KConfigGroup current_containments = KConfigGroup(filePtr, "Containments");

    current_containments.copyTo(&copyGroup);

    copyGroup.sync();

    //! update ids to unique ones
    QString temp2File = newUniqueIdsLayoutFromFile(temp1FilePath);


    //! Finally import the configuration
    importLayoutFile(temp2File);
}

bool GenericLayout::appletGroupIsValid(KConfigGroup appletGroup)
{
    return !( appletGroup.keyList().count() == 0
              && appletGroup.groupList().count() == 1
              && appletGroup.groupList().at(0) == "Configuration"
              && appletGroup.group("Configuration").keyList().count() == 1
              && appletGroup.group("Configuration").hasKey("PreloadWeight") );
}

bool GenericLayout::layoutIsBroken() const
{
    if (m_layoutFile.isEmpty() || !QFile(m_layoutFile).exists()) {
        return false;
    }

    QStringList ids;
    QStringList conts;
    QStringList applets;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layoutFile);


    if (!m_corona) {
        KConfigGroup containmentsEntries = KConfigGroup(lFile, "Containments");
        ids << containmentsEntries.groupList();
        conts << ids;

        for (const auto &cId : containmentsEntries.groupList()) {
            auto appletsEntries = containmentsEntries.group(cId).group("Applets");

            QStringList validAppletIds;
            bool updated{false};

            for (const auto &appletId : appletsEntries.groupList()) {
                KConfigGroup appletGroup = appletsEntries.group(appletId);

                if (appletGroupIsValid(appletGroup)) {
                    validAppletIds << appletId;
                } else {
                    updated = true;
                    //! heal layout file by removing applet config records that are not used any more
                    qDebug() << "Layout: " << name() << " removing deprecated applet : " << appletId;
                    appletsEntries.deleteGroup(appletId);
                }
            }

            if (updated) {
                appletsEntries.sync();
            }

            ids << validAppletIds;
            applets << validAppletIds;
        }
    } else {
        for (const auto containment : m_containments) {
            ids << QString::number(containment->id());
            conts << QString::number(containment->id());

            for (const auto applet : containment->applets()) {
                ids << QString::number(applet->id());
                applets << QString::number(applet->id());
            }
        }
    }

    QSet<QString> idsSet = QSet<QString>::fromList(ids);

    /* a different way to count duplicates
    QMap<QString, int> countOfStrings;

    for (int i = 0; i < ids.count(); i++) {
        countOfStrings[ids[i]]++;
    }*/

    if (idsSet.count() != ids.count()) {
        qDebug() << "   ----   ERROR - BROKEN LAYOUT :: " << m_layoutName << " ----";

        if (!m_corona) {
            qDebug() << "   --- file : " << m_layoutFile;
        } else {
            if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
                qDebug() << "   --- in multiple layouts hidden file : " << Importer::layoutFilePath(AbstractLayout::MultipleLayoutsName);
            } else {
                qDebug() << "   --- in layout file : " << m_layoutFile;
            }
        }

        qDebug() << "Containments :: " << conts;
        qDebug() << "Applets :: " << applets;

        for (const QString &c : conts) {
            if (applets.contains(c)) {
                qDebug() << "Error: Same applet and containment id found ::: " << c;
            }
        }

        for (int i = 0; i < ids.count(); ++i) {
            for (int j = i + 1; j < ids.count(); ++j) {
                if (ids[i] == ids[j]) {
                    qDebug() << "Error: Applets with same id ::: " << ids[i];
                }
            }
        }

        qDebug() << "  -- - -- - -- - -- - - -- - - - - -- - - - - ";

        if (!m_corona) {
            KConfigGroup containmentsEntries = KConfigGroup(lFile, "Containments");

            for (const auto &cId : containmentsEntries.groupList()) {
                auto appletsEntries = containmentsEntries.group(cId).group("Applets");

                qDebug() << " CONTAINMENT : " << cId << " APPLETS : " << appletsEntries.groupList();
            }
        } else {
            for (const auto containment : m_containments) {
                QStringList appletsIds;

                for (const auto applet : containment->applets()) {
                    appletsIds << QString::number(applet->id());
                }

                qDebug() << " CONTAINMENT : " << containment->id() << " APPLETS : " << appletsIds.join(",");
            }
        }

        return true;
    }

    return false;
}

QString GenericLayout::availableId(QStringList all, QStringList assigned, int base)
{
    bool found = false;

    int i = base;

    while (!found && i < 32000) {
        QString iStr = QString::number(i);

        if (!all.contains(iStr) && !assigned.contains(iStr)) {
            return iStr;
        }

        i++;
    }

    return QString("");
}

QString GenericLayout::newUniqueIdsLayoutFromFile(QString file)
{
    if (!m_corona) {
        return QString();
    }

    QString tempFile = QDir::homePath() + "/.config/lattedock.copy2.bak";

    QFile copyFile(tempFile);

    if (copyFile.exists())
        copyFile.remove();

    //! BEGIN updating the ids in the temp file
    QStringList allIds;
    allIds << m_corona->containmentsIds();
    allIds << m_corona->appletsIds();

    QStringList toInvestigateContainmentIds;
    QStringList toInvestigateAppletIds;
    QStringList toInvestigateSystrayContIds;

    //! first is the systray containment id
    QHash<QString, QString> systrayParentContainmentIds;
    QHash<QString, QString> systrayAppletIds;

    //qDebug() << "Ids:" << allIds;

    //qDebug() << "to copy containments: " << toCopyContainmentIds;
    //qDebug() << "to copy applets: " << toCopyAppletIds;

    QStringList assignedIds;
    QHash<QString, QString> assigned;

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(file);
    KConfigGroup investigate_conts = KConfigGroup(filePtr, "Containments");
    //KConfigGroup copied_c1 = KConfigGroup(&copied_conts, QString::number(containment->id()));

    //! Record the containment and applet ids
    for (const auto &cId : investigate_conts.groupList()) {
        toInvestigateContainmentIds << cId;
        auto appletsEntries = investigate_conts.group(cId).group("Applets");
        toInvestigateAppletIds << appletsEntries.groupList();

        //! investigate for systrays
        for (const auto &appletId : appletsEntries.groupList()) {
            KConfigGroup appletSettings = appletsEntries.group(appletId).group("Configuration");

            int tSysId = appletSettings.readEntry("SystrayContainmentId", -1);

            //! It is a systray !!!
            if (tSysId != -1) {
                QString tSysIdStr = QString::number(tSysId);
                toInvestigateSystrayContIds << tSysIdStr;
                systrayParentContainmentIds[tSysIdStr] = cId;
                systrayAppletIds[tSysIdStr] = appletId;
                qDebug() << "systray was found in the containment...";
            }
        }
    }

    //! Reassign containment and applet ids to unique ones
    for (const auto &contId : toInvestigateContainmentIds) {
        QString newId = availableId(allIds, assignedIds, 12);

        assignedIds << newId;
        assigned[contId] = newId;
    }

    for (const auto &appId : toInvestigateAppletIds) {
        QString newId = availableId(allIds, assignedIds, 40);

        assignedIds << newId;
        assigned[appId] = newId;
    }

    qDebug() << "ALL CORONA IDS ::: " << allIds;
    qDebug() << "FULL ASSIGNMENTS ::: " << assigned;

    for (const auto &cId : toInvestigateContainmentIds) {
        QString value = assigned[cId];

        if (assigned.contains(value)) {
            QString value2 = assigned[value];

            if (cId != assigned[cId] && !value2.isEmpty() && cId == value2) {
                qDebug() << "PROBLEM APPEARED !!!! FOR :::: " << cId << " .. fixed ..";
                assigned[cId] = cId;
                assigned[value] = value;
            }
        }
    }

    for (const auto &aId : toInvestigateAppletIds) {
        QString value = assigned[aId];

        if (assigned.contains(value)) {
            QString value2 = assigned[value];

            if (aId != assigned[aId] && !value2.isEmpty() && aId == value2) {
                qDebug() << "PROBLEM APPEARED !!!! FOR :::: " << aId << " .. fixed ..";
                assigned[aId] = aId;
                assigned[value] = value;
            }
        }
    }

    qDebug() << "FIXED FULL ASSIGNMENTS ::: " << assigned;

    //! update applet ids in their containment order and in MultipleLayouts update also the layoutId
    for (const auto &cId : investigate_conts.groupList()) {
        //! Update options that contain applet ids
        //! (appletOrder) and (lockedZoomApplets) and (userBlocksColorizingApplets)
        QStringList options;
        options << "appletOrder" << "lockedZoomApplets" << "userBlocksColorizingApplets";

        for (const auto &settingStr : options) {
            QString order1 = investigate_conts.group(cId).group("General").readEntry(settingStr, QString());

            if (!order1.isEmpty()) {
                QStringList order1Ids = order1.split(";");
                QStringList fixedOrder1Ids;

                for (int i = 0; i < order1Ids.count(); ++i) {
                    fixedOrder1Ids.append(assigned[order1Ids[i]]);
                }

                QString fixedOrder1 = fixedOrder1Ids.join(";");
                investigate_conts.group(cId).group("General").writeEntry(settingStr, fixedOrder1);
            }
        }

        if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
            investigate_conts.group(cId).writeEntry("layoutId", m_layoutName);
        }
    }

    //! must update also the systray id in its applet
    for (const auto &systrayId : toInvestigateSystrayContIds) {
        KConfigGroup systrayParentContainment = investigate_conts.group(systrayParentContainmentIds[systrayId]);
        systrayParentContainment.group("Applets").group(systrayAppletIds[systrayId]).group("Configuration").writeEntry("SystrayContainmentId", assigned[systrayId]);
        systrayParentContainment.sync();
    }

    investigate_conts.sync();

    //! Copy To Temp 2 File And Update Correctly The Ids
    KSharedConfigPtr file2Ptr = KSharedConfig::openConfig(tempFile);
    KConfigGroup fixedNewContainmets = KConfigGroup(file2Ptr, "Containments");

    for (const auto &contId : investigate_conts.groupList()) {
        QString pluginId = investigate_conts.group(contId).readEntry("plugin", "");

        if (pluginId != "org.kde.desktopcontainment") { //!don't add ghost containments
            KConfigGroup newContainmentGroup = fixedNewContainmets.group(assigned[contId]);
            investigate_conts.group(contId).copyTo(&newContainmentGroup);

            newContainmentGroup.group("Applets").deleteGroup();

            for (const auto &appId : investigate_conts.group(contId).group("Applets").groupList()) {
                KConfigGroup appletGroup = investigate_conts.group(contId).group("Applets").group(appId);
                KConfigGroup newAppletGroup = fixedNewContainmets.group(assigned[contId]).group("Applets").group(assigned[appId]);
                appletGroup.copyTo(&newAppletGroup);
            }
        }
    }

    fixedNewContainmets.sync();

    return tempFile;
}

QList<Plasma::Containment *> GenericLayout::importLayoutFile(QString file)
{
    KSharedConfigPtr filePtr = KSharedConfig::openConfig(file);
    auto newContainments = m_corona->importLayout(KConfigGroup(filePtr, ""));

    ///Find latte and systray containments
    qDebug() << " imported containments ::: " << newContainments.length();

    QList<Plasma::Containment *> importedDocks;
    //QList<Plasma::Containment *> systrays;

    for (const auto containment : newContainments) {
        if (isLatteContainment(containment)) {
            qDebug() << "new latte containment id: " << containment->id();
            importedDocks << containment;
        }
    }

    return importedDocks;
}

//////////////////////////////////////// STORAGE ////////////////////////////////


}
}
