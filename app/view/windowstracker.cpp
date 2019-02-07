/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "windowstracker.h"

// local
#include "positioner.h"
#include "view.h"
#include "../lattecorona.h"
#include "../../liblatte2/types.h"

namespace Latte {
namespace ViewPart {

WindowsTracker::WindowsTracker(Latte::View *parent)
    : QObject(parent),
      m_latteView(parent)
{
    qDebug() << "WindowsTracker creating...";

    m_corona = qobject_cast<Latte::Corona *>(m_latteView->corona());
    m_wm = m_corona->wm();
}

WindowsTracker::~WindowsTracker()
{
    qDebug() << "WindowsTracker removing...";
}

bool WindowsTracker::activeWindowTouching() const
{
    return m_activeWindowIsTouchingFlag;
}

void WindowsTracker::setActiveWindowTouching(bool activeTouching)
{
    if (m_activeWindowIsTouchingFlag == activeTouching) {
        return;
    }

    m_activeWindowIsTouchingFlag = activeTouching;
    emit activeWindowTouchingChanged();
}

bool WindowsTracker::existsWindowMaximized() const
{
    return m_windowIsMaximizedFlag;
}

void WindowsTracker::setExistsWindowMaximized(bool windowMaximized)
{
    if (m_windowIsMaximizedFlag == windowMaximized) {
        return;
    }

    m_windowIsMaximizedFlag = windowMaximized;
    emit existsWindowMaximizedChanged();
}

bool WindowsTracker::existsWindowTouching() const
{
    return m_windowIsTouchingFlag;
}

void WindowsTracker::setExistsWindowTouching(bool windowTouching)
{
    if (m_windowIsTouchingFlag == windowTouching) {
        return;
    }

    m_windowIsTouchingFlag = windowTouching;
    emit existsWindowTouchingChanged();
}

SchemeColors *WindowsTracker::activeWindowScheme() const
{
    return m_activeScheme;
}

void WindowsTracker::setActiveWindowScheme(SchemeColors *scheme)
{
    if (m_activeScheme == scheme) {
        return;
    }

    m_activeScheme = scheme;

    emit activeWindowSchemeChanged();
}

SchemeColors *WindowsTracker::touchingWindowScheme() const
{
    return m_touchingScheme;
}

void WindowsTracker::setTouchingWindowScheme(SchemeColors *scheme)
{
    if (m_touchingScheme == scheme) {
        return;
    }

    m_touchingScheme = scheme;

    emit touchingWindowSchemeChanged();
}

bool WindowsTracker::enabled() const
{
    return m_enabled;
}

void WindowsTracker::setEnabled(bool active)
{
    if (m_enabled == active) {
        return;
    }

    m_enabled = active;

    if (m_enabled) {
        m_windows.clear();

        for (const auto &wid : m_wm->windows()) {
             m_windows.insert(wid, m_wm->requestInfo(wid));
        }

        m_connections[0] = connect(m_corona, &Plasma::Corona::availableScreenRectChanged,
                                              this, &WindowsTracker::updateAvailableScreenGeometry);

        m_connections[1] = connect(m_wm, &WindowSystem::windowChanged, this, [&](WindowId wid) {
            m_windows[wid] = m_wm->requestInfo(wid);
            updateFlags();
        });

        m_connections[2] = connect(m_wm, &WindowSystem::windowRemoved, this, [&](WindowId wid) {
            m_windows.remove(wid);
        });

        m_connections[3] = connect(m_wm, &WindowSystem::windowAdded, this, [&](WindowId wid) {
            m_windows.insert(wid, m_wm->requestInfo(wid));
            updateFlags();
        });

        m_connections[4] = connect(m_wm, &WindowSystem::activeWindowChanged, this, [&](WindowId wid) {
            if (m_windows.contains(m_lastActiveWindowWid)) {
                m_windows[m_lastActiveWindowWid] = m_wm->requestInfo(m_lastActiveWindowWid);
            }

            m_windows[wid] = m_wm->requestInfo(wid);
            m_lastActiveWindowWid = wid;

            updateFlags();
        });

        m_connections[5] = connect(m_wm, &WindowSystem::currentDesktopChanged, this, [&] {
            updateFlags();
        });

        m_connections[6] = connect(m_wm, &WindowSystem::currentActivityChanged, this, [&] {
            updateFlags();
        });

        updateAvailableScreenGeometry();
        updateFlags();
    } else {
        // clear mode
        for (auto &c : m_connections) {
            disconnect(c);
        }

        m_windows.clear();

        setActiveWindowTouching(false);
        setExistsWindowMaximized(false);
        setExistsWindowTouching(false);
    }

    emit enabledChanged();
}

void WindowsTracker::updateAvailableScreenGeometry()
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }

    int currentScrId = m_latteView->positioner()->currentScreenId();
    QRect tempAvailableScreenGeometry = m_corona->availableScreenRectWithCriteria(currentScrId, {Types::AlwaysVisible}, {});

    if (tempAvailableScreenGeometry != m_availableScreenGeometry) {
        m_availableScreenGeometry = tempAvailableScreenGeometry;

        updateFlags();
    }
}

