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
#include "windows.h"
#include "../abstractwindowinterface.h"
#include "../tasktools.h"
#include "../../view/view.h"
#include "../../view/visibilitymanager.h"

// Qt
#include <QDebug>
#include <QPoint>
#include <QTimer>


namespace Latte {
namespace WindowSystem {
namespace Tracker {

LastActiveWindow::LastActiveWindow(TrackedInfo *trackedInfo)
    : QObject(trackedInfo),
      m_trackedInfo(trackedInfo),
      m_windowsTracker(trackedInfo->wm()->windowsTracker()),
      m_wm(trackedInfo->wm())
{
}

LastActiveWindow::~LastActiveWindow()
{
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

QString LastActiveWindow::appName() const
{
    return m_appName;
}

void LastActiveWindow::setAppName(QString appName)
{
    if (m_appName == appName) {
        return;
    }

    m_appName = appName;
    emit appNameChanged();
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

QIcon LastActiveWindow::icon() const
{
    return m_icon;
}

void LastActiveWindow::setIcon(QIcon icon)
{
    m_icon = icon;
    emit iconChanged();
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

    setAppName(info.appName());
    setDisplay(info.display());
    setGeometry(info.geometry());

    if (info.appName().isEmpty()) {
        setAppName(m_windowsTracker->appNameFor(info.wid()));
    } else {
        setAppName(info.appName());
    }

    if (info.icon().isNull()) {
        setIcon(m_windowsTracker->iconFor(info.wid()));
    } else {
        setIcon(info.icon());
    }

}

//! FUNCTIONALITY
void LastActiveWindow::requestToggleMaximize()
{
    m_wm->requestToggleMaximized(m_winId);
}

void LastActiveWindow::requestMove(int localX, int localY)
{
    QPoint globalPoint{m_trackedInfo->view()->x() + localX, m_trackedInfo->view()->y() + localY};

    m_wm->requestMoveWindow(m_winId, globalPoint);

    //! This timer is needed because otherwise the mouse position
    //! in the dragged window changes to TopLeft corner
    QTimer::singleShot(250, this, [&]() {
        m_wm->releaseMouseEventFor(m_trackedInfo->view()->winId());
    });

    emit draggingStarted();
}

bool LastActiveWindow::canBeDragged()
{
    return m_wm->windowCanBeDragged(m_winId);
}

}
}
}
