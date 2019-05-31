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

//local
#include "trackedinfo.h"

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
    setGeometry(info.geometry());
}

}
}
}
