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

#include "xwindowinterface.h"

// local
#include "view/screenedgeghostwindow.h"
#include "view/view.h"
#include "../liblatte2/extras.h"

// Qt
#include <QDebug>
#include <QTimer>
#include <QtX11Extras/QX11Info>

// KDE
#include <KWindowSystem>
#include <KWindowInfo>

// X11
#include <NETWM>
#include <xcb/xcb.h>

namespace Latte {

XWindowInterface::XWindowInterface(QObject *parent)
    : AbstractWindowInterface(parent)
{
    m_activities = new KActivities::Consumer(this);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged
            , this, &AbstractWindowInterface::activeWindowChanged);
    connect(KWindowSystem::self()
            , static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>
            (&KWindowSystem::windowChanged)
            , this, &XWindowInterface::windowChangedProxy);

    auto addWindow = [&](WindowId wid) {
        if (std::find(m_windows.cbegin(), m_windows.cend(), wid) == m_windows.cend()) {
            if (isValidWindow(KWindowInfo(wid.value<WId>(), NET::WMWindowType))) {
                m_windows.push_back(wid);
                emit windowAdded(wid);
            }
        }
    };

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, addWindow);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, [this](WindowId wid) noexcept {
        if (std::find(m_windows.cbegin(), m_windows.cend(), wid) != m_windows.end()) {
            m_windows.remove(wid);
            emit windowRemoved(wid);
        }
    });
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged
            , this, &XWindowInterface::currentDesktopChanged);
    connect(m_activities.data(), &KActivities::Consumer::currentActivityChanged
            , this, &XWindowInterface::currentActivityChanged);

    // fill windows list
    foreach (const auto &wid, KWindowSystem::self()->windows()) {
        addWindow(wid);
    }
}

XWindowInterface::~XWindowInterface()
{
}

void XWindowInterface::setViewExtraFlags(QWindow &view)
{
    NETWinInfo winfo(QX11Info::connection()
                     , static_cast<xcb_window_t>(view.winId())
                     , static_cast<xcb_window_t>(view.winId())
                     , 0, 0);

    winfo.setAllowedActions(NET::ActionChangeDesktop);
    KWindowSystem::setType(view.winId(), NET::Dock);
    KWindowSystem::setState(view.winId(), NET::SkipTaskbar | NET::SkipPager);
    KWindowSystem::setOnAllDesktops(view.winId(), true);
}

void XWindowInterface::setViewStruts(QWindow &view, const QRect &rect
                                     , Plasma::Types::Location location)
{
    NETExtendedStrut strut;

    const auto screen = view.screen();

    const QRect currentScreen {screen->geometry()};
    const QRect wholeScreen {{0, 0}, screen->virtualSize()};

    switch (location) {
        case Plasma::Types::TopEdge: {
            const int topOffset {screen->geometry().top()};
            strut.top_width = rect.height() + topOffset;
            strut.top_start = rect.x();
            strut.top_end = rect.x() + rect.width() - 1;
            break;
        }

        case Plasma::Types::BottomEdge: {
            const int bottomOffset {wholeScreen.bottom() - currentScreen.bottom()};
            strut.bottom_width = rect.height() + bottomOffset;
            strut.bottom_start = rect.x();
            strut.bottom_end = rect.x() + rect.width() - 1;
            break;
        }

        case Plasma::Types::LeftEdge: {
            const int leftOffset = {screen->geometry().left()};
            strut.left_width = rect.width() + leftOffset;
            strut.left_start = rect.y();
            strut.left_end = rect.y() + rect.height() - 1;
            break;
        }

        case Plasma::Types::RightEdge: {
            const int rightOffset = {wholeScreen.right() - currentScreen.right()};
            strut.right_width = rect.width() + rightOffset;
            strut.right_start = rect.y();
            strut.right_end = rect.y() + rect.height() - 1;
            break;
        }

        default:
            qWarning() << "wrong location:" << qEnumToStr(location);
            return;
    }

    KWindowSystem::setExtendedStrut(view.winId(),
                                    strut.left_width,   strut.left_start,   strut.left_end,
                                    strut.right_width,  strut.right_start,  strut.right_end,
                                    strut.top_width,    strut.top_start,    strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end
                                   );
}

void XWindowInterface::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    KWindowSystem::setOnActivities(window.winId(), activities);
}

void XWindowInterface::removeViewStruts(QWindow &view) const
{
    KWindowSystem::setStrut(view.winId(), 0, 0, 0, 0);
}

WindowId XWindowInterface::activeWindow() const
{
    return KWindowSystem::self()->activeWindow();
}

const std::list<WindowId> &XWindowInterface::windows() const
{
    return m_windows;
}

void XWindowInterface::setKeepAbove(const QDialog &dialog, bool above) const
{
    if (above) {
        KWindowSystem::setState(dialog.winId(), NET::KeepAbove);
    } else {
        KWindowSystem::clearState(dialog.winId(), NET::KeepAbove);
    }
}

