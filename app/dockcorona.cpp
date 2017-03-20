/*
*  Copyright 2016  Smith AR <audoban@openmaibox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "dockcorona.h"
#include "dockview.h"
#include "packageplugins/shell/dockpackage.h"
#include "abstractwindowinterface.h"
#include "alternativeshelper.h"
#include "screenpool.h"
//dbus adaptor
#include "lattedockadaptor.h"

#include <QAction>
#include <QApplication>
#include <QScreen>
#include <QDBusConnection>
#include <QDebug>
#include <QDesktopWidget>
#include <QQmlContext>

#include <Plasma>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <KActionCollection>
#include <KPluginMetaData>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KAboutData>
#include <KActivities/Consumer>

namespace Latte {

DockCorona::DockCorona(QObject *parent)
    : Plasma::Corona(parent),
      m_screenPool(new ScreenPool(KSharedConfig::openConfig(), this)),
      m_globalSettings(new GlobalSettings(this)),
      m_activityConsumer(new KActivities::Consumer(this))
{
    KPackage::Package package(new DockPackage(this));
    m_screenPool->load();
    m_globalSettings->load();

    if (!package.isValid()) {
        qWarning() << staticMetaObject.className()
                   << "the package" << package.metadata().rawData() << "is invalid!";
        return;
    } else {
        qDebug() << staticMetaObject.className()
                 << "the package" << package.metadata().rawData() << "is valid!";
    }

    setKPackage(package);
    qmlRegisterTypes();
    connect(this, &Corona::containmentAdded, this, &DockCorona::addDock);

    if (m_activityConsumer && (m_activityConsumer->serviceStatus() == KActivities::Consumer::Running)) {
        load();
    }

    connect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);

    m_docksScreenSyncTimer.setSingleShot(true);
    m_docksScreenSyncTimer.setInterval(2500);
    connect(&m_docksScreenSyncTimer, &QTimer::timeout, this, &DockCorona::syncDockViews);

    //! Dbus adaptor initialization
    new LatteDockAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/Latte"), this);
}

DockCorona::~DockCorona()
{
    m_docksScreenSyncTimer.stop();
    cleanConfig();

    while (!containments().isEmpty()) {
        //deleting a containment will remove it from the list due to QObject::destroyed connect in Corona
        delete containments().first();
    }

    m_globalSettings->deleteLater();
    qDeleteAll(m_dockViews);
    qDeleteAll(m_waitingDockViews);
    m_dockViews.clear();
    m_waitingDockViews.clear();
    disconnect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);
    delete m_activityConsumer;
    qDebug() << "deleted" << this;
}

void DockCorona::load()
{
    if (m_activityConsumer && (m_activityConsumer->serviceStatus() == KActivities::Consumer::Running) && m_activitiesStarting) {
        disconnect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);

        m_activitiesStarting = false;
        m_tasksWillBeLoaded =  heuresticForLoadingDockWithTasks();
        qDebug() << "TASKS WILL BE PRESENT AFTER LOADING ::: " << m_tasksWillBeLoaded;

        //  connect(qGuiApp, &QGuiApplication::screenAdded, this, &DockCorona::addOutput, Qt::UniqueConnection);
        connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &DockCorona::primaryOutputChanged, Qt::UniqueConnection);
        //  connect(qGuiApp, &QGuiApplication::screenRemoved, this, &DockCorona::screenRemoved, Qt::UniqueConnection);
        connect(QApplication::desktop(), &QDesktopWidget::screenCountChanged, this, &DockCorona::screenCountChanged);
        connect(m_screenPool, &ScreenPool::primaryPoolChanged, this, &DockCorona::screenCountChanged);

        loadLayout();
    }
}

void DockCorona::cleanConfig()
{
    auto containmentsEntries = config()->group("Containments");
    bool changed = false;

    foreach (auto cId, containmentsEntries.groupList()) {
        if (!containmentExists(cId.toUInt())) {
            //cleanup obsolete containments
            containmentsEntries.group(cId).deleteGroup();
            changed = true;
            qDebug() << "obsolete containment configuration deleted:" << cId;
        } else {
            //cleanup obsolete applets of running containments
            auto appletsEntries = containmentsEntries.group(cId).group("Applets");

            foreach (auto appletId, appletsEntries.groupList()) {
                if (!appletExists(cId.toUInt(), appletId.toUInt())) {
                    appletsEntries.group(appletId).deleteGroup();
                    changed = true;
                    qDebug() << "obsolete applet configuration deleted:" << appletId;
                }
            }
        }
    }

    if (changed) {
        config()->sync();
        qDebug() << "configuration file cleaned...";
    }
}

bool DockCorona::containmentExists(uint id) const
{
    foreach (auto containment, containments()) {
        if (id == containment->id()) {
            return true;
        }
    }

    return false;
}

bool DockCorona::appletExists(uint containmentId, uint appletId) const
{
    Plasma::Containment *containment = nullptr;

    foreach (auto cont, containments()) {
        if (containmentId == cont->id()) {
            containment = cont;
            break;
        }
    }

    if (!containment) {
        return false;
    }

    foreach (auto applet, containment->applets()) {
        if (applet->id() == appletId) {
            return true;
        }
    }

    return false;
}

ScreenPool *DockCorona::screenPool() const
{
    return m_screenPool;
}

GlobalSettings *DockCorona::globalSettings() const
{
    return m_globalSettings;
}

int DockCorona::numScreens() const
{
    return qGuiApp->screens().count();
}

QRect DockCorona::screenGeometry(int id) const
{
    const auto screens = qGuiApp->screens();

    if (id >= 0 && id < screens.count()) {
        return screens[id]->geometry();
    }

    return qGuiApp->primaryScreen()->geometry();
}

QRegion DockCorona::availableScreenRegion(int id) const
{
    const auto screens = qGuiApp->screens();
    const QScreen *screen{qGuiApp->primaryScreen()};

    QString screenName;

    if (m_screenPool->knownIds().contains(id))
        screenName = m_screenPool->connector(id);

    foreach (auto scr, screens) {
        if (scr->name() == screenName) {
            screen = scr;
            break;
        }
    }

    if (!screen)
        return QRegion();

    QRegion available(screen->geometry());

    for (const auto *view : m_dockViews) {
        if (view && view->containment() && view->screen() == screen) {
            int realThickness = view->normalThickness() - view->shadow();

            // Usually availableScreenRect is used by the desktop,
            // but Latte dont have desktop, then here just
            // need calculate available space for top and bottom location,
            // because the left and right are those who dodge others docks
            switch (view->location()) {
                case Plasma::Types::TopEdge:
                    if (view->drawShadows()) {
                        available -= view->geometry();
                    } else {
                        QRect realGeometry;
                        int realWidth = view->maxLength() * view->width();

                        switch (view->alignment()) {
                            case Latte::Dock::Left:
                                realGeometry = QRect(view->x(), view->y(),
                                                     realWidth, realThickness);
                                break;

                            case Latte::Dock::Center:
                            case Latte::Dock::Justify:
                                realGeometry = QRect(qMax(view->geometry().x(), view->geometry().center().x() - realWidth / 2) , view->y(),
                                                     realWidth , realThickness);
                                break;

                            case Latte::Dock::Right:
                                realGeometry = QRect(view->geometry().right() - realWidth + 1, view->y(),
                                                     realWidth, realThickness);
                                break;
                        }

                        available -= realGeometry;
                    }

                    break;

                case Plasma::Types::BottomEdge:
                    if (view->drawShadows()) {
                        available -= view->geometry();
                    } else {
                        QRect realGeometry;
                        int realWidth = view->maxLength() * view->width();
                        int realY = view->geometry().bottom() - realThickness + 1;

                        switch (view->alignment()) {
                            case Latte::Dock::Left:
                                realGeometry = QRect(view->x(), realY,
                                                     realWidth, realThickness);
                                break;

                            case Latte::Dock::Center:
                            case Latte::Dock::Justify:
                                realGeometry = QRect(qMax(view->geometry().x(), view->geometry().center().x() - realWidth / 2),
                                                     realY, realWidth, realThickness);
                                break;

                            case Latte::Dock::Right:
                                realGeometry = QRect(view->geometry().right() - realWidth + 1, realY,
                                                     realWidth, realThickness);
                                break;
                        }

                        available -= realGeometry;
                    }

                    break;
            }
        }
    }

    /*qDebug() << "::::: FREE AREAS :::::";

    for (int i = 0; i < available.rectCount(); ++i) {
        qDebug() << available.rects().at(i);
    }

    qDebug() << "::::: END OF FREE AREAS :::::";*/

    return available;
}

