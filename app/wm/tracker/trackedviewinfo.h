/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

    bool activeWindowTouchingEdge() const;
    void setActiveWindowTouchingEdge(bool touching);

    bool existsWindowTouchingEdge() const;
    void setExistsWindowTouchingEdge(bool touching);

    bool isTouchingBusyVerticalView() const;
    void setIsTouchingBusyVerticalView(bool touching);

    QRect screenGeometry() const;
    void setScreenGeometry(QRect geometry);

    SchemeColors *touchingWindowScheme() const;
    void setTouchingWindowScheme(SchemeColors *scheme);

    Latte::View *view() const;

    bool isTracking(const WindowInfoWrap &winfo) const override;

private:
    bool m_activeWindowTouching{false};
    bool m_existsWindowTouching{false};
    bool m_activeWindowTouchingEdge{false};
    bool m_existsWindowTouchingEdge{false};
    bool m_isTouchingBusyVerticalView{false};

    QRect m_screenGeometry;

    SchemeColors *m_touchingWindowScheme{nullptr};

    Latte::View *m_view{nullptr};
};

}
}
}

#endif
