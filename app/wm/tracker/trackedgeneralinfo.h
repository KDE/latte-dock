/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

    bool isTrackingCurrentActivity() const;
    bool isTrackingActivity(const QString &activity);

    LastActiveWindow *lastActiveWindow() const;

    SchemeColors *activeWindowScheme() const;
    void setActiveWindowScheme(SchemeColors *scheme);

    AbstractWindowInterface *wm();

    void setActiveWindow(const WindowId &wid);

    //! windows that are not shown but are tracked should not be removed from history but give their place to other shown windows
    bool isShown(const WindowInfoWrap &winfo) const;
    //! windows that are not tracked can be removed totally from history as they are not relevant any more
    virtual bool isTracking(const WindowInfoWrap &winfo) const;

signals:
    void lastActiveWindowChanged();

protected:
    void updateTrackingCurrentActivity();

protected:
    QStringList m_activities;

    LastActiveWindow *m_lastActiveWindow{nullptr};
    AbstractWindowInterface *m_wm{nullptr};
    Tracker::Windows *m_tracker{nullptr};

private:
    bool m_enabled{false};
    bool m_activeWindowMaximized{false};
    bool m_existsWindowActive{false};
    bool m_existsWindowMaximized{false};

    bool m_isTrackingCurrentActivity{true};

    SchemeColors *m_activeWindowScheme{nullptr};
};

}
}
}

#endif
