/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmaibox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lattecorona.h"

// local
#include <coretypes.h>
#include "alternativeshelper.h"
#include "apptypes.h"
#include "lattedockadaptor.h"
#include "screenpool.h"
#include "data/generictable.h"
#include "data/layouticondata.h"
#include "declarativeimports/interfaces.h"
#include "declarativeimports/contextmenulayerquickitem.h"
#include "indicator/factory.h"
#include "layout/abstractlayout.h"
#include "layout/centrallayout.h"
#include "layout/genericlayout.h"
#include "layouts/importer.h"
#include "layouts/manager.h"
#include "layouts/synchronizer.h"
#include "shortcuts/globalshortcuts.h"
#include "package/lattepackage.h"
#include "plasma/extended/backgroundcache.h"
#include "plasma/extended/backgroundtracker.h"
#include "plasma/extended/screengeometries.h"
#include "plasma/extended/screenpool.h"
#include "plasma/extended/theme.h"
#include "settings/universalsettings.h"
#include "templates/templatesmanager.h"
#include "view/originalview.h"
#include "view/view.h"
#include "view/settings/viewsettingsfactory.h"
#include "view/windowstracker/windowstracker.h"
#include "view/windowstracker/allscreenstracker.h"
#include "view/windowstracker/currentscreentracker.h"
#include "wm/abstractwindowinterface.h"
#include "wm/schemecolors.h"
#include "wm/waylandinterface.h"
#include "wm/xwindowinterface.h"
#include "wm/tracker/lastactivewindow.h"
#include "wm/tracker/schemes.h"
#include "wm/tracker/windowstracker.h"

// Qt
#include <QAction>
#include <QApplication>
#include <QScreen>
#include <QDBusConnection>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QFontDatabase>
#include <QQmlContext>
#include <QProcess>

// Plasma
#include <Plasma>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <PlasmaQuick/ConfigView>

// KDE
#include <KActionCollection>
#include <KPluginMetaData>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KAboutData>
#include <KActivities/Consumer>
#include <KDeclarative/QmlObjectSharedEngine>
#include <KWindowSystem>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/plasmawindowmanagement.h>