QRect DockCorona::availableScreenRect(int id) const
{
    const auto screens = qGuiApp->screens();
    const QScreen *screen{qGuiApp->primaryScreen()};

    if (m_screenPool->knownIds().contains(id)) {
        QString scrName = m_screenPool->connector(id);

        foreach (auto scr, screens) {
            if (scr->name() == scrName) {
                screen = scr;
                break;
            }
        }
    }

    if (!screen)
        return {};

    auto available = screen->geometry();

    for (const auto *view : m_dockViews) {
        if (view && view->containment() && view->screen() == screen) {
            auto dockRect = view->absGeometry();

            // Usually availableScreenRect is used by the desktop,
            // but Latte dont have desktop, then here just
            // need calculate available space for top and bottom location,
            // because the left and right are those who dodge others docks
            switch (view->location()) {
                case Plasma::Types::TopEdge:
                    available.setTopLeft({available.x(), dockRect.bottom()});
                    break;

                case Plasma::Types::BottomEdge:
                    available.setBottomLeft({available.x(), dockRect.top()});
                    break;
            }
        }
    }

    return available;
}

//! the number of currently running docks containing
//! tasks plasmoid
int DockCorona::noDocksWithTasks() const
{
    int result = 0;

    foreach (auto view, m_dockViews) {
        if (view->tasksPresent() && view->session() == m_session) {
            result++;
        }
    }

    return result;
}

