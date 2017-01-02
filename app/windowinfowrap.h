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

#include <QObject>
#include <QWidget>
#include <QRect>

namespace Latte {

class WindowInfoWrap {

public:
    explicit WindowInfoWrap();
    WindowInfoWrap(const WindowInfoWrap &other);
    
    WindowInfoWrap &operator=(const WindowInfoWrap &rhs);
    
    bool operator==(const WindowInfoWrap &rhs) const;
    bool operator<(const WindowInfoWrap &rhs) const;
    bool operator>(const WindowInfoWrap &rhs) const;
    
    bool isValid() const;
    void setIsValid(bool isValid);
    
    bool isActive() const;
    void setIsActive(bool isActive);
    
    bool isMinimized() const;
    void setIsMinimized(bool isMinimized);
    
    bool isMaximized() const;
    void setIsMaximized(bool isMaximized);
    
    bool isFullscreen() const;
    void setIsFullscreen(bool isFullscreen);
    
    bool isPlasmaDesktop() const;
    void setIsPlasmaDesktop(bool isPlasmaDesktop);
    
    QRect geometry() const;
    void setGeometry(const QRect &geometry);
    
    WId wid() const;
    void setWid(WId wid);
    
private:
    bool m_isValid : 1;
    bool m_isActive : 1;
    bool m_isMinimized : 1;
    bool m_isMaximized : 1;
    bool m_isFullscreen : 1;
    bool m_isPlasmaDesktop : 1;
    QRect m_geometry;
    WId m_wid{0};
};

}

#endif // WINDOWINFOWRAP_H
