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

#ifndef WINDOWSCURRENTSCREENTRACKER_H
#define WINDOWSCURRENTSCREENTRACKER_H

// local
#include "../../wm/abstractwindowinterface.h"

// Qt
#include <QObject>

namespace Latte{
class View;

namespace ViewPart {
class WindowsTracker;
}

namespace WindowSystem {
class AbstractWindowInterface;
class SchemeColors;
namespace Tracker {
class LastActiveWindow;
}
}
}

namespace Latte {
namespace ViewPart {
namespace TrackerPart {

class CurrentScreenTracker : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool activeWindowMaximized READ activeWindowMaximized NOTIFY activeWindowMaximizedChanged)
    Q_PROPERTY(bool activeWindowTouching READ activeWindowTouching NOTIFY activeWindowTouchingChanged)
    Q_PROPERTY(bool existsWindowActive READ existsWindowActive NOTIFY existsWindowActiveChanged)
    Q_PROPERTY(bool existsWindowMaximized READ existsWindowMaximized NOTIFY existsWindowMaximizedChanged)
    Q_PROPERTY(bool existsWindowTouching READ existsWindowTouching NOTIFY existsWindowTouchingChanged)
    Q_PROPERTY(Latte::WindowSystem::SchemeColors *activeWindowScheme READ activeWindowScheme NOTIFY activeWindowSchemeChanged)
    Q_PROPERTY(Latte::WindowSystem::SchemeColors *touchingWindowScheme READ touchingWindowScheme NOTIFY touchingWindowSchemeChanged)

    Q_PROPERTY(Latte::WindowSystem::Tracker::LastActiveWindow *lastActiveWindow READ lastActiveWindow NOTIFY lastActiveWindowChanged)

public:
    explicit CurrentScreenTracker(WindowsTracker *parent);
    virtual ~CurrentScreenTracker();

    bool enabled() const;
    void setEnabled(bool active);

    bool activeWindowMaximized() const;
    bool activeWindowTouching() const;
    bool existsWindowActive() const;
    bool existsWindowMaximized() const;
    bool existsWindowTouching() const;

    WindowSystem::SchemeColors *activeWindowScheme() const;
    WindowSystem::SchemeColors *touchingWindowScheme() const;

    WindowSystem::Tracker::LastActiveWindow *lastActiveWindow();

public slots:
    Q_INVOKABLE void requestMoveLastWindow(int localX, int localY);

signals:
    void enabledChanged();
    void activeWindowDraggingStarted();
    void activeWindowMaximizedChanged();
    void activeWindowTouchingChanged();
    void existsWindowActiveChanged();
    void existsWindowMaximizedChanged();
    void existsWindowTouchingChanged();
    void activeWindowSchemeChanged();
    void touchingWindowSchemeChanged();

    void lastActiveWindowChanged();

private slots:
    void initSignalsForInformation();

private:
    void init();

    Latte::View *m_latteView{nullptr};
    WindowSystem::AbstractWindowInterface *m_wm{nullptr};
};

}
}
}

#endif