void DockCorona::addOutput(QScreen *screen)
{
    Q_ASSERT(screen);

    int id = m_screenPool->id(screen->name());

    if (id == -1) {
        int newId = m_screenPool->firstAvailableId();
        m_screenPool->insertScreenMapping(newId, screen->name());
    }
}

void DockCorona::primaryOutputChanged()
{
    /* qDebug() << "primary changed ### "<< qGuiApp->primaryScreen()->name();
     foreach(auto scr, qGuiApp->screens()){
         qDebug() << "Found screen: "<<scr->name();
     }*/

    //if (m_dockViews.count()==1 && qGuiApp->screens().size()==1) {
    //  foreach(auto view, m_dockViews) {
    //      view->setScreenToFollow(qGuiApp->primaryScreen());
    //  }
    // }
}

void DockCorona::screenRemoved(QScreen *screen)
{
    Q_ASSERT(screen);
}

void DockCorona::screenCountChanged()
{
    m_docksScreenSyncTimer.start();
}

//! the central functions that updates loading/unloading dockviews
//! concerning screen changed (for multi-screen setups mainly)
void DockCorona::syncDockViews()
{
    qDebug() << "screen count changed -+-+ " << qGuiApp->screens().size();

    qDebug() << "adding consideration....";
    qDebug() << "dock view running : " << m_dockViews.count();

    foreach (auto scr, qGuiApp->screens()) {
        qDebug() << "Found screen: " << scr->name();

        foreach (auto cont, containments()) {
            int id = cont->screen();

            if (id == -1) {
                id = cont->lastScreen();
            }

            bool onPrimary = cont->config().readEntry("onPrimary", true);
            Plasma::Types::Location location = static_cast<Plasma::Types::Location>((int)cont->config().readEntry("location", (int)Plasma::Types::BottomEdge));
            Dock::SessionType session = static_cast<Dock::SessionType>((int)cont->config().readEntry("session", (int)Dock::DefaultSession));

            //! two main situations that a dock must be added when it is not already running
            //! 1. when a dock is primary, not running and the edge for which is associated is free
            //! 2. when a dock in explicit, not running and the associated screen currently exists
            //! e.g. the screen has just been added
            if (((onPrimary && freeEdges(qGuiApp->primaryScreen()).contains(location)) || (!onPrimary && (m_screenPool->connector(id) == scr->name())))
                && (!m_dockViews.contains(cont)) && session == currentSession()) {
                qDebug() << "screen Count signal: view must be added... for:" << scr->name();
                addDock(cont);
            }
        }
    }

    qDebug() << "removing consideration & updating screen for always on primary docks....";

    //! this code trys to find a containment that must not be deleted by
    //! automatic algorithm. Currently the containment with the minimum id
    //! containing tasks plasmoid wins
    int preserveContainmentId{ -1};
    bool dockWithTasksWillBeShown{false};

    //! associate correct values for preserveContainmentId and
    //! dockWithTasksWillBeShown
    foreach (auto view, m_dockViews) {
        bool found{false};

        foreach (auto scr, qGuiApp->screens()) {
            int id = view->containment()->screen();

            if (id == -1) {
                id = view->containment()->lastScreen();
            }

            if (scr->name() == view->currentScreen()) {
                found = true;
                break;
            }
        }

        //!check if a tasks dock will be shown (try to prevent its deletion)
        if (found && view->tasksPresent()) {
            dockWithTasksWillBeShown = true;
        }

        if (!found && !view->onPrimary() && (m_dockViews.size() > 1) && m_dockViews.contains(view->containment())
            && !(view->tasksPresent() && noDocksWithTasks() == 1)) { //do not delete last dock containing tasks
            if (view->tasksPresent()) {
                if (preserveContainmentId == -1)
                    preserveContainmentId = view->containment()->id();
                else if (view->containment()->id() < preserveContainmentId)
                    preserveContainmentId = view->containment()->id();
            }
        }
    }

    //! check which docks must be deleted e.g. when the corresponding
    //! screen does not exist any more.
    //! The code is smart enough in order
    //! to never delete the last tasks dock and also it makes sure that
    //! the last tasks dock which will exist in the end will be the one
    //! with the lowest containment id
    foreach (auto view, m_dockViews) {
        bool found{false};

        foreach (auto scr, qGuiApp->screens()) {
            if (scr->name() == view->currentScreen()) {
                found = true;
                break;
            }
        }

        if (view->session() != currentSession()) {
            qDebug() << "deleting view that does not belong in this session...";
            auto viewToDelete = m_dockViews.take(view->containment());
            viewToDelete->deactivateApplets();
            viewToDelete->deleteLater();
            //! which explicit docks can be deleted
        } else if (!found && !view->onPrimary() && (m_dockViews.size() > 1) && m_dockViews.contains(view->containment())
                   && !(view->tasksPresent() && noDocksWithTasks() == 1)) {
            //do not delete last dock containing tasks
            if (dockWithTasksWillBeShown || preserveContainmentId != view->containment()->id()) {
                qDebug() << "screen Count signal: view must be deleted... for:" << view->currentScreen();
                auto viewToDelete = m_dockViews.take(view->containment());
                viewToDelete->deleteLater();
            }

            //!which primary docks can be deleted
        } else if (view->onPrimary() && !found
                   && !freeEdges(qGuiApp->primaryScreen()).contains(view->location())) {
            qDebug() << "screen Count signal: primary view must be deleted... for:" << view->currentScreen();
            auto viewToDelete = m_dockViews.take(view->containment());
            viewToDelete->deleteLater();
        } else {
            //! if the dock will not be deleted its a very good point to reconsider
            //! if the screen in which is running is the correct one
            view->reconsiderScreen();
        }
    }

    qDebug() << "end of screens count change....";
}

