#include "visibilitymanager.h"

#include "abstractinterface.h"
#include "xwindowinterface.h"
#include "plasmaquick/containmentview.h"

#include "../libnowdock/types.h"

#include <QDebug>

#include <Plasma/Containment>

VisibilityManager::VisibilityManager(PlasmaQuick::ContainmentView *view) :
    QObject(view),
    m_disableHiding(false),
    m_isAutoHidden(false),
    m_isDockWindowType(true),
    m_isHovered(false),
    m_secondInitPass(false),
    m_windowIsInAttention(false),
    m_childrenLength(-1),
    m_view(view)
{
//    m_windowSystem = new WindowSystem(this);
//    connect(m_windowSystem, SIGNAL(compositingChanged()), this, SLOT(compositingChanged()));

    m_interface = new NowDock::XWindowInterface(m_view);
    connect(m_interface, SIGNAL(windowInAttention(bool)), this, SLOT(setWindowInAttention(bool)));
    connect(m_interface, SIGNAL(activeWindowChanged()), this, SLOT(activeWindowChanged()));
    m_interface->setDockToAllDesktops();
    //fixes a bug in plasma-framework with wrong popups placement
    m_interface->setDockNumber(2);
    
//    connect(this, SIGNAL(screenChanged(QScreen *)), this, SLOT(screenChanged(QScreen *)));
//    setPanelScreen(screen());

    m_updateStateTimer.setSingleShot(true);
    m_updateStateTimer.setInterval(900);
    connect(&m_updateStateTimer, &QTimer::timeout, this, &VisibilityManager::updateState);
    
    m_initTimer.setSingleShot(true);
    m_initTimer.setInterval(400);
    connect(&m_initTimer, &QTimer::timeout, this, &VisibilityManager::initWindow);
    
    connect(this, SIGNAL(panelVisibilityChanged()), this, SLOT(updateVisibilityFlags()));
    setPanelVisibility(NowDock::Types::BelowActive);
//   updateVisibilityFlags();

//    connect(this, SIGNAL(locationChanged()), this, SLOT(updateWindowPosition()));
    connect(this, SIGNAL(windowInAttentionChanged()), this, SLOT(updateState()));
    
    initialize();
}

VisibilityManager::~VisibilityManager()
{
}

void VisibilityManager::setContainment(Plasma::Containment *containment)
{
    if (containment == m_containment) {
        return;
    }
    
    m_containment = containment;
//    setVisibility(mode);
}


NowDock::Types::Visibility VisibilityManager::panelVisibility() const
{
    return m_panelVisibility;
}

void VisibilityManager::setPanelVisibility(NowDock::Types::Visibility state)
{
    if (m_panelVisibility == state) {
        return;
    }
    
    m_panelVisibility = state;
    emit panelVisibilityChanged();
}

bool VisibilityManager::isAutoHidden() const
{
    return m_isAutoHidden;
}

void VisibilityManager::setIsAutoHidden(bool state)
{
    if (m_isAutoHidden == state) {
        return;
    }
    
    m_isAutoHidden = state;
    emit isAutoHiddenChanged();
}

bool VisibilityManager::windowInAttention() const
{
    return m_windowIsInAttention;
}

void VisibilityManager::setWindowInAttention(bool state)
{
    if (m_windowIsInAttention == state) {
        return;
    }
    
    m_windowIsInAttention = state;
    emit windowInAttentionChanged();
}

bool VisibilityManager::disableHiding() const
{
    return m_disableHiding;
}

void VisibilityManager::setDisableHiding(bool value)
{
    if (m_disableHiding == value) {
        return;
    }
    
    m_disableHiding = value;
    
    emit disableHidingChanged();
    
    if (!m_disableHiding) {
        m_updateStateTimer.start();
    }
}

bool VisibilityManager::isDockWindowType() const
{
    return m_isDockWindowType;
}

void VisibilityManager::setIsDockWindowType(bool state)
{
    if (m_isDockWindowType == state) {
        return;
    }
    
    m_isDockWindowType = state;
    
    updateVisibilityFlags();
    
    emit isDockWindowTypeChanged();
    
    m_updateStateTimer.start();
}

bool VisibilityManager::isHovered() const
{
    return m_isHovered;
}

void VisibilityManager::setIsHovered(bool state)
{
    if (m_isHovered == state) {
        return;
    }
    
    m_isHovered = state;
    emit isHoveredChanged();
}

