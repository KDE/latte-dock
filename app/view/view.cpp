/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
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

#include "view.h"

// local
#include "contextmenu.h"
#include "effects.h"
#include "positioner.h"
#include "visibilitymanager.h"
#include "settings/primaryconfigview.h"
#include "settings/secondaryconfigview.h"
#include "../apptypes.h"
#include "../lattecorona.h"
#include "../declarativeimports/interfaces.h"
#include "../indicator/factory.h"
#include "../layout/genericlayout.h"
#include "../layouts/manager.h"
#include "../plasma/extended/theme.h"
#include "../screenpool.h"
#include "../settings/universalsettings.h"
#include "../shortcuts/globalshortcuts.h"
#include "../shortcuts/shortcutstracker.h"

// Qt
#include <QAction>
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

//! both alwaysVisible and byPassWM are passed through corona because
//! during the view window creation containment hasn't been set, but these variables
//! are needed in order for window flags to be set correctly
View::View(Plasma::Corona *corona, QScreen *targetScreen, bool byPassWM)
    : PlasmaQuick::ContainmentView(corona),
      m_contextMenu(new ViewPart::ContextMenu(this)),
      m_effects(new ViewPart::Effects(this)),
      m_interface(new ViewPart::ContainmentInterface(this))
{      
    //! needs to be created after Effects because it catches some of its signals
    //! and avoid a crash from View::winId() at the same time
    m_positioner = new ViewPart::Positioner(this);

    // setTitle(corona->kPackage().metadata().name());
    setIcon(qGuiApp->windowIcon());
    setResizeMode(QuickViewSharedEngine::SizeRootObjectToView);
    setColor(QColor(Qt::transparent));
    setDefaultAlphaBuffer(true);
    setClearBeforeRendering(true);

    const auto flags = Qt::FramelessWindowHint
            | Qt::NoDropShadowWindowHint
            | Qt::WindowDoesNotAcceptFocus;

    if (byPassWM) {
        setFlags(flags | Qt::BypassWindowManagerHint);
    } else {
        setFlags(flags);
    }

    if (targetScreen)
        m_positioner->setScreenToFollow(targetScreen);
    else
        m_positioner->setScreenToFollow(qGuiApp->primaryScreen());

    m_releaseGrabTimer.setInterval(400);
    m_releaseGrabTimer.setSingleShot(true);
    connect(&m_releaseGrabTimer, &QTimer::timeout, this, &View::releaseGrab);

    connect(m_contextMenu, &ViewPart::ContextMenu::menuChanged, this, &View::updateTransientWindowsTracking);

    connect(this, &View::containmentChanged
            , this, [ &, byPassWM]() {
        qDebug() << "dock view c++ containment changed 1...";

        if (!this->containment())
            return;

        qDebug() << "dock view c++ containment changed 2...";

        setTitle(validTitle());

        //! First load default values from file
        restoreConfig();

        //! Afterwards override that values in case during creation something different is needed
        setByPassWM(byPassWM);

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

            connect(m_visibility, &ViewPart::VisibilityManager::frameExtentsCleared, this, [&]() {
                if (behaveAsPlasmaPanel()) {
                    //! recreate view because otherwise compositor frame extents implementation
                    //! is triggering a crazy behavior of moving/hiding the view and freezing Latte
                    //! in some cases.
                    reloadSource();
                }
            });

            emit visibilityChanged();
        }

        if (!m_indicator) {
            m_indicator = new ViewPart::Indicator(this);
            emit indicatorChanged();
        }

        connect(this->containment(), SIGNAL(statusChanged(Plasma::Types::ItemStatus)), SLOT(statusChanged(Plasma::Types::ItemStatus)));
    }, Qt::DirectConnection);

    m_corona = qobject_cast<Latte::Corona *>(this->corona());

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

    if (m_configView) {
        delete m_configView;
    }

    if (m_contextMenu) {
        delete m_contextMenu;
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
    connect(this, &QQuickWindow::xChanged, this, &View::xChanged);
    connect(this, &QQuickWindow::xChanged, this, &View::updateAbsoluteGeometry);
    connect(this, &QQuickWindow::yChanged, this, &View::yChanged);
    connect(this, &QQuickWindow::yChanged, this, &View::updateAbsoluteGeometry);
    connect(this, &QQuickWindow::widthChanged, this, &View::widthChanged);
    connect(this, &QQuickWindow::widthChanged, this, &View::updateAbsoluteGeometry);
    connect(this, &QQuickWindow::heightChanged, this, &View::heightChanged);
    connect(this, &QQuickWindow::heightChanged, this, &View::updateAbsoluteGeometry);

    connect(this, &View::activitiesChanged, this, &View::applyActivitiesToWindows);

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

    connect(m_contextMenu, &ViewPart::ContextMenu::menuChanged, this, &View::contextMenuIsShownChanged);

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
    connect(m_corona->universalSettings(), &Latte::UniversalSettings::hiddenConfigurationWindowsAreDeletedChanged,
            this, &View::hiddenConfigurationWindowsAreDeletedChanged);

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

    m_positioner->syncGeometry();

    qDebug() << "SOURCE:" << source();
}