int DockCorona::primaryScreenId() const
{
    //this is not the proper way because kwin probably uses a different
    //index of screens...
    //This needs a lot of testing...
    return m_screenPool->id(qGuiApp->primaryScreen()->name());
}

int DockCorona::docksCount(int screen) const
{
    if (screen == -1)
        return 0;

    int docks{0};

    for (const auto &view : m_dockViews) {
        if (view && view->containment()
            && view->containment()->screen() == screen
            && !view->containment()->destroyed()) {
            ++docks;
        }
    }

    // qDebug() << docks << "docks on screen:" << screen;
    return docks;
}

int DockCorona::docksCount() const
{
    int docks{0};

    for (const auto &view : m_dockViews) {
        if (view && view->containment()
            && !view->containment()->destroyed()) {
            ++docks;
        }
    }

    // qDebug() << docks << "docks on screen:" << screen;
    return docks;
}

void DockCorona::closeApplication()
{
    qGuiApp->quit();
}

void DockCorona::aboutApplication()
{
    if (aboutDialog) {
        aboutDialog->hide();
        aboutDialog->deleteLater();
    }

    aboutDialog = new KAboutApplicationDialog(KAboutData::applicationData());
    connect(aboutDialog.data(), &QDialog::finished, aboutDialog.data(), &QObject::deleteLater);
    WindowSystem::self().skipTaskBar(*aboutDialog);

    aboutDialog->show();
}