void XWindowInterface::skipTaskBar(const QDialog &dialog) const
{
    KWindowSystem::setState(dialog.winId(), NET::SkipTaskbar);
}

void XWindowInterface::slideWindow(QWindow &view, AbstractWindowInterface::Slide location) const
{
    auto slideLocation = KWindowEffects::NoEdge;

    switch (location) {
        case Slide::Top:
            slideLocation = KWindowEffects::TopEdge;
            break;

        case Slide::Bottom:
            slideLocation = KWindowEffects::BottomEdge;
            break;

        case Slide::Left:
            slideLocation = KWindowEffects::LeftEdge;
            break;

        case Slide::Right:
            slideLocation = KWindowEffects::RightEdge;
            break;

        default:
            break;
    }

    KWindowEffects::slideWindow(view.winId(), slideLocation, -1);
}

void XWindowInterface::enableBlurBehind(QWindow &view) const
{
    KWindowEffects::enableBlurBehind(view.winId());
}

void XWindowInterface::setEdgeStateFor(QWindow *view, bool active) const
{
    ViewPart::ScreenEdgeGhostWindow *window = qobject_cast<ViewPart::ScreenEdgeGhostWindow *>(view);

    if (!window) {
        return;
    }

    xcb_connection_t *c = QX11Info::connection();

    const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_SCREEN_EDGE_SHOW");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());

    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));

    if (!atom) {
        return;
    }

    if (!active) {
        xcb_delete_property(c, window->winId(), atom->atom);
        window->hideWithMask();
        return;
    }

    window->showWithMask();

    uint32_t value = 0;

    switch (window->location()) {
        case Plasma::Types::TopEdge:
            value = 0;
            break;

        case Plasma::Types::RightEdge:
            value = 1;
            break;

        case Plasma::Types::BottomEdge:
            value = 2;
            break;

        case Plasma::Types::LeftEdge:
            value = 3;
            break;

        case Plasma::Types::Floating:
        default:
            value = 4;
            break;
    }

    int hideType = 0;

    value |= hideType << 8;

    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom->atom, XCB_ATOM_CARDINAL, 32, 1, &value);
}

WindowInfoWrap XWindowInterface::requestInfoActive() const
{
    return requestInfo(KWindowSystem::activeWindow());
}

bool XWindowInterface::isOnCurrentDesktop(WindowId wid) const
{
    KWindowInfo winfo(wid.value<WId>(), NET::WMDesktop);
    return winfo.valid() && winfo.isOnCurrentDesktop();
}

bool XWindowInterface::isOnCurrentActivity(WindowId wid) const
{
    KWindowInfo winfo(wid.value<WId>(), 0, NET::WM2Activities);

    return winfo.valid()
           && (winfo.activities().contains(m_activities->currentActivity()) || winfo.activities().empty());
}

WindowInfoWrap XWindowInterface::requestInfo(WindowId wid) const
{
    const KWindowInfo winfo{wid.value<WId>(), NET::WMFrameExtents
                            | NET::WMWindowType
                            | NET::WMGeometry
                            | NET::WMState};

    WindowInfoWrap winfoWrap;

    if (isValidWindow(winfo)) {
        winfoWrap.setIsValid(true);
        winfoWrap.setWid(wid);
        winfoWrap.setIsActive(KWindowSystem::activeWindow() == wid.value<WId>());
        winfoWrap.setIsMinimized(winfo.hasState(NET::Hidden));
        winfoWrap.setIsMaxVert(winfo.hasState(NET::MaxVert));
        winfoWrap.setIsMaxHoriz(winfo.hasState(NET::MaxHoriz));
        winfoWrap.setIsFullscreen(winfo.hasState(NET::FullScreen));
        winfoWrap.setIsShaded(winfo.hasState(NET::Shaded));
        winfoWrap.setGeometry(winfo.frameGeometry());
        winfoWrap.setIsKeepAbove(winfo.hasState(NET::KeepAbove));
        winfoWrap.setHasSkipTaskbar(winfo.hasState(NET::SkipTaskbar));
    } else if (m_desktopId == wid) {
        winfoWrap.setIsValid(true);
        winfoWrap.setIsPlasmaDesktop(true);
        winfoWrap.setWid(wid);
        winfoWrap.setHasSkipTaskbar(true);
    }

    return winfoWrap;
}

bool XWindowInterface::windowCanBeDragged(WindowId wid) const
{
    WindowInfoWrap winfo = requestInfo(wid);
    return (winfo.isValid() && !winfo.isPlasmaDesktop() && !winfo.hasSkipTaskbar());
}

