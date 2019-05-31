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

#ifndef WINDOWSYSTEMTRACKEDINFO_H
#define WINDOWSYSTEMTRACKEDINFO_H

// local
#include "lastactivewindow.h"
#include "../windowinfowrap.h"

// Qt
#include <QObject>
#include <QRect>

namespace Latte {
namespace WindowSystem {
class SchemeColors;
namespace Tracker {
class Windows;
}
}
}


namespace Latte {
namespace WindowSystem {
namespace Tracker {

class TrackedInfo : public QObject {
    Q_OBJECT

    Q_PROPERTY(Latte::WindowSystem::Tracker::LastActiveWindow *activeWindow READ lastActiveWindow NOTIFY lastActiveWindowChanged)

public:
    TrackedInfo(Tracker::Windows *parent);
    ~TrackedInfo() override;

    bool enabled() const;
    void setEnabled(bool enabled);

    bool activeWindowMaximized() const;
    void setActiveWindowMaximized(bool activeMaximized);

    bool activeWindowTouching() const;
    void setActiveWindowTouching(bool touching);

    bool existsWindowActive() const;
    void setExistsWindowActive(bool exists);

    bool existsWindowMaximized() const;
    void setExistsWindowMaximized(bool maximized);

    bool existsWindowTouching() const;
    void setExistsWindowTouching(bool touching);

    QRect availableScreenGeometry() const;
    void setAvailableScreenGeometry(QRect geometry);

    LastActiveWindow *lastActiveWindow() const;

    SchemeColors *activeWindowScheme() const;
    void setActiveWindowScheme(SchemeColors *scheme);

    SchemeColors *touchingWindowScheme() const;
    void setTouchingWindowScheme(SchemeColors *scheme);

    AbstractWindowInterface *wm();

signals:
    void lastActiveWindowChanged();

private:
    bool m_enabled;
    bool m_activeWindowMaximized;
    bool m_activeWindowTouching;
    bool m_existsWindowActive;
    bool m_existsWindowMaximized;
    bool m_existsWindowTouching;

    QRect m_availableScreenGeometry;

    LastActiveWindow *m_lastActiveWindow{nullptr};

    SchemeColors *m_activeWindowScheme{nullptr};
    SchemeColors *m_touchingWindowScheme{nullptr};

    AbstractWindowInterface *m_wm{nullptr};
};

}
}
}

#endif
