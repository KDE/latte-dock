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

#include "windowstracker.h"

// local
#include "positioner.h"
#include "view.h"
#include "../lattecorona.h"
#include "../layouts/manager.h"
#include "../wm/schemecolors.h"
#include "../wm/tracker/windows.h"
#include "../wm/tracker/lastactivewindow.h"
#include "../../liblatte2/types.h"

namespace Latte {
namespace ViewPart {

WindowsTracker::WindowsTracker(Latte::View *parent)
    : QObject(parent),
      m_latteView(parent)
{
    qDebug() << "WindowsTracker creating...";

    m_corona = qobject_cast<Latte::Corona *>(m_latteView->corona());
    m_wm = m_corona->wm();

    init();
    m_wm->windowsTracker()->addView(m_latteView);

    emit lastActiveWindowChanged();
}

WindowsTracker::~WindowsTracker()
{
    qDebug() << "WindowsTracker removing...";
    m_wm->windowsTracker()->removeView(m_latteView);
}

void  WindowsTracker::init()
{
    connect(m_wm->windowsTracker(), &WindowSystem::Tracker::Windows::enabledChanged, this, [&](const Latte::View *view) {
        if (m_latteView == view) {
            emit enabledChanged();
        }
    });

    connect(m_wm->windowsTracker(), &WindowSystem::Tracker::Windows::activeWindowMaximizedChanged, this, [&](const Latte::View *view) {
        if (m_latteView == view) {
            emit activeWindowMaximizedChanged();
        }
    });

    connect(m_wm->windowsTracker(), &WindowSystem::Tracker::Windows::activeWindowTouchingChanged, this, [&](const Latte::View *view) {
        if (m_latteView == view) {
            emit activeWindowTouchingChanged();
        }
    });

    connect(m_wm->windowsTracker(), &WindowSystem::Tracker::Windows::existsWindowActiveChanged, this, [&](const Latte::View *view) {
        if (m_latteView == view) {
            emit existsWindowActiveChanged();
        }
    });

    connect(m_wm->windowsTracker(), &WindowSystem::Tracker::Windows::existsWindowMaximizedChanged, this, [&](const Latte::View *view) {
        if (m_latteView == view) {
            emit existsWindowMaximizedChanged();
        }
    });

    connect(m_wm->windowsTracker(), &WindowSystem::Tracker::Windows::existsWindowTouchingChanged, this, [&](const Latte::View *view) {
        if (m_latteView == view) {
            emit existsWindowTouchingChanged();
        }
    });

    connect(m_wm->windowsTracker(), &WindowSystem::Tracker::Windows::activeWindowSchemeChanged, this, [&](const Latte::View *view) {
        if (m_latteView == view) {
            emit activeWindowSchemeChanged();
        }
    });

    connect(m_wm->windowsTracker(), &WindowSystem::Tracker::Windows::touchingWindowSchemeChanged, this, [&](const Latte::View *view) {
        if (m_latteView == view) {
            emit touchingWindowSchemeChanged();
        }
    });
}

bool WindowsTracker::activeWindowMaximized() const
{
    return m_wm->windowsTracker()->activeWindowMaximized(m_latteView);
}

bool WindowsTracker::activeWindowTouching() const
{
    return m_wm->windowsTracker()->activeWindowTouching(m_latteView);
}

bool WindowsTracker::existsWindowActive() const
{
    return m_wm->windowsTracker()->existsWindowActive(m_latteView);
}

bool WindowsTracker::existsWindowMaximized() const
{
    return m_wm->windowsTracker()->existsWindowMaximized(m_latteView);
}

bool WindowsTracker::existsWindowTouching() const
{
    return m_wm->windowsTracker()->existsWindowTouching(m_latteView);
}

WindowSystem::SchemeColors *WindowsTracker::activeWindowScheme() const
{
    return m_wm->windowsTracker()->activeWindowScheme(m_latteView);
}

WindowSystem::SchemeColors *WindowsTracker::touchingWindowScheme() const
{
    return m_wm->windowsTracker()->touchingWindowScheme(m_latteView);
}

bool WindowsTracker::enabled() const
{
    return m_wm->windowsTracker()->enabled(m_latteView);
}

void WindowsTracker::setEnabled(bool active)
{
    m_wm->windowsTracker()->setEnabled(m_latteView, active);
}

WindowSystem::Tracker::LastActiveWindow *WindowsTracker::lastActiveWindow()
{
    return m_wm->windowsTracker()->lastActiveWindow(m_latteView);
}


//! Window Functions
void WindowsTracker::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    m_wm->setWindowOnActivities(window, activities);
}

void WindowsTracker::requestToggleMaximizeForActiveWindow()
{
    Latte::WindowSystem::Tracker::LastActiveWindow *actInfo = m_wm->windowsTracker()->lastActiveWindow(m_latteView);

    //active window can be toggled only when it is in the same screen
    if (actInfo && !actInfo->geometry().isNull() && m_latteView->screenGeometry().contains(actInfo->geometry().center())) {
        m_wm->requestToggleMaximized(actInfo->winId());
    }
}

void WindowsTracker::requestMoveActiveWindow(int localX, int localY)
{
    Latte::WindowSystem::Tracker::LastActiveWindow *actInfo = m_wm->windowsTracker()->lastActiveWindow(m_latteView);

    //active window can be dragged only when it is in the same screen
    if (actInfo && !actInfo->geometry().isNull() && m_latteView->screenGeometry().contains(actInfo->geometry().center())) {
        QPoint globalPoint{m_latteView->x() + localX, m_latteView->y() + localY};

        m_wm->requestMoveWindow(actInfo->winId(), globalPoint);

        //! This timer is needed because otherwise the mouse position
        //! in the dragged window changes to TopLeft corner
        QTimer::singleShot(250, this, [&]() {
            m_wm->releaseMouseEventFor(m_latteView->winId());
        });

        emit activeWindowDraggingStarted();
    }
}

bool WindowsTracker::activeWindowCanBeDragged()
{
    Latte::WindowSystem::Tracker::LastActiveWindow *actInfo = m_wm->windowsTracker()->lastActiveWindow(m_latteView);

    //active window can be dragged only when it is in the same screen
    if (actInfo && !actInfo->geometry().isNull() && m_latteView->screenGeometry().contains(actInfo->geometry().center())) {
        return m_wm->windowCanBeDragged(actInfo->winId());
    }

    return false;
}

}
}