void VisibilityManager::setMaskArea(QRect area)
{
    m_interface->setMaskArea(area);
}

/*******************************/

void VisibilityManager::initialize()
{
    m_secondInitPass = true;
    m_initTimer.start();
}

void VisibilityManager::initWindow()
{
    updateVisibilityFlags();
    
//   updateWindowPosition();

    // The initialization phase makes two passes because
    // changing the window style and type wants a small delay
    // and afterwards the second pass positions them correctly
    if (m_secondInitPass) {
        m_initTimer.start();
        m_secondInitPass = false;
    }
}


/*void VisibilityManager::updateWindowPosition()
{
    //setPanelScreen(screen());
    //   qDebug() <<  "updateWindowPosition: start...";
    if (!transientParent() || !transientParent()->screen()) {
        //       qDebug() <<  "updateWindowPosition: break transient...";
        return;
    }

    //      qDebug() <<  "updateWindowPosition: transientParent setting screen position...";
    setPanelScreen(transientParent()->screen());

    if (!m_screen || m_screenGeometry.isNull()) {
        //      qDebug() <<  "updateWindowPosition: break m_screen...";
        return;
    }
    //   qDebug() <<  "updateWindowPosition: check passed...";
    // qDebug() << m_screen->geometry().x() << " - " << m_screen->geometry().y() << " - " << m_screen->geometry().width() << " - " << m_screen->geometry().height();

    if (m_location == Plasma::Types::BottomEdge) {
        setX(m_screenGeometry.x());
        setY(m_screenGeometry.y()+m_screenGeometry.height() - height());
    } else if (m_location == Plasma::Types::TopEdge) {
        setX(m_screenGeometry.x());
        setY(m_screenGeometry.y());
    } else if (m_location == Plasma::Types::LeftEdge) {
        setX(m_screenGeometry.x());
        setY(m_screenGeometry.y());
    } else if (m_location == Plasma::Types::RightEdge) {
        setX(m_screenGeometry.x()+m_screenGeometry.width() - width());
        setY(m_screenGeometry.y());
    }

    ///FIXME: in come cases the below can not catch up and this may be the reason
    //that on start up in some cases dock's contents are not shown,
    //needs a timer maybe?
    /*if (m_screen != screen()) {
        setScreen(m_screen);
    }*/
//}

void VisibilityManager::updateVisibilityFlags()
{
    m_interface->setDockToAllDesktops();
    
    /* if ((m_panelVisibility == AutoHide)||(m_isDockWindowType)) {
         m_updateStateTimer.setInterval(2500);
     } else {
         m_updateStateTimer.setInterval(1500);
     }*/
    
    m_interface->setDockDefaultFlags(m_isDockWindowType);
    
//   updateWindowPosition();
    if (!m_isDockWindowType) {
        showOnTop();
    }
    
    m_updateStateTimer.start();
}

/*
 * It is used from the m_updateStateTimer in order to check the dock's
 * visibility and trigger events and actions which are needed to
 * respond accordingly
 */