void WindowsTracker::updateFlags()
{
    bool foundActive{false};
    bool foundActiveTouch{false};
    bool foundTouch{false};
    bool foundMaximized{false};

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! maybe a garbage collector here is a good idea!!!
    bool existsFaultyWindow{false};

    WindowId maxWinId;
    WindowId activeWinId;
    WindowId touchWinId;
    WindowId activeTouchWinId;

    for (const auto &winfo : m_windows) {
        if (isActiveInCurrentScreen(winfo)) {
            foundActive = true;
            activeWinId = winfo.wid();
        }

        if (isMaximizedInCurrentScreen(winfo)) {
            foundMaximized = true;
            maxWinId = winfo.wid();
        }

        if ((isTouchingPanelEdge(winfo) || (m_latteView->visibility()->intersects(winfo)))) {
            if (winfo.isActive()) {
                foundActiveTouch = true;
                activeTouchWinId = winfo.wid();
            } else {
                foundTouch = true;
                touchWinId = winfo.wid();
            }
        }

        if (!existsFaultyWindow && winfo.geometry() == QRect(0, 0, 0, 0)) {
            existsFaultyWindow = true;
        }

        //qDebug() << "window geometry ::: " << winfo.geometry();
    }

    if (existsFaultyWindow) {
        cleanupFaultyWindows();
    }

    setActiveWindowTouching(foundActiveTouch);
    setExistsWindowMaximized(foundMaximized);
    setExistsWindowTouching(foundTouch || foundActiveTouch);

    //! update color schemes for active and touching windows

    setActiveWindowScheme(foundActive ? m_wm->schemeForWindow(activeWinId) : nullptr);

    if (foundActiveTouch) {
        setTouchingWindowScheme(m_wm->schemeForWindow(activeTouchWinId));
    } else if (foundMaximized) {
        setTouchingWindowScheme(m_wm->schemeForWindow(maxWinId));
    } else {
        setTouchingWindowScheme(nullptr);
    }
}

bool WindowsTracker::isActiveInCurrentScreen(const WindowInfoWrap &winfo)
{
    if (winfo.isValid() && winfo.isActive() && !winfo.isMinimized()
            && m_wm->isOnCurrentDesktop(winfo.wid()) && m_wm->isOnCurrentActivity(winfo.wid())
            && m_availableScreenGeometry.contains(winfo.geometry().center())) {
        return true;
    }

    return false;
}

bool WindowsTracker::isMaximizedInCurrentScreen(const WindowInfoWrap &winfo)
{
    //! updated implementation to identify the screen that the maximized window is present
    //! in order to avoid: https://bugs.kde.org/show_bug.cgi?id=397700

    if (winfo.isValid() && !winfo.isMinimized() && m_wm->isOnCurrentDesktop(winfo.wid()) && m_wm->isOnCurrentActivity(winfo.wid())) {
        if (winfo.isMaximized() && m_availableScreenGeometry.contains(winfo.geometry().center())) {
            return true;
        }
    }

    return false;
}

bool WindowsTracker::isTouchingPanelEdge(const WindowInfoWrap &winfo)
{
    if (winfo.isValid() && !winfo.isMinimized() && m_wm->isOnCurrentDesktop(winfo.wid()) && m_wm->isOnCurrentActivity(winfo.wid())) {
        bool touchingPanelEdge{false};

        QRect screenGeometry = m_latteView->screenGeometry();
        bool inCurrentScreen{screenGeometry.contains(winfo.geometry().topLeft()) || screenGeometry.contains(winfo.geometry().bottomRight())};

        if (inCurrentScreen) {
            if (m_latteView->location() == Plasma::Types::TopEdge) {
                touchingPanelEdge = (winfo.geometry().y() == m_availableScreenGeometry.y());
            } else if (m_latteView->location() == Plasma::Types::BottomEdge) {
                touchingPanelEdge = (winfo.geometry().bottom() == m_availableScreenGeometry.bottom());
            } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
                touchingPanelEdge = (winfo.geometry().x() == m_availableScreenGeometry.x());
            } else if (m_latteView->location() == Plasma::Types::RightEdge) {
                touchingPanelEdge = (winfo.geometry().right() == m_availableScreenGeometry.right());
            }
        }

        return touchingPanelEdge;
    }

    return false;
}

void WindowsTracker::cleanupFaultyWindows()
{
    foreach (auto key, m_windows.keys()) {
        auto winfo = m_windows[key];

        //! garbage windows removing
        if (winfo.geometry() == QRect(0, 0, 0, 0)) {
            //qDebug() << "Faulty Geometry ::: " << winfo.wid();
            m_windows.remove(key);
        }
    }
}

//! Window Functions
void WindowsTracker::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    m_wm->setWindowOnActivities(window, activities);
}

void WindowsTracker::requestToggleMaximizeForActiveWindow()
{
    WindowInfoWrap actInfo = m_wm->requestInfoActive();

    //active window can be toggled only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        m_wm->requestToggleMaximized(actInfo.wid());
    }
}

void WindowsTracker::requestMoveActiveWindow(int localX, int localY)
{
    WindowInfoWrap actInfo = m_wm->requestInfoActive();

    //active window can be dragged only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        QPoint globalPoint{m_latteView->x() + localX, m_latteView->y() + localY};

        m_wm->requestMoveWindow(actInfo.wid(), globalPoint);

        //! This timer is needed because otherwise the mouse position
        //! in the dragged window changes to TopLeft corner
        QTimer::singleShot(250, this, [&, actInfo, globalPoint]() {
            m_wm->releaseMouseEventFor(m_latteView->winId());
        });

        m_latteView->visibility()->activeWindowDraggingStarted();
    }
}

bool WindowsTracker::activeWindowCanBeDragged()
{
    WindowInfoWrap actInfo = m_wm->requestInfoActive();

    //active window can be dragged only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        return m_wm->windowCanBeDragged(actInfo.wid());
    }

    return false;
}

}
}
