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

#ifndef WINDOWSYSTEMTRACKEDVIEWINFO_H
#define WINDOWSYSTEMTRACKEDVIEWINFO_H

// local
#include "trackedgeneralinfo.h"
#include "../windowinfowrap.h"

// Qt
#include <QObject>
#include <QRect>

namespace Latte {
class View;
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

class TrackedViewInfo : public TrackedGeneralInfo {
    Q_OBJECT

public:
    TrackedViewInfo(Tracker::Windows *tracker, Latte::View *view);
    ~TrackedViewInfo() override;

    bool activeWindowTouching() const;
    void setActiveWindowTouching(bool touching);

    bool existsWindowTouching() const;
    void setExistsWindowTouching(bool touching);

    QRect availableScreenGeometry() const;
    void setAvailableScreenGeometry(QRect geometry);

    SchemeColors *touchingWindowScheme() const;
    void setTouchingWindowScheme(SchemeColors *scheme);

    Latte::View *view() const;

    bool isTracking(const WindowInfoWrap &winfo) const override;

protected:
    bool isTrackedOnActivity(const QString &activity) const override;

private:
    bool m_activeWindowTouching;
    bool m_existsWindowTouching;

    QRect m_availableScreenGeometry;

    SchemeColors *m_touchingWindowScheme{nullptr};

    Latte::View *m_view{nullptr};
};

}
}
}

#endif
