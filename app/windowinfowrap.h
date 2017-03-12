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

#ifndef WINDOWINFOWRAP_H
#define WINDOWINFOWRAP_H

#include <QWindow>
#include <QRect>

namespace Latte {

class WindowInfoWrap {

public:
    constexpr WindowInfoWrap()
        : m_isValid(false)
        , m_isActive(false)
        , m_isMinimized(false)
        , m_isMaxVert(false)
        , m_isMaxHorz(false)
        , m_isFullscreen(false)
        , m_isPlasmaDesktop(false)
        , m_wid(0) {
    }

    constexpr WindowInfoWrap(const WindowInfoWrap &other)
        : m_isValid(other.m_isValid)
        , m_isActive(other.m_isActive)
        , m_isMinimized(other.m_isMinimized)
        , m_isMaxVert(other.m_isMaxVert)
        , m_isMaxHorz(other.m_isMaxHorz)
        , m_isFullscreen(other.m_isFullscreen)
        , m_isPlasmaDesktop(other.m_isPlasmaDesktop)
        , m_wid(other.m_wid) {
    }

    inline WindowInfoWrap &operator=(const WindowInfoWrap &rhs);
    constexpr bool operator==(const WindowInfoWrap &rhs) const;
    constexpr bool operator<(const WindowInfoWrap &rhs) const;
    constexpr bool operator>(const WindowInfoWrap &rhs) const;

    constexpr bool isValid() const;
    inline void setIsValid(bool isValid);

    constexpr bool isActive() const;
    inline void setIsActive(bool isActive);

    constexpr bool isMinimized() const;
    inline void setIsMinimized(bool isMinimized);

    constexpr bool isMaximized() const;

    constexpr bool isMaxVert() const;
    inline void setIsMaxVert(bool isMaxVert);

    constexpr bool isMaxHoriz() const;
    inline void setIsMaxHoriz(bool isMaxHoriz);

    constexpr bool isFullscreen() const;
    inline void setIsFullscreen(bool isFullscreen);

    constexpr bool isPlasmaDesktop() const;
    inline void setIsPlasmaDesktop(bool isPlasmaDesktop);

    constexpr QRect geometry() const;
    inline void setGeometry(const QRect &geometry);

    constexpr WId wid() const;
    inline void setWid(WId wid);

private:
    bool m_isValid : 1;
    bool m_isActive : 1;
    bool m_isMinimized : 1;
    bool m_isMaxVert : 1;
    bool m_isMaxHorz : 1;
    bool m_isFullscreen : 1;
    bool m_isPlasmaDesktop : 1;
    WId m_wid;
    QRect m_geometry;
};

// BEGIN: definitions

inline WindowInfoWrap &WindowInfoWrap::operator=(const WindowInfoWrap &rhs)
{
    m_isValid = rhs.m_isValid;
    m_isActive = rhs.m_isActive;
    m_isMinimized = rhs.m_isMinimized;
    m_isMaxVert = rhs.m_isMaxVert;
    m_isMaxHorz = rhs.m_isMaxHorz;
    m_isFullscreen = rhs.m_isFullscreen;
    m_isPlasmaDesktop = rhs.m_isPlasmaDesktop;
    m_wid = rhs.m_wid;
    m_geometry = rhs.m_geometry;
    return *this;
}

constexpr bool WindowInfoWrap::operator==(const WindowInfoWrap &rhs) const
{
    return m_wid == rhs.m_wid;
}

constexpr bool WindowInfoWrap::operator<(const WindowInfoWrap &rhs) const
{
    return m_wid < rhs.m_wid;
}

constexpr bool WindowInfoWrap::operator>(const WindowInfoWrap &rhs) const
{
    return m_wid > rhs.m_wid;
}

constexpr bool WindowInfoWrap::isValid() const
{
    return m_isValid;
}

inline void WindowInfoWrap::setIsValid(bool isValid)
{
    m_isValid = isValid;
}

constexpr bool WindowInfoWrap::isActive() const
{
    return m_isActive;
}

inline void WindowInfoWrap::setIsActive(bool isActive)
{
    m_isActive = isActive;
}

constexpr bool WindowInfoWrap::isMinimized() const
{
    return m_isMinimized;
}

inline void WindowInfoWrap::setIsMinimized(bool isMinimized)
{
    m_isMinimized = isMinimized;
}

constexpr bool WindowInfoWrap::isMaximized() const
{
    return m_isMaxVert || m_isMaxHorz;
}

constexpr bool WindowInfoWrap::isMaxVert() const
{
    return m_isMaxVert;
}

inline void WindowInfoWrap::setIsMaxVert(bool isMaxVert)
{
    m_isMaxVert = isMaxVert;
}

constexpr bool WindowInfoWrap::isMaxHoriz() const
{
    return m_isMaxHorz;
}

inline void WindowInfoWrap::setIsMaxHoriz(bool isMaxHoriz)
{
    m_isMaxHorz = isMaxHoriz;
}

constexpr bool WindowInfoWrap::isFullscreen() const
{
    return m_isFullscreen;
}

inline void WindowInfoWrap::setIsFullscreen(bool isFullscreen)
{
    m_isFullscreen = isFullscreen;
}

constexpr bool WindowInfoWrap::isPlasmaDesktop() const
{
    return m_isPlasmaDesktop;
}

inline void WindowInfoWrap::setIsPlasmaDesktop(bool isPlasmaDesktop)
{
    m_isPlasmaDesktop = isPlasmaDesktop;
}

constexpr QRect WindowInfoWrap::geometry() const
{
    return m_geometry;
}

inline void WindowInfoWrap::setGeometry(const QRect &geometry)
{
    m_geometry = geometry;
}

constexpr WId WindowInfoWrap::wid() const
{
    return m_wid;
}

inline void WindowInfoWrap::setWid(WId wid)
{
    m_wid = wid;
}

// END: definitions
}

#endif // WINDOWINFOWRAP_H
