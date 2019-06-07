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

#include "trackerwindows.h"

// local
#include "lastactivewindow.h"
#include "schemes.h"
#include "trackedviewinfo.h"
#include "../abstractwindowinterface.h"
#include "../schemecolors.h"
#include "../../lattecorona.h"
#include "../../layouts/manager.h"
#include "../../view/view.h"
#include "../../view/positioner.h"
#include "../../../liblatte2/types.h"

namespace Latte {
namespace WindowSystem {
namespace Tracker {

Windows::Windows(AbstractWindowInterface *parent)
    : QObject(parent)
{
    m_wm = parent;

    init();
}

Windows::~Windows()
{
    //! clear all the m_views tracking information
    for (QHash<Latte::View *, TrackedViewInfo *>::iterator i=m_views.begin(); i!=m_views.end(); ++i) {
        i.value()->deleteLater();
        m_views[i.key()] = nullptr;
    }

    m_views.clear();
}

void Windows::init()
{
    connect(m_wm->corona(), &Plasma::Corona::availableScreenRectChanged, this, &Windows::updateAvailableScreenGeometries);

    connect(m_wm, &AbstractWindowInterface::windowChanged, this, [&](WindowId wid) {
        m_windows[wid] = m_wm->requestInfo(wid);
        updateViewsHints();

        emit windowChanged(wid);
    });

    connect(m_wm, &AbstractWindowInterface::windowRemoved, this, [&](WindowId wid) {
        m_windows.remove(wid);
        updateViewsHints();

        emit windowRemoved(wid);
    });

    connect(m_wm, &AbstractWindowInterface::windowAdded, this, [&](WindowId wid) {
        if (!m_windows.contains(wid)) {
            m_windows.insert(wid, m_wm->requestInfo(wid));
        }
        updateViewsHints();
    });

    connect(m_wm, &AbstractWindowInterface::activeWindowChanged, this, [&](WindowId wid) {
        //! for some reason this is needed in order to update properly activeness values
        //! when the active window changes the previous active windows should be also updated
        for (const auto view : m_views.keys()) {
            WindowId lastWinId = m_views[view]->lastActiveWindow()->winId();
            if ((lastWinId) != wid && m_windows.contains(lastWinId)) {
                m_windows[lastWinId] = m_wm->requestInfo(lastWinId);
            }
        }

        m_windows[wid] = m_wm->requestInfo(wid);
        updateViewsHints();

        emit activeWindowChanged(wid);
    });

    connect(m_wm, &AbstractWindowInterface::currentDesktopChanged, this, [&] {
        updateViewsHints();
    });

    connect(m_wm, &AbstractWindowInterface::currentActivityChanged, this, [&] {
        if (m_wm->corona()->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
            //! this is needed in MultipleLayouts because there is a chance that multiple
            //! layouts are providing different available screen geometries in different Activities
            updateAvailableScreenGeometries();
        }

        updateViewsHints();
    });
}

void Windows::initViewHints(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return;
    }

    setActiveWindowMaximized(view, false);
    setActiveWindowTouching(view, false);
    setExistsWindowActive(view, false);
    setExistsWindowTouching(view, false);
    setExistsWindowMaximized(view, false);
    setActiveWindowScheme(view, nullptr);
    setTouchingWindowScheme(view, nullptr);
}

AbstractWindowInterface *Windows::wm()
{
    return m_wm;
}


void Windows::addView(Latte::View *view)
{
    if (m_views.contains(view)) {
        return;
    }

    m_views[view] = new TrackedViewInfo(this, view);

    updateAvailableScreenGeometries();
    updateHints(view);
}

void Windows::removeView(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return;
    }