Dock::SessionType DockCorona::currentSession()
{
    return m_session;
}

void DockCorona::setCurrentSession(Dock::SessionType session)
{
    if (m_session == session) {
        return;
    }

    m_session = session;

    emit currentSessionChanged(m_session);;
}

void DockCorona::switchToSession(Dock::SessionType session)
{
    if (currentSession() == session) {
        return;
    }

    setCurrentSession(session);

    if (noDocksForSession(session) == 0) {
        m_waitingSessionDocksCreation = true;
        loadDefaultLayout();
    } else {
        m_waitingSessionDocksCreation = false;
        syncDockViews();
    }
}
int DockCorona::noDocksForSession(Dock::SessionType session)
{
    int count{0};

    foreach (auto cont, containments()) {
        Dock::SessionType ses = static_cast<Dock::SessionType>(cont->config().readEntry("session", (int)Dock::DefaultSession));

        if (session == ses)
            count++;
    }

    return count;
}

QList<Plasma::Types::Location> DockCorona::freeEdges(QScreen *screen) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                                 Types::TopEdge, Types::RightEdge};

    for (auto *view : m_dockViews) {
        if (view && view->currentScreen() == screen->name() && view->session() == m_session) {
            edges.removeOne(view->location());
        }
    }

    return edges;
}

QList<Plasma::Types::Location> DockCorona::freeEdges(int screen) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                                 Types::TopEdge, Types::RightEdge};
    //when screen=-1 is passed then the primaryScreenid is used
    int fixedScreen = (screen == -1) ? primaryScreenId() : screen;

    for (auto *view : m_dockViews) {
        if (view && view->containment()
            && view->containment()->screen() == fixedScreen
            && view->session() == m_session) {
            edges.removeOne(view->location());
        }
    }

    return edges;
}

