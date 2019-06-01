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

#include "trackedinfo.h"

//local
#include "windows.h"
#include "../schemecolors.h"
#include "../../view/view.h"

namespace Latte {
namespace WindowSystem {
namespace Tracker {


TrackedInfo::TrackedInfo(Tracker::Windows *tracker, Latte::View *view)
    : QObject(tracker) ,
      m_wm(tracker->wm()),
      m_view(view)
{
    m_lastActiveWindow = new LastActiveWindow(this);

    emit lastActiveWindowChanged();
}

TrackedInfo::~TrackedInfo()
{
    if (m_lastActiveWindow) {
        auto law = m_lastActiveWindow;
        m_lastActiveWindow = nullptr;
        emit lastActiveWindowChanged();

        law->deleteLater();
    }
}

bool TrackedInfo::enabled() const
{
    return m_enabled;
}

void TrackedInfo::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
}

bool TrackedInfo::activeWindowMaximized() const
{
    return m_activeWindowMaximized;
}

void TrackedInfo::setActiveWindowMaximized(bool activeMaximized)
{
    if (m_activeWindowMaximized == activeMaximized) {
        return;
    }

    m_activeWindowMaximized = activeMaximized;
}

bool TrackedInfo::activeWindowTouching() const
{
    return m_activeWindowTouching;
}

void TrackedInfo::setActiveWindowTouching(bool touching)
{
    if (m_activeWindowTouching == touching) {
        return;
    }

    m_activeWindowTouching = touching;
}

bool TrackedInfo::existsWindowActive() const
{
    return m_existsWindowActive;
}

void TrackedInfo::setExistsWindowActive(bool exists)
{
    if (m_existsWindowActive == exists) {
        return;
    }

    m_existsWindowActive = exists;
}

bool TrackedInfo::existsWindowMaximized() const
{
    return m_existsWindowMaximized;
}

void TrackedInfo::setExistsWindowMaximized(bool maximized)
{
    if (m_existsWindowMaximized == maximized) {
        return;
    }

    m_existsWindowMaximized = maximized;
}

bool TrackedInfo::existsWindowTouching() const
{
    return m_existsWindowTouching;
}

void TrackedInfo::setExistsWindowTouching(bool touching)
{
    if (m_existsWindowTouching == touching) {
        return;
    }

    m_existsWindowTouching = touching;
}

QRect TrackedInfo::availableScreenGeometry() const
{
    return m_availableScreenGeometry;
}

void TrackedInfo::setAvailableScreenGeometry(QRect geometry)
{
    if (m_availableScreenGeometry == geometry) {
        return;
    }

    m_availableScreenGeometry = geometry;
}

LastActiveWindow *TrackedInfo::lastActiveWindow() const
{
    return m_lastActiveWindow;
}

SchemeColors *TrackedInfo::activeWindowScheme() const
{
    return m_activeWindowScheme;
}

void TrackedInfo::setActiveWindowScheme(SchemeColors *scheme)
{
    if (m_activeWindowScheme == scheme) {
        return;
    }

    m_activeWindowScheme = scheme;
}

SchemeColors *TrackedInfo::touchingWindowScheme() const
{
    return m_touchingWindowScheme;
}

void TrackedInfo::setTouchingWindowScheme(SchemeColors *scheme)
{
    if (m_touchingWindowScheme == scheme) {
        return;
    }

    m_touchingWindowScheme = scheme;
}

Latte::View *TrackedInfo::view()
{
    return m_view;
}

AbstractWindowInterface *TrackedInfo::wm()
{
    return m_wm;
}

}
}
}