    m_views[view]->deleteLater();
    m_views.remove(view);
}

//! Views Properties And Hints

bool Windows::enabled(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->enabled();
}

void Windows::setEnabled(Latte::View *view, const bool enabled)
{
    if (!m_views.contains(view) || m_views[view]->enabled() == enabled) {
        return;
    }

    m_views[view]->setEnabled(enabled);

    if (enabled) {
        updateHints(view);
    } else {
        initViewHints(view);
    }

    emit enabledChanged(view);
}

bool Windows::activeWindowMaximized(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->activeWindowMaximized();
}

void Windows::setActiveWindowMaximized(Latte::View *view, bool activeMaximized)
{
    if (!m_views.contains(view) || m_views[view]->activeWindowMaximized() == activeMaximized) {
        return;
    }

    m_views[view]->setActiveWindowMaximized(activeMaximized);
    emit activeWindowMaximizedChanged(view);
}

bool Windows::activeWindowTouching(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->activeWindowTouching();
}

void Windows::setActiveWindowTouching(Latte::View *view, bool activeTouching)
{
    if (!m_views.contains(view) || m_views[view]->activeWindowTouching() == activeTouching) {
        return;
    }

    m_views[view]->setActiveWindowTouching(activeTouching);
    emit activeWindowTouchingChanged(view);
}

bool Windows::existsWindowActive(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->existsWindowActive();
}

void Windows::setExistsWindowActive(Latte::View *view, bool windowActive)
{
    if (!m_views.contains(view) || m_views[view]->existsWindowActive() == windowActive) {
        return;
    }

    m_views[view]->setExistsWindowActive(windowActive);
    emit existsWindowActiveChanged(view);
}

bool Windows::existsWindowMaximized(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->existsWindowMaximized();
}

void Windows::setExistsWindowMaximized(Latte::View *view, bool windowMaximized)
{
    if (!m_views.contains(view) || m_views[view]->existsWindowMaximized() == windowMaximized) {
        return;
    }

    m_views[view]->setExistsWindowMaximized(windowMaximized);
    emit existsWindowMaximizedChanged(view);
}

bool Windows::existsWindowTouching(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->existsWindowTouching();
}

void Windows::setExistsWindowTouching(Latte::View *view, bool windowTouching)
{
    if (!m_views.contains(view) || m_views[view]->existsWindowTouching() == windowTouching) {
        return;
    }

    m_views[view]->setExistsWindowTouching(windowTouching);
    emit existsWindowTouchingChanged(view);
}

SchemeColors *Windows::activeWindowScheme(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return nullptr;
    }

    return m_views[view]->activeWindowScheme();
}

void Windows::setActiveWindowScheme(Latte::View *view, WindowSystem::SchemeColors *scheme)
{
    if (!m_views.contains(view) || m_views[view]->activeWindowScheme() == scheme) {
        return;
    }

    m_views[view]->setActiveWindowScheme(scheme);
    emit activeWindowSchemeChanged(view);
}

SchemeColors *Windows::touchingWindowScheme(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return nullptr;
    }

    return m_views[view]->touchingWindowScheme();
}

void Windows::setTouchingWindowScheme(Latte::View *view, WindowSystem::SchemeColors *scheme)
{
    if (!m_views.contains(view) || m_views[view]->touchingWindowScheme() == scheme) {
        return;
    }

    m_views[view]->setTouchingWindowScheme(scheme);
    emit touchingWindowSchemeChanged(view);
}

LastActiveWindow *Windows::lastActiveWindow(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return nullptr;
    }

    return m_views[view]->lastActiveWindow();
}

bool Windows::isValidFor(const WindowId &wid) const
{
    if (!m_windows.contains(wid)) {
        return false;
    }

    return m_windows[wid].isValid() && !m_windows[wid].isPlasmaDesktop();
}

QIcon Windows::iconFor(const WindowId &wid)
{
    if (!m_windows.contains(wid)) {
        return QIcon();
    }

    if (m_windows[wid].icon().isNull()) {
        AppData data = m_wm->appDataFor(wid);

        QIcon icon = data.icon;

        if (icon.isNull()) {
            icon = m_wm->iconFor(wid);
        }

        m_windows[wid].setIcon(icon);
        return icon;
    }

    return m_windows[wid].icon();
}

QString Windows::appNameFor(const WindowId &wid)
{
    if (!m_windows.contains(wid)) {
        return QString();
    }

    if (m_windows[wid].appName().isEmpty()) {
        AppData data = m_wm->appDataFor(wid);

        m_windows[wid].setAppName(data.name);
        return data.name;
    }

    return m_windows[wid].appName();
}

WindowInfoWrap Windows::infoFor(const WindowId &wid) const
{
    if (!m_windows.contains(wid)) {
        return WindowInfoWrap();
    }

    return m_windows[wid];
}



//! Windows Criteria Functions

bool Windows::inCurrentDesktopActivity(const WindowInfoWrap &winfo)
{
    return (winfo.isValid() && winfo.isOnDesktop(m_wm->currentDesktop()) && winfo.isOnActivity(m_wm->currentActivity()));
}

