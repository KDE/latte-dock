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

#include "waylandinterface.h"
#include "../liblattedock/extras.h"

#include <QDebug>
#include <QTimer>
#include <QtX11Extras/QX11Info>

#include <KWindowSystem>
#include <KWindowInfo>
#include <NETWM>

namespace Latte {

WaylandInterface::WaylandInterface(QObject *parent)
    : AbstractWindowInterface(parent)
{
    m_activities = new KActivities::Consumer(this);
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged
            , this, &AbstractWindowInterface::activeWindowChanged);
    connect(KWindowSystem::self()
            , static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>
            (&KWindowSystem::windowChanged)
            , this, &WaylandInterface::windowChangedProxy);

    auto addWindow = [&](WindowId wid) {
        if (std::find(m_windows.cbegin(), m_windows.cend(), wid) == m_windows.cend()) {
            if (isValidWindow(KWindowInfo(wid.value<WId>(), NET::WMWindowType))) {
                m_windows.push_back(wid);
                emit windowAdded(wid);
            }
        }
    };

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, addWindow);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, [this](WindowId wid) {
        if (std::find(m_windows.cbegin(), m_windows.cend(), wid) != m_windows.end()) {
            m_windows.remove(wid);
            emit windowRemoved(wid);
        }
    });
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged
            , this, &WaylandInterface::currentDesktopChanged);
    connect(m_activities.data(), &KActivities::Consumer::currentActivityChanged
            , this, &WaylandInterface::currentActivityChanged);

    // fill windows list
    /* foreach (const auto &wid, KWindowSystem::self()->windows()) {
          addWindow(wid);
      }*/
}

WaylandInterface::~WaylandInterface()
{
}

void WaylandInterface::setDockExtraFlags(QQuickWindow &view)
{
    //  KWindowSystem::setType(view.winId(), NET::Dock);
    //  KWindowSystem::setState(view.winId(), NET::SkipTaskbar | NET::SkipPager);
    KWindowSystem::setOnAllDesktops(view.winId(), true);
    KWindowSystem::setOnActivities(view.winId(), {"0"});
}

void WaylandInterface::setDockStruts(WindowId dockId, const QRect &dockRect
                                     , const QScreen &screen, Plasma::Types::Location location) const
{
    NETExtendedStrut strut;

    const QRect currentScreen {screen.geometry()};
    const QRect wholeScreen {{0, 0}, screen.virtualSize()};

    switch (location) {
        case Plasma::Types::TopEdge: {
            const int topOffset {screen.geometry().top()};
            strut.top_width = dockRect.height() + topOffset;
            strut.top_start = dockRect.x();
            strut.top_end = dockRect.x() + dockRect.width() - 1;
            break;
        }

        case Plasma::Types::BottomEdge: {
            const int bottomOffset {wholeScreen.bottom() - currentScreen.bottom()};
            strut.bottom_width = dockRect.height() + bottomOffset;
            strut.bottom_start = dockRect.x();
            strut.bottom_end = dockRect.x() + dockRect.width() - 1;
            break;
        }

        case Plasma::Types::LeftEdge: {
            const int leftOffset = {screen.geometry().left()};
            strut.left_width = dockRect.width() + leftOffset;
            strut.left_start = dockRect.y();
            strut.left_end = dockRect.y() + dockRect.height() - 1;
            break;
        }

        case Plasma::Types::RightEdge: {
            const int rightOffset = {wholeScreen.right() - currentScreen.right()};
            strut.right_width = dockRect.width() + rightOffset;
            strut.right_start = dockRect.y();
            strut.right_end = dockRect.y() + dockRect.height() - 1;
            break;
        }

        default:
            qWarning() << "wrong location:" << qEnumToStr(location);
            return;
    }

    KWindowSystem::setExtendedStrut(dockId.value<WId>(),
                                    strut.left_width,   strut.left_start,   strut.left_end,
                                    strut.right_width,  strut.right_start,  strut.right_end,
                                    strut.top_width,    strut.top_start,    strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end
                                   );
}

