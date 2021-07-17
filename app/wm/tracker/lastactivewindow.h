/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged)

    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isMinimized READ isMinimized NOTIFY isMinimizedChanged)
    Q_PROPERTY(bool isMaximized READ isMaximized NOTIFY isMaximizedChanged)
    Q_PROPERTY(bool isFullScreen READ isFullScreen NOTIFY isFullScreenChanged)
    Q_PROPERTY(bool isKeepAbove READ isKeepAbove NOTIFY isKeepAboveChanged)
    Q_PROPERTY(bool isOnAllDesktops  READ isOnAllDesktops NOTIFY isOnAllDesktopsChanged)
    Q_PROPERTY(bool isShaded READ isShaded NOTIFY isShadedChanged)
    Q_PROPERTY(bool hasSkipTaskbar READ hasSkipTaskbar NOTIFY hasSkipTaskbarChanged)

    //! BEGIN: Window Abitilities
    /*since Latte v0.9.8*/
    Q_PROPERTY(bool isClosable READ isClosable NOTIFY isClosableChanged)
    Q_PROPERTY(bool isFullScreenable READ isFullScreenable NOTIFY isFullScreenableChanged)
    Q_PROPERTY(bool isGroupable READ isGroupable NOTIFY isGroupableChanged)
    Q_PROPERTY(bool isMaximizable READ isMaximizable NOTIFY isMaximizableChanged)
    Q_PROPERTY(bool isMinimizable READ isMinimizable NOTIFY isMinimizableChanged)
    Q_PROPERTY(bool isMovable READ isMovable NOTIFY isMovableChanged)
    Q_PROPERTY(bool isResizable READ isResizable NOTIFY isResizableChanged)
    Q_PROPERTY(bool isShadeable READ isShadeable NOTIFY isShadeableChanged)
    Q_PROPERTY(bool isVirtualDesktopChangeable READ isVirtualDesktopChangeable NOTIFY isVirtualDesktopChangeableChanged)
    //! END: Window Abitilities

    /*since Latte v0.9.4*/
    Q_PROPERTY(QString colorScheme READ colorScheme NOTIFY colorSchemeChanged)

    Q_PROPERTY(QString appName READ appName NOTIFY appNameChanged)
    Q_PROPERTY(QString display READ display NOTIFY displayChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)

    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged)

    Q_PROPERTY(QVariant winId READ currentWinId NOTIFY currentWinIdChanged)

public:
    LastActiveWindow(TrackedGeneralInfo *trackedInfo);
    ~LastActiveWindow() override;

    bool isValid() const;
    bool isActive() const;
    bool isMinimized() const;
    bool isMaximized() const;
    bool isFullScreen() const;
    bool isKeepAbove() const;
    bool isOnAllDesktops() const;
    bool isShaded() const;
    bool hasSkipTaskbar() const;

    //! BEGIN: Window Abitilities
    /*since Latte v0.9.8*/
    bool isClosable() const;
    bool isFullScreenable() const;
    bool isGroupable() const;
    bool isMaximizable() const;
    bool isMinimizable() const;
    bool isMovable() const;
    bool isResizable() const;
    bool isShadeable() const;
    bool isVirtualDesktopChangeable() const;
    //! END: Window Abitilities

    QString appName() const;
    QString colorScheme() const;
    QString display() const;    

    QRect geometry() const;
    QIcon icon() const;

    QVariant currentWinId() const;

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

    //! Debug
    void printHistory();

private slots:
    void updateInformationFromHistory();

    void applicationDataChanged(const WindowId &wid);

    void windowChanged(const WindowId &wid);
    void windowRemoved(const WindowId &wid);

signals:
    void colorSchemeChanged();
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

    //! BEGIN: Window Abitilities
    /*since Latte v0.9.8*/
    void isClosableChanged();
    void isFullScreenableChanged();
    void isGroupableChanged();
    void isMaximizableChanged();
    void isMinimizableChanged();
    void isMovableChanged();
    void isResizableChanged();
    void isShadeableChanged();
    void isVirtualDesktopChangeableChanged();
    //! END: Window Abitilities

    void appNameChanged();
    void displayChanged();

    void geometryChanged();
    void currentWinIdChanged();

    //! Debug
    void printRequested();

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

    //! BEGIN: Window Abitilities
    /*since Latte v0.9.8*/
    void setIsClosable(bool closable);
    void setIsFullScreenable(bool fullscreenable);
    void setIsGroupable(bool groupable);
    void setIsMaximizable(bool maximizable);
    void setIsMinimizable(bool minimizable);
    void setIsMovable(bool movable);
    void setIsResizable(bool resizable);
    void setIsShadeable(bool shadeable);
    void setIsVirtualDesktopsChangeable(bool virtualdestkopschangeable);
    //! END: Window Abitilities

    void setColorScheme(QString scheme);

    void setAppName(QString appName);
    void setDisplay(QString display);

    void setGeometry(QRect geometry);
    void setIcon(QIcon icon);

    void setCurrentWinId(QVariant winId);

    void cleanHistory();
    void appendInHistory(const QVariant &wid);
    void removeFromHistory(const QVariant &wid);

    void updateColorScheme();

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

    //! BEGIN: Window Abitilities
    /*since Latte v0.9.8*/
    bool m_isClosable{true};
    bool m_isFullScreenable{true};
    bool m_isGroupable{true};
    bool m_isMaximizable{true};
    bool m_isMinimizable{true};
    bool m_isMovable{true};
    bool m_isResizable{true};
    bool m_isShadeable{true};
    bool m_isVirtualDesktopsChangeable{true};
    //! END: Window Abitilities

    QString m_colorScheme;

    QString m_appName;
    QString m_display;

    QRect m_geometry;
    QIcon m_icon;

    QVariant m_currentWinId;

    QList<WindowId> m_history;

    TrackedGeneralInfo *m_trackedInfo{nullptr};
    AbstractWindowInterface *m_wm{nullptr};
    Tracker::Windows *m_windowsTracker{nullptr};
};

}
}
}

#endif
