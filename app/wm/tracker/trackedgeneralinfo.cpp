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

#include "trackedgeneralinfo.h"

//local
#include "windowstracker.h"
#include "../abstractwindowinterface.h"
#include "../schemecolors.h"

namespace Latte {
namespace WindowSystem {
namespace Tracker {


TrackedGeneralInfo::TrackedGeneralInfo(Tracker::Windows *tracker)
    : QObject(tracker) ,
      m_wm(tracker->wm()),
      m_tracker(tracker)
{
    m_lastActiveWindow = new LastActiveWindow(this);

    connect(m_wm, &AbstractWindowInterface::currentActivityChanged, this, [&]() {
        updateTrackingCurrentActivity();
    });

    emit lastActiveWindowChanged();
}

TrackedGeneralInfo::~TrackedGeneralInfo()
{
    if (m_lastActiveWindow) {
        auto law = m_lastActiveWindow;
        m_lastActiveWindow = nullptr;
        emit lastActiveWindowChanged();

        law->deleteLater();
    }
}

bool TrackedGeneralInfo::enabled() const
{
    return m_enabled;
}

void TrackedGeneralInfo::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
}

bool TrackedGeneralInfo::activeWindowMaximized() const
{
    return m_activeWindowMaximized;
}

void TrackedGeneralInfo::setActiveWindowMaximized(bool activeMaximized)
{
    if (m_activeWindowMaximized == activeMaximized) {
        return;
    }

    m_activeWindowMaximized = activeMaximized;
}

bool TrackedGeneralInfo::existsWindowActive() const
{
    return m_existsWindowActive;
}

void TrackedGeneralInfo::setExistsWindowActive(bool exists)
{
    if (m_existsWindowActive == exists) {
        return;
    }

    m_existsWindowActive = exists;
}

bool TrackedGeneralInfo::existsWindowMaximized() const
{
    return m_existsWindowMaximized;
}

void TrackedGeneralInfo::setExistsWindowMaximized(bool maximized)
{
    if (m_existsWindowMaximized == maximized) {
        return;
    }

    m_existsWindowMaximized = maximized;
}

bool TrackedGeneralInfo::isTrackingCurrentActivity() const
{
    return m_isTrackingCurrentActivity;
}

void TrackedGeneralInfo::updateTrackingCurrentActivity()
{
    m_isTrackingCurrentActivity = ( m_activities.isEmpty()
                                    || m_activities[0] == "0"
            || m_activities.contains(m_wm->currentActivity()));
}


LastActiveWindow *TrackedGeneralInfo::lastActiveWindow() const
{
    return m_lastActiveWindow;
}

SchemeColors *TrackedGeneralInfo::activeWindowScheme() const
{
    return m_activeWindowScheme;
}

void TrackedGeneralInfo::setActiveWindowScheme(SchemeColors *scheme)
{
    if (m_activeWindowScheme == scheme) {
        return;
    }

    m_activeWindowScheme = scheme;
}

AbstractWindowInterface *TrackedGeneralInfo::wm()
{
    return m_wm;
}

void TrackedGeneralInfo::setActiveWindow(const WindowId &wid)
{
    m_lastActiveWindow->setInformation(m_tracker->infoFor(wid));
}

bool TrackedGeneralInfo::isTracking(const WindowInfoWrap &winfo) const
{
    return (winfo.isValid()
            && isTrackingCurrentActivity()
            && !winfo.isPlasmaDesktop()
            && !winfo.isMinimized()
            && winfo.isOnDesktop(m_wm->currentDesktop())
            && winfo.isOnActivity(m_wm->currentActivity()));
}

}
}
}