void XWindowInterface::releaseMouseEventFor(WindowId wid) const
{
    auto connection = QX11Info::connection();

    xcb_button_release_event_t releaseEvent;
    memset(&releaseEvent, 0, sizeof(releaseEvent));

    releaseEvent.response_type = XCB_BUTTON_RELEASE;
    releaseEvent.event =  wid.toInt();
    releaseEvent.child = XCB_WINDOW_NONE;
    releaseEvent.root = QX11Info::appRootWindow();
    releaseEvent.event_x = -1;
    releaseEvent.event_y = -1;
    releaseEvent.root_x = -1;
    releaseEvent.root_y = -1;
    releaseEvent.detail = XCB_BUTTON_INDEX_1;
    releaseEvent.state = XCB_BUTTON_MASK_1;
    releaseEvent.time = XCB_CURRENT_TIME;
    releaseEvent.same_screen = true;
    xcb_send_event( connection, false, wid.toInt(), XCB_EVENT_MASK_BUTTON_RELEASE, reinterpret_cast<const char*>(&releaseEvent));
}

void XWindowInterface::requestMoveWindow(WindowId wid, QPoint from) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (!wInfo.isValid() || wInfo.isPlasmaDesktop()) {
        return;
    }

    int borderX{wInfo.geometry().width() > 120 ? 60 : 10};
    int borderY{10};

    //! find min/max values for x,y based on active window geometry
    int minX = wInfo.geometry().x() + borderX;
    int maxX = wInfo.geometry().x() + wInfo.geometry().width() - borderX;
    int minY = wInfo.geometry().y() + borderY;
    int maxY = wInfo.geometry().y() + wInfo.geometry().height() - borderY;

    //! set the point from which this window will be moved,
    //! make sure that it is in window boundaries
    int validX = qBound(minX, from.x(), maxX);
    int validY = qBound(minY, from.y(), maxY);

    NETRootInfo ri(QX11Info::connection(), NET::WMMoveResize);
    ri.moveResizeRequest(wInfo.wid().toUInt(), validX, validY, NET::Move);
}

void XWindowInterface::requestToggleMaximized(WindowId wid) const
{
    WindowInfoWrap wInfo = requestInfo(wid);
    bool restore = wInfo.isMaxHoriz() && wInfo.isMaxVert();

    NETWinInfo ni(QX11Info::connection(), wid.toInt(), QX11Info::appRootWindow(), NET::WMState, NET::Properties2());

    if (restore) {
        ni.setState(NET::States(), NET::Max);
    } else {
        ni.setState(NET::Max, NET::Max);
    }
}

bool XWindowInterface::isValidWindow(const KWindowInfo &winfo) const
{
    constexpr auto types = NET::DockMask | NET::MenuMask | NET::SplashMask | NET::NormalMask;
    auto winType = winfo.windowType(types);

    if (winType == -1) {
        // Trying to get more types for verify if the window have any other type
        winType = winfo.windowType(~types & NET::AllTypesMask);

        if (winType == -1) {
            qWarning() << KWindowInfo(winfo.win(), 0, NET::WM2WindowClass).windowClassName()
                       << "doesn't have any WindowType, assuming as NET::Normal";
            return true;
        }
    }

    return !((winType & NET::Menu) || (winType & NET::Dock) || (winType & NET::Splash));
}

void XWindowInterface::windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2)
{
    //! if the view changed is ignored
    if (std::find(m_views.cbegin(), m_views.cend(), wid) != m_views.cend())
        return;

    const auto winType = KWindowInfo(wid, NET::WMWindowType).windowType(NET::DesktopMask);

    //! update desktop id
    if (winType != -1 && (winType & NET::Desktop)) {
        m_desktopId = wid;
        emit windowChanged(wid);
        return;
    }

    //! accept only NET::Properties events,
    //! ignore when the user presses a key, or a window is sending X events etc.
    //! without needing to (e.g. Firefox, https://bugzilla.mozilla.org/show_bug.cgi?id=1389953)
    //! NET::WM2UserTime, NET::WM2IconPixmap etc....
    if (prop1 == 0) {
        return;
    }

    //! accept only the following NET:Properties changed signals
    //! NET::WMState, NET::WMGeometry, NET::ActiveWindow
    if (!((prop1 & NET::WMState) || (prop1 & NET::WMGeometry) || (prop1 & NET::ActiveWindow))) {
        return;
    }

    //! when only WMState changed we can whitelist the acceptable states
    if ((prop1 & NET::WMState) && !(prop1 & NET::WMGeometry) && !(prop1 & NET::ActiveWindow)) {
        KWindowInfo info(wid, NET::WMState);

        if (info.valid()) {
            if (!info.hasState(NET::Sticky) && !info.hasState(NET::Shaded)
                && !info.hasState(NET::FullScreen) && !info.hasState(NET::Hidden)) {
                return;
            }
        } else {
            return;
        }
    }

    emit windowChanged(wid);
}

}
