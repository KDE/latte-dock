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

#include "lastactivewindow.h"

// local
#include "trackedinfo.h"

// Qt
#include <QDebug>

namespace Latte {
namespace WindowSystem {
namespace Tracker {


LastActiveWindow::LastActiveWindow(TrackedInfo *parent)
    : QObject(parent)
{
}

LastActiveWindow::~LastActiveWindow()
{
}

QString LastActiveWindow::display() const
{
    return m_display;
}

void LastActiveWindow::setDisplay(QString display)
{
    if (m_display == display) {
        return;
    }

    m_display = display;
    emit displayChanged();
}

bool LastActiveWindow::isActive() const
{
    return m_isActive;
}

void LastActiveWindow::setActive(bool active)
{
    if (m_isActive == active) {
        return;
    }

    m_isActive = active;
    emit isActiveChanged();
}

bool LastActiveWindow::isMinimized() const
{
    return m_isMinimized;
}

void LastActiveWindow::setIsMinimized(bool minimized)
{
    if (m_isMinimized == minimized) {
        return;
    }

    m_isMinimized = minimized;
    emit isMinimizedChanged();
}

bool LastActiveWindow::isMaximized() const
{
    return m_isMaximized;
}

void LastActiveWindow::setIsMaximized(bool maximized)
{
    if (m_isMaximized == maximized) {
        return;
    }

    m_isMaximized = maximized;
    emit isMaximizedChanged();
}

bool LastActiveWindow::isFullScreen() const
{
    return m_isFullScreen;
}

void LastActiveWindow::setIsFullScreen(bool fullscreen)
{
    if (m_isFullScreen == fullscreen) {
        return;
    }

    m_isFullScreen = fullscreen;
    emit isFullScreenChanged();
}

bool LastActiveWindow::isKeepAbove() const
{
    return m_isKeepAbove;
}

void LastActiveWindow::setIsKeepAbove(bool above)
{
    if (m_isKeepAbove == above) {
        return;
    }

    m_isKeepAbove = above;
    emit isKeepAboveChanged();
}

bool LastActiveWindow::isShaded() const
{
    return m_isShaded;
}

void LastActiveWindow::setIsShaded(bool shaded)
{
    if (m_isShaded == shaded) {
        return;
    }

    m_isShaded = shaded;
    emit isShadedChanged();
}

bool LastActiveWindow::hasSkipTaskbar() const
{
    return m_hasSkipTaskbar;
}

void LastActiveWindow::setHasSkipTaskbar(bool skip)
{
    if (m_hasSkipTaskbar == skip) {
        return;
    }

    m_hasSkipTaskbar = skip;
    emit hasSkipTaskbarChanged();
}

QRect LastActiveWindow::geometry() const
{
    return m_geometry;
}

void LastActiveWindow::setGeometry(QRect geometry)
{
    if (m_geometry == geometry) {
        return;
    }

    m_geometry = geometry;
    emit geometryChanged();
}

QVariant LastActiveWindow::winId() const
{
    return m_winId;
}

void LastActiveWindow::setWinId(QVariant winId)
{
    if (m_winId == winId) {
        return;
    }

    m_winId = winId;
    emit winIdChanged();
}

void LastActiveWindow::setInformation(const WindowInfoWrap &info)
{
    setWinId(info.wid());

    setActive(info.isActive());
    setIsMinimized(info.isMinimized());
    setIsMaximized(info.isMaxVert() || info.isMaxHoriz());

    setDisplay(info.display());
    setGeometry(info.geometry());
}

}
}
}
