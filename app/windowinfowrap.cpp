#include "windowinfowrap.h"

namespace Latte {

WindowInfoWrap::WindowInfoWrap()
    : m_isValid(false)
    , m_isActive(false)
    , m_isMinimized(false)
    , m_isMaximized(false)
    , m_isFullscreen(false)
    , m_isPlasmaDesktop(false)
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
    m_isPlasmaDesktop = rhs.m_isPlasmaDesktop;
    m_geometry = rhs.m_geometry;
    m_wid = rhs.m_wid;
    
    return *this;
}

bool WindowInfoWrap::operator==(const WindowInfoWrap &rhs) const
{
    return m_wid == rhs.m_wid;
}

bool WindowInfoWrap::operator<(const WindowInfoWrap &rhs) const
{
    return m_wid < rhs.m_wid;
}

bool WindowInfoWrap::operator>(const WindowInfoWrap &rhs) const
{
    return m_wid > rhs.m_wid;
}

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
    return m_isMaximized;
}

void WindowInfoWrap::setIsMaximized(bool isMaximized)
{
    m_isMaximized = isMaximized;
}

bool WindowInfoWrap::isFullscreen() const
{
    return m_isFullscreen;
}

void WindowInfoWrap::setIsFullscreen(bool isFullscreen)
{
    m_isFullscreen = isFullscreen;
}

bool WindowInfoWrap::isPlasmaDesktop() const
{
    return m_isPlasmaDesktop;
}

void WindowInfoWrap::setIsPlasmaDesktop(bool isPlasmaDesktop)
{
    m_isPlasmaDesktop = isPlasmaDesktop;
}

QRect WindowInfoWrap::geometry() const
{
    return m_geometry;
}

void WindowInfoWrap::setGeometry(const QRect &geometry)
{
    m_geometry = geometry;
}

void WindowInfoWrap::setWid(WId wid)
{
    m_wid = wid;
}

WId WindowInfoWrap::wid() const
{
    return m_wid;
}

}
