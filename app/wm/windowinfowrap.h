/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WINDOWINFOWRAP_H
#define WINDOWINFOWRAP_H

// Qt
#include <QWindow>
#include <QIcon>
#include <QRect>
#include <QVariant>

namespace Latte {
namespace WindowSystem {

using WindowId = QVariant;

class WindowInfoWrap
{

public:
    WindowInfoWrap();
    WindowInfoWrap(const WindowInfoWrap &o);
    WindowInfoWrap(WindowInfoWrap &&o);

    WindowInfoWrap &operator=(WindowInfoWrap &&rhs);
    WindowInfoWrap &operator=(const WindowInfoWrap &rhs);

    bool isValid() const;
    void setIsValid(bool isValid);

    bool isActive() const;
    void setIsActive(bool isActive);

    bool isMinimized() const;
    void setIsMinimized(bool isMinimized);

    bool isMaximized() const;

    bool isMaxVert() const;
    void setIsMaxVert(bool isMaxVert);

    bool isMaxHoriz() const;
    void setIsMaxHoriz(bool isMaxHoriz);

    bool isFullscreen() const;
    void setIsFullscreen(bool isFullscreen);

    bool isShaded() const;
    void setIsShaded(bool isShaded);

    bool isKeepAbove() const;
    void setIsKeepAbove(bool isKeepAbove);

    bool isKeepBelow() const;
    void setIsKeepBelow(bool isKeepBelow);

    bool hasSkipPager() const;
    void setHasSkipPager(bool skipPager);

    bool hasSkipSwitcher() const;
    void setHasSkipSwitcher(bool skipSwitcher);

    bool hasSkipTaskbar() const;
    void setHasSkipTaskbar(bool skipTaskbar);

    bool isOnAllDesktops() const;
    void setIsOnAllDesktops(bool alldesktops);

    bool isOnAllActivities() const;
    void setIsOnAllActivities(bool allactivities);

    //!BEGIN: Window Abilities
    bool isCloseable() const;
    void setIsClosable(bool closable);

    bool isFullScreenable() const;
    void setIsFullScreenable(bool fullscreenable);

    bool isGroupable() const;
    void setIsGroupable(bool groupable);

    bool isMaximizable() const;
    void setIsMaximizable(bool maximizable);

    bool isMinimizable() const;
    void setIsMinimizable(bool minimizable);

    bool isMovable() const;
    void setIsMovable(bool movable);

    bool isResizable() const;
    void setIsResizable(bool resizable);

    bool isShadeable() const;
    void setIsShadeable(bool shadeble);

    bool isVirtualDesktopsChangeable() const;
    void setIsVirtualDesktopsChangeable(bool virtualdesktopchangeable);
    //!END: Window Abilities

    bool isMainWindow() const;
    bool isChildWindow() const;

    QRect geometry() const;
    void setGeometry(const QRect &geometry);

    QString appName() const;
    void setAppName(const QString &appName);

    QString display() const;
    void setDisplay(const QString &display);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    WindowId wid() const;
    void setWid(const WindowId &wid);

    WindowId parentId() const;
    void setParentId(const WindowId &parentId);

    QStringList desktops() const;
    void setDesktops(const QStringList &desktops);

    QStringList activities() const;
    void setActivities(const QStringList &activities);

    bool isOnDesktop(const QString &desktop) const;
    bool isOnActivity(const QString &activity) const;

private:
    WindowId m_wid{0};
    WindowId m_parentId{0};

    QRect m_geometry;

    bool m_isValid{false};
    bool m_isActive{false};
    bool m_isMinimized{false};
    bool m_isMaxVert{false};
    bool m_isMaxHoriz{false};
    bool m_isFullscreen{false};
    bool m_isShaded{false};
    bool m_isKeepAbove{false};
    bool m_isKeepBelow{false};
    bool m_hasSkipPager{false};
    bool m_hasSkipSwitcher{false};
    bool m_hasSkipTaskbar{false};
    bool m_isOnAllDesktops{false};
    bool m_isOnAllActivities{false};

    //!BEGIN: Window Abilities
    bool m_isClosable{false};
    bool m_isFullScreenable{false};
    bool m_isGroupable{false};
    bool m_isMaximizable{false};
    bool m_isMinimizable{false};
    bool m_isMovable{false};
    bool m_isResizable{false};
    bool m_isShadeable{false};
    bool m_isVirtualDesktopsChangeable{false};
    //!END: Window Abilities

    QString m_appName;
    QString m_display;

    QIcon m_icon;

    QStringList m_desktops;
    QStringList m_activities;
};

}
}

#endif // WINDOWINFOWRAP_H