bool Windows::intersects(Latte::View *view, const WindowInfoWrap &winfo)
{
    return (!winfo.isMinimized() && !winfo.isShaded() && winfo.geometry().intersects(view->absoluteGeometry()));
}

bool Windows::isActive(const WindowInfoWrap &winfo)
{
    return (winfo.isValid() && winfo.isActive() && !winfo.isPlasmaDesktop() && !winfo.isMinimized());
}

bool Windows::isActiveInViewScreen(Latte::View *view, const WindowInfoWrap &winfo)
{
    return (winfo.isValid() && winfo.isActive() && !winfo.isPlasmaDesktop() &&  !winfo.isMinimized()
            && m_views[view]->availableScreenGeometry().contains(winfo.geometry().center()));
}

bool Windows::isMaximizedInViewScreen(Latte::View *view, const WindowInfoWrap &winfo)
{
    auto viewIntersectsMaxVert = [&]() noexcept -> bool {
            return ((winfo.isMaxVert()
                     || (view->screen() && view->screen()->availableSize().height() <= winfo.geometry().height()))
                    && intersects(view, winfo));
};

    auto viewIntersectsMaxHoriz = [&]() noexcept -> bool {
            return ((winfo.isMaxHoriz()
                     || (view->screen() && view->screen()->availableSize().width() <= winfo.geometry().width()))
                    && intersects(view, winfo));
};

    //! updated implementation to identify the screen that the maximized window is present
    //! in order to avoid: https://bugs.kde.org/show_bug.cgi?id=397700
    return (winfo.isValid() && !winfo.isPlasmaDesktop() && !winfo.isMinimized()
            && (winfo.isMaximized() || viewIntersectsMaxVert() || viewIntersectsMaxHoriz())
            && m_views[view]->availableScreenGeometry().contains(winfo.geometry().center()));
}

bool Windows::isTouchingView(Latte::View *view, const WindowSystem::WindowInfoWrap &winfo)
{
    return (winfo.isValid() && !winfo.isPlasmaDesktop() &&  intersects(view, winfo));
}

bool Windows::isTouchingViewEdge(Latte::View *view, const WindowInfoWrap &winfo)
{
    if (winfo.isValid() && !winfo.isPlasmaDesktop() &&  !winfo.isMinimized()) {
        bool touchingViewEdge{false};

        QRect screenGeometry = view->screenGeometry();
        QRect availableScreenGeometry = m_views[view]->availableScreenGeometry();

        bool inCurrentScreen{screenGeometry.contains(winfo.geometry().topLeft()) || screenGeometry.contains(winfo.geometry().bottomRight())};

        if (inCurrentScreen) {
            if (view->location() == Plasma::Types::TopEdge) {
                touchingViewEdge = (winfo.geometry().y() == availableScreenGeometry.y());
            } else if (view->location() == Plasma::Types::BottomEdge) {
                touchingViewEdge = (winfo.geometry().bottom() == availableScreenGeometry.bottom());
            } else if (view->location() == Plasma::Types::LeftEdge) {
                touchingViewEdge = (winfo.geometry().x() == availableScreenGeometry.x());
            } else if (view->location() == Plasma::Types::RightEdge) {
                touchingViewEdge = (winfo.geometry().right() == availableScreenGeometry.right());
            }
        }

        return touchingViewEdge;
    }

    return false;
}

void Windows::cleanupFaultyWindows()
{
    for (const auto &key : m_windows.keys()) {
        auto winfo = m_windows[key];

        //! garbage windows removing
        if (winfo.geometry() == QRect(0, 0, 0, 0)) {
            //qDebug() << "Faulty Geometry ::: " << winfo.wid();
            m_windows.remove(key);
        }
    }
}


void Windows::updateAvailableScreenGeometries()
{
    for (const auto view : m_views.keys()) {
        if (m_views[view]->enabled()) {
            int currentScrId = view->positioner()->currentScreenId();
            QRect tempAvailableScreenGeometry = m_wm->corona()->availableScreenRectWithCriteria(currentScrId, {Types::AlwaysVisible}, {});

            if (tempAvailableScreenGeometry != m_views[view]->availableScreenGeometry()) {
                m_views[view]->setAvailableScreenGeometry(tempAvailableScreenGeometry);

                updateHints(view);
            }
        }
    }
}

