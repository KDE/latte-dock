/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "view.h"

// local
#include "effects.h"
#include "positioner.h"
#include "visibilitymanager.h"
#include "settings/primaryconfigview.h"
#include "settings/secondaryconfigview.h"
#include "settings/viewsettingsfactory.h"
#include "settings/widgetexplorerview.h"
#include "../apptypes.h"
#include "../lattecorona.h"
#include "../data/layoutdata.h"
#include "../data/viewstable.h"
#include "../declarativeimports/interfaces.h"
#include "../indicator/factory.h"
#include "../layout/genericlayout.h"
#include "../layouts/manager.h"
#include "../layouts/storage.h"
#include "../plasma/extended/theme.h"
#include "../screenpool.h"
#include "../settings/universalsettings.h"
#include "../settings/exporttemplatedialog/exporttemplatedialog.h"
#include "../shortcuts/globalshortcuts.h"
#include "../shortcuts/shortcutstracker.h"

// Qt
#include <QAction>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QMenu>

// KDe
#include <KActionCollection>
#include <KActivities/Consumer>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>

// Plasma
#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <PlasmaQuick/AppletQuickItem>

#define BLOCKHIDINGDRAGTYPE "View::ContainsDrag()"
#define BLOCKHIDINGNEEDSATTENTIONTYPE "View::Containment::NeedsAttentionState()"
#define BLOCKHIDINGREQUESTSINPUTTYPE "View::Containment::RequestsInputState()"

