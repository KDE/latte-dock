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

#ifndef WINDOWSYSTEMTRACKEDGENERALINFO_H
#define WINDOWSYSTEMTRACKEDGENERALINFO_H

// local
#include "lastactivewindow.h"
#include "../windowinfowrap.h"

// Qt
#include <QObject>

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

class TrackedGeneralInfo : public QObject {
    Q_OBJECT

    Q_PROPERTY(Latte::WindowSystem::Tracker::LastActiveWindow *activeWindow READ lastActiveWindow NOTIFY lastActiveWindowChanged)

public:
    TrackedGeneralInfo(Tracker::Windows *tracker);
    ~TrackedGeneralInfo() override;

    bool enabled() const;
    void setEnabled(bool enabled);

    bool activeWindowMaximized() const;
    void setActiveWindowMaximized(bool activeMaximized);

    bool existsWindowActive() const;
    void setExistsWindowActive(bool exists);

    bool existsWindowMaximized() const;
    void setExistsWindowMaximized(bool maximized);

    LastActiveWindow *lastActiveWindow() const;

    SchemeColors *activeWindowScheme() const;
    void setActiveWindowScheme(SchemeColors *scheme);

    AbstractWindowInterface *wm();

    void setActiveWindow(const WindowId &wid);

    virtual bool isTracking(const WindowInfoWrap &winfo) const;

signals:
    void lastActiveWindowChanged();

private:
    bool m_enabled;
    bool m_activeWindowMaximized;
    bool m_existsWindowActive;
    bool m_existsWindowMaximized;

    LastActiveWindow *m_lastActiveWindow{nullptr};

    SchemeColors *m_activeWindowScheme{nullptr};

    AbstractWindowInterface *m_wm{nullptr};
    Tracker::Windows *m_tracker{nullptr};
};

}
}
}

#endif
