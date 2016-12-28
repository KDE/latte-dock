#include "windowinfowrap.h"

using namespace Latte;

WindowInfoWrap::WindowInfoWrap()
    : m_isValid(false)
    , m_isActive(false)
    , m_isMinimized(false)
    , m_isMaximized(false)
    , m_isFullscreen(false)
    , m_isOnCurrentDesktop(false)
    , m_wid(0)
{

}

WindowInfoWrap::WindowInfoWrap(const WindowInfoWrap &other)
{
    *this = other;
}

WindowInfoWrap &WindowInfoWrap::operator=(const WindowInfoWrap &rhs)
{
    m_isValid = rhs.m_isValid;
    m_isActive = rhs.m_isActive;
    m_isMinimized = rhs.m_isMinimized;
    m_isMaximized = rhs.m_isMaximized;
    m_isFullscreen = rhs.m_isFullscreen;
    m_isOnCurrentDesktop = rhs.m_isOnCurrentDesktop;
    m_geometry = rhs.m_geometry;
    m_wid = rhs.m_wid;
    
    return *this;
}

inline bool WindowInfoWrap::operator==(const WindowInfoWrap &rhs) const
{
    return m_wid == rhs.m_wid;
}

inline bool WindowInfoWrap::operator<(const WindowInfoWrap &rhs) const
{
    return m_wid < rhs.m_wid;
}

inline bool WindowInfoWrap::operator>(const WindowInfoWrap &rhs) const
{
    return m_wid > rhs.m_wid;
}

inline bool WindowInfoWrap::isValid() const
{
    return m_isValid;
}

inline void WindowInfoWrap::setIsValid(bool isValid)
{
    m_isValid = isValid;
}

inline bool WindowInfoWrap::isActive() const
{
    return m_isActive;
}

inline void WindowInfoWrap::setIsActive(bool isActive)
{
    m_isActive = isActive;
}

inline bool WindowInfoWrap::isMinimized() const
{
    return m_isMinimized;
}

inline void WindowInfoWrap::setIsMinimized(bool isMinimized)
{
    m_isMinimized = isMinimized;
}

inline bool WindowInfoWrap::isMaximized() const
{
    return m_isMaximized;
}

inline void WindowInfoWrap::setIsMaximized(bool isMaximized)
{
    m_isMaximized = isMaximized;
}

inline bool WindowInfoWrap::isFullscreen() const
{
    return m_isFullscreen;
}

inline void WindowInfoWrap::setIsFullscreen(bool isFullscreen)
{
    m_isFullscreen = isFullscreen;
}

inline bool WindowInfoWrap::isOnCurrentDesktop() const
{
    return m_isOnCurrentDesktop;
}

inline void WindowInfoWrap::setIsOnCurrentDesktop(bool isOnCurrentDesktop)
{
    m_isOnCurrentDesktop = isOnCurrentDesktop;
}

inline QRect WindowInfoWrap::geometry() const
{
    return m_geometry;
}

inline void WindowInfoWrap::setGeometry(const QRect &geometry)
{
    m_geometry = geometry;
}

inline WId WindowInfoWrap::wid() const
{
    return m_wid;
}

inline void WindowInfoWrap::setWid(WId wid)
{
    m_wid = wid;
}