void Windows::setPlasmaDesktop(WindowId wid)
{
    if (!m_windows.contains(wid)) {
        return;
    }

    if (!m_windows[wid].isPlasmaDesktop()) {
        m_windows[wid].setIsPlasmaDesktop(true);
        qDebug() << " plasmashell updated...";
        updateViewsHints();
    }
}

void Windows::updateViewsHints()
{
    for (const auto view : m_views.keys()) {
        if (m_views[view]->enabled()) {
            updateHints(view);
        }
    }
}

void Windows::updateHints(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return;
    }

    bool foundActive{false};
    bool foundActiveInCurScreen{false};
    bool foundActiveTouchInCurScreen{false};
    bool foundTouchInCurScreen{false};
    bool foundMaximizedInCurScreen{false};

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! maybe a garbage collector here is a good idea!!!
    bool existsFaultyWindow{false};

    WindowId maxWinId;
    WindowId activeWinId;
    WindowId touchWinId;
    WindowId activeTouchWinId;

    for (const auto &winfo : m_windows) {
        if (winfo.isPlasmaDesktop() || !inCurrentDesktopActivity(winfo)) {
            continue;
        }

        if (isActive(winfo)) {
            foundActive = true;
        }

        if (isActiveInViewScreen(view, winfo)) {
            foundActiveInCurScreen = true;
            activeWinId = winfo.wid();
        }

        if (isTouchingViewEdge(view, winfo) || isTouchingView(view, winfo)) {
            if (winfo.isActive()) {
                foundActiveTouchInCurScreen = true;
                activeTouchWinId = winfo.wid();

                if (isMaximizedInViewScreen(view, winfo)) {
                    //! active maximized windows have higher priority than the rest maximized windows
                    foundMaximizedInCurScreen = true;
                    maxWinId = winfo.wid();
                }
            } else {
                foundTouchInCurScreen = true;
                touchWinId = winfo.wid();
            }

            if (!foundMaximizedInCurScreen && isMaximizedInViewScreen(view, winfo)) {
                foundMaximizedInCurScreen = true;
                maxWinId = winfo.wid();
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

    //! HACK: KWin Effects such as ShowDesktop have no way to be identified and as such
    //! create issues with identifying properly touching and maximized windows. BUT when
    //! they are enabled then NO ACTIVE window is found. This is a way to identify these
    //! effects trigerring and disable the touch flags.
    //! BUG: 404483
    //! Disabled because it has fault identifications, e.g. when a window is maximized and
    //! Latte or Plasma are showing their View settings
    //foundMaximizedInCurScreen = foundMaximizedInCurScreen && foundActive;
    //foundTouchInCurScreen = foundTouchInCurScreen && foundActive;

    //! assign flags
    setExistsWindowActive(view, foundActiveInCurScreen);
    setActiveWindowTouching(view, foundActiveTouchInCurScreen);
    setActiveWindowMaximized(view, (maxWinId.toInt()>0 && (maxWinId == activeTouchWinId)));
    setExistsWindowMaximized(view, foundMaximizedInCurScreen);
    setExistsWindowTouching(view, (foundTouchInCurScreen || foundActiveTouchInCurScreen));

    //! update color schemes for active and touching windows
    setActiveWindowScheme(view, (foundActiveInCurScreen ? m_wm->schemesTracker()->schemeForWindow(activeWinId) : nullptr));

    if (foundActiveTouchInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(activeTouchWinId));
    } else if (foundMaximizedInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(maxWinId));
    } else if (foundTouchInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(touchWinId));
    } else {
        setTouchingWindowScheme(view, nullptr);
    }

    //! update LastActiveWindow
    if (foundActiveInCurScreen) {
        m_views[view]->setActiveWindow(activeWinId);
    }

    //! Debug
    //qDebug() << "TRACKING | SCREEN: " << view->positioner()->currentScreenId() << " , EDGE:" << view->location() << " , ENABLED:" << enabled(view);
    //qDebug() << "TRACKING | activeWindowTouching: " << foundActiveTouchInCurScreen << " ,activeWindowMaximized: " << activeWindowMaximized(view);
    //qDebug() << "TRACKING | existsWindowActive: " << foundActiveInCurScreen << " , existsWindowMaximized:" << existsWindowMaximized(view)
    //         << " , existsWindowTouching:"<<existsWindowTouching(view);
}

}
}
}