namespace Latte {

Corona::Corona(bool defaultLayoutOnStartup, QString layoutNameOnStartUp, QString addViewTemplateName, int userSetMemoryUsage, QObject *parent)
    : Plasma::Corona(parent),
      m_defaultLayoutOnStartup(defaultLayoutOnStartup),
      m_startupAddViewTemplateName(addViewTemplateName),
      m_userSetMemoryUsage(userSetMemoryUsage),
      m_layoutNameOnStartUp(layoutNameOnStartUp),
      m_activitiesConsumer(new KActivities::Consumer(this)),
      m_screenPool(new ScreenPool(KSharedConfig::openConfig(), this)),
      m_indicatorFactory(new Indicator::Factory(this)),
      m_universalSettings(new UniversalSettings(KSharedConfig::openConfig(), this)),
      m_globalShortcuts(new GlobalShortcuts(this)),
      m_plasmaScreenPool(new PlasmaExtended::ScreenPool(this)),
      m_themeExtended(new PlasmaExtended::Theme(KSharedConfig::openConfig(), this)),
      m_viewSettingsFactory(new ViewSettingsFactory(this)),
      m_templatesManager(new Templates::Manager(this)),
      m_layoutsManager(new Layouts::Manager(this)),
      m_plasmaGeometries(new PlasmaExtended::ScreenGeometries(this)),
      m_dialogShadows(new PanelShadows(this, QStringLiteral("dialogs/background")))
{
    connect(qApp, &QApplication::aboutToQuit, this, &Corona::onAboutToQuit);

    //! create the window manager
    if (KWindowSystem::isPlatformWayland()) {
        m_wm = new WindowSystem::WaylandInterface(this);
    } else {
        m_wm = new WindowSystem::XWindowInterface(this);
    }

    setupWaylandIntegration();

    KPackage::Package package(new Latte::Package(this));

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
    //! universal settings / extendedtheme must be loaded after the package has been set
    m_universalSettings->load();
    m_themeExtended->load();

    qmlRegisterTypes();

    if (m_activitiesConsumer && (m_activitiesConsumer->serviceStatus() == KActivities::Consumer::Running)) {
        load();
    }

    connect(m_activitiesConsumer, &KActivities::Consumer::serviceStatusChanged, this, &Corona::load);

    m_viewsScreenSyncTimer.setSingleShot(true);
    m_viewsScreenSyncTimer.setInterval(m_universalSettings->screenTrackerInterval());
    connect(&m_viewsScreenSyncTimer, &QTimer::timeout, this, &Corona::syncLatteViewsToScreens);
    connect(m_universalSettings, &UniversalSettings::screenTrackerIntervalChanged, this, [this]() {
        m_viewsScreenSyncTimer.setInterval(m_universalSettings->screenTrackerInterval());
    });

    //! Dbus adaptor initialization
    new LatteDockAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/Latte"), this);
}

Corona::~Corona()
{
    /*m_inQuit = true;

    //! BEGIN: Give the time to slide-out views when closing
    m_layoutsManager->synchronizer()->hideAllViews();
    m_viewSettingsFactory->deleteLater();

    m_viewsScreenSyncTimer.stop();

    if (m_layoutsManager->memoryUsage() == MemoryUsage::SingleLayout) {
        cleanConfig();
    }

    qDebug() << "Latte Corona - unload: containments ...";
    m_layoutsManager->unload();*/

    m_plasmaGeometries->deleteLater();
    m_wm->deleteLater();
    m_dialogShadows->deleteLater();
    m_globalShortcuts->deleteLater();
    m_layoutsManager->deleteLater();
    m_screenPool->deleteLater();
    m_universalSettings->deleteLater();
    m_plasmaScreenPool->deleteLater();
    m_themeExtended->deleteLater();
    m_indicatorFactory->deleteLater();

    disconnect(m_activitiesConsumer, &KActivities::Consumer::serviceStatusChanged, this, &Corona::load);
    delete m_activitiesConsumer;

    qDebug() << "Latte Corona - deleted...";

    if (!m_importFullConfigurationFile.isEmpty()) {
        //!NOTE: Restart latte to import the new configuration
        QString importCommand = "latte-dock --import-full \"" + m_importFullConfigurationFile + "\"";
        qDebug() << "Executing Import Full Configuration command : " << importCommand;

        QProcess::startDetached(importCommand);
    }
}

void Corona::onAboutToQuit()
{
    m_inQuit = true;

    //! BEGIN: Give the time to slide-out views when closing
    m_layoutsManager->synchronizer()->hideAllViews();
    m_viewSettingsFactory->deleteLater();

    m_viewsScreenSyncTimer.stop();

    if (m_layoutsManager->memoryUsage() == MemoryUsage::SingleLayout) {
        cleanConfig();
    }

    if (m_layoutsManager->memoryUsage() == Latte::MemoryUsage::MultipleLayouts) {
        m_layoutsManager->importer()->setMultipleLayoutsStatus(Latte::MultipleLayouts::Paused);
    }

    qDebug() << "Latte Corona - unload: containments ...";
    m_layoutsManager->unload();
}

void Corona::load()
{
    if (m_activitiesConsumer && (m_activitiesConsumer->serviceStatus() == KActivities::Consumer::Running) && m_activitiesStarting) {
        m_activitiesStarting = false;

        disconnect(m_activitiesConsumer, &KActivities::Consumer::serviceStatusChanged, this, &Corona::load);

        m_templatesManager->init();
        m_layoutsManager->init();

        connect(this, &Corona::availableScreenRectChangedFrom, this, &Plasma::Corona::availableScreenRectChanged, Qt::UniqueConnection);
        connect(this, &Corona::availableScreenRegionChangedFrom, this, &Plasma::Corona::availableScreenRegionChanged, Qt::UniqueConnection);
        connect(m_screenPool, &ScreenPool::primaryScreenChanged, this, &Corona::onScreenCountChanged, Qt::UniqueConnection);

        QString loadLayoutName = "";

        if (m_userSetMemoryUsage != -1) {
            MemoryUsage::LayoutsMemory usage = static_cast<MemoryUsage::LayoutsMemory>(m_userSetMemoryUsage);
            m_universalSettings->setLayoutsMemoryUsage(usage);
        }

        if (!m_defaultLayoutOnStartup && m_layoutNameOnStartUp.isEmpty()) {
            if (m_universalSettings->layoutsMemoryUsage() == MemoryUsage::MultipleLayouts) {
                loadLayoutName = "";
            } else {
                loadLayoutName = m_universalSettings->singleModeLayoutName();

                if (!m_layoutsManager->synchronizer()->layoutExists(loadLayoutName)) {
                    //! If chosen layout does not exist, force Default layout loading
                    QString defaultLayoutTemplateName = i18n(Templates::DEFAULTLAYOUTTEMPLATENAME);
                    loadLayoutName = defaultLayoutTemplateName;

                    if (!m_layoutsManager->synchronizer()->layoutExists(defaultLayoutTemplateName)) {
                        //! If Default layout does not exist at all, create it
                        QString path = m_templatesManager->newLayout("", defaultLayoutTemplateName);
                        m_layoutsManager->setOnAllActivities(Layout::AbstractLayout::layoutName(path));
                    }
                }
            }
        } else if (m_defaultLayoutOnStartup) {
            //! force loading a NEW default layout even though a default layout may already exists
            QString newDefaultLayoutPath = m_templatesManager->newLayout("", i18n(Templates::DEFAULTLAYOUTTEMPLATENAME));
            loadLayoutName = Layout::AbstractLayout::layoutName(newDefaultLayoutPath);
            m_universalSettings->setLayoutsMemoryUsage(MemoryUsage::SingleLayout);
        } else {
            loadLayoutName = m_layoutNameOnStartUp;
            m_universalSettings->setLayoutsMemoryUsage(MemoryUsage::SingleLayout);
        }

        m_layoutsManager->loadLayoutOnStartup(loadLayoutName);

        //! load screens signals such screenGeometryChanged in order to support
        //! plasmoid.screenGeometry properly
        for (QScreen *screen : qGuiApp->screens()) {
            onScreenAdded(screen);
        }

        connect(m_layoutsManager->synchronizer(), &Layouts::Synchronizer::initializationFinished, [this]() {
            if (!m_startupAddViewTemplateName.isEmpty()) {
                //! user requested through cmd startup to add view from specific view template and we can add it after the startup
                //! sequence has loaded all required layouts properly
                addView(0, m_startupAddViewTemplateName);
                m_startupAddViewTemplateName = "";
            }
        });

        m_inStartup = false;

        connect(qGuiApp, &QGuiApplication::screenAdded, this, &Corona::onScreenAdded, Qt::UniqueConnection);
        connect(qGuiApp, &QGuiApplication::screenRemoved, this, &Corona::onScreenRemoved, Qt::UniqueConnection);
    }
}

void Corona::unload()
{
    qDebug() << "unload: removing containments...";

    while (!containments().isEmpty()) {
        //deleting a containment will remove it from the list due to QObject::destroyed connect in Corona
        //this form doesn't crash, while qDeleteAll(containments()) does
        delete containments().first();
    }
}

void Corona::setupWaylandIntegration()
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
        m_waylandCorona = registry->createPlasmaShell(name, version, this);
    });

    QObject::connect(registry, &KWayland::Client::Registry::plasmaWindowManagementAnnounced,
                     [this, registry](quint32 name, quint32 version) {
        KWayland::Client::PlasmaWindowManagement *pwm = registry->createPlasmaWindowManagement(name, version, this);

        WindowSystem::WaylandInterface *wI = qobject_cast<WindowSystem::WaylandInterface *>(m_wm);

        if (wI) {
            wI->initWindowManagement(pwm);
        }
    });


    QObject::connect(registry, &KWayland::Client::Registry::plasmaVirtualDesktopManagementAnnounced,
                     [this, registry] (quint32 name, quint32 version) {
        KWayland::Client::PlasmaVirtualDesktopManagement *vdm = registry->createPlasmaVirtualDesktopManagement(name, version, this);

        WindowSystem::WaylandInterface *wI = qobject_cast<WindowSystem::WaylandInterface *>(m_wm);

        if (wI) {
            wI->initVirtualDesktopManagement(vdm);
        }
    });


    registry->setup();
    connection->roundtrip();
}

