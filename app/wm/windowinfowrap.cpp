/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "windowinfowrap.h"


namespace Latte {
namespace WindowSystem {

WindowInfoWrap::WindowInfoWrap()
{
}

WindowInfoWrap::WindowInfoWrap(const WindowInfoWrap &o)
    : m_wid(o.m_wid)
    , m_parentId(o.m_parentId)
    , m_geometry(o.m_geometry)
    , m_isValid(o.m_isValid)
    , m_isActive(o.m_isActive)
    , m_isMinimized(o.m_isMinimized)
    , m_isMaxVert(o.m_isMaxVert)
    , m_isMaxHoriz(o.m_isMaxHoriz)
    , m_isFullscreen(o.m_isFullscreen)
    , m_isShaded(o.m_isShaded)
    , m_isKeepAbove(o.m_isKeepAbove)
    , m_isKeepBelow(o.m_isKeepBelow)
    , m_hasSkipPager(o.m_hasSkipPager)
    , m_hasSkipSwitcher(o.m_hasSkipSwitcher)
    , m_hasSkipTaskbar(o.m_hasSkipTaskbar)
    , m_isOnAllDesktops(o.m_isOnAllDesktops)
    , m_isOnAllActivities(o.m_isOnAllActivities)
    , m_isClosable(o.m_isClosable)
    , m_isFullScreenable(o.m_isFullScreenable)
    , m_isGroupable(o.m_isGroupable)
    , m_isMaximizable(o.m_isMaximizable)
    , m_isMinimizable(o.m_isMinimizable)
    , m_isMovable(o.m_isMovable)
    , m_isResizable(o.m_isResizable)
    , m_isShadeable(o.m_isShadeable)
    , m_isVirtualDesktopsChangeable(o.m_isVirtualDesktopsChangeable)
    , m_desktops(o.m_desktops)
    , m_activities(o.m_activities)
    , m_display(o.m_display)
{
}

WindowInfoWrap::WindowInfoWrap(WindowInfoWrap &&o)
    : m_wid(o.m_wid)
    , m_parentId(o.m_parentId)
    , m_geometry(o.m_geometry)
    , m_isValid(o.m_isValid)
    , m_isActive(o.m_isActive)
    , m_isMinimized(o.m_isMinimized)
    , m_isMaxVert(o.m_isMaxVert)
    , m_isMaxHoriz(o.m_isMaxHoriz)
    , m_isFullscreen(o.m_isFullscreen)
    , m_isShaded(o.m_isShaded)
    , m_isKeepAbove(o.m_isKeepAbove)
    , m_isKeepBelow(o.m_isKeepBelow)
    , m_hasSkipPager(o.m_hasSkipPager)
    , m_hasSkipSwitcher(o.m_hasSkipSwitcher)
    , m_hasSkipTaskbar(o.m_hasSkipTaskbar)
    , m_isOnAllDesktops(o.m_isOnAllDesktops)
    , m_isOnAllActivities(o.m_isOnAllActivities)
    , m_isClosable(o.m_isClosable)
    , m_isFullScreenable(o.m_isFullScreenable)
    , m_isGroupable(o.m_isGroupable)
    , m_isMaximizable(o.m_isMaximizable)
    , m_isMinimizable(o.m_isMinimizable)
    , m_isMovable(o.m_isMovable)
    , m_isResizable(o.m_isResizable)
    , m_isShadeable(o.m_isShadeable)
    , m_isVirtualDesktopsChangeable(o.m_isVirtualDesktopsChangeable)
    , m_desktops(o.m_desktops)
    , m_activities(o.m_activities)
    , m_display(o.m_display)
{
}

//! Operators
// BEGIN: definitions
WindowInfoWrap &WindowInfoWrap::operator=(WindowInfoWrap &&rhs)
{
    m_wid = rhs.m_wid;
    m_parentId = rhs.m_parentId;
    m_geometry = rhs.m_geometry;
    m_isValid = rhs.m_isValid;
    m_isActive = rhs.m_isActive;
    m_isMinimized = rhs.m_isMinimized;
    m_isMaxVert = rhs.m_isMaxVert;
    m_isMaxHoriz = rhs.m_isMaxHoriz;
    m_isFullscreen = rhs.m_isFullscreen;
    m_isShaded = rhs.m_isShaded;
    m_isKeepAbove = rhs.m_isKeepAbove;
    m_isKeepBelow = rhs.m_isKeepBelow;
    m_hasSkipPager = rhs.m_hasSkipPager;
    m_hasSkipSwitcher = rhs.m_hasSkipSwitcher;
    m_hasSkipTaskbar = rhs.m_hasSkipTaskbar;
    m_isOnAllDesktops = rhs.m_isOnAllDesktops;
    m_isOnAllActivities = rhs.m_isOnAllActivities;
    m_isClosable = rhs.m_isClosable;
    m_isFullScreenable = rhs.m_isFullScreenable;
    m_isGroupable = rhs.m_isGroupable;
    m_isMaximizable = rhs.m_isMaximizable;
    m_isMinimizable = rhs.m_isMinimizable;
    m_isMovable = rhs.m_isMovable;
    m_isResizable = rhs.m_isResizable;
    m_isShadeable = rhs.m_isShadeable;
    m_isVirtualDesktopsChangeable = rhs.m_isVirtualDesktopsChangeable;

    m_display = rhs.m_display;
    m_desktops = rhs.m_desktops;
    m_activities = rhs.m_activities;
    return *this;
}

WindowInfoWrap &WindowInfoWrap::operator=(const WindowInfoWrap &rhs)
{
    m_wid = rhs.m_wid;
    m_parentId = rhs.m_parentId;
    m_geometry = rhs.m_geometry;
    m_isValid = rhs.m_isValid;
    m_isActive = rhs.m_isActive;
    m_isMinimized = rhs.m_isMinimized;
    m_isMaxVert = rhs.m_isMaxVert;
    m_isMaxHoriz = rhs.m_isMaxHoriz;
    m_isFullscreen = rhs.m_isFullscreen;
    m_isShaded = rhs.m_isShaded;
    m_isKeepAbove = rhs.m_isKeepAbove;
    m_isKeepBelow = rhs.m_isKeepBelow;
    m_hasSkipPager = rhs.m_hasSkipPager;
    m_hasSkipSwitcher = rhs.m_hasSkipSwitcher;
    m_hasSkipTaskbar = rhs.m_hasSkipTaskbar;
    m_isOnAllDesktops = rhs.m_isOnAllDesktops;
    m_isOnAllActivities = rhs.m_isOnAllActivities;
    m_isClosable = rhs.m_isClosable;
    m_isFullScreenable = rhs.m_isFullScreenable;
    m_isGroupable = rhs.m_isGroupable;
    m_isMaximizable = rhs.m_isMaximizable;
    m_isMinimizable = rhs.m_isMinimizable;
    m_isMovable = rhs.m_isMovable;
    m_isResizable = rhs.m_isResizable;
    m_isShadeable = rhs.m_isShadeable;
    m_isVirtualDesktopsChangeable = rhs.m_isVirtualDesktopsChangeable;

    m_display = rhs.m_display;
    m_desktops = rhs.m_desktops;
    m_activities = rhs.m_activities;
    return *this;
}

//! Access properties
bool WindowInfoWrap::isValid() const
{
    return m_isValid;
}

void WindowInfoWrap::setIsValid(bool isValid)
{
    m_isValid = isValid;
}

bool WindowInfoWrap::isActive() const
{
    return m_isActive;
}

void WindowInfoWrap::setIsActive(bool isActive)
{
    m_isActive = isActive;
}

bool WindowInfoWrap::isMinimized() const
{
    return m_isMinimized;
}

void WindowInfoWrap::setIsMinimized(bool isMinimized)
{
    m_isMinimized = isMinimized;
}

bool WindowInfoWrap::isMaximized() const
{
    return m_isMaxVert && m_isMaxHoriz;
}

bool WindowInfoWrap::isMaxVert() const
{
    return m_isMaxVert;
}

void WindowInfoWrap::setIsMaxVert(bool isMaxVert)
{
    m_isMaxVert = isMaxVert;
}

bool WindowInfoWrap::isMaxHoriz() const
{
    return m_isMaxHoriz;
}

void WindowInfoWrap::setIsMaxHoriz(bool isMaxHoriz)
{
    m_isMaxHoriz = isMaxHoriz;
}

bool WindowInfoWrap::isFullscreen() const
{
    return m_isFullscreen;
}

void WindowInfoWrap::setIsFullscreen(bool isFullscreen)
{
    m_isFullscreen = isFullscreen;
}

bool WindowInfoWrap::isShaded() const
{
    return m_isShaded;
}

void WindowInfoWrap::setIsShaded(bool isShaded)
{
    m_isShaded = isShaded;
}

bool WindowInfoWrap::isKeepAbove() const
{
    return m_isKeepAbove;
}

void WindowInfoWrap::setIsKeepAbove(bool isKeepAbove)
{
    m_isKeepAbove = isKeepAbove;
}

bool WindowInfoWrap::isKeepBelow() const
{
    return m_isKeepBelow;
}

void WindowInfoWrap::setIsKeepBelow(bool isKeepBelow)
{
    m_isKeepBelow = isKeepBelow;
}

bool WindowInfoWrap::hasSkipPager() const
{
    return m_hasSkipPager;
}

void WindowInfoWrap::setHasSkipPager(bool skipPager)
{
    m_hasSkipPager = skipPager;
}

bool WindowInfoWrap::hasSkipSwitcher() const
{
    return m_hasSkipSwitcher;
}

void WindowInfoWrap::setHasSkipSwitcher(bool skipSwitcher)
{
    m_hasSkipSwitcher = skipSwitcher;
}

bool WindowInfoWrap::hasSkipTaskbar() const
{
    return m_hasSkipTaskbar;
}

void WindowInfoWrap::setHasSkipTaskbar(bool skipTaskbar)
{
    m_hasSkipTaskbar = skipTaskbar;
}

bool WindowInfoWrap::isOnAllDesktops() const
{
    return m_isOnAllDesktops;
}

void WindowInfoWrap::setIsOnAllDesktops(bool alldesktops)
{
    m_isOnAllDesktops = alldesktops;
}

bool WindowInfoWrap::isOnAllActivities() const
{
    return m_isOnAllActivities;
}

void WindowInfoWrap::setIsOnAllActivities(bool allactivities)
{
    m_isOnAllActivities = allactivities;
}

//!BEGIN: Window Abilities
bool WindowInfoWrap::isCloseable() const
{
    return m_isClosable;
}
void WindowInfoWrap::setIsClosable(bool closable)
{
    m_isClosable = closable;
}

bool WindowInfoWrap::isFullScreenable() const
{
    return m_isFullScreenable;
}
void WindowInfoWrap::setIsFullScreenable(bool fullscreenable)
{
    m_isFullScreenable = fullscreenable;
}

bool WindowInfoWrap::isGroupable() const
{
    return m_isGroupable;
}
void WindowInfoWrap::setIsGroupable(bool groupable)
{
    m_isGroupable = groupable;
}

bool WindowInfoWrap::isMaximizable() const
{
    return m_isMaximizable;
}
void WindowInfoWrap::setIsMaximizable(bool maximizable)
{
    m_isMaximizable = maximizable;
}

bool WindowInfoWrap::isMinimizable() const
{
    return m_isMinimizable;
}
void WindowInfoWrap::setIsMinimizable(bool minimizable)
{
    m_isMinimizable = minimizable;
}

bool WindowInfoWrap::isMovable() const
{
    return m_isMovable;
}
void WindowInfoWrap::setIsMovable(bool movable)
{
    m_isMovable = movable;
}

bool WindowInfoWrap::isResizable() const
{
    return m_isResizable;
}
void WindowInfoWrap::setIsResizable(bool resizable)
{
    m_isResizable = resizable;
}

bool WindowInfoWrap::isShadeable() const
{
    return m_isShadeable;
}
void WindowInfoWrap::setIsShadeable(bool shadeble)
{
    m_isShadeable = shadeble;
}

bool WindowInfoWrap::isVirtualDesktopsChangeable() const
{
    return m_isVirtualDesktopsChangeable;
}
void WindowInfoWrap::setIsVirtualDesktopsChangeable(bool virtualdesktopchangeable)
{
    m_isVirtualDesktopsChangeable = virtualdesktopchangeable;
}
//!END: Window Abilities



bool WindowInfoWrap::isMainWindow() const
{
    return (m_parentId.toInt() <= 0);
}

bool WindowInfoWrap::isChildWindow() const
{
    return (m_parentId.toInt() > 0);
}


QString WindowInfoWrap::appName() const
{
    return m_appName;
}

void WindowInfoWrap::setAppName(const QString &appName)
{
    m_appName = appName;
}

QString WindowInfoWrap::display() const
{
    return m_display;
}

void WindowInfoWrap::setDisplay(const QString &display)
{
    m_display = display;
}

QIcon WindowInfoWrap::icon() const
{
    return m_icon;
}

void WindowInfoWrap::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

QRect WindowInfoWrap::geometry() const
{
    return m_geometry;
}

void WindowInfoWrap::setGeometry(const QRect &geometry)
{
    m_geometry = geometry;
}

WindowId WindowInfoWrap::wid() const
{
    return m_wid;
}

void WindowInfoWrap::setWid(const WindowId &wid)
{
    m_wid = wid;
}

WindowId WindowInfoWrap::parentId() const
{
    return m_parentId;
}

void WindowInfoWrap::setParentId(const WindowId &parentId)
{
    if (m_wid == parentId) {
        return;
    }

    m_parentId = parentId;
}

QStringList WindowInfoWrap::desktops() const
{
    return m_desktops;
}

void WindowInfoWrap::setDesktops(const QStringList &desktops)
{
    m_desktops = desktops;
}

QStringList WindowInfoWrap::activities() const
{
    return m_activities;
}

void WindowInfoWrap::setActivities(const QStringList &activities)
{
    m_activities = activities;
}

bool WindowInfoWrap::isOnDesktop(const QString &desktop) const
{
    return m_isOnAllDesktops || m_desktops.contains(desktop);
}

bool WindowInfoWrap::isOnActivity(const QString &activity) const
{
    return m_isOnAllActivities || m_activities.contains(activity);
}

}
}
