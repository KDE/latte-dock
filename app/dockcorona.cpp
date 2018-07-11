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

#include "abstractwindowinterface.h"
#include "alternativeshelper.h"
#include "globalshortcuts.h"
#include "importer.h"
#include "launcherssignals.h"
#include "layoutmanager.h"
#include "screenpool.h"
#include "universalsettings.h"
#include "waylandinterface.h"
#include "xwindowinterface.h"
#include "dock/dockview.h"
#include "packageplugins/shell/dockpackage.h"

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

#include <PlasmaQuick/ConfigView>

#include <KActionCollection>
#include <KPluginMetaData>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KAboutData>
#include <KActivities/Consumer>
#include <KDeclarative/QmlObject>

#include <KWindowSystem>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/plasmawindowmanagement.h>

namespace Latte {

DockCorona::DockCorona(bool defaultLayoutOnStartup, QString layoutNameOnStartUp, int userSetMemoryUsage, QObject *parent)
    : Plasma::Corona(parent),
      m_defaultLayoutOnStartup(defaultLayoutOnStartup),
      m_userSetMemoryUsage(userSetMemoryUsage),
      m_layoutNameOnStartUp(layoutNameOnStartUp),
      m_activityConsumer(new KActivities::Consumer(this)),
      m_screenPool(new ScreenPool(KSharedConfig::openConfig(), this)),
      m_globalShortcuts(new GlobalShortcuts(this)),
      m_universalSettings(new UniversalSettings(KSharedConfig::openConfig(), this)),
      m_layoutManager(new LayoutManager(this))
{
    //! create the window manager

    if (KWindowSystem::isPlatformWayland()) {
        m_wm = new WaylandInterface(this);
    } else {
        m_wm = new XWindowInterface(this);
    }

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

    if (m_activityConsumer && (m_activityConsumer->serviceStatus() == KActivities::Consumer::Running)) {
        load();
    }

    connect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);

    m_docksScreenSyncTimer.setSingleShot(true);
    m_docksScreenSyncTimer.setInterval(m_universalSettings->screenTrackerInterval());
    connect(&m_docksScreenSyncTimer, &QTimer::timeout, this, &DockCorona::syncDockViewsToScreens);
    connect(m_universalSettings, &UniversalSettings::screenTrackerIntervalChanged, this, [this]() {
        m_docksScreenSyncTimer.setInterval(m_universalSettings->screenTrackerInterval());
    });

    //! Dbus adaptor initialization
    new LatteDockAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/Latte"), this);
}

DockCorona::~DockCorona()
{
    //! BEGIN: Give the time to slide-out docks when closing
    m_layoutManager->hideAllDocks();

    //! Dont delay the destruction under wayland in any case
    //! because it creates a crash with kwin effects
    //! https://bugs.kde.org/show_bug.cgi?id=392890
    if (!KWindowSystem::isPlatformWayland()) {
        QTimer::singleShot(400, [this]() {
            m_quitTimedEnded = true;
        });

        while (!m_quitTimedEnded) {
            QGuiApplication::processEvents(QEventLoop::AllEvents, 50);
        }
    }

    //! END: slide-out docks when closing

    m_docksScreenSyncTimer.stop();

    if (m_layoutManager->memoryUsage() == Dock::SingleLayout) {
        cleanConfig();
    }

    qDebug() << "Latte Corona - unload: containments ...";

    m_layoutManager->unload();

    m_wm->deleteLater();
    m_globalShortcuts->deleteLater();
    m_layoutManager->deleteLater();
    m_screenPool->deleteLater();
    m_universalSettings->deleteLater();

    disconnect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);
    delete m_activityConsumer;

    qDebug() << "Latte Corona - deleted...";
}

void DockCorona::load()
{
    if (m_activityConsumer && (m_activityConsumer->serviceStatus() == KActivities::Consumer::Running) && m_activitiesStarting) {
        disconnect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);
        m_layoutManager->load();

        m_activitiesStarting = false;

        connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &DockCorona::primaryOutputChanged, Qt::UniqueConnection);
        connect(QApplication::desktop(), &QDesktopWidget::screenCountChanged, this, &DockCorona::screenCountChanged);

        connect(m_screenPool, &ScreenPool::primaryPoolChanged, this, &DockCorona::screenCountChanged);

        QString assignedLayout = m_layoutManager->shouldSwitchToLayout(m_activityConsumer->currentActivity());

        QString loadLayoutName = "";

        if (!m_defaultLayoutOnStartup && m_layoutNameOnStartUp.isEmpty()) {
            if (!assignedLayout.isEmpty() && assignedLayout != m_universalSettings->currentLayoutName()) {
                loadLayoutName = assignedLayout;
            } else {
                loadLayoutName = m_universalSettings->currentLayoutName();
            }

            if (!m_layoutManager->layoutExists(loadLayoutName)) {
                loadLayoutName = m_layoutManager->defaultLayoutName();
                m_layoutManager->importDefaultLayout(false);
            }
        } else if (m_defaultLayoutOnStartup) {
            loadLayoutName = m_layoutManager->importer()->uniqueLayoutName(m_layoutManager->defaultLayoutName());
            m_layoutManager->importDefaultLayout(true);
        } else {
            loadLayoutName = m_layoutNameOnStartUp;
        }

        if (m_userSetMemoryUsage != -1 && !KWindowSystem::isPlatformWayland()) {
            Dock::LayoutsMemoryUsage usage = static_cast<Dock::LayoutsMemoryUsage>(m_userSetMemoryUsage);

            m_universalSettings->setLayoutsMemoryUsage(usage);
        }

        if (KWindowSystem::isPlatformWayland()) {
            m_universalSettings->setLayoutsMemoryUsage(Dock::SingleLayout);
        }

        m_layoutManager->loadLayoutOnStartup(loadLayoutName);
    }
}

