/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WINDOWSALLSCREENSTRACKER_H
#define WINDOWSALLSCREENSTRACKER_H

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

class AllScreensTracker : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool activeWindowMaximized READ activeWindowMaximized NOTIFY activeWindowMaximizedChanged)
    Q_PROPERTY(bool existsWindowActive READ existsWindowActive NOTIFY existsWindowActiveChanged)
    Q_PROPERTY(bool existsWindowMaximized READ existsWindowMaximized NOTIFY existsWindowMaximizedChanged)
    Q_PROPERTY(Latte::WindowSystem::SchemeColors *activeWindowScheme READ activeWindowScheme NOTIFY activeWindowSchemeChanged)

    Q_PROPERTY(Latte::WindowSystem::Tracker::LastActiveWindow *lastActiveWindow READ lastActiveWindow NOTIFY lastActiveWindowChanged)

public:
    explicit AllScreensTracker(WindowsTracker *parent);
    virtual ~AllScreensTracker();

    bool activeWindowMaximized() const;
    bool existsWindowActive() const;
    bool existsWindowMaximized() const;

    WindowSystem::SchemeColors *activeWindowScheme() const;

    WindowSystem::Tracker::LastActiveWindow *lastActiveWindow();

public slots:
    Q_INVOKABLE void requestMoveLastWindow(int localX, int localY);

signals:
    void activeWindowMaximizedChanged();
    void existsWindowActiveChanged();
    void existsWindowMaximizedChanged();
    void activeWindowSchemeChanged();

    void lastActiveWindowChanged();

private slots:
    void initSignalsForInformation();

private:
    void init();

private:
    Latte::WindowSystem::Tracker::LastActiveWindow *m_currentLastActiveWindow{nullptr};

    Latte::View *m_latteView{nullptr};
    WindowSystem::AbstractWindowInterface *m_wm{nullptr};
};

}
}
}

#endif