namespace Latte {

//! both alwaysVisible and byPassWMX11 are passed through corona because
//! during the view window creation containment hasn't been set, but these variables
//! are needed in order for window flags to be set correctly
View::View(Plasma::Corona *corona, QScreen *targetScreen, bool byPassX11WM)
    : PlasmaQuick::ContainmentView(corona),
      m_effects(new ViewPart::Effects(this)),
      m_interface(new ViewPart::ContainmentInterface(this)),
      m_parabolic(new ViewPart::Parabolic(this)),
      m_sink(new ViewPart::EventsSink(this))
{
    //this is disabled because under wayland breaks Views positioning
    //setVisible(false);

    m_corona = qobject_cast<Latte::Corona *>(corona);

    //! needs to be created after Effects because it catches some of its signals
    //! and avoid a crash from View::winId() at the same time
    m_positioner = new ViewPart::Positioner(this);

    // setTitle(corona->kPackage().metadata().name());
    setIcon(qGuiApp->windowIcon());
    setResizeMode(QuickViewSharedEngine::SizeRootObjectToView);
    setColor(QColor(Qt::transparent));
    setClearBeforeRendering(true);

    const auto flags = Qt::FramelessWindowHint
            | Qt::NoDropShadowWindowHint
            | Qt::WindowDoesNotAcceptFocus;

    if (byPassX11WM) {
        setFlags(flags | Qt::BypassWindowManagerHint);
        //! needs to be set early enough
        m_byPassWM = byPassX11WM;
    } else {
        setFlags(flags);
    }

    if (KWindowSystem::isPlatformX11()) {
        //! Enable OnAllDesktops during creation in order to protect corner cases that is ignored
        //! during startup. Such corner case is bug #447689.
        //! Best guess is that this is needed because OnAllDesktops is set through visibilitymanager
        //! after containment has been assigned. That delay might lead wm ignoring the flag
        //! until it is reapplied.
        KWindowSystem::setOnAllDesktops(winId(), true);
    }

    if (targetScreen) {
        m_positioner->setScreenToFollow(targetScreen);
    } else {
        qDebug() << "org.kde.view :::: corona was found properly!!!";
        m_positioner->setScreenToFollow(m_corona->screenPool()->primaryScreen());
    }

    m_releaseGrabTimer.setInterval(400);
    m_releaseGrabTimer.setSingleShot(true);
    connect(&m_releaseGrabTimer, &QTimer::timeout, this, &View::releaseGrab);

    connect(m_interface, &ViewPart::ContainmentInterface::hasExpandedAppletChanged, this, &View::updateTransientWindowsTracking);

    connect(this, &View::containmentChanged, this, &View::groupIdChanged);
    connect(this, &View::containmentChanged
            , this, [ &, byPassX11WM]() {
        qDebug() << "dock view c++ containment changed 1...";

        if (!this->containment())
            return;

        qDebug() << "dock view c++ containment changed 2...";

        setTitle(validTitle());

        //! First load default values from file
        restoreConfig();

        //! Afterwards override that values in case during creation something different is needed
        setByPassWM(byPassX11WM);

        //! Check the screen assigned to this dock
        reconsiderScreen();

        //! needs to be created before visibility creation because visibility uses it
        if (!m_windowsTracker) {
            m_windowsTracker = new ViewPart::WindowsTracker(this);
            emit windowsTrackerChanged();
        }

        if (!m_visibility) {
            m_visibility = new ViewPart::VisibilityManager(this);

            connect(m_visibility, &ViewPart::VisibilityManager::isHiddenChanged, this, [&]() {
                if (m_visibility->isHidden()) {
                    m_interface->deactivateApplets();
                }
            });

            connect(m_visibility, &ViewPart::VisibilityManager::containsMouseChanged,
                    this, &View::updateTransientWindowsTracking);

            //! Deprecated because with Plasma 5.19.3 the issue does not appear.
            //! The issue was that when FrameExtents where zero strange behaviors were
            //! occuring from KWin, e.g. the panels were moving outside of screen and
            //! panel external shadows were positioned out of place.
            /*connect(m_visibility, &ViewPart::VisibilityManager::frameExtentsCleared, this, [&]() {
                if (behaveAsPlasmaPanel()) {
                    //! recreate view because otherwise compositor frame extents implementation
                    //! is triggering a crazy behavior of moving/hiding the view and freezing Latte
                    //! in some cases.
                    //reloadSource();
                }
            });*/

            emit visibilityChanged();
        }

        if (!m_indicator) {
            m_indicator = new ViewPart::Indicator(this);
            emit indicatorChanged();
        }

        if (m_positioner) {
            //! immediateSyncGeometry helps avoiding binding loops from containment qml side
            m_positioner->immediateSyncGeometry();
        }

        connect(this->containment(), SIGNAL(statusChanged(Plasma::Types::ItemStatus)), SLOT(statusChanged(Plasma::Types::ItemStatus)));
        connect(this->containment(), &Plasma::Containment::showAddWidgetsInterface, this, &View::showWidgetExplorer);
        connect(this->containment(), &Plasma::Containment::userConfiguringChanged, this, [&]() {
            emit inEditModeChanged();
        });

        connect(this->containment(), &Plasma::Containment::destroyedChanged, this, [&]() {
            m_inDelete = containment()->destroyed();
        });

        if (m_corona->viewSettingsFactory()->hasOrphanSettings()
                && m_corona->viewSettingsFactory()->hasVisibleSettings()
                && m_corona->viewSettingsFactory()->lastContainment() == containment()) {
            //! used mostly from view recreations in order to inform config windows that view has been updated
            m_primaryConfigView = m_corona->viewSettingsFactory()->primaryConfigView();
            m_primaryConfigView->setParentView(this, true);
        }

        emit containmentActionsChanged();
    }, Qt::DirectConnection);

    if (m_corona) {
        connect(m_corona, &Latte::Corona::viewLocationChanged, this, &View::dockLocationChanged);
    }
}

View::~View()
{
    m_inDelete = true;

    //! clear Layout connections
    m_visibleHackTimer1.stop();
    m_visibleHackTimer2.stop();
    for (auto &c : connectionsLayout) {
        disconnect(c);
    }

    //! unload indicators
    if (m_indicator) {
        m_indicator->unloadIndicators();
    }

    disconnectSensitiveSignals();
    disconnect(containment(), SIGNAL(statusChanged(Plasma::Types::ItemStatus)), this, SLOT(statusChanged(Plasma::Types::ItemStatus)));

    qDebug() << "dock view deleting...";

    //! this disconnect does not free up connections correctly when
    //! latteView is deleted. A crash for this example is the following:
    //! switch to Alternative Session and disable compositing,
    //! the signal creating the crash was probably from deleted
    //! windows.
    //! this->disconnect();

    if (m_primaryConfigView && m_corona->inQuit()) {
        //! delete only when application is quitting
        delete m_primaryConfigView;
    }

    if (m_appletConfigView) {
        delete m_appletConfigView;
    }

    //needs to be deleted before Effects because it catches some of its signals
    if (m_positioner) {
        delete m_positioner;
    }

    if (m_effects) {
        delete m_effects;
    }

    if (m_indicator) {
        delete m_indicator;
    }

    if (m_interface) {
        delete m_interface;
    }

    if (m_visibility) {
        delete m_visibility;
    }

    if (m_windowsTracker) {
        delete m_windowsTracker;
    }
}

void View::init(Plasma::Containment *plasma_containment)
{
    connect(this, &QQuickWindow::xChanged, this, &View::geometryChanged);
    connect(this, &QQuickWindow::yChanged, this, &View::geometryChanged);
    connect(this, &QQuickWindow::widthChanged, this, &View::geometryChanged);
    connect(this, &QQuickWindow::heightChanged, this, &View::geometryChanged);

    connect(this, &QQuickWindow::xChanged, this, &View::xChanged);
    connect(this, &QQuickWindow::xChanged, this, &View::updateAbsoluteGeometry);
    connect(this, &QQuickWindow::yChanged, this, &View::yChanged);
    connect(this, &QQuickWindow::yChanged, this, &View::updateAbsoluteGeometry);
    connect(this, &QQuickWindow::widthChanged, this, &View::widthChanged);
    connect(this, &QQuickWindow::widthChanged, this, &View::updateAbsoluteGeometry);
    connect(this, &QQuickWindow::heightChanged, this, &View::heightChanged);
    connect(this, &QQuickWindow::heightChanged, this, &View::updateAbsoluteGeometry);

    connect(this, &View::fontPixelSizeChanged, this, &View::editThicknessChanged);
    connect(this, &View::maxNormalThicknessChanged, this, &View::editThicknessChanged);

    connect(this, &View::activitiesChanged, this, &View::applyActivitiesToWindows);
    connect(m_positioner, &ViewPart::Positioner::winIdChanged, this, &View::applyActivitiesToWindows);

    connect(this, &View::alignmentChanged, this, [&](){
        // inform neighbour vertical docks/panels to adjust their positioning
        if (m_inDelete || formFactor() == Plasma::Types::Vertical) {
            return;
        }

        emit availableScreenRectChangedFrom(this);
        emit availableScreenRegionChangedFrom(this);
    });

    connect(this, &View::maxLengthChanged, this, [&]() {
        if (m_inDelete) {
            return;
        }

        emit availableScreenRectChangedFrom(this);
        emit availableScreenRegionChangedFrom(this);
    });

    connect(this, &View::offsetChanged, this, [&]() {
        if (m_inDelete ) {
            return;
        }

        emit availableScreenRectChangedFrom(this);
        emit availableScreenRegionChangedFrom(this);
    });

    connect(this, &View::localGeometryChanged, this, [&]() {
        updateAbsoluteGeometry();
    });
    connect(this, &View::screenEdgeMarginEnabledChanged, this, [&]() {
        updateAbsoluteGeometry();
    });

    //! used in order to disconnect it when it should NOT be called because it creates crashes
    connect(this, &View::availableScreenRectChangedFrom, m_corona, &Latte::Corona::availableScreenRectChangedFrom);
    connect(this, &View::availableScreenRegionChangedFrom, m_corona, &Latte::Corona::availableScreenRegionChangedFrom);
    connect(m_corona, &Latte::Corona::availableScreenRectChangedFrom, this, &View::availableScreenRectChangedFromSlot);
    connect(m_corona, &Latte::Corona::verticalUnityViewHasFocus, this, &View::topViewAlwaysOnTop);

    connect(this, &View::byPassWMChanged, this, &View::saveConfig);
    connect(this, &View::isPreferredForShortcutsChanged, this, &View::saveConfig);
    connect(this, &View::nameChanged, this, &View::saveConfig);
    connect(this, &View::onPrimaryChanged, this, &View::saveConfig);
    connect(this, &View::typeChanged, this, &View::saveConfig);

    connect(this, &View::normalThicknessChanged, this, [&]() {
        emit availableScreenRectChangedFrom(this);
    });

    connect(m_effects, &ViewPart::Effects::innerShadowChanged, this, [&]() {
        emit availableScreenRectChangedFrom(this);
    });
    connect(m_positioner, &ViewPart::Positioner::onHideWindowsForSlidingOut, this, &View::hideWindowsForSlidingOut);
    connect(m_positioner, &ViewPart::Positioner::screenGeometryChanged, this, &View::screenGeometryChanged);
    connect(m_positioner, &ViewPart::Positioner::windowSizeChanged, this, [&]() {
        emit availableScreenRectChangedFrom(this);
    });

    connect(m_interface, &ViewPart::ContainmentInterface::hasExpandedAppletChanged, this, &View::verticalUnityViewHasFocus);

    //! View sends this signal in order to avoid crashes from ViewPart::Indicator when the view is recreated
    connect(m_corona->indicatorFactory(), &Latte::Indicator::Factory::indicatorChanged, this, [&](const QString &indicatorId) {
        emit indicatorPluginChanged(indicatorId);
    });

    connect(this, &View::indicatorPluginChanged, this, [&](const QString &indicatorId) {
        if (m_indicator && m_indicator->isCustomIndicator() && m_indicator->type() == indicatorId) {
            reloadSource();
        }
    });

    connect(m_corona->indicatorFactory(), &Latte::Indicator::Factory::indicatorRemoved, this, &View::indicatorPluginRemoved);

    //! Assign app interfaces in be accessible through containment graphic item
    QQuickItem *containmentGraphicItem = qobject_cast<QQuickItem *>(plasma_containment->property("_plasma_graphicObject").value<QObject *>());

    if (containmentGraphicItem) {
        containmentGraphicItem->setProperty("_latte_globalShortcuts_object", QVariant::fromValue(m_corona->globalShortcuts()->shortcutsTracker()));
        containmentGraphicItem->setProperty("_latte_layoutsManager_object", QVariant::fromValue(m_corona->layoutsManager()));
        containmentGraphicItem->setProperty("_latte_themeExtended_object", QVariant::fromValue(m_corona->themeExtended()));
        containmentGraphicItem->setProperty("_latte_universalSettings_object", QVariant::fromValue(m_corona->universalSettings()));
        containmentGraphicItem->setProperty("_latte_view_object", QVariant::fromValue(this));

        Latte::Interfaces *ifacesGraphicObject = qobject_cast<Latte::Interfaces *>(containmentGraphicItem->property("_latte_view_interfacesobject").value<QObject *>());

        if (ifacesGraphicObject) {
            ifacesGraphicObject->updateView();
            setInterfacesGraphicObj(ifacesGraphicObject);
        }
    }

    setSource(corona()->kPackage().filePath("lattedockui"));

    //! immediateSyncGeometry helps avoiding binding loops from containment qml side
    m_positioner->immediateSyncGeometry();

    qDebug() << "SOURCE:" << source();
}

void View::reloadSource()
{
    if (m_layout && containment()) {
        // if (settingsWindowIsShown()) {
        //     m_configView->deleteLater();
        // }

        engine()->clearComponentCache();
        m_layout->recreateView(containment(), settingsWindowIsShown());
    }
}

bool View::inDelete() const
{
    return m_inDelete;
}

bool View::inReadyState() const
{
    return (m_layout != nullptr);
}

void View::disconnectSensitiveSignals()
{
    m_initLayoutTimer.stop();

    disconnect(this, &View::availableScreenRectChangedFrom, m_corona, &Latte::Corona::availableScreenRectChangedFrom);
    disconnect(this, &View::availableScreenRegionChangedFrom, m_corona, &Latte::Corona::availableScreenRegionChangedFrom);
    disconnect(m_corona, &Latte::Corona::availableScreenRectChangedFrom, this, &View::availableScreenRectChangedFromSlot);
    disconnect(m_corona, &Latte::Corona::verticalUnityViewHasFocus, this, &View::topViewAlwaysOnTop);

    setLayout(nullptr);
}

void View::availableScreenRectChangedFromSlot(View *origin)
{
    if (m_inDelete || origin == this || !origin) {
        return;
    }

    if (formFactor() == Plasma::Types::Vertical
            && origin->formFactor() == Plasma::Types::Horizontal //! accept only horizontal views
            && !(origin->location() == Plasma::Types::TopEdge && m_positioner->isStickedOnTopEdge()) //! ignore signals in such case
            && !(origin->location() == Plasma::Types::BottomEdge && m_positioner->isStickedOnBottomEdge()) //! ignore signals in such case
            && origin->layout()
            && m_layout
            && origin->layout()->lastUsedActivity() == m_layout->lastUsedActivity()) {
        //! must be in same activity
        m_positioner->syncGeometry();
    }
}

void View::setupWaylandIntegration()
{
    if (m_shellSurface)
        return;

    if (Latte::Corona *c = qobject_cast<Latte::Corona *>(corona())) {
        using namespace KWayland::Client;
        PlasmaShell *interface {c->waylandCoronaInterface()};

        if (!interface)
            return;

        Surface *s{Surface::fromWindow(this)};

        if (!s)
            return;

        m_shellSurface = interface->createSurface(s, this);
        qDebug() << "WAYLAND dock window surface was created...";
        if (m_visibility) {
            m_visibility->initViewFlags();
        }
        if (m_positioner) {
            m_positioner->updateWaylandId();
        }
    }
}

KWayland::Client::PlasmaShellSurface *View::surface()
{
    return m_shellSurface;
}

//! the main function which decides if this dock is at the
//! correct screen
void View::reconsiderScreen()
{
    m_positioner->reconsiderScreen();
}

void View::duplicateView()
{
    QString storedTmpViewFilepath = m_layout->storedView(containment()->id());
    newView(storedTmpViewFilepath);
}

void View::exportTemplate()
{
    Latte::Settings::Dialog::ExportTemplateDialog *exportDlg = new Latte::Settings::Dialog::ExportTemplateDialog(this);
    exportDlg->show();
}

void View::newView(const QString &templateFile)
{
    if (templateFile.isEmpty() || !m_layout) {
        return;
    }

    Data::ViewsTable templateviews = Layouts::Storage::self()->views(templateFile);

    if (templateviews.rowCount() <= 0) {
        return;
    }

    Data::View nextdata = templateviews[0];
    int scrId = onPrimary() ? m_corona->screenPool()->primaryScreenId() : m_positioner->currentScreenId();

    QList<Plasma::Types::Location> freeedges = m_layout->freeEdges(scrId);

    if (!freeedges.contains(nextdata.edge)) {
        nextdata.edge = (freeedges.count() > 0 ? freeedges[0] : Plasma::Types::BottomEdge);
    }

    nextdata.setState(Data::View::OriginFromViewTemplate, templateFile);

    m_layout->newView(nextdata);
}

void View::removeView()
{
    if (m_layout) {
        QAction *removeAct = action("remove");

        if (removeAct) {
            removeAct->trigger();
        }
    }
}

bool View::settingsWindowIsShown()
{
    return m_primaryConfigView && (m_primaryConfigView->parentView()==this) && m_primaryConfigView->isVisible();
}

void View::showSettingsWindow()
{
    if (!settingsWindowIsShown()) {
        emit m_visibility->mustBeShown();
        showConfigurationInterface(containment());
        applyActivitiesToWindows();
    }
}

QQuickView *View::configView()
{
    return m_primaryConfigView.data();
}

void View::showConfigurationInterface(Plasma::Applet *applet)
{
    if (!applet || !applet->containment())
        return;

    Plasma::Containment *c = qobject_cast<Plasma::Containment *>(applet);

    if (m_primaryConfigView && c && c->isContainment() && c == this->containment()) {
        if (m_primaryConfigView->isVisible()) {
            m_primaryConfigView->hideConfigWindow();
        } else {
            m_primaryConfigView->showConfigWindow();
            applyActivitiesToWindows();
        }

        return;
    } else if (m_appletConfigView) {
        if (m_appletConfigView->applet() == applet) {
            m_appletConfigView->show();

            if (KWindowSystem::isPlatformX11()) {
                m_appletConfigView->requestActivate();
            }
            return;
        } else {
            m_appletConfigView->hide();
        }
    }

    bool delayConfigView = false;

    if (c && containment() && c->isContainment() && c->id() == containment()->id()) {
        m_primaryConfigView = m_corona->viewSettingsFactory()->primaryConfigView(this);
        applyActivitiesToWindows();
    } else {
        m_appletConfigView = new PlasmaQuick::ConfigView(applet);
        m_appletConfigView.data()->init();

        //! center applet config window
        m_appletConfigView->setScreen(screen());
        QRect scrgeometry = screenGeometry();
        QPoint position{scrgeometry.center().x() - m_appletConfigView->width() / 2, scrgeometry.center().y() - m_appletConfigView->height() / 2 };
        //!under wayland probably needs another workaround
        m_appletConfigView->setPosition(position);

        m_appletConfigView->show();
    }
}

void View::showWidgetExplorer(const QPointF &point)
{
    auto widgetExplorerView = m_corona->viewSettingsFactory()->widgetExplorerView(this);

    if (!widgetExplorerView->isVisible()) {
        widgetExplorerView->showAfter(250);
    }
}

QRect View::localGeometry() const
{
    return m_localGeometry;
}

void View::setLocalGeometry(const QRect &geometry)
{
    if (m_localGeometry == geometry) {
        return;
    }

    m_localGeometry = geometry;
    emit localGeometryChanged();
}


QString View::name() const
{
    return m_name;
}

void View::setName(const QString &newname)
{
    if (m_name == newname) {
        return;
    }

    m_name = newname;
    emit nameChanged();
}

QString View::validTitle() const
{
    if (!containment()) {
        return QString();
    }

    return QString("#view#" + QString::number(containment()->id()));
}

void View::updateAbsoluteGeometry(bool bypassChecks)
{
    //! there was a -1 in height and width here. The reason of this
    //! if I remember correctly was related to multi-screen but I cant
    //! remember exactly the reason, something related to right edge in
    //! multi screen environment. BUT this was breaking the entire AlwaysVisible
    //! experience with struts. Removing them in order to restore correct
    //! behavior and keeping this comment in order to check for
    //! multi-screen breakage
    QRect absGeometry = m_localGeometry;
    absGeometry.moveLeft(x() + m_localGeometry.x());
    absGeometry.moveTop(y() + m_localGeometry.y());

    if (behaveAsPlasmaPanel()) {
        int currentScreenEdgeMargin = m_screenEdgeMarginEnabled ? qMax(0, m_screenEdgeMargin) : 0;

        if (location() == Plasma::Types::BottomEdge) {
            absGeometry.moveTop(screenGeometry().bottom() - currentScreenEdgeMargin - m_normalThickness);
        } else if (location() == Plasma::Types::TopEdge) {
            absGeometry.moveTop(screenGeometry().top() + currentScreenEdgeMargin);
        } else if (location() == Plasma::Types::LeftEdge) {
            absGeometry.moveLeft(screenGeometry().left() + currentScreenEdgeMargin);
        } else if (location() == Plasma::Types::RightEdge) {
            absGeometry.moveLeft(screenGeometry().right() - currentScreenEdgeMargin - m_normalThickness);
        }
    }

    if (KWindowSystem::isPlatformX11() && devicePixelRatio() != 1.0) {
        //!Fix for X11 Global Scale, I dont think this could be pixel perfect accurate
        auto factor = devicePixelRatio();
        absGeometry = QRect(qRound(absGeometry.x() * factor),
                            qRound(absGeometry.y() * factor),
                            qRound(absGeometry.width() * factor),
                            qRound(absGeometry.height() * factor));
    }

    if (m_absoluteGeometry == absGeometry && !bypassChecks) {
        return;
    }

    if (m_absoluteGeometry != absGeometry) {
        m_absoluteGeometry = absGeometry;
        emit absoluteGeometryChanged(m_absoluteGeometry);
    }

    if ((m_absoluteGeometry != absGeometry) || bypassChecks) {
        //! inform others such as neighbour vertical views that new geometries are applied
        //! main use of BYPASSCKECKS is from Positioner when the view changes screens
        emit availableScreenRectChangedFrom(this);
        emit availableScreenRegionChangedFrom(this);
    }
}

void View::statusChanged(Plasma::Types::ItemStatus status)
{
    if (!containment()) {
        return;
    }

    //! Fix for #443236, following setFlags(...) need to be added at all three cases
    //! but initViewFlags() should be called afterwards because setFlags(...) breaks
    //! the Dock window default behavior under x11
    if (status == Plasma::Types::NeedsAttentionStatus || status == Plasma::Types::RequiresAttentionStatus) {
        m_visibility->addBlockHidingEvent(BLOCKHIDINGNEEDSATTENTIONTYPE);
        setFlags(flags() | Qt::WindowDoesNotAcceptFocus);
        m_visibility->initViewFlags();
        if (m_shellSurface) {
            m_shellSurface->setPanelTakesFocus(false);
        }
    } else if (status == Plasma::Types::AcceptingInputStatus) {
        m_visibility->removeBlockHidingEvent(BLOCKHIDINGNEEDSATTENTIONTYPE);
        setFlags(flags() & ~Qt::WindowDoesNotAcceptFocus);
        m_visibility->initViewFlags();
        if (KWindowSystem::isPlatformX11()) {
            KWindowSystem::forceActiveWindow(winId());
        }
        if (m_shellSurface) {
            m_shellSurface->setPanelTakesFocus(true);
        }
    } else {
        updateTransientWindowsTracking();
        m_visibility->removeBlockHidingEvent(BLOCKHIDINGNEEDSATTENTIONTYPE);
        setFlags(flags() | Qt::WindowDoesNotAcceptFocus);
        m_visibility->initViewFlags();
        if (m_shellSurface) {
            m_shellSurface->setPanelTakesFocus(false);
        }
    }
}

void View::addTransientWindow(QWindow *window)
{
    if (!m_transientWindows.contains(window) && !window->flags().testFlag(Qt::ToolTip) && !window->title().startsWith("#debugwindow#")) {
        m_transientWindows.append(window);

        QString winPtrStr = "0x" + QString::number((qulonglong)window,16);
        m_visibility->addBlockHidingEvent(winPtrStr);

        if (m_visibility->hasBlockHidingEvent(Latte::GlobalShortcuts::SHORTCUTBLOCKHIDINGTYPE)) {
            m_visibility->removeBlockHidingEvent(Latte::GlobalShortcuts::SHORTCUTBLOCKHIDINGTYPE);
        }

        connect(window, &QWindow::visibleChanged, this, &View::removeTransientWindow);
    }
}

void View::removeTransientWindow(const bool &visible)
{
    QWindow *window = static_cast<QWindow *>(QObject::sender());

    if (window && !visible) {
        QString winPtrStr = "0x" + QString::number((qulonglong)window,16);
        m_visibility->removeBlockHidingEvent(winPtrStr);
        disconnect(window, &QWindow::visibleChanged, this, &View::removeTransientWindow);
        m_transientWindows.removeAll(window);

        if (m_visibility->hasBlockHidingEvent(Latte::GlobalShortcuts::SHORTCUTBLOCKHIDINGTYPE)) {
            m_visibility->removeBlockHidingEvent(Latte::GlobalShortcuts::SHORTCUTBLOCKHIDINGTYPE);
        }

        updateTransientWindowsTracking();
    }
}

void View::updateTransientWindowsTracking()
{
    for(QWindow *window: qApp->topLevelWindows()) {
        if (window->transientParent() == this && window->isVisible()){
            addTransientWindow(window);
            break;
        }
    }
}

Types::ViewType View::type() const
{
    return m_type;
}

void View::setType(Types::ViewType type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    emit typeChanged();
}

bool View::alternativesIsShown() const
{
    return m_alternativesIsShown;
}

void View::setAlternativesIsShown(bool show)
{
    if (m_alternativesIsShown == show) {
        return;
    }

    m_alternativesIsShown = show;

    emit alternativesIsShownChanged();
}

bool View::containsDrag() const
{
    return m_containsDrag;
}

void View::setContainsDrag(bool contains)
{
    if (m_containsDrag == contains) {
        return;
    }

    m_containsDrag = contains;


    if (m_containsDrag) {
        m_visibility->addBlockHidingEvent(BLOCKHIDINGDRAGTYPE);
    } else {
        m_visibility->removeBlockHidingEvent(BLOCKHIDINGDRAGTYPE);
    }

    emit containsDragChanged();
}

bool View::containsMouse() const
{
    return m_containsMouse;
}

int View::normalThickness() const
{
    return m_normalThickness;
}

void View::setNormalThickness(int thickness)
{
    if (m_normalThickness == thickness) {
        return;
    }

    m_normalThickness = thickness;
    emit normalThicknessChanged();
}

int View::maxNormalThickness() const
{
    return m_maxNormalThickness;
}

void View::setMaxNormalThickness(int thickness)
{
    if (m_maxNormalThickness == thickness) {
        return;
    }

    m_maxNormalThickness = thickness;
    emit maxNormalThicknessChanged();
}

int View::headThicknessGap() const
{
    return m_headThicknessGap;
}

void View::setHeadThicknessGap(int thickness)
{
    if (m_headThicknessGap == thickness) {
        return;
    }

    m_headThicknessGap = thickness;
    emit headThicknessGapChanged();
}

bool View::byPassWM() const
{
    return m_byPassWM;
}

void View::setByPassWM(bool bypass)
{
    if (m_byPassWM == bypass) {
        return;
    }

    m_byPassWM = bypass;
    emit byPassWMChanged();
}

bool View::behaveAsPlasmaPanel() const
{
    return m_behaveAsPlasmaPanel;
}

void View::setBehaveAsPlasmaPanel(bool behavior)
{
    if (m_behaveAsPlasmaPanel == behavior) {
        return;
    }

    m_behaveAsPlasmaPanel = behavior;

    emit behaveAsPlasmaPanelChanged();
}

bool View::inEditMode() const
{
    return containment() && containment()->isUserConfiguring();
}

bool View::isFloatingPanel() const
{
    return m_behaveAsPlasmaPanel && m_screenEdgeMarginEnabled && (m_screenEdgeMargin>0);
}

bool View::isPreferredForShortcuts() const
{
    return m_isPreferredForShortcuts;
}

void View::setIsPreferredForShortcuts(bool preferred)
{
    if (m_isPreferredForShortcuts == preferred) {
        return;
    }

    m_isPreferredForShortcuts = preferred;

    emit isPreferredForShortcutsChanged();

    if (m_isPreferredForShortcuts && m_layout) {
        emit m_layout->preferredViewForShortcutsChanged(this);
    }
}

bool View::inSettingsAdvancedMode() const
{
    return m_primaryConfigView && m_corona->universalSettings()->inAdvancedModeForEditSettings();
}

bool View::isTouchingBottomViewAndIsBusy() const
{
    return m_isTouchingBottomViewAndIsBusy;
}

void View::setIsTouchingBottomViewAndIsBusy(bool touchAndBusy)
{
    if (m_isTouchingBottomViewAndIsBusy == touchAndBusy) {
        return;
    }

    m_isTouchingBottomViewAndIsBusy = touchAndBusy;

    emit isTouchingBottomViewAndIsBusyChanged();
}

bool View::isTouchingTopViewAndIsBusy() const
{
    return m_isTouchingTopViewAndIsBusy;
}

void View::setIsTouchingTopViewAndIsBusy(bool touchAndBusy)
{
    if (m_isTouchingTopViewAndIsBusy == touchAndBusy) {
        return;
    }

    m_isTouchingTopViewAndIsBusy = touchAndBusy;
    emit isTouchingTopViewAndIsBusyChanged();
}

void View::preferredViewForShortcutsChangedSlot(Latte::View *view)
{
    if (view != this) {
        setIsPreferredForShortcuts(false);
    }
}

bool View::onPrimary() const
{
    return m_onPrimary;
}

void View::setOnPrimary(bool flag)
{
    if (m_onPrimary == flag) {
        return;
    }

    m_onPrimary = flag;
    emit onPrimaryChanged();
}

int View::groupId() const
{
    if (!containment()) {
        return -1;
    }

    return containment()->id();
}

float View::maxLength() const
{
    return m_maxLength;
}

void View::setMaxLength(float length)
{
    if (m_maxLength == length) {
        return;
    }

    m_maxLength = length;
    emit maxLengthChanged();
}

int View::editThickness() const
{
    int smallspacing = 4;
    int ruler_height{m_fontPixelSize};
    int header_height{m_fontPixelSize + 2*smallspacing};

    int edgeThickness = behaveAsPlasmaPanel() && screenEdgeMarginEnabled() ? m_screenEdgeMargin : 0;

    return edgeThickness + m_maxNormalThickness + ruler_height + header_height + 6*smallspacing;
}

int View::maxThickness() const
{
    return m_maxThickness;
}

void View::setMaxThickness(int thickness)
{
    if (m_maxThickness == thickness)
        return;

    m_maxThickness = thickness;
    emit maxThicknessChanged();
}

int View::alignment() const
{
    return m_alignment;
}

void View::setAlignment(int alignment)
{
    Types::Alignment align = static_cast<Types::Alignment>(alignment);

    if (m_alignment == alignment) {
        return;
    }

    m_alignment = align;
    emit alignmentChanged();
}

QRect View::absoluteGeometry() const
{
    return m_absoluteGeometry;
}

QRect View::screenGeometry() const
{
    if (this->screen()) {
        QRect geom = this->screen()->geometry();
        return geom;
    }

    return QRect();
}

float View::offset() const
{
    return m_offset;
}

void View::setOffset(float offset)
{
    if (m_offset == offset) {
        return;
    }

    m_offset = offset;
    emit offsetChanged();
}

bool View::screenEdgeMarginEnabled() const
{
    return m_screenEdgeMarginEnabled;
}

void View::setScreenEdgeMarginEnabled(bool enabled)
{
    if (m_screenEdgeMarginEnabled == enabled) {
        return;
    }

    m_screenEdgeMarginEnabled = enabled;
    emit screenEdgeMarginEnabledChanged();
}

int View::screenEdgeMargin() const
{
    return m_screenEdgeMargin;
}

void View::setScreenEdgeMargin(int margin)
{
    if (m_screenEdgeMargin == margin) {
        return;
    }



    m_screenEdgeMargin = margin;
    emit screenEdgeMarginChanged();
}

int View::fontPixelSize() const
{
    return m_fontPixelSize;
}

void View::setFontPixelSize(int size)
{
    if (m_fontPixelSize == size) {
        return;
    }

    m_fontPixelSize = size;

    emit fontPixelSizeChanged();
}

bool View::isOnAllActivities() const
{
    return m_activities.isEmpty() || m_activities[0] == Data::Layout::ALLACTIVITIESID;
}

bool View::isOnActivity(const QString &activity) const
{
    return isOnAllActivities() || m_activities.contains(activity);
}

QStringList View::activities() const
{
    QStringList running;

    QStringList runningAll = m_corona->activitiesConsumer()->runningActivities();

    for(int i=0; i<m_activities.count(); ++i) {
        if (runningAll.contains(m_activities[i])) {
            running << m_activities[i];
        }
    }

    return running;
}

void View::setActivities(const QStringList &ids)
{
    if (m_activities == ids) {
        return;
    }

    m_activities = ids;
    emit activitiesChanged();
}

void View::applyActivitiesToWindows()
{
    if (m_visibility && m_positioner && m_layout) {
        QStringList runningActivities = activities();

        m_positioner->setWindowOnActivities(m_positioner->trackedWindowId(), runningActivities);

        //! config windows
        if (m_primaryConfigView) {
            m_primaryConfigView->setOnActivities(runningActivities);
        }

        if (m_appletConfigView) {
            Latte::WindowSystem::WindowId appletconfigviewid;

            if (KWindowSystem::isPlatformX11()) {
                appletconfigviewid = m_appletConfigView->winId();
            } else {
                appletconfigviewid = m_corona->wm()->winIdFor("latte-dock", m_appletConfigView->title());
            }

            m_positioner->setWindowOnActivities(appletconfigviewid, runningActivities);
        }

        //! hidden windows
        if (m_visibility->supportsKWinEdges()) {
            m_visibility->applyActivitiesToHiddenWindows(runningActivities);
        }
    }
}

void View::showHiddenViewFromActivityStopping()
{
    if (m_layout && m_visibility && !inDelete() && !isVisible() && !m_visibility->isHidden()) {
        show();

        if (m_effects) {
            m_effects->updateEnabledBorders();
        }

        //qDebug() << "View:: Enforce reshow from timer 1...";
        emit forcedShown();
    } else if (m_layout && isVisible()) {
        m_inDelete = false;
        //qDebug() << "View:: No needed reshow from timer 1...";
    }
}

Layout::GenericLayout *View::layout() const
{
    return m_layout;
}

void View::setLayout(Layout::GenericLayout *layout)
{
    if (m_layout == layout) {
        return;
    }

    // clear mode
    for (auto &c : connectionsLayout) {
        disconnect(c);
    }

    m_layout = layout;

    if (m_layout) {
        connectionsLayout << connect(containment(), &Plasma::Applet::destroyedChanged, m_layout, &Layout::GenericLayout::destroyedChanged);
        connectionsLayout << connect(containment(), &Plasma::Applet::locationChanged, m_corona, &Latte::Corona::viewLocationChanged);
        connectionsLayout << connect(containment(), &Plasma::Containment::appletAlternativesRequested, m_corona, &Latte::Corona::showAlternativesForApplet, Qt::QueuedConnection);

        if (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
            connectionsLayout << connect(containment(), &Plasma::Containment::appletCreated, m_layout, &Layout::GenericLayout::appletCreated);
        }

        connectionsLayout << connect(m_positioner, &Latte::ViewPart::Positioner::edgeChanged, m_layout, &Layout::GenericLayout::viewEdgeChanged);
        connectionsLayout << connect(m_layout, &Layout::GenericLayout::popUpMarginChanged, m_effects, &Latte::ViewPart::Effects::popUpMarginChanged);

        //! Sometimes the activity isnt completely ready, by adding a delay
        //! we try to catch up
        m_initLayoutTimer.setInterval(100);
        m_initLayoutTimer.setSingleShot(true);
        connectionsLayout << connect(&m_initLayoutTimer, &QTimer::timeout, this, [&]() {
            if (m_layout && m_visibility) {
                setActivities(m_layout->appliedActivities());
                qDebug() << "DOCK VIEW FROM LAYOUT ::: " << m_layout->name() << " - activities: " << m_activities;
            }
        });
        m_initLayoutTimer.start();

        connectionsLayout << connect(m_layout, &Layout::GenericLayout::preferredViewForShortcutsChanged, this, &View::preferredViewForShortcutsChangedSlot);

        Latte::Corona *latteCorona = qobject_cast<Latte::Corona *>(this->corona());

        connectionsLayout << connect(latteCorona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, [&]() {
            if (m_layout && m_visibility) {
                setActivities(m_layout->appliedActivities());
                //! update activities in case KWin did its magic and assigned windows to faulty activities
                applyActivitiesToWindows();
                showHiddenViewFromActivityStopping();
                qDebug() << "DOCK VIEW FROM LAYOUT (currentActivityChanged) ::: " << m_layout->name() << " - activities: " << m_activities;
            }
        });

        if (latteCorona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
            connectionsLayout << connect(latteCorona->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged, this, [&]() {
                if (m_layout && m_visibility) {
                    setActivities(m_layout->appliedActivities());
                    qDebug() << "DOCK VIEW FROM LAYOUT (runningActivitiesChanged) ::: " << m_layout->name()
                             << " - activities: " << m_activities;
                }
            });

            connectionsLayout << connect(m_layout, &Layout::GenericLayout::activitiesChanged, this, [&]() {
                if (m_layout) {
                    setActivities(m_layout->appliedActivities());
                }
            });

            connectionsLayout << connect(latteCorona->layoutsManager()->synchronizer(), &Layouts::Synchronizer::layoutsChanged, this, [&]() {
                if (m_layout) {
                    setActivities(m_layout->appliedActivities());
                }
            });

            //! BEGIN OF KWIN HACK
            //! IMPORTANT ::: Fixing KWin Faulty Behavior that KWin hides ALL Views when an Activity stops
            //! with no reason!!

            m_visibleHackTimer1.setInterval(400);
            m_visibleHackTimer2.setInterval(2500);
            m_visibleHackTimer1.setSingleShot(true);
            m_visibleHackTimer2.setSingleShot(true);

            connectionsLayout << connect(this, &QWindow::visibleChanged, this, [&]() {
                if (m_layout && !inDelete() && !isVisible() && !m_positioner->inLayoutUnloading()) {
                    m_visibleHackTimer1.start();
                    m_visibleHackTimer2.start();
                }
            });

            connectionsLayout << connect(&m_visibleHackTimer1, &QTimer::timeout, this, [&]() {
                applyActivitiesToWindows();
                showHiddenViewFromActivityStopping();
                emit activitiesChanged();
            });

            connectionsLayout << connect(&m_visibleHackTimer2, &QTimer::timeout, this, [&]() {
                applyActivitiesToWindows();
                showHiddenViewFromActivityStopping();
                emit activitiesChanged();
            });

            //! END OF KWIN HACK
        }

        emit layoutChanged();
    } else {
        m_activities.clear();
    }
}

void View::hideWindowsForSlidingOut()
{
    if (m_primaryConfigView) {
        m_primaryConfigView->hideConfigWindow();
    }
}

//!check if the plasmoid with _name_ exists in the midedata
bool View::mimeContainsPlasmoid(QMimeData *mimeData, QString name)
{
    if (!mimeData) {
        return false;
    }

    if (mimeData->hasFormat(QStringLiteral("text/x-plasmoidservicename"))) {
        QString data = mimeData->data(QStringLiteral("text/x-plasmoidservicename"));
        const QStringList appletNames = data.split('\n', QString::SkipEmptyParts);

        for (const QString &appletName : appletNames) {
            if (appletName == name)
                return true;
        }
    }

    return false;
}

Latte::Data::View View::data() const
{
    Latte::Data::View vdata;
    vdata.id = QString::number(containment()->id());
    vdata.name = name();
    vdata.isActive = true;
    vdata.onPrimary = onPrimary();

    vdata.screen = containment()->screen();
    if (!Layouts::Storage::isValid(vdata.screen)) {
        vdata.screen = containment()->lastScreen();
    }

    vdata.screensGroup = screensGroup();

    //!screen edge margin can be more accurate in the config file
    vdata.screenEdgeMargin = m_screenEdgeMargin > 0 ? m_screenEdgeMargin : containment()->config().group("General").readEntry("screenEdgeMargin", (int)-1);

    vdata.edge = location();
    vdata.maxLength = m_maxLength * 100;
    vdata.alignment = m_alignment;
    vdata.subcontainments = Layouts::Storage::self()->subcontainments(layout(), containment());

    vdata.setState(Latte::Data::View::IsCreated);
    return vdata;
}

QQuickItem *View::colorizer() const
{
    return m_colorizer;
}

void View::setColorizer(QQuickItem *colorizer)
{
    if (m_colorizer == colorizer) {
        return;
    }

    m_colorizer = colorizer;
    emit colorizerChanged();
}

QQuickItem *View::metrics() const
{
    return m_metrics;
}

void View::setMetrics(QQuickItem *metrics)
{
    if (m_metrics == metrics) {
        return;
    }

    m_metrics = metrics;
    emit metricsChanged();
}

ViewPart::Effects *View::effects() const
{
    return m_effects;
}

ViewPart::Indicator *View::indicator() const
{
    return m_indicator;
}

ViewPart::ContainmentInterface *View::extendedInterface() const
{
    return m_interface;
}

ViewPart::Parabolic *View::parabolic() const
{
    return m_parabolic;
}

ViewPart::Positioner *View::positioner() const
{
    return m_positioner;
}

ViewPart::EventsSink *View::sink() const
{
    return m_sink;
}

ViewPart::VisibilityManager *View::visibility() const
{
    return m_visibility;
}

ViewPart::WindowsTracker *View::windowsTracker() const
{
    return m_windowsTracker;
}

Latte::Interfaces *View::interfacesGraphicObj() const
{
    return m_interfacesGraphicObj;
}

void View::setInterfacesGraphicObj(Latte::Interfaces *ifaces)
{
    if (m_interfacesGraphicObj == ifaces) {
        return;
    }

    m_interfacesGraphicObj = ifaces;

    if (containment()) {
        QQuickItem *containmentGraphicItem = qobject_cast<QQuickItem *>(containment()->property("_plasma_graphicObject").value<QObject *>());

        if (containmentGraphicItem) {
            containmentGraphicItem->setProperty("_latte_view_interfacesobject", QVariant::fromValue(m_interfacesGraphicObj));
        }
    }

    emit interfacesGraphicObjChanged();
}

bool View::event(QEvent *e)
{   
    QEvent *sunkevent = e;

    if (!m_inDelete) {
        emit eventTriggered(e);

        bool sinkableevent{false};

        switch (e->type()) {
        case QEvent::Enter:
            m_containsMouse = true;
            break;

        case QEvent::Leave:
            m_containsMouse = false;
            setContainsDrag(false);
            sinkableevent = true;
            break;

        case QEvent::DragEnter:
            setContainsDrag(true);
            sinkableevent = true;
            break;

        case QEvent::DragLeave:
            setContainsDrag(false);
            break;

        case QEvent::DragMove:
            sinkableevent = true;
            break;

        case QEvent::Drop:
            setContainsDrag(false);
            sinkableevent = true;
            break;

        case QEvent::MouseMove:
            sinkableevent = true;
            break;

        case QEvent::MouseButtonPress:
            if (auto me = dynamic_cast<QMouseEvent *>(e)) {
                emit mousePressed(me->pos(), me->button());
                sinkableevent = true;
                verticalUnityViewHasFocus();
            }
            break;

        case QEvent::MouseButtonRelease:
            if (auto me = dynamic_cast<QMouseEvent *>(e)) {
                emit mouseReleased(me->pos(), me->button());
                sinkableevent = true;
            }
            break;

        case QEvent::PlatformSurface:
            if (auto pe = dynamic_cast<QPlatformSurfaceEvent *>(e)) {
                switch (pe->surfaceEventType()) {
                case QPlatformSurfaceEvent::SurfaceCreated:
                    setupWaylandIntegration();

                    if (m_shellSurface) {
                        //! immediateSyncGeometry helps avoiding binding loops from containment qml side
                        m_positioner->immediateSyncGeometry();
                        m_effects->updateShadows();
                    }

                    break;

                case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
                    if (m_shellSurface) {
                        delete m_shellSurface;
                        m_shellSurface = nullptr;
                        qDebug() << "WAYLAND dock window surface was deleted...";
                        m_effects->clearShadows();
                    }

                    break;
                }
            }

            break;

        case QEvent::Show:
            if (m_visibility) {
                m_visibility->initViewFlags();
            }
            break;

        case QEvent::Wheel:
            if (auto we = dynamic_cast<QWheelEvent *>(e)) {
                QPoint pos = we->position().toPoint();
                emit wheelScrolled(pos, we->angleDelta(), we->buttons());
                sinkableevent = true;
            }
            break;
        default:
            break;
        }

        if (sinkableevent && m_sink->isActive()) {
            sunkevent = m_sink->onEvent(e);
        }
    }

    return ContainmentView::event(sunkevent);
}

void View::releaseConfigView()
{
    m_primaryConfigView = nullptr;
}

//! release grab and restore mouse state
void View::unblockMouse(int x, int y)
{
    setMouseGrabEnabled(false);

    m_releaseGrab_x = x;
    m_releaseGrab_y = y;
    m_releaseGrabTimer.start();
}

void View::releaseGrab()
{
    //! ungrab mouse
    if (mouseGrabberItem()) {
        mouseGrabberItem()->ungrabMouse();
    }

    //! properly release grabbed mouse in order to inform all views
    setMouseGrabEnabled(true);
    setMouseGrabEnabled(false);

    //! Send a fake QEvent::Leave to inform applets for mouse leaving the view
    QHoverEvent e(QEvent::Leave, QPoint(-5,-5),  QPoint(m_releaseGrab_x, m_releaseGrab_y));
    QCoreApplication::instance()->sendEvent(this, &e);
}

QAction *View::action(const QString &name)
{
    if (!containment()) {
        return nullptr;
    }

    return this->containment()->actions()->action(name);
}

QVariantList View::containmentActions() const
{
    QVariantList actions;

    if (!containment()) {
        return actions;
    }

    const QString trigger = "RightButton;NoModifier";
    Plasma::ContainmentActions *plugin = this->containment()->containmentActions().value(trigger);

    if (!plugin) {
        return actions;
    }

    if (plugin->containment() != this->containment()) {
        plugin->setContainment(this->containment());
        // now configure it
        KConfigGroup cfg(this->containment()->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(this->containment()->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    for (QAction *ac : plugin->contextualActions()) {
        actions << QVariant::fromValue<QAction *>(ac);
    }

    return actions;
}

bool View::isHighestPriorityView() {
    if (m_layout) {
        return this == m_layout->highestPriorityView();
    }

    return false;
}

//! BEGIN: WORKAROUND order to force top panels always on top and above left/right panels
void View::topViewAlwaysOnTop()
{
    if (!m_visibility) {
        return;
    }

    if (location() == Plasma::Types::TopEdge
            && m_visibility->mode() != Latte::Types::WindowsCanCover
            && m_visibility->mode() != Latte::Types::WindowsAlwaysCover) {
        //! this is needed in order to preserve that the top dock will be above others.
        //! Unity layout paradigm is a good example for this. The top panel shadow
        //! should be always on top compared to left panel
        m_visibility->setViewOnFrontLayer();
    }
}

void View::verticalUnityViewHasFocus()
{
    if (formFactor() == Plasma::Types::Vertical
            && (y() != screenGeometry().y())
            && ( (m_alignment == Latte::Types::Justify && m_maxLength == 1.0)
                 ||(m_alignment == Latte::Types::Top && m_offset == 0.0) )) {
        emit m_corona->verticalUnityViewHasFocus();
    }
}
//! END: WORKAROUND

//!BEGIN configuration functions
void View::saveConfig()
{
    if (!this->containment()) {
        return;
    }

    auto config = this->containment()->config();
    config.writeEntry("onPrimary", onPrimary());
    config.writeEntry("byPassWM", byPassWM());
    config.writeEntry("isPreferredForShortcuts", isPreferredForShortcuts());
    config.writeEntry("name", m_name);
    config.writeEntry("viewType", (int)m_type);
}

void View::restoreConfig()
{
    if (!this->containment()) {
        return;
    }

    auto config = this->containment()->config();
    m_onPrimary = config.readEntry("onPrimary", true);
    m_alignment = static_cast<Latte::Types::Alignment>(config.group("General").readEntry("alignment", (int)Latte::Types::Center));
    m_byPassWM = config.readEntry("byPassWM", false);
    m_isPreferredForShortcuts = config.readEntry("isPreferredForShortcuts", false);
    m_name = config.readEntry("name", QString());

    //! Send changed signals at the end in order to be sure that saveConfig
    //! wont rewrite default/invalid values
    emit alignmentChanged();
    emit nameChanged();
    emit onPrimaryChanged();
    emit byPassWMChanged();
}
//!END configuration functions

}
//!END namespace
