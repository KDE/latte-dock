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
    connections << connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged
                           , this, &AbstractWindowInterface::activeWindowChanged);
    connections << connect(KWindowSystem::self()
                           , static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>
                           (&KWindowSystem::windowChanged)
                           , this, &XWindowInterface::windowChangedProxy);
    auto addWindow = [&](WId wid) {
        if (std::find(m_windows.cbegin(), m_windows.cend(), wid) == m_windows.cend()) {
            if (isValidWindow(KWindowInfo(wid, NET::WMWindowType))) {
                m_windows.push_back(wid);
                emit windowAdded(wid);
            }
        }
    };
    connections << connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, addWindow);
    connections << connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, [this](WId wid) {
        if (std::find(m_windows.cbegin(), m_windows.cend(), wid) != m_windows.end()) {
            m_windows.remove(wid);
            emit windowRemoved(wid);
        }
    });
    connections << connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged
                           , this, &AbstractWindowInterface::currentDesktopChanged);

    // fill windows list
    foreach (const auto &wid, KWindowSystem::self()->windows()) {
        addWindow(wid);
    }
}

XWindowInterface::~XWindowInterface()
{
    qDebug() << "x window interface deleting...";

    foreach (auto var, connections) {
        QObject::disconnect(var);
    }

    qDebug() << "x window interface connections removed...";
}

void XWindowInterface::setDockDefaultFlags()
{
    m_view->setFlags(Qt::FramelessWindowHint
                     | Qt::WindowStaysOnTopHint
                     | Qt::NoDropShadowWindowHint
                     | Qt::WindowDoesNotAcceptFocus);
    NETWinInfo winfo(QX11Info::connection()
                     , static_cast<xcb_window_t>(m_view->winId())
                     , static_cast<xcb_window_t>(m_view->winId())
                     , 0, 0);
    winfo.setAllowedActions(NET::ActionChangeDesktop);
    KWindowSystem::setType(m_view->winId(), NET::Dock);
    KWindowSystem::setState(m_view->winId(), NET::SkipTaskbar | NET::SkipPager);
    KWindowSystem::setOnAllDesktops(m_view->winId(), true);
    KWindowSystem::setOnActivities(m_view->winId(), {"0"});
}

WId XWindowInterface::activeWindow() const
{
    return KWindowSystem::self()->activeWindow();
}

const std::list<WId> &XWindowInterface::windows() const
{
    return m_windows;
}

void XWindowInterface::setDockStruts(const QRect &dockRect, Plasma::Types::Location location) const
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

void XWindowInterface::removeDockStruts() const
{
    KWindowSystem::setStrut(m_view->winId(), 0, 0, 0, 0);
}
WindowInfoWrap XWindowInterface::requestInfoActive() const
{
    return requestInfo(KWindowSystem::activeWindow());
}

bool XWindowInterface::isOnCurrentDesktop(WId wid) const
{
    KWindowInfo winfo(wid, NET::WMDesktop);
    return winfo.valid() && winfo.isOnCurrentDesktop();
}

WindowInfoWrap XWindowInterface::requestInfo(WId wid) const
{
    const KWindowInfo winfo{wid, NET::WMFrameExtents | NET::WMWindowType | NET::WMGeometry | NET::WMState};
    WindowInfoWrap winfoWrap;

    if (!winfo.valid()) {
        return winfoWrap;
    } else if (isValidWindow(winfo)) {
        winfoWrap.setIsValid(true);
        winfoWrap.setWid(wid);
        winfoWrap.setIsActive(KWindowSystem::activeWindow() == wid);
        winfoWrap.setIsMinimized(winfo.hasState(NET::Hidden));
        winfoWrap.setIsMaximized(winfo.hasState(NET::Max));
        winfoWrap.setIsFullscreen(winfo.hasState(NET::FullScreen));
        winfoWrap.setGeometry(winfo.geometry());
    } else if (m_desktopId == wid) {
        winfoWrap.setIsValid(true);
        winfoWrap.setIsPlasmaDesktop(true);
        winfoWrap.setWid(wid);
    }

    return winfoWrap;
}


bool XWindowInterface::isValidWindow(const KWindowInfo &winfo) const
{
    const auto winType = winfo.windowType(NET::DockMask
                                          | NET::MenuMask | NET::SplashMask
                                          | NET::NormalMask);

    if (winType == -1) {
        const KWindowInfo win{winfo.win(), 0, NET::WM2WindowClass};

        // NOTE: Impossible to get type information from the spotify, then I need add a exception
        // maybe is a bug of spotify.
        if (win.windowClassName() == "spotify")
            return true;
        else
            return false;
    }

    return !((winType & NET::Menu) || (winType & NET::Dock) || (winType & NET::Splash));
}

void XWindowInterface::windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2)
{
    //! if the dock changed is ignored
    if (wid == m_view->winId())
        return;

    //! ignore when, eg: the user presses a key
    const auto winType = KWindowInfo(wid, NET::WMWindowType).windowType(NET::DesktopMask);

    if (winType != -1 && (winType & NET::Desktop)) {
        m_desktopId = wid;
        emit windowChanged(wid);
        return;
    }

    if (prop1 == 0 && prop2 == NET::WM2UserTime) {
        return;
    }

    if (prop1 && !(prop1 & NET::WMState || prop1 & NET::WMGeometry || prop1 & NET::ActiveWindow))
        return;

    emit windowChanged(wid);
}

}