int DockCorona::screenForContainment(const Plasma::Containment *containment) const
{
    //FIXME: indexOf is not a proper way to support multi-screen
    // as for environment to environment the indexes change
    // also there is the following issue triggered
    // from dockView adaptToScreen()
    //
    // in a multi-screen environment that
    // primary screen is not set to 0 it was
    // created an endless showing loop at
    // startup (catch-up race) between
    // screen:0 and primaryScreen

    //case in which this containment is child of an applet, hello systray :)
    if (Plasma::Applet *parentApplet = qobject_cast<Plasma::Applet *>(containment->parent())) {
        if (Plasma::Containment *cont = parentApplet->containment()) {
            return screenForContainment(cont);
        } else {
            return -1;
        }
    }

    //if the panel views already exist, base upon them
    DockView *view = m_dockViews.value(containment);

    if (view && view->screen()) {
        return m_screenPool->id(view->screen()->name());
    }

    //Failed? fallback on lastScreen()
    //lastScreen() is the correct screen for panels
    //It is also correct for desktops *that have the correct activity()*
    //a containment with lastScreen() == 0 but another activity,
    //won't be associated to a screen
    //     qDebug() << "ShellCorona screenForContainment: " << containment << " Last screen is " << containment->lastScreen();

    for (auto screen : qGuiApp->screens()) {
        // containment->lastScreen() == m_screenPool->id(screen->name()) to check if the lastScreen refers to a screen that exists/it's known
        if (containment->lastScreen() == m_screenPool->id(screen->name()) &&
            (containment->activity() == m_activityConsumer->currentActivity() ||
             containment->containmentType() == Plasma::Types::PanelContainment || containment->containmentType() == Plasma::Types::CustomPanelContainment)) {
            return containment->lastScreen();
        }
    }

    return -1;
}

