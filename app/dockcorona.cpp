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
#include <QFile>
#include <QFontDatabase>
#include <QQmlContext>

#include <Plasma>
#include <Plasma/Corona>
#include <Plasma/Containment>

#include <KActionCollection>
#include <KPluginMetaData>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KAboutData>
#include <KActivities/Consumer>

#include <KWindowSystem>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/plasmashell.h>

namespace Latte {

DockCorona::DockCorona(QObject *parent)
    : Plasma::Corona(parent),
      m_activityConsumer(new KActivities::Consumer(this)),
      m_screenPool(new ScreenPool(KSharedConfig::openConfig(), this)),
      m_globalShortcuts(new GlobalShortcuts(this)),
      m_universalSettings(new UniversalSettings(KSharedConfig::openConfig(), this)),
      m_layoutManager(new LayoutManager(this))
{
    setupWaylandIntegration();

    KPackage::Package package(new DockPackage(this));
    m_screenPool->load();

    if (!package.isValid()) {
        qWarning() << staticMetaObject.className()
                   << "the package" << package.metadata().rawData() << "is invalid!";
        return;
    } else {
        qDebug() << staticMetaObject.className()
                 << "the package" << package.metadata().rawData() << "is valid!";
    }

    setKPackage(package);
    //! universal settings must be loaded after the package has been set
    m_universalSettings->load();

    qmlRegisterTypes();
    QFontDatabase::addApplicationFont(kPackage().filePath("tangerineFont"));

    //connect(this, &Corona::containmentAdded, this, &DockCorona::addDock);

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

    //qDebug() << "corona config file:" << config()->name();

    while (!containments().isEmpty()) {
        //deleting a containment will remove it from the list due to QObject::destroyed connect in Corona
        delete containments().first();
    }

    m_globalShortcuts->deleteLater();
    m_screenPool->deleteLater();
    m_layoutManager->deleteLater();
    m_universalSettings->deleteLater();

    qDeleteAll(m_dockViews);
    qDeleteAll(m_waitingDockViews);
    m_dockViews.clear();
    m_waitingDockViews.clear();

    disconnect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);
    delete m_activityConsumer;

    qDebug() << "latte corona deleted..." << this;
}

void DockCorona::load()
{
    if (m_activityConsumer && (m_activityConsumer->serviceStatus() == KActivities::Consumer::Running) && m_activitiesStarting) {
        disconnect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);
        m_layoutManager->load();

        m_activitiesStarting = false;

        //  connect(qGuiApp, &QGuiApplication::screenAdded, this, &DockCorona::addOutput, Qt::UniqueConnection);
        connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &DockCorona::primaryOutputChanged, Qt::UniqueConnection);
        //  connect(qGuiApp, &QGuiApplication::screenRemoved, this, &DockCorona::screenRemoved, Qt::UniqueConnection);
        connect(QApplication::desktop(), &QDesktopWidget::screenCountChanged, this, &DockCorona::screenCountChanged);
        connect(m_screenPool, &ScreenPool::primaryPoolChanged, this, &DockCorona::screenCountChanged);

        QString assignedLayout = m_layoutManager->shouldSwitchToLayout(m_activityConsumer->currentActivity());

        if (!assignedLayout.isEmpty() && assignedLayout != m_universalSettings->currentLayoutName()) {
            m_layoutManager->switchToLayout(assignedLayout);
        } else {
            m_layoutManager->switchToLayout(m_universalSettings->currentLayoutName());
        }

        /*foreach (auto containment, containments())
            addDock(containment);*/
    }
}

void DockCorona::unload()
{
    qDebug() << "unload: removing dockViews and containments...";

    while (!containments().isEmpty()) {
        //deleting a containment will remove it from the list due to QObject::destroyed connect in Corona
        //this form doesn't crash, while qDeleteAll(containments()) does
        delete containments().first();
    }

    qDeleteAll(m_dockViews);
    qDeleteAll(m_waitingDockViews);
    m_dockViews.clear();
    m_waitingDockViews.clear();
}

void DockCorona::loadLatteLayout(QString layoutPath)
{
    if (!layoutPath.isEmpty()) {
        qDebug() << "corona is unloading the interface...";
        unload();
        qDebug() << "loading layout:" << layoutPath;
        loadLayout(layoutPath);

        m_firstContainmentWithTasks = -1;
        m_tasksWillBeLoaded =  heuresticForLoadingDockWithTasks();
        qDebug() << "TASKS WILL BE PRESENT AFTER LOADING ::: " << m_tasksWillBeLoaded;

        foreach (auto containment, containments())
            addDock(containment);
    }
}

