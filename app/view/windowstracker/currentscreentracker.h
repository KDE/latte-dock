/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    Q_PROPERTY(bool activeWindowMaximized READ activeWindowMaximized NOTIFY activeWindowMaximizedChanged)
    Q_PROPERTY(bool activeWindowTouching READ activeWindowTouching NOTIFY activeWindowTouchingChanged)
    Q_PROPERTY(bool activeWindowTouchingEdge READ activeWindowTouchingEdge NOTIFY activeWindowTouchingEdgeChanged)
    Q_PROPERTY(bool existsWindowActive READ existsWindowActive NOTIFY existsWindowActiveChanged)
    Q_PROPERTY(bool existsWindowMaximized READ existsWindowMaximized NOTIFY existsWindowMaximizedChanged)
    Q_PROPERTY(bool existsWindowTouching READ existsWindowTouching NOTIFY existsWindowTouchingChanged)
    Q_PROPERTY(bool existsWindowTouchingEdge READ existsWindowTouchingEdge NOTIFY existsWindowTouchingEdgeChanged)
    Q_PROPERTY(bool isTouchingBusyVerticalView READ isTouchingBusyVerticalView NOTIFY isTouchingBusyVerticalViewChanged)
    Q_PROPERTY(Latte::WindowSystem::SchemeColors *activeWindowScheme READ activeWindowScheme NOTIFY activeWindowSchemeChanged)
    Q_PROPERTY(Latte::WindowSystem::SchemeColors *touchingWindowScheme READ touchingWindowScheme NOTIFY touchingWindowSchemeChanged)

    Q_PROPERTY(Latte::WindowSystem::Tracker::LastActiveWindow *lastActiveWindow READ lastActiveWindow NOTIFY lastActiveWindowChanged)

public:
    explicit CurrentScreenTracker(WindowsTracker *parent);
    virtual ~CurrentScreenTracker();

    bool activeWindowMaximized() const;
    bool activeWindowTouching() const;
    bool activeWindowTouchingEdge() const;
    bool existsWindowActive() const;
    bool existsWindowMaximized() const;
    bool existsWindowTouching() const;
    bool existsWindowTouchingEdge() const;

    bool isTouchingBusyVerticalView() const;

    WindowSystem::SchemeColors *activeWindowScheme() const;
    WindowSystem::SchemeColors *touchingWindowScheme() const;

    WindowSystem::Tracker::LastActiveWindow *lastActiveWindow();

public slots:
    Q_INVOKABLE void requestMoveLastWindow(int localX, int localY);

signals:
    void activeWindowMaximizedChanged();
    void activeWindowTouchingChanged();
    void activeWindowTouchingEdgeChanged();
    void existsWindowActiveChanged();
    void existsWindowMaximizedChanged();
    void existsWindowTouchingChanged();
    void existsWindowTouchingEdgeChanged();
    void isTouchingBusyVerticalViewChanged();

    void activeWindowSchemeChanged();
    void touchingWindowSchemeChanged();

    void lastActiveWindowChanged();

private slots:
    void initSignalsForInformation();

private:
    void init();

private:
    Latte::View *m_latteView{nullptr};
    WindowSystem::AbstractWindowInterface *m_wm{nullptr};
};

}
}
}

#endif