void DockCorona::addDock(Plasma::Containment *containment)
{
    if (!containment || !containment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    auto metadata = containment->kPackage().metadata();

    if (metadata.pluginId() != "org.kde.latte.containment")
        return;


    int session = containment->config().readEntry("session", (int)Dock::DefaultSession);

    //! when this containment does not belong to this session
    if (session != currentSession() && !m_waitingSessionDocksCreation) {
        return;
    }

    for (auto *dock : m_dockViews) {
        if (dock->containment() == containment)
            return;
    }

    QScreen *nextScreen{qGuiApp->primaryScreen()};

    //! forceDockLoading is used when a latte configuration based on the
    //! current running screens does not provide a dock containing tasks.
    //! in such case the lowest latte containment containing tasks is loaded
    //! and it forcefully becomes primary dock
    bool forceDockLoading = false;

    if (!m_tasksWillBeLoaded && m_firstContainmentWithTasks == static_cast<int>(containment->id())) {
        m_tasksWillBeLoaded = true; //this protects by loading more than one dock at startup
        forceDockLoading = true;
    }

    bool onPrimary = containment->config().readEntry("onPrimary", true);
    int id = containment->screen();

    if (id == -1) {
        id = containment->lastScreen();
    }

    qDebug() << "add dock - containment id : " << id;

    if (id >= 0 && !onPrimary && !forceDockLoading) {
        QString connector = m_screenPool->connector(id);
        qDebug() << "add dock - connector : " << connector;
        bool found{false};

        foreach (auto scr, qGuiApp->screens()) {
            if (scr && scr->name() == connector) {
                found = true;
                nextScreen = scr;
                break;
            }
        }

        if (!found) {
            qDebug() << "adding dock rejected, screen not available : " << connector;
            return;
        }
    }

    qDebug() << "Adding dock for container...";
    qDebug() << "onPrimary: " << onPrimary << "screen!!! :" << containment->screen();

    //! it is used to set the correct flag during the creation
    //! of the window... This of course is also used during
    //! recreations of the window between different visibility modes
    auto mode = static_cast<Dock::Visibility>(containment->config().readEntry("visibility", static_cast<int>(Dock::DodgeActive)));
    bool dockWin = containment->config().readEntry("dockWindowBehavior", false);
    bool alwaysVisible{false};

    if (mode == Latte::Dock::AlwaysVisible) {
        alwaysVisible = true;
    }

    auto dockView = new DockView(this, nextScreen, alwaysVisible, dockWin);
    dockView->init();
    dockView->setContainment(containment);

    //! force this special dock case to become primary
    //! even though it isnt
    if (forceDockLoading) {
        dockView->setOnPrimary(true);
    }

    dockView->setSession(currentSession());

    connect(containment, &QObject::destroyed, this, &DockCorona::dockContainmentDestroyed);
    connect(containment, &Plasma::Applet::destroyedChanged, this, &DockCorona::destroyedChanged);
    connect(containment, &Plasma::Applet::locationChanged, this, &DockCorona::dockLocationChanged);
    connect(containment, &Plasma::Containment::appletAlternativesRequested
            , this, &DockCorona::showAlternativesForApplet, Qt::QueuedConnection);

    dockView->show();
    m_dockViews[containment] = dockView;

    if (m_waitingSessionDocksCreation) {
        m_waitingSessionDocksCreation = false;

        if (noDocksForSession(currentSession()) == 1) {
            syncDockViews();
        }
    }

    emit docksCountChanged();
}

void DockCorona::recreateDock(Plasma::Containment *containment)
{
    auto view = m_dockViews.take(containment);

    if (view) {
        view->setVisible(false);
        view->deleteLater();
        addDock(view->containment());
    }
}

void DockCorona::destroyedChanged(bool destroyed)
{
    qDebug() << "dock containment destroyed changed!!!!";
    Plasma::Containment *sender = qobject_cast<Plasma::Containment *>(QObject::sender());

    if (!sender) {
        return;
    }

    if (destroyed) {
        m_waitingDockViews[sender] = m_dockViews.take(static_cast<Plasma::Containment *>(sender));
    } else {
        m_dockViews[sender] = m_waitingDockViews.take(static_cast<Plasma::Containment *>(sender));
    }

    emit docksCountChanged();
}

void DockCorona::dockContainmentDestroyed(QObject *cont)
{
    qDebug() << "dock containment destroyed!!!!";
    auto view = m_waitingDockViews.take(static_cast<Plasma::Containment *>(cont));

    if (view)
        delete view;

    emit docksCountChanged();
}

void DockCorona::showAlternativesForApplet(Plasma::Applet *applet)
{
    const QString alternativesQML = kPackage().filePath("appletalternativesui");

    if (alternativesQML.isEmpty()) {
        return;
    }

    KDeclarative::QmlObject *qmlObj = new KDeclarative::QmlObject(this);
    qmlObj->setInitializationDelayed(true);
    qmlObj->setSource(QUrl::fromLocalFile(alternativesQML));

    AlternativesHelper *helper = new AlternativesHelper(applet, qmlObj);
    qmlObj->rootContext()->setContextProperty(QStringLiteral("alternativesHelper"), helper);

    m_alternativesObjects << qmlObj;
    qmlObj->completeInitialization();
    connect(qmlObj->rootObject(), SIGNAL(visibleChanged(bool)),
            this, SLOT(alternativesVisibilityChanged(bool)));

    connect(applet, &Plasma::Applet::destroyedChanged, this, [this, qmlObj](bool destroyed) {
        if (!destroyed) {
            return;
        }

        QMutableListIterator<KDeclarative::QmlObject *> it(m_alternativesObjects);

        while (it.hasNext()) {
            KDeclarative::QmlObject *obj = it.next();

            if (obj == qmlObj) {
                it.remove();
                obj->deleteLater();
            }
        }
    });
}

void DockCorona::alternativesVisibilityChanged(bool visible)
{
    if (visible) {
        return;
    }

    QObject *root = sender();

    QMutableListIterator<KDeclarative::QmlObject *> it(m_alternativesObjects);

    while (it.hasNext()) {
        KDeclarative::QmlObject *obj = it.next();

        if (obj->rootObject() == root) {
            it.remove();
            obj->deleteLater();
        }
    }
}

void DockCorona::loadDefaultLayout()
{
    qDebug() << "loading default layout";
    //! Settting mutable for create a containment
    setImmutability(Plasma::Types::Mutable);
    QVariantList args;
    auto defaultContainment = createContainmentDelayed("org.kde.latte.containment", args);
    defaultContainment->setContainmentType(Plasma::Types::PanelContainment);
    defaultContainment->init();

    if (!defaultContainment || !defaultContainment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    auto config = defaultContainment->config();
    defaultContainment->restore(config);
    QList<Plasma::Types::Location> edges = freeEdges(defaultContainment->screen());

    if ((edges.count() > 0) && !m_waitingSessionDocksCreation) {
        defaultContainment->setLocation(edges.at(0));
    } else {
        defaultContainment->setLocation(Plasma::Types::BottomEdge);
    }

    defaultContainment->updateConstraints(Plasma::Types::StartupCompletedConstraint);

    defaultContainment->save(config);
    requestConfigSync();

    defaultContainment->flushPendingConstraintsEvents();
    emit containmentAdded(defaultContainment);
    emit containmentCreated(defaultContainment);

    m_waitingSessionDocksCreation = true;
    addDock(defaultContainment);
    defaultContainment->createApplet(QStringLiteral("org.kde.latte.plasmoid"));
    defaultContainment->createApplet(QStringLiteral("org.kde.plasma.analogclock"));
}

//! This function figures in the beginning if a dock with tasks
//! in it will be loaded taking into account also the screens are present.
bool DockCorona::heuresticForLoadingDockWithTasks()
{
    auto containmentsEntries = config()->group("Containments");

    foreach (auto cId, containmentsEntries.groupList()) {
        QString plugin = containmentsEntries.group(cId).readEntry("plugin");

        if (plugin == "org.kde.latte.containment") {
            bool onPrimary = containmentsEntries.group(cId).readEntry("onPrimary", true);
            int lastScreen = containmentsEntries.group(cId).readEntry("lastScreen", -1);
            Dock::SessionType session = static_cast<Dock::SessionType>(containmentsEntries.group(cId).readEntry("session", (int)Dock::DefaultSession));

            qDebug() << "containment values: " << onPrimary << " - " << lastScreen;

            auto appletEntries = containmentsEntries.group(cId).group("Applets");

            bool containsTasks = false;

            foreach (auto appId, appletEntries.groupList()) {
                if (appletEntries.group(appId).readEntry("plugin") == "org.kde.latte.plasmoid") {
                    containsTasks = true;
                    break;
                }
            }

            if (containsTasks && session == Dock::DefaultSession) {
                m_firstContainmentWithTasks = cId.toInt();

                if (onPrimary) {
                    return true;
                } else {
                    if (lastScreen >= 0) {
                        QString connector = m_screenPool->connector(lastScreen);

                        foreach (auto scr, qGuiApp->screens()) {
                            if (scr && scr->name() == connector) {
                                return true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

//! This function figures if a latte containment contains a
//! latte tasks plasmoid
bool DockCorona::containmentContainsTasks(Plasma::Containment *cont)
{
    auto containmentsEntries = config()->group("Containments");

    foreach (auto cId, containmentsEntries.groupList()) {
        QString plugin = containmentsEntries.group(cId).readEntry("plugin");

        if ((plugin == "org.kde.latte.containment") && (cId.toUInt() == cont->id())) {
            auto appletEntries = containmentsEntries.group(cId).group("Applets");

            foreach (auto appId, appletEntries.groupList()) {
                if (appletEntries.group(appId).readEntry("plugin") == "org.kde.latte.plasmoid") {
                    return true;
                    break;
                }
            }
        }
    }

    return false;
}

//! Activate launcher menu through dbus interface
void DockCorona::activateLauncherMenu()
{
    for (auto it = m_dockViews.constBegin(), end = m_dockViews.constEnd(); it != end; ++it) {
        const auto applets = it.key()->applets();

        for (auto applet : applets) {
            const auto provides = applet->kPackage().metadata().value(QStringLiteral("X-Plasma-Provides"));

            if (provides.contains(QLatin1String("org.kde.plasma.launchermenu"))) {
                emit applet->activated();
                return;
            }
        }
    }
}

inline void DockCorona::qmlRegisterTypes() const
{
    qmlRegisterType<QScreen>();
}

}
