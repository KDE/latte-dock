/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "trackedgeneralinfo.h"

#include <QLatin1String>

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
    m_isTrackingCurrentActivity = ( m_activities.isEmpty() || m_activities[0] == QLatin1String("{0}") || m_activities.contains(m_wm->currentActivity()));
}

bool TrackedGeneralInfo::isTrackingActivity(const QString &activity)
{
    return (m_activities.isEmpty() || m_activities[0] == QLatin1String("{0}") || m_activities.contains(activity));
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
    bool isignored = winfo.isMinimized() || (winfo.hasSkipTaskbar() && (winfo.hasSkipPager() || winfo.hasSkipSwitcher()));

    return (winfo.isValid() && !isignored);
}

bool TrackedGeneralInfo::isShown(const WindowInfoWrap &winfo) const
{
    return (winfo.isValid()
            && !m_wm->isShowingDesktop()
            && isTrackingCurrentActivity()
            && winfo.isOnDesktop(m_wm->currentDesktop())
            && winfo.isOnActivity(m_wm->currentActivity()));
}

}
}
}