void DockCorona::setupWaylandIntegration()
{
    using namespace KWayland::Client;

    if (!KWindowSystem::isPlatformWayland()) {
        return;
    }

    auto connection = ConnectionThread::fromApplication(this);

    if (!connection)
        return;

    Registry *registry{new Registry(this)};
    registry->create(connection);

    connect(registry, &Registry::plasmaShellAnnounced, this
    , [this, registry](quint32 name, quint32 version) {
        m_waylandDockCorona = registry->createPlasmaShell(name, version, this);
    });

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this, registry]() {
        if (m_waylandDockCorona)
            m_waylandDockCorona->release();

        registry->release();
    });

    registry->setup();
}

KWayland::Client::PlasmaShell *DockCorona::waylandDockCoronaInterface() const
{
    return m_waylandDockCorona;
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

UniversalSettings *DockCorona::universalSettings() const
{
    return m_universalSettings;
}

LayoutManager *DockCorona::layoutManager() const
{
    return m_layoutManager;
}

int DockCorona::numScreens() const
{
    return qGuiApp->screens().count();
}

QRect DockCorona::screenGeometry(int id) const
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

    return screen->geometry();
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
                    if (view->behaveAsPlasmaPanel()) {
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
                    if (view->behaveAsPlasmaPanel()) {
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
        if (view->tasksPresent()) {
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

            //! two main situations that a dock must be added when it is not already running
            //! 1. when a dock is primary, not running and the edge for which is associated is free
            //! 2. when a dock in explicit, not running and the associated screen currently exists
            //! e.g. the screen has just been added
            if (((onPrimary && freeEdges(qGuiApp->primaryScreen()).contains(location)) || (!onPrimary && (m_screenPool->connector(id) == scr->name())))
                && (!m_dockViews.contains(cont))) {
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
            if (scr->name() == view->currentScreen()
                || (view->onPrimary() && scr == qGuiApp->primaryScreen())) {
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
            if (scr->name() == view->currentScreen()
                || (view->onPrimary() && scr == qGuiApp->primaryScreen())) {
                found = true;
                break;
            }
        }


        //! which explicit docks can be deleted
        if (!found && !view->onPrimary() && (m_dockViews.size() > 1) && m_dockViews.contains(view->containment())
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
    QScreen *scr = m_screenPool->screenForId(screen);

    int docks{0};

    for (const auto &view : m_dockViews) {
        if (view && view->screen() == scr && !view->containment()->destroyed()) {
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
        if (view && view->containment() && !view->containment()->destroyed()) {
            ++docks;
        }
    }

    // qDebug() << docks << "docks on screen:" << screen;
    return docks;
}

int DockCorona::docksCount(QScreen *screen) const
{
    int docks{0};

    for (const auto &view : m_dockViews) {
        if (view && view->screen() == screen && !view->containment()->destroyed()) {
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

int DockCorona::noOfDocks()
{
    return m_dockViews.count();
}

QList<Plasma::Types::Location> DockCorona::freeEdges(QScreen *screen) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                                 Types::TopEdge, Types::RightEdge};

    for (auto *view : m_dockViews) {
        if (view && view->currentScreen() == screen->name()) {
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

    QScreen *scr = m_screenPool->screenForId(screen);

    for (auto *view : m_dockViews) {
        if (view && scr && view->currentScreen() == scr->name()) {
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

void DockCorona::addDock(Plasma::Containment *containment, int expDockScreen)
{
    if (!containment || !containment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    auto metadata = containment->kPackage().metadata();

    if (metadata.pluginId() != "org.kde.latte.containment")
        return;

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

    if (id == -1 && expDockScreen == -1) {
        id = containment->lastScreen();
    }

    if (expDockScreen > -1) {
        id = expDockScreen;
    }

    qDebug() << "add dock - containment id: " << containment->id() << " ,screen id : " << id << " ,onprimary:" << onPrimary << " ,forceDockLoad:" << forceDockLoading;

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
    } else if (onPrimary) {
        if (explicitDockOccupyEdge(primaryScreenId(), containment->location())) {
            //we must check that an onPrimary dock should never catch up the same edge on
            //the same screen with an explicit dock
            return;
        }
    }

    qDebug() << "Adding dock for container...";
    qDebug() << "onPrimary: " << onPrimary << "screen!!! :" << nextScreen->name();

    //! it is used to set the correct flag during the creation
    //! of the window... This of course is also used during
    //! recreations of the window between different visibility modes
    auto mode = static_cast<Dock::Visibility>(containment->config().readEntry("visibility", static_cast<int>(Dock::DodgeActive)));
    bool dockWin{true};

    if (mode == Dock::AlwaysVisible || mode == Dock::WindowsGoBelow) {
        dockWin = true;
    } else {
        dockWin = containment->config().readEntry("dockWindowBehavior", true);
    }

    auto dockView = new DockView(this, nextScreen, dockWin);
    dockView->init();
    dockView->setContainment(containment);

    //! force this special dock case to become primary
    //! even though it isnt
    if (forceDockLoading) {
        dockView->setOnPrimary(true);
    }

    connect(containment, &QObject::destroyed, this, &DockCorona::dockContainmentDestroyed);
    connect(containment, &Plasma::Applet::destroyedChanged, this, &DockCorona::destroyedChanged);
    connect(containment, &Plasma::Applet::locationChanged, this, &DockCorona::dockLocationChanged);
    connect(containment, &Plasma::Containment::appletAlternativesRequested
            , this, &DockCorona::showAlternativesForApplet, Qt::QueuedConnection);

    //! Qt 5.9 creates a crash for this in wayland, that is why the check is used
    //! but on the other hand we need this for copy to work correctly and show
    //! the copied dock under X11
    //if (!KWindowSystem::isPlatformWayland()) {
    dockView->show();
    //}

    m_dockViews[containment] = dockView;

    emit docksCountChanged();
}

bool DockCorona::explicitDockOccupyEdge(int screen, Plasma::Types::Location location) const
{
    foreach (auto containment, containments()) {
        bool onPrimary = containment->config().readEntry("onPrimary", true);
        int id = containment->lastScreen();
        Plasma::Types::Location contLocation = containment->location();

        if (!onPrimary && id == screen && contLocation == location) {
            return true;
        }

    }

    return false;
}

void DockCorona::recreateDock(Plasma::Containment *containment)
{
    //! give the time to config window to close itself first and then recreate the dock
    //! step:1 remove the dockview
    QTimer::singleShot(350, [this, containment]() {
        auto view = m_dockViews.take(containment);

        if (view) {
            qDebug() << "recreate - step 1: removing dock for containment:" << containment->id();

            //! step:2 add the new dockview
            connect(view, &QObject::destroyed, this, [this, containment]() {
                QTimer::singleShot(250, this, [this, containment]() {
                    if (!m_dockViews.contains(containment)) {
                        qDebug() << "recreate - step 2: adding dock for containment:" << containment->id();
                        addDock(containment);
                    }
                });
            });

            view->deleteLater();

        }
    });
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
    auto view = m_dockViews.take(static_cast<Plasma::Containment *>(cont));

    if (!view) {
        view = m_waitingDockViews.take(static_cast<Plasma::Containment *>(cont));
    }

    if (view) {
        view->deleteLater();
    }

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

    DockView *dockView = m_dockViews[applet->containment()];

    if (dockView) {
        dockView->setAlternativesIsShown(true);
    }

    connect(helper, &QObject::destroyed, this, [dockView]() {
        dockView->setAlternativesIsShown(false);
    });

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

    if ((edges.count() > 0)) {
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

    addDock(defaultContainment);
    defaultContainment->createApplet(QStringLiteral("org.kde.latte.plasmoid"));
    defaultContainment->createApplet(QStringLiteral("org.kde.plasma.analogclock"));
}

void DockCorona::copyDock(Plasma::Containment *containment)
{
    if (!containment)
        return;

    qDebug() << "copying containment layout";
    //! Settting mutable for create a containment
    setImmutability(Plasma::Types::Mutable);

    QStringList toCopyContainmentIds;
    QStringList toCopyAppletIds;

    QString temp1File = QDir::homePath() + "/.config/lattedock.copy1.bak";
    QString temp2File = QDir::homePath() + "/.config/lattedock.copy2.bak";

    //! WE NEED A WAY TO COPY A CONTAINMENT!!!!
    QFile copyFile(temp1File);
    QFile copyFile2(temp2File);

    if (copyFile.exists())
        copyFile.remove();

    if (copyFile2.exists())
        copyFile2.remove();

    KSharedConfigPtr newFile = KSharedConfig::openConfig(QDir::homePath() + "/.config/lattedock.copy1.bak");
    KConfigGroup copied_conts = KConfigGroup(newFile, "Containments");
    KConfigGroup copied_c1 = KConfigGroup(&copied_conts, QString::number(containment->id()));
    KConfigGroup copied_systray;

    toCopyContainmentIds << QString::number(containment->id());
    toCopyAppletIds << containment->config().group("Applets").groupList();
    containment->config().copyTo(&copied_c1);

    //!investigate if there is a systray in the containment to copy also
    int systrayId = -1;
    QString systrayAppletId;
    auto applets = containment->config().group("Applets");

    foreach (auto applet, applets.groupList()) {
        KConfigGroup appletSettings = applets.group(applet).group("Configuration");

        int tSysId = appletSettings.readEntry("SystrayContainmentId", "-1").toInt();

        if (tSysId != -1) {
            systrayId = tSysId;
            systrayAppletId = applet;
            qDebug() << "systray was found in the containment...";
            break;
        }
    }

    if (systrayId != -1) {
        Plasma::Containment *systray{nullptr};

        foreach (auto containment, containments()) {
            if (containment->id() == systrayId) {
                systray = containment;
                break;
            }
        }

        if (systray) {
            copied_systray = KConfigGroup(&copied_conts, QString::number(systray->id()));
            toCopyContainmentIds << QString::number(systray->id());
            toCopyAppletIds << systray->config().group("Applets").groupList();
            systray->config().copyTo(&copied_systray);
        }
    }

    //! end of systray specific code

    //! BEGIN updating the ids in the temp file
    QStringList allIds;
    allIds << containmentsIds();
    allIds << appletsIds();

    //qDebug() << "Ids:" << allIds;

    //qDebug() << "to copy containments: " << toCopyContainmentIds;
    //qDebug() << "to copy applets: " << toCopyAppletIds;

    QStringList assignedIds;
    QHash<QString, QString> assigned;

    foreach (auto contId, toCopyContainmentIds) {
        QString newId = availableId(allIds, assignedIds, 12);
        assignedIds << newId;
        assigned[contId] = newId;
    }

    foreach (auto appId, toCopyAppletIds) {
        QString newId = availableId(allIds, assignedIds, 40);
        assignedIds << newId;
        assigned[appId] = newId;
    }

    qDebug() << "full assignments ::: " << assigned;

    QString order1 = copied_c1.group("General").readEntry("appletOrder", QString());
    QStringList order1Ids = order1.split(";");
    QStringList fixedOrder1Ids;

    //qDebug() << "order1 :: " << order1;

    for (int i = 0; i < order1Ids.count(); ++i) {
        fixedOrder1Ids.append(assigned[order1Ids[i]]);
    }

    QString fixedOrder1 = fixedOrder1Ids.join(";");
    //qDebug() << "fixed order ::: " << fixedOrder1;
    copied_c1.group("General").writeEntry("appletOrder", fixedOrder1);

    //! must update also the systray id in its applet
    if (systrayId > -1) {
        copied_c1.group("Applets").group(systrayAppletId).group("Configuration").writeEntry("SystrayContainmentId", assigned[QString::number(systrayId)]);
        copied_systray.sync();
    }

    copied_c1.sync();

    QFile(temp1File).copy(temp2File);

    QFile f(temp2File);

    if (!f.open(QFile::ReadOnly)) {
        qDebug() << "temp file couldnt be opened...";
        return;
    }

    QTextStream in(&f);
    QString fileText = in.readAll();

    foreach (auto contId, toCopyContainmentIds) {
        fileText = fileText.replace("[Containments][" + contId + "]", "[Containments][" + assigned[contId] + "]");
    }

    foreach (auto appId, toCopyAppletIds) {
        fileText = fileText.replace("][Applets][" + appId + "]", "][Applets][" + assigned[appId] + "]");
    }

    f.close();

    if (!f.open(QFile::WriteOnly)) {
        qDebug() << "temp file couldnt be opened for writing...";
        return;
    }

    QTextStream outputStream(&f);
    outputStream << fileText;
    f.close();
    //! END of updating the ids in the temp file

    //! Finally import the configuration
    KSharedConfigPtr newFile2 = KSharedConfig::openConfig(QDir::homePath() + "/.config/lattedock.copy2.bak");
    auto nConts = importLayout(KConfigGroup(newFile2, ""));

    ///Find latte and systray containments
    qDebug() << " imported containments ::: " << nConts.length();

    Plasma::Containment *newContainment{nullptr};
    int newSystrayId = -1;

    foreach (auto containment, nConts) {
        KPluginMetaData meta = containment->kPackage().metadata();

        if (meta.pluginId() == "org.kde.latte.containment") {
            qDebug() << "new latte containment id: " << containment->id();
            newContainment = containment;
        } else if (meta.pluginId() == "org.kde.plasma.private.systemtray") {
            qDebug() << "new systray containment id: " << containment->id();
            newSystrayId = containment->id();
        }
    }

    if (!newContainment)
        return;

    ///after systray was found we must update in latte the relevant id
    if (newSystrayId != -1) {
        applets = newContainment->config().group("Applets");

        qDebug() << "systray found with id : " << newSystrayId << " and applets in the containment :" << applets.groupList().count();

        foreach (auto applet, applets.groupList()) {
            KConfigGroup appletSettings = applets.group(applet).group("Configuration");

            if (appletSettings.hasKey("SystrayContainmentId")) {
                qDebug() << "!!! updating systray id to : " << newSystrayId;
                appletSettings.writeEntry("SystrayContainmentId", newSystrayId);
            }
        }
    }

    if (!newContainment || !newContainment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    auto config = newContainment->config();

    //in multi-screen environment the copied dock is moved to alternative screens first
    const auto screens = qGuiApp->screens();
    auto dock = m_dockViews[containment];

    bool setOnExplicitScreen = false;

    int dockScrId = -1;
    int copyScrId = -1;

    if (dock) {
        dockScrId = m_screenPool->id(dock->currentScreen());
        qDebug() << "COPY DOCK SCREEN ::: " << dockScrId;

        if (dockScrId != -1 && screens.count() > 1) {
            foreach (auto scr, screens) {
                copyScrId = m_screenPool->id(scr->name());

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
        addDock(newContainment, copyScrId);
        newContainment->reactToScreenChange();
    } else {
        qDebug() << "Copy Dock in current screen...";
        addDock(newContainment, dockScrId);
    }
}

QString DockCorona::availableId(QStringList all, QStringList assigned, int base)
{
    bool found = false;

    int i = base;

    while (!found && i < 30000) {
        QString iStr = QString::number(i);

        if (!all.contains(iStr) && !assigned.contains(iStr)) {
            return iStr;
        }

        i++;
    }

    return QString("");
}

QStringList DockCorona::containmentsIds()
{
    auto containmentsEntries = config()->group("Containments");

    return containmentsEntries.groupList();
}

QStringList DockCorona::appletsIds()
{
    QStringList ids;
    auto containmentsEntries = config()->group("Containments");

    foreach (auto cId, containmentsEntries.groupList()) {
        auto appletsEntries = containmentsEntries.group(cId).group("Applets");

        ids << appletsEntries.groupList();
    }

    return ids;
}

//! This function figures in the beginning if a dock with tasks
//! in it will be loaded taking into account also the screens are present.
bool DockCorona::heuresticForLoadingDockWithTasks()
{
    foreach (auto containment, containments()) {
        QString plugin = containment->pluginMetaData().pluginId();

        if (plugin == "org.kde.latte.containment") {
            bool onPrimary = containment->config().readEntry("onPrimary", true);
            int lastScreen =  containment->lastScreen();

            qDebug() << "containment values: " << onPrimary << " - " << lastScreen;


            bool containsTasks = false;

            foreach (auto applet, containment->applets()) {
                const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

                if (provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
                    containsTasks = true;
                    break;
                }
            }

            if (containsTasks) {
                m_firstContainmentWithTasks = containment->id();

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

//! Activate launcher menu through dbus interface
void DockCorona::activateLauncherMenu()
{
    m_globalShortcuts->activateLauncherMenu();
}

//! update badge for specific dock item
void DockCorona::updateDockItemBadge(QString identifier, QString value)
{
    m_globalShortcuts->updateDockItemBadge(identifier, value);
}

inline void DockCorona::qmlRegisterTypes() const
{
    qmlRegisterType<QScreen>();
}

}