void DockCorona::unload()
{
    qDebug() << "unload: removing containments...";

    while (!containments().isEmpty()) {
        //deleting a containment will remove it from the list due to QObject::destroyed connect in Corona
        //this form doesn't crash, while qDeleteAll(containments()) does
        delete containments().first();
    }
}

void DockCorona::setupWaylandIntegration()
{
    if (!KWindowSystem::isPlatformWayland()) {
        return;
    }

    using namespace KWayland::Client;

    auto connection = ConnectionThread::fromApplication(this);

    if (!connection) {
        return;
    }

    Registry *registry{new Registry(this)};
    registry->create(connection);

    connect(registry, &Registry::plasmaShellAnnounced, this
    , [this, registry](quint32 name, quint32 version) {
        m_waylandDockCorona = registry->createPlasmaShell(name, version, this);
    });

    QObject::connect(registry, &KWayland::Client::Registry::plasmaWindowManagementAnnounced,
    [this, registry](quint32 name, quint32 version) {
        KWayland::Client::PlasmaWindowManagement *pwm = registry->createPlasmaWindowManagement(name, version, this);

        WaylandInterface *wI = qobject_cast<WaylandInterface *>(m_wm);

        if (wI) {
            wI->initWindowManagement(pwm);
        }
    });

    registry->setup();
    connection->roundtrip();
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

KActivities::Consumer *DockCorona::activitiesConsumer() const
{
    return m_activityConsumer;
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

AbstractWindowInterface *DockCorona::wm() const
{
    return m_wm;
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
    return availableScreenRegionWithCriteria(id);
}

QRegion DockCorona::availableScreenRegionWithCriteria(int id, QString forLayout) const
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

    QHash<const Plasma::Containment *, DockView *> *views;

    if (forLayout.isEmpty()) {
        views = m_layoutManager->currentDockViews();
    } else {
        views = m_layoutManager->layoutDockViews(forLayout);
    }

    QRegion available(screen->geometry());

    if (views) {
        for (const auto *view : *views) {
            if (view && view->containment() && view->screen() == screen
                && view->visibility() && (view->visibility()->mode() != Latte::Dock::AutoHide)) {
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
                                    realGeometry = QRect(qMax(view->geometry().x(), view->geometry().center().x() - realWidth / 2), view->y(),
                                                         realWidth, realThickness);
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

                    default:
                        //! bypass clang warnings
                        break;
                }
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
    return availableScreenRectWithCriteria(id);
}

QRect DockCorona::availableScreenRectWithCriteria(int id, QList<Dock::Visibility> modes, QList<Plasma::Types::Location> edges) const
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

    bool allModes = modes.isEmpty();

    bool allEdges = edges.isEmpty();

    auto available = screen->geometry();

    QHash<const Plasma::Containment *, DockView *> *views = m_layoutManager->currentDockViews();

    if (views) {
        for (const auto *view : *views) {
            if (view && view->containment() && view->screen() == screen
                && ((allEdges || edges.contains(view->location()))
                    && (allModes || (view->visibility() && modes.contains(view->visibility()->mode()))))) {

                auto dockRect = view->absGeometry();

                // Usually availableScreenRect is used by the desktop,
                // but Latte dont have desktop, then here just
                // need calculate available space for top and bottom location,
                // because the left and right are those who dodge others docks
                switch (view->location()) {
                    case Plasma::Types::TopEdge:
                        available.setTop(dockRect.bottom() + 1);
                        break;

                    case Plasma::Types::BottomEdge:
                        available.setBottom(dockRect.top() - 1);
                        break;

                    case Plasma::Types::LeftEdge:
                        available.setLeft(dockRect.right() + 1);

                        break;

                    case Plasma::Types::RightEdge:
                        available.setRight(dockRect.left() - 1);

                        break;

                    default:
                        //! bypass clang warnings
                        break;
                }
            }
        }
    }

    return available;
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
    m_docksScreenSyncTimer.start();
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
void DockCorona::syncDockViewsToScreens()
{
    m_layoutManager->syncDockViewsToScreens();
}

int DockCorona::primaryScreenId() const
{
    return m_screenPool->id(qGuiApp->primaryScreen()->name());
}

void DockCorona::closeApplication()
{
    //! this code must be called asynchronously because it is called
    //! also from qml (Settings window).
    QTimer::singleShot(5, [this]() {
        m_layoutManager->hideLatteSettingsDialog();
        m_layoutManager->hideAllDocks();
    });

    //! give the time for the docks to hide themselves
    QTimer::singleShot(500, [this]() {
        qGuiApp->quit();
    });
}

void DockCorona::aboutApplication()
{
    if (aboutDialog) {
        aboutDialog->hide();
        aboutDialog->deleteLater();
    }

    aboutDialog = new KAboutApplicationDialog(KAboutData::applicationData());
    connect(aboutDialog.data(), &QDialog::finished, aboutDialog.data(), &QObject::deleteLater);
    m_wm->skipTaskBar(*aboutDialog);
    m_wm->setKeepAbove(*aboutDialog, true);

    aboutDialog->show();
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

    QHash<const Plasma::Containment *, DockView *> *views = m_layoutManager->currentDockViews();

    //if the panel views already exist, base upon them

    DockView *view = views ? views->value(containment) : nullptr;

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

void DockCorona::showAlternativesForApplet(Plasma::Applet *applet)
{
    const QString alternativesQML = kPackage().filePath("appletalternativesui");

    if (alternativesQML.isEmpty()) {
        return;
    }

    QHash<const Plasma::Containment *, DockView *> *views = m_layoutManager->currentDockViews();

    DockView *dockView = (*views)[applet->containment()];

    KDeclarative::QmlObject *qmlObj{nullptr};

    if (dockView) {
        dockView->setAlternativesIsShown(true);
        qmlObj = new KDeclarative::QmlObject(dockView);
    } else {
        qmlObj = new KDeclarative::QmlObject(this);
    }

    qmlObj->setInitializationDelayed(true);
    qmlObj->setSource(QUrl::fromLocalFile(alternativesQML));

    AlternativesHelper *helper = new AlternativesHelper(applet, qmlObj);
    qmlObj->rootContext()->setContextProperty(QStringLiteral("alternativesHelper"), helper);

    m_alternativesObjects << qmlObj;
    qmlObj->completeInitialization();

    //! Alternative dialog signals
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

    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                                 Types::TopEdge, Types::RightEdge};

    Layout *currentLayout = m_layoutManager->activeLayout(m_layoutManager->currentLayoutName());

    if (currentLayout) {
        edges = currentLayout->freeEdges(defaultContainment->screen());
    }

    if ((edges.count() > 0)) {
        defaultContainment->setLocation(edges.at(0));
    } else {
        defaultContainment->setLocation(Plasma::Types::BottomEdge);
    }

    if (m_layoutManager->memoryUsage() == Dock::MultipleLayouts) {
        config.writeEntry("layoutId", m_layoutManager->currentLayoutName());
    }

    defaultContainment->updateConstraints(Plasma::Types::StartupCompletedConstraint);

    defaultContainment->save(config);
    requestConfigSync();

    defaultContainment->flushPendingConstraintsEvents();
    emit containmentAdded(defaultContainment);
    emit containmentCreated(defaultContainment);

    //m_layoutManager->addDock(defaultContainment);
    defaultContainment->createApplet(QStringLiteral("org.kde.latte.plasmoid"));
    defaultContainment->createApplet(QStringLiteral("org.kde.plasma.analogclock"));
}

QStringList DockCorona::containmentsIds()
{
    QStringList ids;

    foreach (auto containment, containments()) {
        ids << QString::number(containment->id());
    }

    return ids;
}

QStringList DockCorona::appletsIds()
{
    QStringList ids;

    foreach (auto containment, containments()) {
        auto applets = containment->config().group("Applets");
        ids << applets.groupList();
    }

    return ids;
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


void DockCorona::switchToLayout(QString layout)
{
    m_layoutManager->switchToLayout(layout);
}

void DockCorona::showSettingsWindow(int page)
{
    Dock::LatteConfigPage p = Dock::LayoutPage;

    if (page >= Dock::LayoutPage && page <= Dock::PreferencesPage) {
        p = static_cast<Dock::LatteConfigPage>(page);
    }

    m_layoutManager->showLatteSettingsDialog(p);
}

QStringList DockCorona::contextMenuData()
{
    QStringList data;

    data << QString::number((int)m_layoutManager->memoryUsage());
    data << m_layoutManager->currentLayoutName();

    foreach (auto layoutName, m_layoutManager->menuLayouts()) {
        if (m_layoutManager->activeLayout(layoutName)) {
            data << QString("1," + layoutName);
        } else {
            data << QString("0," + layoutName);
        }
    }

    return data;
}

inline void DockCorona::qmlRegisterTypes() const
{
    qmlRegisterType<QScreen>();
}

}
