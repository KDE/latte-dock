/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "trackedviewinfo.h"

//local
#include "windowstracker.h"
#include "../schemecolors.h"
#include "../../view/view.h"


namespace Latte {
namespace WindowSystem {
namespace Tracker {


TrackedViewInfo::TrackedViewInfo(Tracker::Windows *tracker, Latte::View *view)
    : TrackedGeneralInfo(tracker) ,
      m_view(view)
{
    m_activities = m_view->activities();

    connect(m_view, &Latte::View::activitiesChanged, this, [&]() {
        m_activities = m_view->activities();
        updateTrackingCurrentActivity();
    });
}

TrackedViewInfo::~TrackedViewInfo()
{
}

bool TrackedViewInfo::activeWindowTouching() const
{
    return m_activeWindowTouching;
}

void TrackedViewInfo::setActiveWindowTouching(bool touching)
{
    if (m_activeWindowTouching == touching) {
        return;
    }

    m_activeWindowTouching = touching;
}

bool TrackedViewInfo::existsWindowTouching() const
{
    return m_existsWindowTouching;
}

void TrackedViewInfo::setExistsWindowTouching(bool touching)
{
    if (m_existsWindowTouching == touching) {
        return;
    }

    m_existsWindowTouching = touching;
}

bool TrackedViewInfo::activeWindowTouchingEdge() const
{
    return m_activeWindowTouchingEdge;
}

void TrackedViewInfo::setActiveWindowTouchingEdge(bool touching)
{
    if (m_activeWindowTouchingEdge == touching) {
        return;
    }

    m_activeWindowTouchingEdge = touching;
}

bool TrackedViewInfo::existsWindowTouchingEdge() const
{
    return m_existsWindowTouchingEdge;
}

void TrackedViewInfo::setExistsWindowTouchingEdge(bool touching)
{
    if (m_existsWindowTouchingEdge == touching) {
        return;
    }

    m_existsWindowTouchingEdge = touching;
}

bool TrackedViewInfo::isTouchingBusyVerticalView() const
{
    return m_isTouchingBusyVerticalView;
}

void TrackedViewInfo::setIsTouchingBusyVerticalView(bool touching)
{
    if (m_isTouchingBusyVerticalView == touching) {
        return;
    }

    m_isTouchingBusyVerticalView = touching;
}

QRect TrackedViewInfo::availableScreenGeometry() const
{
    return m_availableScreenGeometry;
}

void TrackedViewInfo::setAvailableScreenGeometry(QRect geometry)
{
    if (m_availableScreenGeometry == geometry) {
        return;
    }

    m_availableScreenGeometry = geometry;
}

SchemeColors *TrackedViewInfo::touchingWindowScheme() const
{
    return m_touchingWindowScheme;
}

void TrackedViewInfo::setTouchingWindowScheme(SchemeColors *scheme)
{
    if (m_touchingWindowScheme == scheme) {
        return;
    }

    m_touchingWindowScheme = scheme;
}

Latte::View *TrackedViewInfo::view() const
{
    return m_view;
}

bool TrackedViewInfo::isTracking(const WindowInfoWrap &winfo) const
{   
    return  TrackedGeneralInfo::isTracking(winfo)
            && m_availableScreenGeometry.contains(winfo.geometry().center());
}

}
}
}
