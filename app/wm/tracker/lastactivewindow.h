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

#ifndef WINDOWSYSTEMLASTACTIVEWINDOW_H
#define WINDOWSYSTEMLASTACTIVEWINDOW_H

// local
#include "../windowinfowrap.h"
#include "../abstractwindowinterface.h"

// Qt
#include <QObject>
#include <QRect>

namespace Latte {
class View;
namespace WindowSystem {
class AbstractWindowInterface;
namespace Tracker {
class TrackedGeneralInfo;
class Windows;
}
}
}

namespace Latte {
namespace WindowSystem {
namespace Tracker {

class LastActiveWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isMinimized READ isMinimized NOTIFY isMinimizedChanged)
    Q_PROPERTY(bool isMaximized READ isMaximized NOTIFY isMaximizedChanged)
    Q_PROPERTY(bool isFullScreen READ isFullScreen NOTIFY isFullScreenChanged)
    Q_PROPERTY(bool isKeepAbove READ isKeepAbove NOTIFY isKeepAboveChanged)
    Q_PROPERTY(bool isOnAllDesktops  READ isOnAllDesktops NOTIFY isOnAllDesktopsChanged)
    Q_PROPERTY(bool isShaded READ isShaded NOTIFY isShadedChanged)
    Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged)
    Q_PROPERTY(bool hasSkipTaskbar READ hasSkipTaskbar NOTIFY hasSkipTaskbarChanged)

    Q_PROPERTY(QString appName READ appName NOTIFY appNameChanged)
    Q_PROPERTY(QString display READ display NOTIFY displayChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)

    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged)

    Q_PROPERTY(QVariant winId READ winId NOTIFY winIdChanged)

public:
    LastActiveWindow(TrackedGeneralInfo *trackedInfo);
    ~LastActiveWindow() override;

    bool isActive() const;
    bool isMinimized() const;
    bool isMaximized() const;
    bool isFullScreen() const;
    bool isKeepAbove() const;
    bool isOnAllDesktops() const;
    bool isShaded() const;
    bool isValid() const;
    bool hasSkipTaskbar() const;

    QString appName() const;
    QString display() const;

    QRect geometry() const;
    QIcon icon() const;

    QVariant winId() const;

    void setInformation(const WindowInfoWrap &info);

public slots:
    Q_INVOKABLE void requestActivate();
    Q_INVOKABLE void requestClose();
    Q_INVOKABLE void requestToggleIsOnAllDesktops();
    Q_INVOKABLE void requestToggleKeepAbove();
    Q_INVOKABLE void requestToggleMinimized();
    Q_INVOKABLE void requestToggleMaximized();

    Q_INVOKABLE bool canBeDragged();

    void requestMove(Latte::View *fromView, int localX, int localY);

private slots:
    void windowChanged(const WindowId &wid);
    void windowRemoved(const WindowId &wid);


signals:
    void iconChanged();
    void isActiveChanged();
    void isMinimizedChanged();
    void isMaximizedChanged();
    void isFullScreenChanged();
    void isKeepAboveChanged();
    void isOnAllDesktopsChanged();
    void isShadedChanged();
    void isValidChanged();
    void hasSkipTaskbarChanged();

    void appNameChanged();
    void displayChanged();

    void geometryChanged();
    void winIdChanged();

private:
    void setActive(bool active);
    void setIsMinimized(bool minimized);
    void setIsMaximized(bool maximized);
    void setIsFullScreen(bool fullscreen);
    void setIsKeepAbove(bool above);
    void setIsOnAllDesktops(bool all);
    void setIsShaded(bool shaded);
    void setIsValid(bool valid);
    void setHasSkipTaskbar(bool skip);

    void setAppName(QString appName);
    void setDisplay(QString display);

    void setGeometry(QRect geometry);
    void setIcon(QIcon icon);

    void setWinId(QVariant winId);

    void clearHistory();

private:
    bool m_isActive{false};
    bool m_isMinimized{false};
    bool m_isMaximized{false};
    bool m_isFullScreen{false};
    bool m_isKeepAbove{false};
    bool m_isOnAllDesktops{false};
    bool m_isShaded{false};
    bool m_isValid{false};
    bool m_hasSkipTaskbar{false};

    QString m_appName;
    QString m_display;

    QRect m_geometry;
    QIcon m_icon;

    QVariant m_winId;

    QList<WindowId> m_history;

    TrackedGeneralInfo *m_trackedInfo{nullptr};
    AbstractWindowInterface *m_wm{nullptr};
    Tracker::Windows *m_windowsTracker{nullptr};
};

}
}
}

#endif