void WaylandInterface::removeDockStruts(WindowId dockId) const
{
    KWindowSystem::setStrut(dockId.value<WId>(), 0, 0, 0, 0);
}

WindowId WaylandInterface::activeWindow() const
{
    return KWindowSystem::self()->activeWindow();
}

const std::list<WindowId> &WaylandInterface::windows() const
{
    return m_windows;
}

void WaylandInterface::skipTaskBar(const QDialog &dialog) const
{
    KWindowSystem::setState(dialog.winId(), NET::SkipTaskbar);
}

void WaylandInterface::slideWindow(QQuickWindow &view, AbstractWindowInterface::Slide location) const
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

void WaylandInterface::enableBlurBehind(QQuickWindow &view) const
{
    KWindowEffects::enableBlurBehind(view.winId());
}

WindowInfoWrap WaylandInterface::requestInfoActive() const
{
    return requestInfo(KWindowSystem::activeWindow());
}

bool WaylandInterface::isOnCurrentDesktop(WindowId wid) const
{
    KWindowInfo winfo(wid.value<WId>(), NET::WMDesktop);
    return winfo.valid() && winfo.isOnCurrentDesktop();
}

WindowInfoWrap WaylandInterface::requestInfo(WindowId wid) const
{
    /* const KWindowInfo winfo{wid, NET::WMFrameExtents
         | NET::WMWindowType
         | NET::WMGeometry
         | NET::WMState};*/

    WindowInfoWrap winfoWrap;

    /* if (isValidWindow(winfo)) {
         winfoWrap.setIsValid(true);
         winfoWrap.setWid(wid);
         winfoWrap.setIsActive(KWindowSystem::activeWindow() == wid);
         winfoWrap.setIsMinimized(winfo.hasState(NET::Hidden));
         winfoWrap.setIsMaxVert(winfo.hasState(NET::MaxVert));
         winfoWrap.setIsMaxHoriz(winfo.hasState(NET::MaxHoriz));
         winfoWrap.setIsFullscreen(winfo.hasState(NET::FullScreen));
         winfoWrap.setIsShaded(winfo.hasState(NET::Shaded));
         winfoWrap.setGeometry(winfo.frameGeometry());
     } else if (m_desktopId == wid) {
         winfoWrap.setIsValid(true);
         winfoWrap.setIsPlasmaDesktop(true);
         winfoWrap.setWid(wid);
     }*/

    return winfoWrap;
}


bool WaylandInterface::isValidWindow(const KWindowInfo &winfo) const
{
    /*  constexpr auto types = NET::DockMask | NET::MenuMask | NET::SplashMask | NET::NormalMask;
      auto winType = winfo.windowType(types);

      if (winType == -1) {
          // Trying to get more types for verify if the window have any other type
          winType = winfo.windowType(~types & NET::AllTypesMask);

          if (winType == -1) {
              qWarning() << KWindowInfo(winfo.win(), 0, NET::WM2WindowClass).windowClassName()
                         << "doesn't have any WindowType, assuming as NET::Normal";
              return true;
          }
      }*/
    return true;

    //return !((winType & NET::Menu) || (winType & NET::Dock) || (winType & NET::Splash));
}

void WaylandInterface::windowChangedProxy(WindowId wid, NET::Properties prop1, NET::Properties2 prop2)
{
    //! if the dock changed is ignored
    /* if (std::find(m_docks.cbegin(), m_docks.cend(), wid) != m_docks.cend())
         return;

     const auto winType = KWindowInfo(wid, NET::WMWindowType).windowType(NET::DesktopMask);

     if (winType != -1 && (winType & NET::Desktop)) {
         m_desktopId = wid;
         emit windowChanged(wid);
         return;
     }

     //! ignore when, eg: the user presses a key
     if (prop1 == 0 && prop2 == NET::WM2UserTime) {
         return;
     }

     if (prop1 && !(prop1 & NET::WMState || prop1 & NET::WMGeometry || prop1 & NET::ActiveWindow))
         return;

     emit windowChanged(wid);*/
}

}
