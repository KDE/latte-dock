#include "xwindowinterface.h"
#include "../liblattedock/extras.h"

#include <QDebug>
#include <QtX11Extras/QX11Info>

#include <KWindowSystem>
#include <KWindowInfo>
#include <NETWM>

namespace Latte {

XWindowInterface::XWindowInterface(QQuickWindow *const view, QObject *parent)
    : AbstractWindowInterface(view, parent)
{
    Q_ASSERT(view != nullptr);
    
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged
            , this, &AbstractWindowInterface::activeWindowChanged);
            
    connect(KWindowSystem::self()
            , static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>
            (&KWindowSystem::windowChanged)
            , this, &XWindowInterface::windowChangedProxy);
            
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, [this](WId wid) {
        if (std::find(m_windows.cbegin(), m_windows.cend(), wid) != m_windows.cend()) {
            m_windows.push_back(wid);
        }
        
        emit windowAdded(wid);
    });
    
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, [this](WId wid) {
        m_windows.remove(wid);
        emit windowRemoved(wid);
    });
    
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged
            , this, &AbstractWindowInterface::currentDesktopChanged);
}

XWindowInterface::~XWindowInterface()
{

}

void XWindowInterface::setDockDefaultFlags()
{
    NETWinInfo winfo(QX11Info::connection()
                     , static_cast<xcb_window_t>(m_view->winId())
                     , static_cast<xcb_window_t>(m_view->winId())
                     , 0, 0);
                     
    winfo.setAllowedActions(NET::ActionChangeDesktop);
    KWindowSystem::setType(m_view->winId(), NET::Dock);
    KWindowSystem::setOnAllDesktops(m_view->winId(), true);
}

WId XWindowInterface::activeWindow() const
{
    return KWindowSystem::self()->activeWindow();
}

const std::list<WId> &XWindowInterface::windows()
{
    return m_windows;
}

void XWindowInterface::setDockStruts(const QRect &dockRect, Plasma::Types::Location location)
{
    NETExtendedStrut strut;
    
    switch (location) {
        case Plasma::Types::TopEdge:
            strut.top_width = dockRect.height();
            strut.top_start = dockRect.x();
            strut.top_end = dockRect.x() + dockRect.width() - 1;
            break;
            
        case Plasma::Types::BottomEdge:
            strut.bottom_width = dockRect.height();
            strut.bottom_start = dockRect.x();
            strut.bottom_end = dockRect.x() + dockRect.width() - 1;
            break;
            
        case Plasma::Types::LeftEdge:
            strut.left_width = dockRect.width();
            strut.left_start = dockRect.y();
            strut.left_end = dockRect.y() + dockRect.height() - 1;
            break;
            
        case Plasma::Types::RightEdge:
            strut.right_width = dockRect.width();
            strut.right_start = dockRect.y();
            strut.right_end = dockRect.y() + dockRect.height() - 1;
            break;
            
        default:
            qWarning() << "wrong location:" << qEnumToStr(location);
            return;
    }
    
    
    KWindowSystem::setExtendedStrut(m_view->winId(),
                                    strut.left_width,   strut.left_start,   strut.left_end,
                                    strut.right_width,  strut.right_start,  strut.right_end,
                                    strut.top_width,    strut.top_start,    strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end
                                   );
}

void XWindowInterface::removeDockStruts()
{
    KWindowSystem::setStrut(m_view->winId(), 0, 0, 0, 0);
}

WindowInfoWrap XWindowInterface::requestInfoActive()
{
    return requestInfo(KWindowSystem::activeWindow());
}

WindowInfoWrap XWindowInterface::requestInfo(WId wid)
{
    const KWindowInfo winfo{wid, NET::WMDesktop | NET::WMFrameExtents | NET::WMWindowType | NET::WMState};
    
    WindowInfoWrap winfoWrap;
    
    if (!winfo.valid() || !isValidWindow(winfo))
        return winfoWrap;
        
    winfoWrap.setIsValid(true);
    winfoWrap.setWid(wid);
    winfoWrap.setIsActive(KWindowSystem::activeWindow() == wid);
    winfoWrap.setIsMinimized(winfo.hasState(NET::Hidden));
    winfoWrap.setIsMaximized(winfo.hasState(NET::Max));
    winfoWrap.setIsFullscreen(winfo.hasState(NET::FullScreen));
    winfoWrap.setIsOnCurrentDesktop(winfo.isOnCurrentDesktop());
    winfoWrap.setGeometry(winfo.geometry());
    
    return winfoWrap;
}

bool XWindowInterface::isValidWindow(const KWindowInfo &winfo)
{
    const auto winType = winfo.windowType(NET::DesktopMask | NET::DockMask
                                          | NET::MenuMask | NET::SplashMask
                                          | NET::NormalMask);
                                          
    if (winType == -1 || (winType & NET::Desktop) || (winType & NET::Menu)
        || (winType & NET::Dock) || (winType & NET::Splash)) {
        return false;
    }
    
    return true;
}

void XWindowInterface::windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2)
{
    //! if the dock changed is ignored
    if (wid == m_view->winId())
        return;
        
    //! ignore when, eg: the user presses a key
    if (prop1 == 0 && prop2 == NET::WM2UserTime)
        return;
        
    if (prop1 && !(prop1 & NET::WMState || prop1 & NET::WMGeometry || prop1 & NET::ActiveWindow))
        return;
        
    emit windowChanged(wid);
}

}