void VisibilityManager::updateState()
{
    //   qDebug() << "in update state disableHiding:" <<m_disableHiding;
    
    //update the dock behavior
    switch (m_panelVisibility) {
        case NowDock::Types::BelowActive:
            if (!m_interface->desktopIsActive() && m_interface->dockIntersectsActiveWindow()) {
                if (m_interface->dockIsOnTop() || (m_isDockWindowType && !m_isAutoHidden)) {
                    // qDebug() << m_isHovered  << " - " << m_windowIsInAttention << " - "<< m_disableHiding;
                    if (!m_isHovered && !m_windowIsInAttention && !m_disableHiding) {
                        //  qDebug() << "must be lowered....";
                        emit mustBeLowered();                    //showNormal();
                    }
                } else {
                    if (m_windowIsInAttention) {
                        if (!m_isDockWindowType || (m_isDockWindowType && m_isAutoHidden)) {
                            emit mustBeRaised();                     //showOnTop();
                        }
                    }
                }
            } else {
                if (!m_interface->activeIsDialog()) {
                    if ((!m_interface->desktopIsActive() && m_interface->dockIsCovered())
                        || (m_isDockWindowType && m_isAutoHidden)) {
                        //   qDebug() << "must be raised....";
                        emit mustBeRaised();
                    } else {
                        showOnTop();
                    }
                }
            }
            
            break;
            
        case NowDock::Types::BelowMaximized:
            if (!m_interface->desktopIsActive() && m_interface->activeIsMaximized() && m_interface->dockIntersectsActiveWindow()) {
                if (m_interface->dockIsOnTop() || (m_isDockWindowType && !m_isAutoHidden)) {
                    if (!m_isHovered && !m_windowIsInAttention && !m_disableHiding) {
                        emit mustBeLowered();                    //showNormal();
                    }
                } else {
                    if (m_windowIsInAttention) {
                        if (!m_isDockWindowType || (m_isDockWindowType && m_isAutoHidden)) {
                            emit mustBeRaised();                     //showOnTop();
                        }
                    }
                }
            } else {
                if ((!m_interface->desktopIsActive() && m_interface->dockIsCovered())
                    || (m_isDockWindowType && m_isAutoHidden)) {
                    emit mustBeRaised();
                } else {
                    showOnTop();
                }
            }
            
            break;
            
        case NowDock::Types::LetWindowsCover:
        
            //this is not supported in clean Dock Window Types such as in wayland case
            if (m_isDockWindowType) {
                return;
            }
            
            if (!m_isHovered && m_interface->dockIsOnTop()) {
                if (m_interface->dockIsCovering()) {
                    if (!m_disableHiding) {
                        emit mustBeLowered();
                    }
                } else {
                    showOnBottom();
                }
            } else if (m_windowIsInAttention) {
                if (!m_interface->dockIsOnTop()) {
                    if (m_interface->dockIsCovered()) {
                        emit mustBeRaised();
                    } else {
                        showOnTop();
                    }
                }
            }
            
            break;
            
        case NowDock::Types::WindowsGoBelow:
            //Do nothing, the dock is OnTop state in every case
            break;
            
        case NowDock::Types::AutoHide:
            if (m_windowIsInAttention && m_isAutoHidden) {
                emit mustBeRaised();
            } else if (!m_isHovered && !m_disableHiding) {
                emit mustBeLowered();
            }
            
            break;
            
        case NowDock::Types::AlwaysVisible:
            //Do nothing, the dock in OnTop state in every case
            break;
    }
    
}

void VisibilityManager::showOnTop()
{
    //  qDebug() << "reached make top...";
    m_interface->showDockOnTop();
}

void VisibilityManager::showNormal()
{
    //   qDebug() << "reached make normal...";
    m_interface->showDockAsNormal();
}

void VisibilityManager::showOnBottom()
{
    //   qDebug() << "reached make bottom...";
    m_interface->showDockOnBottom();
}

/***************/
void VisibilityManager::activeWindowChanged()
{
    if ((m_panelVisibility == NowDock::Types::WindowsGoBelow)
        || (m_panelVisibility == NowDock::Types::AlwaysVisible)
        || (m_panelVisibility == NowDock::Types::AutoHide)) {
        return;
    }
    
    //this check is important because otherwise the signals are so often
    //that the timer is never triggered
    if (!m_updateStateTimer.isActive()) {
        m_updateStateTimer.start();
    }
}


//It is used in order to trigger a beautiful slide in effect when
//the dock is totally hidden underneath
void VisibilityManager::showOnTopCheck()
{
    if ((m_panelVisibility == NowDock::Types::BelowActive) || (m_panelVisibility == NowDock::Types::BelowMaximized)
        || (m_panelVisibility == NowDock::Types::LetWindowsCover)) {
        if (m_interface->dockIsCovered(true)) {
            m_updateStateTimer.stop();
            setIsHovered(true);
            
            emit mustBeRaisedImmediately();
        } else {
            showOnTop();
        }
    }
}

bool VisibilityManager::event(QEvent *event)
{
    if (!event) {
        return false;
    }
    
    if ((event->type() == QEvent::Enter) && !m_isHovered) {
        m_updateStateTimer.stop();
        setIsHovered(true);
        
        if ((m_panelVisibility == NowDock::Types::AutoHide) || (m_isDockWindowType)) {
            if (m_isAutoHidden) {
                emit mustBeRaised();
            }
        } else {
            showOnTop();
        }
    } else if (event->type() == QEvent::Leave) {
        setIsHovered(false);
        
        if ((m_panelVisibility != NowDock::Types::WindowsGoBelow)
            && (m_panelVisibility != NowDock::Types::AlwaysVisible)) {
            m_updateStateTimer.start();
        }
    }
    
    return true;
}