KWayland::Client::PlasmaShell *Corona::waylandCoronaInterface() const
{
    return m_waylandCorona;
}

void Corona::cleanConfig()
{
    auto containmentsEntries = config()->group("Containments");
    bool changed = false;

    for(const auto &cId : containmentsEntries.groupList()) {
        if (!containmentExists(cId.toUInt())) {
            //cleanup obsolete containments
            containmentsEntries.group(cId).deleteGroup();
            changed = true;
            qDebug() << "obsolete containment configuration deleted:" << cId;
        } else {
            //cleanup obsolete applets of running containments
            auto appletsEntries = containmentsEntries.group(cId).group("Applets");

            for(const auto &appletId : appletsEntries.groupList()) {
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

bool Corona::containmentExists(uint id) const
{
    for(const auto containment : containments()) {
        if (id == containment->id()) {
            return true;
        }
    }

    return false;
}

bool Corona::appletExists(uint containmentId, uint appletId) const
{
    Plasma::Containment *containment = nullptr;

    for(const auto cont : containments()) {
        if (containmentId == cont->id()) {
            containment = cont;
            break;
        }
    }

    if (!containment) {
        return false;
    }

    for(const auto applet : containment->applets()) {
        if (applet->id() == appletId) {
            return true;
        }
    }

    return false;
}

bool Corona::inQuit() const
{
    return m_inQuit;
}

KActivities::Consumer *Corona::activitiesConsumer() const
{
    return m_activitiesConsumer;
}

PanelShadows *Corona::dialogShadows() const
{
    return m_dialogShadows;
}

GlobalShortcuts *Corona::globalShortcuts() const
{
    return m_globalShortcuts;
}

ScreenPool *Corona::screenPool() const
{
    return m_screenPool;
}

UniversalSettings *Corona::universalSettings() const
{
    return m_universalSettings;
}

ViewSettingsFactory *Corona::viewSettingsFactory() const
{
    return m_viewSettingsFactory;
}

WindowSystem::AbstractWindowInterface *Corona::wm() const
{
    return m_wm;
}

Indicator::Factory *Corona::indicatorFactory() const
{
    return m_indicatorFactory;
}

Layouts::Manager *Corona::layoutsManager() const
{
    return m_layoutsManager;
}

Templates::Manager *Corona::templatesManager() const
{
    return m_templatesManager;
}

PlasmaExtended::ScreenPool *Corona::plasmaScreenPool() const
{
    return m_plasmaScreenPool;
}

PlasmaExtended::Theme *Corona::themeExtended() const
{
    return m_themeExtended;
}

int Corona::numScreens() const
{
    return qGuiApp->screens().count();
}

QRect Corona::screenGeometry(int id) const
{
    const auto screens = qGuiApp->screens();
    const QScreen *screen{m_screenPool->primaryScreen()};

    QString screenName;

    if (m_screenPool->hasScreenId(id)) {
        screenName = m_screenPool->connector(id);
    }

    for(const auto scr : screens) {
        if (scr->name() == screenName) {
            screen = scr;
            break;
        }
    }

    return screen->geometry();
}

CentralLayout *Corona::centralLayout(QString name) const
{
    CentralLayout *result{nullptr};

    if (!name.isEmpty()) {
        result = m_layoutsManager->synchronizer()->centralLayout(name);
    }

    return result;
}

Layout::GenericLayout *Corona::layout(QString name) const
{
    Layout::GenericLayout *result{nullptr};

    if (!name.isEmpty()) {
        result = m_layoutsManager->synchronizer()->layout(name);
    }

    return result;
}

QRegion Corona::availableScreenRegion(int id) const
{   
    //! ignore modes are added in order for notifications to be placed
    //! in better positioning and not overlap with sidebars or usually hidden views
    QList<Types::Visibility> ignoremodes({Latte::Types::AutoHide,
                                          Latte::Types::SidebarOnDemand,
                                          Latte::Types::SidebarAutoHide});


    return availableScreenRegionWithCriteria(id,
                                             QString(),
                                             ignoremodes);
}

QRegion Corona::availableScreenRegionWithCriteria(int id,
                                                  QString activityid,
                                                  QList<Types::Visibility> ignoreModes,
                                                  QList<Plasma::Types::Location> ignoreEdges,
                                                  bool ignoreExternalPanels,
                                                  bool desktopUse) const
{
    const QScreen *screen = m_screenPool->screenForId(id);
    bool inCurrentActivity{activityid.isEmpty()};

    if (!screen) {
        return {};
    }

    QRegion available = ignoreExternalPanels ? screen->geometry() : screen->availableGeometry();

    QList<Latte::View *> views;

    if (inCurrentActivity) {
        views = m_layoutsManager->synchronizer()->viewsBasedOnActivityId(m_activitiesConsumer->currentActivity());
    } else {
        views = m_layoutsManager->synchronizer()->viewsBasedOnActivityId(activityid);
    }

    if (views.isEmpty()) {
        return available;
    }

    //! blacklist irrelevant visibility modes
    if (!ignoreModes.contains(Latte::Types::None)) {
        ignoreModes << Latte::Types::None;
    }

    if (!ignoreModes.contains(Latte::Types::NormalWindow)) {
        ignoreModes << Latte::Types::NormalWindow;
    }

    bool allEdges = ignoreEdges.isEmpty();

    for (const auto *view : views) {
        bool inDesktopOffScreenStartup = desktopUse && view && view->positioner() && view->positioner()->isOffScreen();

        if (view && view->containment() && view->screen() == screen && !inDesktopOffScreenStartup
                && ((allEdges || !ignoreEdges.contains(view->location()))
                    && (view->visibility() && !ignoreModes.contains(view->visibility()->mode())))) {
            int realThickness = view->normalThickness();

            int x = 0; int y = 0; int w = 0; int h = 0;

            switch (view->formFactor()) {
            case Plasma::Types::Horizontal:
                if (view->behaveAsPlasmaPanel()) {
                    w = view->width();
                    x = view->x();
                } else {
                    w = view->maxLength() * view->width();
                    int offsetW = view->offset() * view->width();

                    switch (view->alignment()) {
                    case Latte::Types::Left:
                        x = view->x() + offsetW;
                        break;

                    case Latte::Types::Center:
                    case Latte::Types::Justify:
                        x = (view->geometry().center().x() - w/2) + 1 + offsetW;
                        break;

                    case Latte::Types::Right:
                        x = view->geometry().right() + 1 - w - offsetW;
                        break;
                    }
                }
                break;
            case Plasma::Types::Vertical:
                if (view->behaveAsPlasmaPanel()) {
                    h = view->height();
                    y = view->y();
                } else {
                    h = view->maxLength() * view->height();
                    int offsetH = view->offset() * view->height();

                    switch (view->alignment()) {
                    case Latte::Types::Top:
                        y = view->y() + offsetH;
                        break;

                    case Latte::Types::Center:
                    case Latte::Types::Justify:
                        y = (view->geometry().center().y() - h/2) + 1 + offsetH;
                        break;

                    case Latte::Types::Bottom:
                        y = view->geometry().bottom() - h - offsetH;
                        break;
                    }
                }
                break;
            }

            // Usually availableScreenRect is used by the desktop,
            // but Latte don't have desktop, then here just
            // need calculate available space for top and bottom location,
            // because the left and right are those who dodge others views
            switch (view->location()) {
            case Plasma::Types::TopEdge:
                if (view->behaveAsPlasmaPanel()) {
                    QRect viewGeometry = view->geometry();

                    if (desktopUse) {
                        //! ignore any real window slide outs in all cases
                        viewGeometry.moveTop(view->screen()->geometry().top() + view->screenEdgeMargin());
                    }

                    available -= viewGeometry;
                } else {                  
                    y = view->y();
                    available -= QRect(x, y, w, realThickness);
                }

                break;

            case Plasma::Types::BottomEdge:
                if (view->behaveAsPlasmaPanel()) {
                    QRect viewGeometry = view->geometry();

                    if (desktopUse) {
                        //! ignore any real window slide outs in all cases
                        viewGeometry.moveTop(view->screen()->geometry().bottom() - view->screenEdgeMargin() - viewGeometry.height());
                    }

                    available -= viewGeometry;
                } else {
                    y = view->geometry().bottom() - realThickness + 1;
                    available -= QRect(x, y, w, realThickness);
                }

                break;

            case Plasma::Types::LeftEdge:
                if (view->behaveAsPlasmaPanel()) {
                    QRect viewGeometry = view->geometry();

                    if (desktopUse) {
                        //! ignore any real window slide outs in all cases
                        viewGeometry.moveLeft(view->screen()->geometry().left() + view->screenEdgeMargin());
                    }

                    available -= viewGeometry;
                } else {
                    x = view->x();
                    available -= QRect(x, y, realThickness, h);
                }

                break;

            case Plasma::Types::RightEdge:
                if (view->behaveAsPlasmaPanel()) {
                    QRect viewGeometry = view->geometry();

                    if (desktopUse) {
                        //! ignore any real window slide outs in all cases
                        viewGeometry.moveLeft(view->screen()->geometry().right() - view->screenEdgeMargin() - viewGeometry.width());
                    }

                    available -= viewGeometry;
                } else {                    
                    x = view->geometry().right() - realThickness + 1;
                    available -= QRect(x, y, realThickness, h);
                }

                break;

            default:
                //! bypass clang warnings
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

QRect Corona::availableScreenRect(int id) const
{
    //! ignore modes are added in order for notifications to be placed
    //! in better positioning and not overlap with sidebars or usually hidden views
    QList<Types::Visibility> ignoremodes({Latte::Types::AutoHide,
                                          Latte::Types::SidebarOnDemand,
                                          Latte::Types::SidebarAutoHide});

    return availableScreenRectWithCriteria(id,
                                           QString(),
                                           ignoremodes);
}

QRect Corona::availableScreenRectWithCriteria(int id,
                                              QString activityid,
                                              QList<Types::Visibility> ignoreModes,
                                              QList<Plasma::Types::Location> ignoreEdges,
                                              bool ignoreExternalPanels,
                                              bool desktopUse) const
{
    const QScreen *screen = m_screenPool->screenForId(id);
    bool inCurrentActivity{activityid.isEmpty()};

    if (!screen) {
        return {};
    }

    QRect available = ignoreExternalPanels ? screen->geometry() : screen->availableGeometry();

    QList<Latte::View *> views;

    if (inCurrentActivity) {
        views = m_layoutsManager->synchronizer()->viewsBasedOnActivityId(m_activitiesConsumer->currentActivity());
    } else {
        views = m_layoutsManager->synchronizer()->viewsBasedOnActivityId(activityid);
    }

    if (views.isEmpty()) {
        return available;
    }

    //! blacklist irrelevant visibility modes
    if (!ignoreModes.contains(Latte::Types::None)) {
        ignoreModes << Latte::Types::None;
    }

    if (!ignoreModes.contains(Latte::Types::NormalWindow)) {
        ignoreModes << Latte::Types::NormalWindow;
    }

    bool allEdges = ignoreEdges.isEmpty();

    for (const auto *view : views) {
        bool inDesktopOffScreenStartup = desktopUse && view && view->positioner() && view->positioner()->isOffScreen();

        if (view && view->containment() && view->screen() == screen && !inDesktopOffScreenStartup
                && ((allEdges || !ignoreEdges.contains(view->location()))
                    && (view->visibility() && !ignoreModes.contains(view->visibility()->mode())))) {

            int appliedThickness = view->behaveAsPlasmaPanel() ? view->screenEdgeMargin() + view->normalThickness() : view->normalThickness();

            // Usually availableScreenRect is used by the desktop,
            // but Latte don't have desktop, then here just
            // need calculate available space for top and bottom location,
            // because the left and right are those who dodge others docks
            switch (view->location()) {
            case Plasma::Types::TopEdge:
                if (view->behaveAsPlasmaPanel() && desktopUse) {
                    //! ignore any real window slide outs in all cases
                    available.setTop(qMax(available.top(), view->screen()->geometry().top() + appliedThickness));
                } else {
                    available.setTop(qMax(available.top(), view->y() + appliedThickness));
                }
                break;

            case Plasma::Types::BottomEdge:
                if (view->behaveAsPlasmaPanel() && desktopUse) {
                    //! ignore any real window slide outs in all cases
                    available.setBottom(qMin(available.bottom(), view->screen()->geometry().bottom() - appliedThickness));
                } else {
                    available.setBottom(qMin(available.bottom(), view->y() + view->height() - appliedThickness));
                }
                break;

            case Plasma::Types::LeftEdge:
                if (view->behaveAsPlasmaPanel() && desktopUse) {
                    //! ignore any real window slide outs in all cases
                    available.setLeft(qMax(available.left(), view->screen()->geometry().left() + appliedThickness));
                } else {
                    available.setLeft(qMax(available.left(), view->x() + appliedThickness));
                }
                break;

            case Plasma::Types::RightEdge:
                if (view->behaveAsPlasmaPanel() && desktopUse) {
                    //! ignore any real window slide outs in all cases
                    available.setRight(qMin(available.right(), view->screen()->geometry().right() - appliedThickness));
                } else {
                    available.setRight(qMin(available.right(), view->x() + view->width() - appliedThickness));
                }
                break;

            default:
                //! bypass clang warnings
                break;
            }
        }
    }

    return available;
}

void Corona::onScreenAdded(QScreen *screen)
{
    Q_ASSERT(screen);

    int id = m_screenPool->id(screen->name());

    if (id == -1) {
        m_screenPool->insertScreenMapping(screen->name());
    }

    connect(screen, &QScreen::geometryChanged, this, &Corona::onScreenGeometryChanged);

    emit availableScreenRectChanged();
    emit screenAdded(m_screenPool->id(screen->name()));

    onScreenCountChanged();
}

void Corona::onScreenRemoved(QScreen *screen)
{
    disconnect(screen, &QScreen::geometryChanged, this, &Corona::onScreenGeometryChanged);
    onScreenCountChanged();
}

void Corona::onScreenCountChanged()
{
    m_viewsScreenSyncTimer.start();
}

void Corona::onScreenGeometryChanged(const QRect &geometry)
{
    Q_UNUSED(geometry);

    QScreen *screen = qobject_cast<QScreen *>(sender());

    if (!screen) {
        return;
    }

    const int id = m_screenPool->id(screen->name());

    if (id >= 0) {
        emit screenGeometryChanged(id);
        emit availableScreenRegionChanged();
        emit availableScreenRectChanged();
    }
}

//! the central functions that updates loading/unloading latteviews
//! concerning screen changed (for multi-screen setups mainly)
void Corona::syncLatteViewsToScreens()
{
    m_layoutsManager->synchronizer()->syncLatteViewsToScreens();
}

int Corona::primaryScreenId() const
{
    return m_screenPool->primaryScreenId();
}

void Corona::quitApplication()
{
    m_inQuit = true;

    //! this code must be called asynchronously because it is called
    //! also from qml (Settings window).
    QTimer::singleShot(300, [this]() {
        m_layoutsManager->hideLatteSettingsDialog();
        m_layoutsManager->synchronizer()->hideAllViews();
    });

    //! give the time for the views to hide themselves
    QTimer::singleShot(800, [this]() {
        qGuiApp->quit();
    });
}

void Corona::aboutApplication()
{
    if (aboutDialog) {
        aboutDialog->hide();
        aboutDialog->deleteLater();
    }

    aboutDialog = new KAboutApplicationDialog(KAboutData::applicationData());
    connect(aboutDialog.data(), &QDialog::finished, aboutDialog.data(), &QObject::deleteLater);
    m_wm->skipTaskBar(*aboutDialog);
    m_wm->setKeepAbove(aboutDialog->winId(), true);

    aboutDialog->show();
}

void Corona::loadDefaultLayout()
{
  //disabled
}

int Corona::screenForContainment(const Plasma::Containment *containment) const
{
    //FIXME: indexOf is not a proper way to support multi-screen
    // as for environment to environment the indexes change
    // also there is the following issue triggered
    // from latteView adaptToScreen()
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

    Plasma::Containment *c = const_cast<Plasma::Containment *>(containment);
    int scrId = m_layoutsManager->synchronizer()->screenForContainment(c);

    if (scrId >= 0) {
        return scrId;
    }

    return containment->lastScreen();
}

void Corona::showAlternativesForApplet(Plasma::Applet *applet)
{
    const QString alternativesQML = kPackage().filePath("appletalternativesui");

    if (alternativesQML.isEmpty()) {
        return;
    }

    Latte::View *latteView =  m_layoutsManager->synchronizer()->viewForContainment(applet->containment());

    KDeclarative::QmlObjectSharedEngine *qmlObj{nullptr};

    if (latteView) {
        latteView->setAlternativesIsShown(true);
        qmlObj = new KDeclarative::QmlObjectSharedEngine(latteView);
    } else {
        qmlObj = new KDeclarative::QmlObjectSharedEngine(this);
    }

    qmlObj->setInitializationDelayed(true);
    qmlObj->setSource(QUrl::fromLocalFile(alternativesQML));

    AlternativesHelper *helper = new AlternativesHelper(applet, qmlObj);
    qmlObj->rootContext()->setContextProperty(QStringLiteral("alternativesHelper"), helper);

    m_alternativesObjects << qmlObj;
    qmlObj->completeInitialization();

    //! Alternative dialog signals
    connect(helper, &QObject::destroyed, this, [latteView]() {
        latteView->setAlternativesIsShown(false);
    });

    connect(qmlObj->rootObject(), SIGNAL(visibleChanged(bool)),
            this, SLOT(alternativesVisibilityChanged(bool)));

    connect(applet, &Plasma::Applet::destroyedChanged, this, [this, qmlObj](bool destroyed) {
        if (!destroyed) {
            return;
        }

        QMutableListIterator<KDeclarative::QmlObjectSharedEngine *> it(m_alternativesObjects);

        while (it.hasNext()) {
            KDeclarative::QmlObjectSharedEngine *obj = it.next();

            if (obj == qmlObj) {
                it.remove();
                obj->deleteLater();
            }
        }
    });
}

void Corona::alternativesVisibilityChanged(bool visible)
{
    if (visible) {
        return;
    }

    QObject *root = sender();

    QMutableListIterator<KDeclarative::QmlObjectSharedEngine *> it(m_alternativesObjects);

    while (it.hasNext()) {
        KDeclarative::QmlObjectSharedEngine *obj = it.next();

        if (obj->rootObject() == root) {
            it.remove();
            obj->deleteLater();
        }
    }
}

QStringList Corona::containmentsIds()
{
    QStringList ids;

    for(const auto containment : containments()) {
        ids << QString::number(containment->id());
    }

    return ids;
}

QStringList Corona::appletsIds()
{
    QStringList ids;

    for(const auto containment : containments()) {
        auto applets = containment->config().group("Applets");
        ids << applets.groupList();
    }

    return ids;
}

//! Activate launcher menu through dbus interface
void Corona::activateLauncherMenu()
{
    m_globalShortcuts->activateLauncherMenu();
}

void Corona::windowColorScheme(QString windowIdAndScheme)
{
    int firstSlash = windowIdAndScheme.indexOf("-");
    QString windowIdStr = windowIdAndScheme.mid(0, firstSlash);
    QString schemeStr = windowIdAndScheme.mid(firstSlash + 1);

    if (KWindowSystem::isPlatformWayland()) {
        QTimer::singleShot(200, [this, schemeStr]() {
            //! [Wayland Case] - give the time to be informed correctly for the active window id
            //! otherwise the active window id may not be the same with the one trigerred
            //! the color scheme dbus signal
            QString windowIdStr = m_wm->activeWindow().toString();
            m_wm->schemesTracker()->setColorSchemeForWindow(windowIdStr.toUInt(), schemeStr);
        });
    } else {
        m_wm->schemesTracker()->setColorSchemeForWindow(windowIdStr.toUInt(), schemeStr);
    }
}

//! update badge for specific view item
void Corona::updateDockItemBadge(QString identifier, QString value)
{
    m_globalShortcuts->updateViewItemBadge(identifier, value);
}

void Corona::setAutostart(const bool &enabled)
{
    m_universalSettings->setAutostart(enabled);
}

void Corona::switchToLayout(QString layout)
{
    if ((layout.startsWith("file:/") || layout.startsWith("/")) && layout.endsWith(".layout.latte")) {
        importLayoutFile(layout);
    } else {
        m_layoutsManager->switchToLayout(layout);
    }
}

void Corona::importLayoutFile(const QString &filepath, const QString &suggestedLayoutName)
{
    bool isFilepathValid = (filepath.startsWith("file:/") || filepath.startsWith("/")) && filepath.endsWith(".layout.latte");

    if (!isFilepathValid) {
        qDebug() << i18n("The layout cannot be imported from file :: ") << filepath;
        return;
    }

    //! Import and load runtime a layout through dbus interface
    //! It can be used from external programs that want to update runtime
    //! the Latte shown layout
    QString layoutPath = filepath;

    //! cleanup layout path
    if (layoutPath.startsWith("file:///")) {
        layoutPath = layoutPath.remove("file://");
    } else if (layoutPath.startsWith("file://")) {
        layoutPath = layoutPath.remove("file:/");
    }

    //! check out layoutpath existence
    if (QFileInfo(layoutPath).exists()) {
        qDebug() << " Layout is going to be imported and loaded from file :: " << layoutPath << " with suggested name :: " << suggestedLayoutName;

        QString importedLayout = m_layoutsManager->importer()->importLayout(layoutPath, suggestedLayoutName);

        if (importedLayout.isEmpty()) {
            qDebug() << i18n("The layout cannot be imported from file :: ") << layoutPath;
        } else {
           m_layoutsManager->switchToLayout(importedLayout, MemoryUsage::SingleLayout);
        }
    } else {
        qDebug() << " Layout from missing file can not be imported and loaded :: " << layoutPath;
    }
}

void Corona::showSettingsWindow(int page)
{
    if (m_inStartup) {
        return;
    }

    Settings::Dialog::ConfigurationPage p = Settings::Dialog::LayoutPage;

    if (page >= Settings::Dialog::LayoutPage && page <= Settings::Dialog::PreferencesPage) {
        p = static_cast<Settings::Dialog::ConfigurationPage>(page);
    }

    m_layoutsManager->showLatteSettingsDialog(p);
}

QStringList Corona::contextMenuData(const uint &containmentId)
{
    QStringList data;
    Types::ViewType viewType{Types::DockView};
    auto view = m_layoutsManager->synchronizer()->viewForContainment(containmentId);

    if (view) {
        viewType = view->type();
    }

    data << QString::number((int)m_layoutsManager->memoryUsage()); // Memory Usage
    data << m_layoutsManager->centralLayoutsNames().join(";;"); // All Active layouts
    data << m_layoutsManager->synchronizer()->currentLayoutsNames().join(";;"); // All Current layouts
    data << m_universalSettings->contextMenuActionsAlwaysShown().join(";;");

    QStringList layoutsmenu;

    for(const auto &layoutName : m_layoutsManager->synchronizer()->menuLayouts()) {
        if (m_layoutsManager->synchronizer()->centralLayout(layoutName)
                || m_layoutsManager->memoryUsage() == Latte::MemoryUsage::SingleLayout) {
            QStringList layoutdata;
            Data::LayoutIcon layouticon = m_layoutsManager->iconForLayout(layoutName);
            layoutdata << layoutName;
            layoutdata << QString::number(layouticon.isBackgroundFile);
            layoutdata << layouticon.name;
            layoutsmenu << layoutdata.join("**");
        }
    }

    data << layoutsmenu.join(";;");
    data << (view ? view->layout()->name() : QString());   //Selected View layout*/

    QStringList viewtype;
    viewtype << QString::number((int)viewType); //Selected View type

    if (view && view->isOriginal()) { /*View*/
        auto originalview = qobject_cast<Latte::OriginalView *>(view);
        viewtype << "0";              //original view
        viewtype <<  QString::number(originalview->clonesCount());
    } else if (view && view->isCloned()) {
        viewtype << "1";              //cloned view
        viewtype << "0";              //has no clones
    } else {
        viewtype << "0";              //original view
        viewtype << "0";              //has no clones
    }

    data << viewtype.join(";;");

    return data;
}

QStringList Corona::viewTemplatesData()
{
    QStringList data;

    Latte::Data::GenericTable<Data::Generic> viewtemplates = m_templatesManager->viewTemplates();

    for(int i=0; i<viewtemplates.rowCount(); ++i) {
        data << viewtemplates[i].name;
        data << viewtemplates[i].id;
    }

    return data;
}

void Corona::addView(const uint &containmentId, const QString &templateId)
{
    if (containmentId <= 0) {
        auto currentlayouts = m_layoutsManager->currentLayouts();
        if (currentlayouts.count() > 0) {
            currentlayouts[0]->newView(templateId);
        }
    } else {
        auto view = m_layoutsManager->synchronizer()->viewForContainment((int)containmentId);
        if (view) {
            view->newView(templateId);
        }
    }
}

void Corona::duplicateView(const uint &containmentId)
{
    auto view = m_layoutsManager->synchronizer()->viewForContainment((int)containmentId);
    if (view) {
        view->duplicateView();
    }
}

void Corona::exportViewTemplate(const uint &containmentId)
{
    auto view = m_layoutsManager->synchronizer()->viewForContainment((int)containmentId);
    if (view) {
        view->exportTemplate();
    }
}

void Corona::moveViewToLayout(const uint &containmentId, const QString &layoutName)
{
    auto view = m_layoutsManager->synchronizer()->viewForContainment((int)containmentId);
    if (view && !layoutName.isEmpty() && view->layout()->name() != layoutName) {
        Latte::Types::ScreensGroup screensgroup{Latte::Types::SingleScreenGroup};

        if (view->isOriginal()) {
            auto originalview = qobject_cast<Latte::OriginalView *>(view);
            screensgroup = originalview->screensGroup();
        }

        view->positioner()->setNextLocation(layoutName, screensgroup, "", Plasma::Types::Floating, Latte::Types::NoneAlignment);
    }
}

void Corona::removeView(const uint &containmentId)
{
    auto view = m_layoutsManager->synchronizer()->viewForContainment((int)containmentId);
    if (view) {
        view->removeView();
    }
}

void Corona::setBackgroundFromBroadcast(QString activity, QString screenName, QString filename)
{
    if (filename.startsWith("file://")) {
        filename = filename.remove(0,7);
    }

    PlasmaExtended::BackgroundCache::self()->setBackgroundFromBroadcast(activity, screenName, filename);
}

void Corona::setBroadcastedBackgroundsEnabled(QString activity, QString screenName, bool enabled)
{
    PlasmaExtended::BackgroundCache::self()->setBroadcastedBackgroundsEnabled(activity, screenName, enabled);
}

void Corona::toggleHiddenState(QString layoutName, QString viewName, QString screenName, int screenEdge)
{
    if (layoutName.isEmpty()) {
        for(auto layout : m_layoutsManager->currentLayouts()) {
            layout->toggleHiddenState(viewName, screenName, (Plasma::Types::Location)screenEdge);
        }
    } else {
        Layout::GenericLayout *gLayout = layout(layoutName);

        if (gLayout) {
            gLayout->toggleHiddenState(viewName, screenName, (Plasma::Types::Location)screenEdge);
        }
    }
}

void Corona::importFullConfiguration(const QString &file)
{
    m_importFullConfigurationFile = file;
    quitApplication();
}

inline void Corona::qmlRegisterTypes() const
{   
    qmlRegisterUncreatableMetaObject(Latte::Settings::staticMetaObject,
                                     "org.kde.latte.private.app",          // import statement
                                     0, 1,                                 // major and minor version of the import
                                     "Settings",                           // name in QML
                                     "Error: only enums of latte app settings");

    qmlRegisterType<Latte::BackgroundTracker>("org.kde.latte.private.app", 0, 1, "BackgroundTracker");
    qmlRegisterType<Latte::Interfaces>("org.kde.latte.private.app", 0, 1, "Interfaces");
    qmlRegisterType<Latte::ContextMenuLayerQuickItem>("org.kde.latte.private.app", 0, 1, "ContextMenuLayer");
    qmlRegisterAnonymousType<QScreen>("latte-dock", 1);
    qmlRegisterAnonymousType<Latte::View>("latte-dock", 1);
    qmlRegisterAnonymousType<Latte::ViewPart::WindowsTracker>("latte-dock", 1);
    qmlRegisterAnonymousType<Latte::ViewPart::TrackerPart::CurrentScreenTracker>("latte-dock", 1);
    qmlRegisterAnonymousType<Latte::ViewPart::TrackerPart::AllScreensTracker>("latte-dock", 1);
    qmlRegisterAnonymousType<Latte::WindowSystem::SchemeColors>("latte-dock", 1);
    qmlRegisterAnonymousType<Latte::WindowSystem::Tracker::LastActiveWindow>("latte-dock", 1);
    qmlRegisterAnonymousType<Latte::Types>("latte-dock", 1);

}

}