void View::reloadSource()
{
    if (m_layout && containment()) {
        if (settingsWindowIsShown()) {
            m_configView->deleteLater();
        }

        engine()->clearComponentCache();
        m_layout->recreateView(containment(), settingsWindowIsShown());
    }
}

bool View::hiddenConfigurationWindowsAreDeleted() const
{
    return m_corona->universalSettings()->hiddenConfigurationWindowsAreDeleted();
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
    if (m_inDelete || origin == this)
        return;

    if (formFactor() == Plasma::Types::Vertical) {
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

void View::copyView()
{
    m_layout->copyView(containment());
}

void View::removeView()
{
    if (m_layout && m_layout->viewsCount() > 1) {
        m_inDelete = true;

        QAction *removeAct = this->containment()->actions()->action(QStringLiteral("remove"));

        if (removeAct) {
            removeAct->trigger();
        }
    }
}

bool View::settingsWindowIsShown()
{
    auto cview = qobject_cast<ViewPart::PrimaryConfigView *>(m_configView);

    if (hiddenConfigurationWindowsAreDeleted()) {
        return (cview != nullptr);
    } else if (cview) {
        return cview->isVisible();
    }

    return false;
}

void View::showSettingsWindow()
{
    if (!settingsWindowIsShown()) {
        emit m_visibility->mustBeShown();
        showConfigurationInterface(containment());
        applyActivitiesToWindows();
    }
}

PlasmaQuick::ConfigView *View::configView()
{
    return m_configView;
}

void View::showConfigurationInterface(Plasma::Applet *applet)
{
    if (!applet || !applet->containment())
        return;

    Plasma::Containment *c = qobject_cast<Plasma::Containment *>(applet);

    if (m_configView && c && c->isContainment() && c == this->containment()) {
        if (m_configView->isVisible()) {
            m_configView->hide();
        } else {
            m_configView->show();
        }

        return;
    } else if (m_configView) {
        if (m_configView->applet() == applet) {
            m_configView->show();

            if (KWindowSystem::isPlatformX11()) {
                m_configView->requestActivate();
            }
            return;
        } else {
            m_configView->hide();
        }
    }

    bool delayConfigView = false;

    if (c && containment() && c->isContainment() && c->id() == this->containment()->id()) {
        m_configView = new ViewPart::PrimaryConfigView(c, this);
        delayConfigView = true;
    } else {
        m_configView = new PlasmaQuick::ConfigView(applet);
    }

    m_configView.data()->init();

    if (!delayConfigView) {
        m_configView->show();
    } else {
        //add a timer for showing the configuration window the first time it is
        //created in order to give the containment's layouts the time to
        //calculate the window's height
        QTimer::singleShot(150, [this]() {
            if (m_configView) {
                m_configView->show();
            }
        });
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

    if (m_absoluteGeometry == absGeometry && !bypassChecks) {
        return;
    }

    if (m_absoluteGeometry != absGeometry) {
        m_absoluteGeometry = absGeometry;
        emit absoluteGeometryChanged(m_absoluteGeometry);
    }

    //! this is needed in order to update correctly the screenGeometries
    if (visibility() && corona() && visibility()->mode() == Types::AlwaysVisible) {
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

    if (status == Plasma::Types::NeedsAttentionStatus) {
        m_visibility->addBlockHidingEvent(BLOCKHIDINGNEEDSATTENTIONTYPE);
        m_visibility->initViewFlags();
    } else if (status == Plasma::Types::AcceptingInputStatus) {
        m_visibility->removeBlockHidingEvent(BLOCKHIDINGNEEDSATTENTIONTYPE);
        setFlags(flags() & ~Qt::WindowDoesNotAcceptFocus);
        KWindowSystem::forceActiveWindow(winId());
    } else {
        updateTransientWindowsTracking();
        m_visibility->removeBlockHidingEvent(BLOCKHIDINGNEEDSATTENTIONTYPE);
        m_visibility->initViewFlags();
    }
}

void View::addTransientWindow(QWindow *window)
{
    if (!m_transientWindows.contains(window) && !window->title().startsWith("#debugwindow#")) {
        m_transientWindows.append(window);

        QString winPtrStr = "0x" + QString::number((qulonglong)window,16);
        m_visibility->addBlockHidingEvent(winPtrStr);
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

        updateTransientWindowsTracking();
    }
}

void View::updateTransientWindowsTracking()
{
    for(QWindow *window: qApp->topLevelWindows()) {
        if (window->transientParent() == this){
            if (window->isVisible()) {
                addTransientWindow(window);
                break;
            }
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

bool View::contextMenuIsShown() const
{
    if (!m_contextMenu) {
        return false;
    }

    return m_contextMenu->menu();
}

int View::currentThickness() const
{
    if (formFactor() == Plasma::Types::Vertical) {
        return m_effects->mask().isNull() ? width() : m_effects->mask().width() - m_effects->innerShadow();
    } else {
        return m_effects->mask().isNull() ? height() : m_effects->mask().height() - m_effects->innerShadow();
    }
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
    return m_inEditMode;
}

void View::setInEditMode(bool edit)
{
    if (m_inEditMode == edit) {
        return;
    }

    m_inEditMode = edit;

    emit inEditModeChanged();
}

bool View::isFloatingWindow() const
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
    if (m_configView) {
        auto configView = qobject_cast<ViewPart::PrimaryConfigView *>(m_configView);

        if (configView) {
            return configView->inAdvancedMode();
        }
    }

    return false;
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
    return m_editThickness;
}

void View::setEditThickness(int thickness)
{
    if (m_editThickness == thickness) {
        return;
    }

    m_editThickness = thickness;

    emit editThicknessChanged();
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
    return m_activities.isEmpty() || m_activities[0] == "0";
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
    if (m_visibility && m_layout) {
        QStringList runningActivities = activities();

        m_windowsTracker->setWindowOnActivities(*this, runningActivities);

        //! config windows
        if (m_configView) {
            m_windowsTracker->setWindowOnActivities(*m_configView, runningActivities);

            auto configView = qobject_cast<ViewPart::PrimaryConfigView *>(m_configView);

            if (configView && configView->secondaryWindow()) {
                m_windowsTracker->setWindowOnActivities(*configView->secondaryWindow(), runningActivities);
            }
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
        connectionsLayout << connect(m_layout, &Layout::GenericLayout::lastConfigViewForChanged, this, &View::configViewShownFor);

        Latte::Corona *latteCorona = qobject_cast<Latte::Corona *>(this->corona());

        connectionsLayout << connect(latteCorona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, [&]() {
            if (m_layout && m_visibility) {
                setActivities(m_layout->appliedActivities());
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

            connectionsLayout << connect(latteCorona->layoutsManager(), &Layouts::Manager::layoutsChanged, this, [&]() {
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
                if (m_layout && !inDelete() & !isVisible()) {
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

void View::moveToLayout(QString layoutName)
{
    if (!m_layout) {
        return;
    }

    QList<Plasma::Containment *> containments = m_layout->unassignFromLayout(this);

    Latte::Corona *latteCorona = qobject_cast<Latte::Corona *>(this->corona());

    if (latteCorona && containments.size() > 0) {
        Layout::GenericLayout *newLayout = latteCorona->layoutsManager()->synchronizer()->layout(layoutName);

        if (newLayout) {
            newLayout->assignToLayout(this, containments);
        }
    }
}

void View::configViewShownFor(Latte::View *view)
{
    if (view!=this && m_configView) {
        //! for each layout only one dock should show its configuration windows
        //! otherwise we could reach a point that because a settings window
        //! is below another Latte View its options are not reachable
        auto configDialog = qobject_cast<ViewPart::PrimaryConfigView *>(m_configView);

        if (configDialog) {
            if (hiddenConfigurationWindowsAreDeleted()) {
                configDialog->deleteLater();
            } else if (configDialog->isVisible()) {
                configDialog->hideConfigWindow();
            }
        }
    }
}

void View::hideWindowsForSlidingOut()
{
    if (m_configView) {
        auto configDialog = qobject_cast<ViewPart::PrimaryConfigView *>(m_configView);

        if (configDialog) {
            configDialog->hideConfigWindow();
        }
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

ViewPart::Positioner *View::positioner() const
{
    return m_positioner;
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
    if (!m_inDelete) {
        emit eventTriggered(e);

        switch (e->type()) {
        case QEvent::Enter:
            m_containsMouse = true;

            if (m_configView) {
                ViewPart::PrimaryConfigView *primaryConfigView = qobject_cast<ViewPart::PrimaryConfigView *>(m_configView);

                if (primaryConfigView) {
                    if (primaryConfigView->secondaryWindow()) {
                        ViewPart::SecondaryConfigView *secConfigView = qobject_cast<ViewPart::SecondaryConfigView *>(primaryConfigView->secondaryWindow());
                        if (secConfigView) {
                            secConfigView->requestActivate();
                        }
                    }

                    primaryConfigView->requestActivate();
                }
            }
            break;

        case QEvent::Leave:
            m_containsMouse = false;
            setContainsDrag(false);
            engine()->trimComponentCache();
            break;

        case QEvent::DragEnter:
            setContainsDrag(true);
            break;

        case QEvent::DragLeave:
        case QEvent::Drop:
            setContainsDrag(false);
            break;

        case QEvent::MouseButtonPress:
            if (auto mouseEvent = dynamic_cast<QMouseEvent *>(e)) {
                emit mousePressed(mouseEvent->pos(), mouseEvent->button());
            }
            break;
        case QEvent::MouseButtonRelease:
            if (auto mouseEvent = dynamic_cast<QMouseEvent *>(e)) {
                emit mouseReleased(mouseEvent->pos(), mouseEvent->button());
            }
            break;
            /* case QEvent::DragMove:
            qDebug() << "DRAG MOVING>>>>>>";
            break;*/
        case QEvent::PlatformSurface:
            if (auto pe = dynamic_cast<QPlatformSurfaceEvent *>(e)) {
                switch (pe->surfaceEventType()) {
                case QPlatformSurfaceEvent::SurfaceCreated:
                    setupWaylandIntegration();

                    if (m_shellSurface) {
                        m_positioner->syncGeometry();
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
            if (auto wheelEvent = dynamic_cast<QWheelEvent *>(e)) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
                QPoint position = QPoint(wheelEvent->x(), wheelEvent->y());
#else
                QPoint position = wheelEvent->position().toPoint();
#endif
                emit wheelScrolled(position, wheelEvent->angleDelta(), wheelEvent->buttons());
            }
            break;
        default:
            break;
        }
    }

    return ContainmentView::event(e);
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

QVariantList View::containmentActions()
{
    QVariantList actions;
    /*if (containment()->corona()->immutability() != Plasma::Types::Mutable) {
        return actions;
    }*/
    //FIXME: the trigger string it should be better to be supported this way
    //const QString trigger = Plasma::ContainmentActions::eventToString(event);
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

//!BEGIN overriding context menus behavior
void View::mousePressEvent(QMouseEvent *event)
{
    bool result = m_contextMenu->mousePressEvent(event);

    if (result) {
        PlasmaQuick::ContainmentView::mousePressEvent(event);
        updateTransientWindowsTracking();
    }

    verticalUnityViewHasFocus();
}
//!END overriding context menus behavior

//!BEGIN configuration functions
void View::saveConfig()
{
    if (!this->containment())
        return;

    auto config = this->containment()->config();
    config.writeEntry("onPrimary", onPrimary());
    config.writeEntry("byPassWM", byPassWM());
    config.writeEntry("isPreferredForShortcuts", isPreferredForShortcuts());
    config.writeEntry("viewType", (int)m_type);
}

void View::restoreConfig()
{
    if (!this->containment())
        return;

    auto config = this->containment()->config();
    m_onPrimary = config.readEntry("onPrimary", true);
    m_byPassWM = config.readEntry("byPassWM", false);
    m_isPreferredForShortcuts = config.readEntry("isPreferredForShortcuts", false);

    //! Send changed signals at the end in order to be sure that saveConfig
    //! wont rewrite default/invalid values
    emit onPrimaryChanged();
    emit byPassWMChanged();
}
//!END configuration functions

}
//!END namespace
