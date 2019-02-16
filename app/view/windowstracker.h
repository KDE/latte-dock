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

#ifndef WINDOWSTRACKER_H
#define WINDOWSTRACKER_H

// local
#include "../schemecolors.h"
#include "../wm/abstractwindowinterface.h"
#include "../wm/windowinfowrap.h"

// Qt
#include <QObject>

namespace Latte{
    class AbstractWindowInterface;
    class Corona;
    class View;
}

namespace Latte {
namespace ViewPart {

class WindowsTracker : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool activeWindowTouching READ activeWindowTouching NOTIFY activeWindowTouchingChanged)
    Q_PROPERTY(bool existsWindowActive READ existsWindowActive NOTIFY existsWindowActiveChanged)
    Q_PROPERTY(bool existsWindowMaximized READ existsWindowMaximized NOTIFY existsWindowMaximizedChanged)
    Q_PROPERTY(bool existsWindowTouching READ existsWindowTouching NOTIFY existsWindowTouchingChanged)
    Q_PROPERTY(SchemeColors *activeWindowScheme READ activeWindowScheme NOTIFY activeWindowSchemeChanged)
    Q_PROPERTY(SchemeColors *touchingWindowScheme READ touchingWindowScheme NOTIFY touchingWindowSchemeChanged)

public:
    explicit WindowsTracker(Latte::View *parent);
    virtual ~WindowsTracker();

    bool enabled() const;
    void setEnabled(bool active);

    bool existsWindowActive() const;
    bool activeWindowTouching() const;
    bool existsWindowMaximized() const;
    bool existsWindowTouching() const;

    SchemeColors *activeWindowScheme() const;
    SchemeColors *touchingWindowScheme() const;

    void setWindowOnActivities(QWindow &window, const QStringList &activities);

signals:
    void enabledChanged();
    void activeWindowTouchingChanged();
    void existsWindowActiveChanged();
    void existsWindowMaximizedChanged();
    void existsWindowTouchingChanged();
    void activeWindowSchemeChanged();
    void touchingWindowSchemeChanged();

public slots:
    Q_INVOKABLE void requestToggleMaximizeForActiveWindow();
    Q_INVOKABLE void requestMoveActiveWindow(int localX, int localY);
    Q_INVOKABLE bool activeWindowCanBeDragged();

private:
    void setActiveWindowTouching(bool activeTouching);
    void setExistsWindowActive(bool windowActive);
    void setExistsWindowMaximized(bool windowMaximized);
    void setExistsWindowTouching(bool windowTouching);
    void setActiveWindowScheme(SchemeColors *scheme);
    void setTouchingWindowScheme(SchemeColors *scheme);
    void updateAvailableScreenGeometry();
    void updateFlags();

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! this is a garbage collector to collect such windows in order to not break the windows array validity.
    void cleanupFaultyWindows();

    bool isActiveInCurrentScreen(const WindowInfoWrap &winfo);
    bool isMaximizedInCurrentScreen(const WindowInfoWrap &winfo);
    bool isTouchingPanelEdge(const WindowInfoWrap &winfo);

private:
    bool m_enabled{false};
    bool m_activeWindowIsTouchingFlag{false};
    bool m_windowIsActiveFlag{false};
    bool m_windowIsTouchingFlag{false};
    bool m_windowIsMaximizedFlag{false};

    QRect m_availableScreenGeometry;

    WindowId m_lastActiveWindowWid;

    std::array<QMetaObject::Connection, 7> m_connections;
    QMap<WindowId, WindowInfoWrap> m_windows;

    SchemeColors *m_activeScheme{nullptr};
    SchemeColors *m_touchingScheme{nullptr};

    Latte::AbstractWindowInterface *m_wm;
    Latte::Corona *m_corona{nullptr};
    Latte::View *m_latteView{nullptr};
};

}
}

#endif
