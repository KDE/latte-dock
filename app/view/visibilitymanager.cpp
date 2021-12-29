/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "visibilitymanager.h"

// local
#include "positioner.h"
#include "view.h"
#include "helpers/floatinggapwindow.h"
#include "helpers/screenedgeghostwindow.h"
#include "windowstracker/currentscreentracker.h"
#include "../apptypes.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/manager.h"
#include "../wm/abstractwindowinterface.h"

// Qt
#include <QDebug>

// KDE
#include <KWindowSystem>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

//! Hide Timer can create cases that when it is low it does not allow the
//! view to be show. For example !compositing+kwin_edges+hide inteval<50ms
//!   FIXED: As it appears because we dont hide any view anymore before its sliding in
//!   animation has ended that probably allows to set the hide minimum interval to zero
//!   without any further issues, such as to not show the view even though the
//!   user is touching the screen edge
const int HIDEMINIMUMINTERVAL = 0;
//! After calling SidebarAutoHide panel to show for example through Sidebar button
//! or global shortcuts we make sure bar will be shown enough time
//! in order for the user to observe its contents
const int SIDEBARAUTOHIDEMINIMUMSHOW = 1000;


namespace Latte {
namespace ViewPart {

//! BEGIN: VisiblityManager implementation
const QRect VisibilityManager::ISHIDDENMASK = QRect(-1, -1, 1, 1);

VisibilityManager::VisibilityManager(PlasmaQuick::ContainmentView *view)
    : QObject(view)
{
    qDebug() << "VisibilityManager creating...";

    m_latteView = qobject_cast<Latte::View *>(view);
    m_corona = qobject_cast<Latte::Corona *>(view->corona());
    m_wm = m_corona->wm();

    connect(this, &VisibilityManager::hidingIsBlockedChanged, this, &VisibilityManager::onHidingIsBlockedChanged);

    connect(this, &VisibilityManager::slideOutFinished, this, &VisibilityManager::updateHiddenState);
    connect(this, &VisibilityManager::slideInFinished, this, &VisibilityManager::updateHiddenState);

    connect(this, &VisibilityManager::enableKWinEdgesChanged, this, &VisibilityManager::updateKWinEdgesSupport);
    connect(this, &VisibilityManager::modeChanged, this, &VisibilityManager::updateKWinEdgesSupport);
    connect(this, &VisibilityManager::modeChanged, this, &VisibilityManager::updateSidebarState);

    connect(this, &VisibilityManager::isFloatingGapWindowEnabledChanged, this, &VisibilityManager::onIsFloatingGapWindowEnabledChanged);

    connect(this, &VisibilityManager::mustBeShown, this, [&]() {
        if (m_latteView && !m_latteView->isVisible()) {
            m_latteView->setVisible(true);
        }
    });

    if (m_latteView) {
        connect(m_latteView, &Latte::View::eventTriggered, this, &VisibilityManager::viewEventManager);
        connect(m_latteView, &Latte::View::behaveAsPlasmaPanelChanged , this, &VisibilityManager::updateKWinEdgesSupport);
        connect(m_latteView, &Latte::View::byPassWMChanged, this, &VisibilityManager::updateKWinEdgesSupport);

        connect(m_latteView, &Latte::View::inEditModeChanged, this, &VisibilityManager::initViewFlags);

        //! Frame Extents
        connect(m_latteView, &Latte::View::headThicknessGapChanged, this, &VisibilityManager::onHeadThicknessChanged);
        connect(m_latteView, &Latte::View::locationChanged, this, [&]() {
            if (!m_latteView->behaveAsPlasmaPanel()) {
                //! Resend frame extents because their geometry has changed
                const bool forceUpdate{true};
                publishFrameExtents(forceUpdate);
            }
        });

        connect(m_latteView, &Latte::View::typeChanged, this, [&]() {
            if (m_latteView->inEditMode()) {
                //! Resend frame extents because type has changed
                const bool forceUpdate{true};
                publishFrameExtents(forceUpdate);
            }
        });

        connect(m_latteView, &Latte::View::forcedShown, this, [&]() {
            //! Resend frame extents to compositor otherwise because compositor cleared
            //! them with no reason when the user is closing an activity
            const bool forceUpdate{true};
            publishFrameExtents(forceUpdate);
        });

        connect(this, &VisibilityManager::modeChanged, this, [&]() {
            emit m_latteView->availableScreenRectChangedFrom(m_latteView);
        });

        //! Send frame extents on startup, this is really necessary when recreating a view.
        //! Such a case is when toggling byPassWM and a view is recreated after disabling editing mode
        const bool forceUpdate{true};
        publishFrameExtents(forceUpdate);
    }

    m_timerShow.setSingleShot(true);
    m_timerHide.setSingleShot(true);

    connect(&m_timerShow, &QTimer::timeout, this, [&]() {
        if (m_isHidden ||  m_isBelowLayer) {
            //   qDebug() << "must be shown";
            emit mustBeShown();
        }
    });
    connect(&m_timerHide, &QTimer::timeout, this, [&]() {
        if (!hidingIsBlocked() && !m_isHidden && !m_isBelowLayer && !m_dragEnter) {
            if (m_isFloatingGapWindowEnabled) {
                //! first check if mouse is inside the floating gap
                checkMouseInFloatingArea();
            } else {
                //! immediate call
                emit mustBeHide();
            }
        }
    });

    m_timerPublishFrameExtents.setInterval(1500);
    m_timerPublishFrameExtents.setSingleShot(true);
    connect(&m_timerPublishFrameExtents, &QTimer::timeout, this, [&]() { publishFrameExtents(); });

    m_timerBlockStrutsUpdate.setInterval(1000);
    m_timerBlockStrutsUpdate.setSingleShot(true);
    connect(&m_timerBlockStrutsUpdate, &QTimer::timeout, this, [&]() { updateStrutsBasedOnLayoutsAndActivities(); });

    restoreConfig();

    //! connect save values after they have been restored
    connect(this, &VisibilityManager::enableKWinEdgesChanged, this, &VisibilityManager::saveConfig);
    connect(this, &VisibilityManager::modeChanged, this, &VisibilityManager::saveConfig);
    connect(this, &VisibilityManager::raiseOnDesktopChanged, this, &VisibilityManager::saveConfig);
    connect(this, &VisibilityManager::raiseOnActivityChanged, this, &VisibilityManager::saveConfig);
    connect(this, &VisibilityManager::timerShowChanged, this, &VisibilityManager::saveConfig);
    connect(this, &VisibilityManager::timerHideChanged, this, &VisibilityManager::saveConfig);
}

VisibilityManager::~VisibilityManager()
{
    qDebug() << "VisibilityManager deleting...";
    m_wm->removeViewStruts(*m_latteView);

    if (m_edgeGhostWindow) {
        m_edgeGhostWindow->deleteLater();
    }

    if (m_floatingGapWindow) {
        m_floatingGapWindow->deleteLater();
    }
}

//! Struts
int VisibilityManager::strutsThickness() const
{
    return m_strutsThickness;
}

void VisibilityManager::setStrutsThickness(int thickness)
{
    if (m_strutsThickness == thickness) {
        return;
    }

    m_strutsThickness = thickness;
    emit strutsThicknessChanged();
}

Types::Visibility VisibilityManager::mode() const
{
    return m_mode;
}

void VisibilityManager::initViewFlags()
{
    if ((m_mode == Types::WindowsCanCover || m_mode == Types::WindowsAlwaysCover) && (!m_latteView->inEditMode())) {
        setViewOnBackLayer();
    } else {
        setViewOnFrontLayer();
    }
}

void VisibilityManager::setViewOnBackLayer()
{
    m_wm->setViewExtraFlags(m_latteView, false, Types::WindowsAlwaysCover);
    setIsBelowLayer(true);
}

void VisibilityManager::setViewOnFrontLayer()
{
    m_wm->setViewExtraFlags(m_latteView, true);
    setIsBelowLayer(false);
    if (KWindowSystem::isPlatformX11()) {
        m_latteView->raise();
    }
}

void VisibilityManager::setMode(Latte::Types::Visibility mode)
{
    if (m_mode == mode) {
        return;
    }

    qDebug() << "Updating visibility mode to  :::: " << mode;

    Q_ASSERT_X(mode != Types::None, staticMetaObject.className(), "set visibility to Types::None");

    // clear mode
    for (auto &c : m_connections) {
        disconnect(c);
    }

    int base{0};

    m_publishedStruts = QRect();

    if (m_mode == Types::AlwaysVisible) {
        //! remove struts for old always visible mode
        m_wm->removeViewStruts(*m_latteView);
    }

    m_timerShow.stop();
    m_timerHide.stop();
    m_mode = mode;

    initViewFlags();

    if (mode != Types::AlwaysVisible && mode != Types::WindowsGoBelow) {
        m_connections[0] = connect(m_wm, &WindowSystem::AbstractWindowInterface::currentDesktopChanged, this, [&] {
            if (m_raiseOnDesktopChange) {
                raiseViewTemporarily();
            }
        });
        m_connections[1] = connect(m_wm, &WindowSystem::AbstractWindowInterface::currentActivityChanged, this, [&]() {
            if (m_raiseOnActivityChange) {
                raiseViewTemporarily();
            } else {
                updateHiddenState();
            }
        });

        base = 2;
    }

    switch (m_mode) {
    case Types::AlwaysVisible: {
        if (m_latteView->containment() && m_latteView->screen()) {
            updateStrutsBasedOnLayoutsAndActivities();
        }

        m_connections[base] = connect(this, &VisibilityManager::strutsThicknessChanged, &VisibilityManager::updateStrutsAfterTimer);

        // disabling this call because it was creating too many struts calls and   ???
        // could create reduced responsiveness for DynamicStruts Scenario(for example ??
        // when dragging active window from a floating dock/panel) ???
        m_connections[base+1] = connect(m_latteView, &Latte::View::absoluteGeometryChanged, this, &VisibilityManager::updateStrutsAfterTimer);

        m_connections[base+2] = connect(m_corona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, [&]() {
            if (m_corona && m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
                updateStrutsBasedOnLayoutsAndActivities(true);
            }
        });

        //! respect canSetStrut that must be disabled under x11 when an alwaysvisible screen edge is common between two or more screens
        m_connections[base+3] = connect(m_corona->screenPool(), &Latte::ScreenPool::screenGeometryChanged, this, &VisibilityManager::updateStrutsAfterTimer);

        m_connections[base+4] = connect(m_latteView, &Latte::View::activitiesChanged, this, [&]() {
            updateStrutsBasedOnLayoutsAndActivities(true);
        });

        raiseView(true);
        break;
    }

    case Types::AutoHide: {
        m_connections[base] = connect(this, &VisibilityManager::containsMouseChanged, this, [&]() {
            raiseView(m_containsMouse);
        });

        raiseView(m_containsMouse);
        break;
    }

    case Types::DodgeActive: {
        m_connections[base] = connect(this, &VisibilityManager::containsMouseChanged
                                      , this, &VisibilityManager::dodgeActive);
        m_connections[base+1] = connect(m_latteView->windowsTracker()->currentScreen(), &TrackerPart::CurrentScreenTracker::activeWindowTouchingChanged
                                        , this, &VisibilityManager::dodgeActive);

        dodgeActive();
        break;
    }

    case Types::DodgeMaximized: {
        m_connections[base] = connect(this, &VisibilityManager::containsMouseChanged
                                      , this, &VisibilityManager::dodgeMaximized);
        m_connections[base+1] = connect(m_latteView->windowsTracker()->currentScreen(), &TrackerPart::CurrentScreenTracker::activeWindowMaximizedChanged
                                        , this, &VisibilityManager::dodgeMaximized);

        dodgeMaximized();
        break;
    }

    case Types::DodgeAllWindows: {
        m_connections[base] = connect(this, &VisibilityManager::containsMouseChanged
                                      , this, &VisibilityManager::dodgeAllWindows);

        m_connections[base+1] = connect(m_latteView->windowsTracker()->currentScreen(), &TrackerPart::CurrentScreenTracker::existsWindowTouchingChanged
                                        , this, &VisibilityManager::dodgeAllWindows);

        dodgeAllWindows();
        break;
    }

    case Types::WindowsGoBelow:
        break;

    case Types::WindowsCanCover:
        m_connections[base] = connect(this, &VisibilityManager::containsMouseChanged, this, [&]() {
            raiseView(m_containsMouse);
        });

        raiseView(m_containsMouse);
        break;

    case Types::WindowsAlwaysCover:
        break;

    case Types::SidebarOnDemand:
        m_connections[base] = connect(m_latteView, &Latte::View::inEditModeChanged, this, [&]() {
            if (!m_latteView->inEditMode()) {
                m_isRequestedShownSidebarOnDemand = false;
                updateHiddenState();
            }
        });

        m_isRequestedShownSidebarOnDemand = false;
        updateHiddenState();
        break;

    case Types::SidebarAutoHide:
        m_connections[base] = connect(this, &VisibilityManager::containsMouseChanged, this, [&]() {
            if (!m_latteView->inEditMode()) {
                updateHiddenState();
            }
        });
        
        m_connections[base+1] = connect(m_latteView, &Latte::View::inEditModeChanged, this, [&]() {
            if (m_latteView->inEditMode() && !m_isHidden) {
                updateHiddenState();
            }
        });

        toggleHiddenState();
        break;

    default:
        break;
    }

    emit modeChanged();
}

void VisibilityManager::updateStrutsAfterTimer()
{
    bool execute = !m_timerBlockStrutsUpdate.isActive();

    m_timerBlockStrutsUpdate.start();

    if (execute) {
        updateStrutsBasedOnLayoutsAndActivities();
    }
}

void VisibilityManager::updateSidebarState()
{
    bool cursidebarstate = ((m_mode == Types::SidebarOnDemand)
                            || (m_mode == Types::SidebarAutoHide));

    if (m_isSidebar == cursidebarstate) {
        return;
    }

    m_isSidebar == cursidebarstate;
    emit isSidebarChanged();

}

void VisibilityManager::updateStrutsBasedOnLayoutsAndActivities(bool forceUpdate)
{
    bool inMultipleLayoutsAndCurrent = (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts
                                        && m_latteView->layout() && !m_latteView->positioner()->inRelocationAnimation()
                                        && m_latteView->layout()->isCurrent());

    if (m_strutsThickness>0 && canSetStrut() && (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout || inMultipleLayoutsAndCurrent)) {
        QRect computedStruts = acceptableStruts();
        if (m_publishedStruts != computedStruts || forceUpdate) {
            //! Force update is needed when very important events happen in DE and there is a chance
            //! that previously even though struts where sent the DE did not accept them.
            //! Such a case is when STOPPING an Activity and windows faulty become invisible even
            //! though they should not. In such case setting struts when the windows are hidden
            //! the struts do not take any effect
            m_publishedStruts = computedStruts;
            m_wm->setViewStruts(*m_latteView, m_publishedStruts, m_latteView->location());
        }
    } else {
        m_publishedStruts = QRect();
        m_wm->removeViewStruts(*m_latteView);
    }
}

bool VisibilityManager::canSetStrut() const
{
    if (m_latteView->positioner()->isOffScreen()) {
        return false;
    }

    if (!KWindowSystem::isPlatformX11() || m_wm->isKWinRunning()) {
        // we always trust wayland and kwin to provide proper struts
        return true;
    }

    if (qGuiApp->screens().count() < 2) {
        return true;
    }

    /*Alternative DEs*/

    const QRect thisScreen = m_latteView->screen()->geometry();

    // Extended struts against a screen edge near to another screen are really harmful, so windows maximized under the panel is a lesser pain
    // TODO: force "windows can cover" in those cases?
    for (QScreen *screen : qGuiApp->screens()) {
        if (!screen || m_latteView->screen() == screen) {
            continue;
        }

        const QRect otherScreen = screen->geometry();

        switch (m_latteView->location()) {
        case Plasma::Types::TopEdge:
            if (otherScreen.bottom() <= thisScreen.top()) {
                return false;
            }
            break;
        case Plasma::Types::BottomEdge:
            if (otherScreen.top() >= thisScreen.bottom()) {
                return false;
            }
            break;
        case Plasma::Types::RightEdge:
            if (otherScreen.left() >= thisScreen.right()) {
                return false;
            }
            break;
        case Plasma::Types::LeftEdge:
            if (otherScreen.right() <= thisScreen.left()) {
                return false;
            }
            break;
        default:
            return false;
        }
    }

    return true;
}

QRect VisibilityManager::acceptableStruts()
{
    QRect calcs;

    switch (m_latteView->location()) {
    case Plasma::Types::TopEdge: {
        calcs = QRect(m_latteView->x(), m_latteView->screenGeometry().top(), m_latteView->width(), m_strutsThickness);
        break;
    }

    case Plasma::Types::BottomEdge: {
        int y = m_latteView->screenGeometry().bottom() - m_strutsThickness + 1 /* +1, is needed in order to not leave a gap at screen_edge*/;
        calcs = QRect(m_latteView->x(), y, m_latteView->width(), m_strutsThickness);
        break;
    }

    case Plasma::Types::LeftEdge: {
        calcs = QRect(m_latteView->screenGeometry().left(), m_latteView->y(), m_strutsThickness, m_latteView->height());
        break;
    }

    case Plasma::Types::RightEdge: {
        int x = m_latteView->screenGeometry().right() - m_strutsThickness + 1 /* +1, is needed in order to not leave a gap at screen_edge*/;
        calcs = QRect(x, m_latteView->y(), m_strutsThickness, m_latteView->height());
        break;
    }
    }

    return calcs;
}

bool VisibilityManager::raiseOnDesktop() const
{
    return m_raiseOnDesktopChange;
}

void VisibilityManager::setRaiseOnDesktop(bool enable)
{
    if (enable == m_raiseOnDesktopChange)
        return;

    m_raiseOnDesktopChange = enable;
    emit raiseOnDesktopChanged();
}

bool VisibilityManager::raiseOnActivity() const
{
    return m_raiseOnActivityChange;
}

void VisibilityManager::setRaiseOnActivity(bool enable)
{
    if (enable == m_raiseOnActivityChange)
        return;

    m_raiseOnActivityChange = enable;
    emit raiseOnActivityChanged();
}

bool VisibilityManager::isBelowLayer() const
{
    return m_isBelowLayer;
}

void VisibilityManager::setIsBelowLayer(bool below)
{
    if (m_isBelowLayer == below) {
        return;
    }

    m_isBelowLayer = below;

    updateGhostWindowState();

    emit isBelowLayerChanged();
}

bool VisibilityManager::isHidden() const
{
    return m_isHidden;
}

void VisibilityManager::setIsHidden(bool isHidden)
{
    if (m_isHidden == isHidden)
        return;

    m_isHidden = isHidden;
    updateGhostWindowState();

    emit isHiddenChanged();
}

bool VisibilityManager::isShownFully() const
{
    return m_isShownFully;
}

void VisibilityManager::setIsShownFully(bool fully)
{
    if (m_isShownFully == fully) {
        return;
    }

    m_isShownFully = fully;
    emit isShownFullyChanged();
}

bool VisibilityManager::hidingIsBlocked() const
{
    return (m_blockHidingEvents.count() > 0);
}

bool VisibilityManager::isFloatingGapWindowEnabled() const
{
    return m_isFloatingGapWindowEnabled;
}

void VisibilityManager::setIsFloatingGapWindowEnabled(bool enabled)
{
    if (m_isFloatingGapWindowEnabled == enabled) {
        return;
    }

    m_isFloatingGapWindowEnabled = enabled;
    emit isFloatingGapWindowEnabledChanged();
}

bool VisibilityManager::hasBlockHidingEvent(const QString &type)
{
    return (!type.isEmpty() && m_blockHidingEvents.contains(type));
}

void VisibilityManager::addBlockHidingEvent(const QString &type)
{
    if (m_blockHidingEvents.contains(type) || type.isEmpty()) {
        return;
    }
    //qDebug() << " org.kde.late {{ ++++ adding block hiding event :: " << type;

    bool prevHidingIsBlocked = hidingIsBlocked();

    m_blockHidingEvents << type;

    if (prevHidingIsBlocked != hidingIsBlocked()) {
        emit hidingIsBlockedChanged();
    }
}

void VisibilityManager::removeBlockHidingEvent(const QString &type)
{
    if (!m_blockHidingEvents.contains(type) || type.isEmpty()) {
        return;
    }
    //qDebug() << " org.kde.latte {{ ---- remove block hiding event :: " << type;

    bool prevHidingIsBlocked = hidingIsBlocked();

    m_blockHidingEvents.removeAll(type);

    if (prevHidingIsBlocked != hidingIsBlocked()) {
        emit hidingIsBlockedChanged();
    }
}

void VisibilityManager::onHidingIsBlockedChanged()
{
    if (hidingIsBlocked()) {
        m_timerHide.stop();
        emit mustBeShown();
    } else {
        updateHiddenState();
    }
}

void VisibilityManager::onHeadThicknessChanged()
{
    if (!m_timerPublishFrameExtents.isActive()) {
        m_timerPublishFrameExtents.start();
    }
}

void VisibilityManager::publishFrameExtents(bool forceUpdate)
{   
    if (m_frameExtentsHeadThicknessGap != m_latteView->headThicknessGap()
            || m_frameExtentsLocation != m_latteView->location()
            || forceUpdate) {

        m_frameExtentsLocation = m_latteView->location();
        m_frameExtentsHeadThicknessGap = m_latteView->headThicknessGap();

        if (KWindowSystem::isPlatformX11() && m_latteView->devicePixelRatio()!=1.0) {
            //!Fix for X11 Global Scale
            m_frameExtentsHeadThicknessGap = qRound(m_frameExtentsHeadThicknessGap * m_latteView->devicePixelRatio());
        }

        QMargins frameExtents(0, 0, 0, 0);

        if (m_latteView->location() == Plasma::Types::LeftEdge) {
            frameExtents.setRight(m_frameExtentsHeadThicknessGap);
        } else if (m_latteView->location() == Plasma::Types::TopEdge) {
            frameExtents.setBottom(m_frameExtentsHeadThicknessGap);
        } else if (m_latteView->location() == Plasma::Types::RightEdge) {
            frameExtents.setLeft(m_frameExtentsHeadThicknessGap);
        } else {
            frameExtents.setTop(m_frameExtentsHeadThicknessGap);
        }

        bool bypasswm{m_latteView->byPassWM() && KWindowSystem::isPlatformX11()};

        qDebug() << " -> Frame Extents :: " << m_frameExtentsLocation << " __ " << " extents :: " << frameExtents << " bypasswm :: " << bypasswm;

        if (!frameExtents.isNull() && !m_latteView->behaveAsPlasmaPanel() && !bypasswm) {
            //! When a view returns its frame extents to zero then that triggers a compositor
            //! strange behavior that moves/hides the view totally and freezes entire Latte
            //! this is why we have blocked that setting
            m_wm->setFrameExtents(m_latteView, frameExtents);
        } else if (m_latteView->behaveAsPlasmaPanel() || bypasswm) {
            QMargins panelExtents(0, 0, 0, 0);
            m_wm->setFrameExtents(m_latteView, panelExtents);
            emit frameExtentsCleared();
        }
    }
}

int VisibilityManager::timerShow() const
{
    return m_timerShow.interval();
}

void VisibilityManager::setTimerShow(int msec)
{
    if (m_timerShow.interval() == msec) {
        return;
    }

    m_timerShow.setInterval(msec);
    emit timerShowChanged();
}

int VisibilityManager::timerHide() const
{
    return m_timerHideInterval;
}

void VisibilityManager::setTimerHide(int msec)
{
    int interval = qMax(HIDEMINIMUMINTERVAL, msec);

    if (m_timerHideInterval == interval) {
        return;
    }

    m_timerHideInterval = interval;
    m_timerHide.setInterval(interval);
    emit timerHideChanged();
}

bool VisibilityManager::isSidebar() const
{
    return m_mode == Latte::Types::SidebarOnDemand || m_mode == Latte::Types::SidebarAutoHide;
}

bool VisibilityManager::supportsKWinEdges() const
{
    return (m_edgeGhostWindow != nullptr);
}

void VisibilityManager::updateGhostWindowState()
{
    if (supportsKWinEdges()) {
        bool inCurrentLayout = (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout ||
                                (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts
                                 && m_latteView->layout() && !m_latteView->positioner()->inRelocationAnimation()
                                 && m_latteView->layout()->isCurrent()));

        if (inCurrentLayout) {
            if (m_mode == Latte::Types::WindowsCanCover) {
                m_wm->setActiveEdge(m_edgeGhostWindow, m_isBelowLayer && !m_containsMouse);
            } else {
                bool activated = (m_isHidden && !windowContainsMouse());

                m_wm->setActiveEdge(m_edgeGhostWindow, activated);
            }
        } else {
            m_wm->setActiveEdge(m_edgeGhostWindow, false);
        }
    }
}

void VisibilityManager::hide()
{
    if (KWindowSystem::isPlatformX11()) {
        m_latteView->setVisible(false);
    }
}

void VisibilityManager::show()
{
    if (KWindowSystem::isPlatformX11()) {
        m_latteView->setVisible(true);
    }
}

void VisibilityManager::toggleHiddenState()
{
    if (!m_latteView->inEditMode()) {
        if (isSidebar()) {
            // if (m_blockHidingEvents.contains(Q_FUNC_INFO)) {
            //    removeBlockHidingEvent(Q_FUNC_INFO);
            // }

            if (m_mode == Latte::Types::SidebarOnDemand) {
                m_isRequestedShownSidebarOnDemand = !m_isRequestedShownSidebarOnDemand;
                updateHiddenState();
            } else if (m_mode == Latte::Types::SidebarAutoHide) {
                if (m_isHidden) {
                    emit mustBeShown();
                    startTimerHide(SIDEBARAUTOHIDEMINIMUMSHOW + m_timerHideInterval);
                } else {
                    emit mustBeHide();
                }
            }
        } else {
            /*    if (!m_isHidden && !m_blockHidingEvents.contains(Q_FUNC_INFO)) {
                addBlockHidingEvent(Q_FUNC_INFO);
            } else if (m_isHidden) {
                removeBlockHidingEvent(Q_FUNC_INFO);
            }*/
        }
    }
}

void VisibilityManager::updateHiddenState()
{
    if (m_dragEnter)
        return;

    switch (m_mode) {
    case Types::AutoHide:
    case Types::WindowsCanCover:
        raiseView(m_containsMouse);
        break;

    case Types::DodgeActive:
        dodgeActive();
        break;

    case Types::DodgeMaximized:
        dodgeMaximized();
        break;

    case Types::DodgeAllWindows:
        dodgeAllWindows();
        break;

    case Types::SidebarOnDemand:
        raiseView(m_isRequestedShownSidebarOnDemand);
        break;

    case Types::SidebarAutoHide:
        raiseView(m_latteView->inEditMode() || (m_containsMouse && !m_isHidden));
        break;

    default:
        break;
    }
}

void VisibilityManager::raiseView(bool raise)
{
    if (m_mode == Latte::Types::SidebarOnDemand) {
        if (raise && m_isHidden) {
            emit mustBeShown();
        } else if (!raise && !m_isHidden && !m_dragEnter && !hidingIsBlocked()) {
            emit mustBeHide();
        }
        return;
    }

    if (raise) {
        m_timerHide.stop();

        if (!m_timerShow.isActive()) {
            m_timerShow.start();
        }
    } else if (!m_dragEnter && !hidingIsBlocked()) {
        m_timerShow.stop();

        if (m_hideNow) {
            m_hideNow = false;
            emit mustBeHide();
        } else if (!m_timerHide.isActive()) {
            startTimerHide();
        }
    }
}

void VisibilityManager::raiseViewTemporarily()
{
    if (m_raiseTemporarily)
        return;

    m_raiseTemporarily = true;
    m_timerHide.stop();
    m_timerShow.stop();

    if (m_isHidden)
        emit mustBeShown();

    QTimer::singleShot(qBound(1800, 2 * m_timerHide.interval(), 3000), this, [&]() {
        m_raiseTemporarily = false;
        m_hideNow = true;
        updateHiddenState();
    });
}

bool VisibilityManager::isValidMode() const
{
    return (m_mode != Types::None && m_mode != Types::NormalWindow);
}

void VisibilityManager::applyActivitiesToHiddenWindows(const QStringList &activities)
{
    if (m_edgeGhostWindow) {
        m_wm->setWindowOnActivities(m_edgeGhostWindow->trackedWindowId(), activities);
    }

    if (m_floatingGapWindow) {
        m_wm->setWindowOnActivities(m_floatingGapWindow->trackedWindowId(), activities);
    }
}

void VisibilityManager::startTimerHide(const int &msec)
{
    if (msec == 0) {
        int secs = m_timerHideInterval;

        if (!KWindowSystem::compositingActive()) {
            //! this is needed in order to give view time to show and
            //! for floating case to give time to user to reach the view with its mouse
            secs = qMax(m_timerHideInterval, m_latteView->screenEdgeMargin() > 0 ? 700 : 200);
        }

        m_timerHide.start(secs);
    } else {
        m_timerHide.start(msec);
    }
}

void VisibilityManager::dodgeActive()
{
    if (m_raiseTemporarily)
        return;

    //!don't send false raiseView signal when containing mouse
    if (m_containsMouse) {
        raiseView(true);
        return;
    }

    raiseView(!m_latteView->windowsTracker()->currentScreen()->activeWindowTouching());
}

void VisibilityManager::dodgeMaximized()
{
    if (m_raiseTemporarily)
        return;

    //!don't send false raiseView signal when containing mouse
    if (m_containsMouse) {
        raiseView(true);
        return;
    }

    raiseView(!m_latteView->windowsTracker()->currentScreen()->activeWindowMaximized());
}

void VisibilityManager::dodgeAllWindows()
{
    if (m_raiseTemporarily)
        return;

    if (m_containsMouse) {
        raiseView(true);
        return;
    }

    bool windowIntersects{m_latteView->windowsTracker()->currentScreen()->activeWindowTouching() || m_latteView->windowsTracker()->currentScreen()->existsWindowTouching()};

    raiseView(!windowIntersects);
}

void VisibilityManager::saveConfig()
{
    if (!m_latteView->containment()) {
        return;
    }

    auto config = m_latteView->containment()->config();

    config.writeEntry("enableKWinEdges", m_enableKWinEdgesFromUser);
    config.writeEntry("timerShow", m_timerShow.interval());
    config.writeEntry("timerHide", m_timerHideInterval);
    config.writeEntry("raiseOnDesktopChange", m_raiseOnDesktopChange);
    config.writeEntry("raiseOnActivityChange", m_raiseOnActivityChange);
    config.writeEntry("visibility", static_cast<int>(m_mode));

}

void VisibilityManager::restoreConfig()
{
    auto config = m_latteView->containment()->config();
    setTimerHide(qMax(HIDEMINIMUMINTERVAL, config.readEntry("timerHide", 700)));
    setTimerShow(config.readEntry("timerShow", 0));
    setEnableKWinEdges(config.readEntry("enableKWinEdges", true));
    setRaiseOnDesktop(config.readEntry("raiseOnDesktopChange", false));
    setRaiseOnActivity(config.readEntry("raiseOnActivityChange", false));

    setMode((Types::Visibility)(config.readEntry("visibility", (int)(Types::DodgeActive))));
}

bool VisibilityManager::containsMouse() const
{
    return m_containsMouse;
}

void VisibilityManager::setContainsMouse(bool contains)
{
    if (m_containsMouse == contains) {
        return;
    }

    m_containsMouse = contains;
    emit containsMouseChanged();
}

bool VisibilityManager::windowContainsMouse()
{
    return m_containsMouse || (m_edgeGhostWindow && m_edgeGhostWindow->containsMouse());
}

void VisibilityManager::checkMouseInFloatingArea()
{
    if (m_isFloatingGapWindowEnabled) {
        if (!m_floatingGapWindow) {
            createFloatingGapWindow();
        }

        m_floatingGapWindow->callAsyncContainsMouse();
    }
}

void VisibilityManager::viewEventManager(QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::Enter:
        setContainsMouse(true);
        break;

    case QEvent::Leave:
        m_dragEnter = false;
        setContainsMouse(false);
        break;

    case QEvent::DragEnter:
        m_dragEnter = true;

        if (m_isHidden && !isSidebar()) {
            emit mustBeShown();
        }

        break;

    case QEvent::DragLeave:
    case QEvent::Drop:
        m_dragEnter = false;
        updateHiddenState();
        break;

    default:
        break;
    }
}

//! KWin Edges Support functions
bool VisibilityManager::enableKWinEdges() const
{
    return m_enableKWinEdgesFromUser;
}

void VisibilityManager::setEnableKWinEdges(bool enable)
{
    if (m_enableKWinEdgesFromUser == enable) {
        return;
    }

    m_enableKWinEdgesFromUser = enable;

    emit enableKWinEdgesChanged();
}

void VisibilityManager::updateKWinEdgesSupport()
{
    if ((m_mode == Types::AutoHide
         || m_mode == Types::DodgeActive
         || m_mode == Types::DodgeAllWindows
         || m_mode == Types::DodgeMaximized)
            && !m_latteView->byPassWM()) {

        if (m_enableKWinEdgesFromUser || m_latteView->behaveAsPlasmaPanel()) {
            createEdgeGhostWindow();
        } else if (!m_enableKWinEdgesFromUser) {
            deleteEdgeGhostWindow();
        }
    } else if (m_mode == Types::WindowsCanCover) {
        createEdgeGhostWindow();
    } else {
        deleteEdgeGhostWindow();
    }
}

void VisibilityManager::onIsFloatingGapWindowEnabledChanged()
{
    if (m_isFloatingGapWindowEnabled) {
        createFloatingGapWindow();
    } else {
        deleteFloatingGapWindow();
    }
}

void VisibilityManager::createEdgeGhostWindow()
{
    if (!m_edgeGhostWindow) {
        m_edgeGhostWindow = new ScreenEdgeGhostWindow(m_latteView);

        connect(m_edgeGhostWindow, &ScreenEdgeGhostWindow::containsMouseChanged, this, [ = ](bool contains) {
            if (contains) {
                raiseView(true);
            } else {
                m_timerShow.stop();
                updateGhostWindowState();
            }
        });

        connect(m_edgeGhostWindow, &ScreenEdgeGhostWindow::dragEntered, this, [&]() {
            if (m_isHidden) {
                emit mustBeShown();
            }
        });

        m_connectionsKWinEdges[0] = connect(m_wm, &WindowSystem::AbstractWindowInterface::currentActivityChanged,
                                            this, [&]() {
            bool inCurrentLayout = (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout ||
                                    (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts
                                     && m_latteView->layout() && !m_latteView->positioner()->inRelocationAnimation()
                                     && m_latteView->layout()->isCurrent()));

            if (m_edgeGhostWindow) {
                if (inCurrentLayout) {
                    m_wm->setActiveEdge(m_edgeGhostWindow, m_isHidden);
                } else {
                    m_wm->setActiveEdge(m_edgeGhostWindow, false);
                }
            }
        });

        emit supportsKWinEdgesChanged();
    }
}

void VisibilityManager::deleteEdgeGhostWindow()
{
    if (m_edgeGhostWindow) {
        m_edgeGhostWindow->deleteLater();
        m_edgeGhostWindow = nullptr;

        for (auto &c : m_connectionsKWinEdges) {
            disconnect(c);
        }

        emit supportsKWinEdgesChanged();
    }
}

void VisibilityManager::createFloatingGapWindow()
{
    if (!m_floatingGapWindow) {
        m_floatingGapWindow = new FloatingGapWindow(m_latteView);

        connect(m_floatingGapWindow, &FloatingGapWindow::asyncContainsMouseChanged, this, [ = ](bool contains) {
            if (contains) {
                if (m_isFloatingGapWindowEnabled && !m_isHidden) {
                    //! immediate call after contains mouse checks for mouse in sensitive floating areas
                    updateHiddenState();
                }
            } else {
                if (m_isFloatingGapWindowEnabled && !m_isHidden) {
                    //! immediate call after contains mouse checks for mouse in sensitive floating areas
                    emit mustBeHide();
                }
            }
        });
    }
}

void VisibilityManager::deleteFloatingGapWindow()
{
    if (m_floatingGapWindow) {
        m_floatingGapWindow->deleteLater();
        m_floatingGapWindow = nullptr;
    }
}

bool VisibilityManager::supportsFloatingGap() const
{
    return (m_floatingGapWindow != nullptr);
}


//! END: VisibilityManager implementation

}
}
