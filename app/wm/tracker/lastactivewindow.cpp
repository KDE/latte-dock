/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lastactivewindow.h"

// local
#include "schemes.h"
#include "trackedgeneralinfo.h"
#include "windowstracker.h"
#include "../abstractwindowinterface.h"
#include "../tasktools.h"
#include "../../view/view.h"

// Qt
#include <QDebug>
#include <QHoverEvent>
#include <QPoint>
#include <QTimer>


namespace Latte {
namespace WindowSystem {
namespace Tracker {

const int INVALIDWID = -1;
const int PREFHISTORY = 8;
const int MAXHISTORY = 15;

LastActiveWindow::LastActiveWindow(TrackedGeneralInfo *trackedInfo)
    : QObject(trackedInfo),
      m_trackedInfo(trackedInfo),
      m_windowsTracker(trackedInfo->wm()->windowsTracker()),
      m_wm(trackedInfo->wm())
{
    connect(m_wm->schemesTracker(), &Schemes::colorSchemeChanged, this, [&](WindowId wid) {
        if (wid == m_winId) {
            updateColorScheme();
        }
    });

    connect(m_windowsTracker, &Windows::applicationDataChanged, this, &LastActiveWindow::applicationDataChanged);
    connect(m_windowsTracker, &Windows::windowChanged, this, &LastActiveWindow::windowChanged);
    connect(m_windowsTracker, &Windows::windowRemoved, this, &LastActiveWindow::windowRemoved);
}

LastActiveWindow::~LastActiveWindow()
{
}

bool LastActiveWindow::isActive() const
{
    return m_isActive;
}

void LastActiveWindow::setActive(bool active)
{
    if (m_isActive == active) {
        return;
    }

    m_isActive = active;
    emit isActiveChanged();
}

bool LastActiveWindow::isMinimized() const
{
    return m_isMinimized;
}

void LastActiveWindow::setIsMinimized(bool minimized)
{
    if (m_isMinimized == minimized) {
        return;
    }

    m_isMinimized = minimized;
    emit isMinimizedChanged();
}

bool LastActiveWindow::isMaximized() const
{
    return m_isMaximized;
}

void LastActiveWindow::setIsMaximized(bool maximized)
{
    if (m_isMaximized == maximized) {
        return;
    }

    m_isMaximized = maximized;
    emit isMaximizedChanged();
}

bool LastActiveWindow::isFullScreen() const
{
    return m_isFullScreen;
}

void LastActiveWindow::setIsFullScreen(bool fullscreen)
{
    if (m_isFullScreen == fullscreen) {
        return;
    }

    m_isFullScreen = fullscreen;
    emit isFullScreenChanged();
}

bool LastActiveWindow::isKeepAbove() const
{
    return m_isKeepAbove;
}

void LastActiveWindow::setIsKeepAbove(bool above)
{
    if (m_isKeepAbove == above) {
        return;
    }

    m_isKeepAbove = above;
    emit isKeepAboveChanged();
}

bool LastActiveWindow::isOnAllDesktops() const
{
    return m_isOnAllDesktops;
}

void LastActiveWindow::setIsOnAllDesktops(bool all)
{
    if (m_isOnAllDesktops == all) {
        return;
    }

    m_isOnAllDesktops = all;
    emit isOnAllDesktopsChanged();
}

bool LastActiveWindow::isShaded() const
{
    return m_isShaded;
}

void LastActiveWindow::setIsShaded(bool shaded)
{
    if (m_isShaded == shaded) {
        return;
    }

    m_isShaded = shaded;
    emit isShadedChanged();
}

bool LastActiveWindow::isValid() const
{
    return m_isValid;
}

void LastActiveWindow::setIsValid(bool valid)
{
    if (m_isValid == valid) {
        return;
    }

    m_isValid = valid;
    emit isValidChanged();
}

bool LastActiveWindow::hasSkipTaskbar() const
{
    return m_hasSkipTaskbar;
}

void LastActiveWindow::setHasSkipTaskbar(bool skip)
{
    if (m_hasSkipTaskbar == skip) {
        return;
    }

    m_hasSkipTaskbar = skip;
    emit hasSkipTaskbarChanged();
}

//! BEGIN: Window Abitilities
bool LastActiveWindow::isClosable() const
{
    return m_isClosable;
}

void LastActiveWindow::setIsClosable(bool closable)
{
    if (m_isClosable == closable) {
        return;
    }

    m_isClosable = closable;
    emit isClosableChanged();
}

bool LastActiveWindow::isFullScreenable() const
{
    return m_isFullScreenable;
}

void LastActiveWindow::setIsFullScreenable(bool fullscreenable)
{
    if (m_isFullScreenable == fullscreenable) {
        return;
    }

    m_isFullScreenable = fullscreenable;
    emit isFullScreenableChanged();
}

bool LastActiveWindow::isGroupable() const
{
    return m_isGroupable;
}

void LastActiveWindow::setIsGroupable(bool groupable)
{
    if (m_isGroupable == groupable) {
        return;
    }

    m_isGroupable = groupable;
    emit isGroupableChanged();
}


bool LastActiveWindow::isMaximizable() const
{
    return m_isMaximizable;
}

void LastActiveWindow::setIsMaximizable(bool maximizable)
{
    if (m_isMaximizable == maximizable) {
        return;
    }

    m_isMaximizable = maximizable;
    emit isMaximizableChanged();
}

bool LastActiveWindow::isMinimizable() const
{
    return m_isMinimizable;
}

void LastActiveWindow::setIsMinimizable(bool minimizable)
{
    if (m_isMinimizable == minimizable) {
        return;
    }

    m_isMinimizable = minimizable;
    emit isMinimizableChanged();
}

bool LastActiveWindow::isMovable() const
{
    return m_isMovable;
}

void LastActiveWindow::setIsMovable(bool movable)
{
    if (m_isMovable == movable) {
        return;
    }

    m_isMovable = movable;
    emit isMovableChanged();
}

bool LastActiveWindow::isResizable() const
{
    return m_isResizable;
}

void LastActiveWindow::setIsResizable(bool resizable)
{
    if (m_isResizable == resizable) {
        return;
    }

    m_isResizable = resizable;
    emit isResizableChanged();
}

bool LastActiveWindow::isShadeable() const
{
    return m_isShadeable;
}

void LastActiveWindow::setIsShadeable(bool shadeable)
{
    if (m_isShadeable == shadeable) {
        return;
    }

    m_isShadeable = shadeable;
    emit isShadeableChanged();
}

bool LastActiveWindow::isVirtualDesktopChangeable() const
{
    return m_isVirtualDesktopsChangeable;
}

void LastActiveWindow::setIsVirtualDesktopsChangeable(bool virtualdestkopschangeable)
{
    if (m_isVirtualDesktopsChangeable == virtualdestkopschangeable) {
        return;
    }

    m_isVirtualDesktopsChangeable = virtualdestkopschangeable;
    emit isVirtualDesktopChangeableChanged();
}
//! END: Window Abitilities


QRect LastActiveWindow::geometry() const
{
    return m_geometry;
}

void LastActiveWindow::setGeometry(QRect geometry)
{
    if (m_geometry == geometry) {
        return;
    }

    m_geometry = geometry;
    emit geometryChanged();
}

QString LastActiveWindow::appName() const
{
    return m_appName;
}

void LastActiveWindow::setAppName(QString appName)
{
    if (m_appName == appName) {
        return;
    }

    m_appName = appName;
    emit appNameChanged();
}

QString LastActiveWindow::colorScheme() const
{
    return m_colorScheme;
}

void LastActiveWindow::setColorScheme(QString scheme)
{
    if (m_colorScheme == scheme){
        return;
    }

    m_colorScheme = scheme;
    emit colorSchemeChanged();
}

QString LastActiveWindow::display() const
{
    return m_display;
}

void LastActiveWindow::setDisplay(QString display)
{
    if (m_display == display) {
        return;
    }

    m_display = display;
    emit displayChanged();
}

QIcon LastActiveWindow::icon() const
{
    return m_icon;
}

void LastActiveWindow::setIcon(QIcon icon)
{
    m_icon = icon;
    emit iconChanged();
}

QVariant LastActiveWindow::winId() const
{
    return m_winId;
}

void LastActiveWindow::setWinId(QVariant winId)
{
    if (m_winId == winId && isValid()) {
        return;
    }

    if (!m_history.contains(winId)) {
        m_history.prepend(winId);
        cleanHistory();
    } else {
        int p = m_history.indexOf(winId);
        //! move to start
        m_history.move(p, 0);
    }

    m_winId = winId;
    emit winIdChanged();
}

void LastActiveWindow::setInformation(const WindowInfoWrap &info)
{
    bool isIgnored = info.hasSkipTaskbar() && (info.hasSkipPager() || info.hasSkipSwitcher());

    if (isIgnored) {
        if (m_history.contains(info.wid())) {
            windowRemoved(info.wid());
        }
        return;
    }

    bool firstActiveness{false};

    if (m_winId != info.wid()) {
        firstActiveness = true;
    }

    setWinId(info.wid());

    setIsValid(true);
    setActive(info.isActive());
    setIsMinimized(info.isMinimized());
    setIsMaximized(info.isMaximized());
    setIsOnAllDesktops(info.isOnAllDesktops());

    //! Window Abilities
    setIsClosable(info.isCloseable());
    setIsFullScreenable(info.isFullScreenable());
    setIsGroupable(info.isGroupable());
    setIsMaximizable(info.isMaximizable());
    setIsMinimizable(info.isMinimizable());
    setIsMovable(info.isMovable());
    setIsResizable(info.isResizable());
    setIsShadeable(info.isShadeable());
    setIsVirtualDesktopsChangeable(info.isVirtualDesktopsChangeable());
    //! Window Abilities

    setAppName(info.appName());
    setDisplay(info.display());
    setGeometry(info.geometry());
    setIsKeepAbove(info.isKeepAbove());

    if (firstActiveness) {
        updateColorScheme();
    }

    if (info.appName().isEmpty()) {
        setAppName(m_windowsTracker->appNameFor(info.wid()));
    } else {
        setAppName(info.appName());
    }

    if (info.icon().isNull()) {
        setIcon(m_windowsTracker->iconFor(info.wid()));
    } else {
        setIcon(info.icon());
    }
}

//! PRIVATE SLOTS
void LastActiveWindow::applicationDataChanged(const WindowId &wid)
{
    if (m_winId == wid) {
        setAppName(m_windowsTracker->appNameFor(wid));
        setIcon(m_windowsTracker->iconFor(wid));
    }
}


void LastActiveWindow::windowChanged(const WindowId &wid)
{
    if (!m_trackedInfo->enabled()) {
        // qDebug() << " Last Active Window, Window Changed : TrackedInfo is disabled...";
        return;
    }

    if (m_history.contains(wid)) {
        //! remove from history minimized windows or windows that changed screen
        //! and update information accordingly with the first valid window found from
        //! history after the removal
        WindowInfoWrap winfo = m_windowsTracker->infoFor(wid);

        bool firstItemRemoved{false};

        //! Remove minimized windows OR NOT-TRACKED windows from history
        if (winfo.isMinimized() || !m_trackedInfo->isTracking(winfo)) {
            if (m_history[0] == wid) {
                firstItemRemoved = true;
            }

            m_history.removeAll(wid);
            cleanHistory();
        }

        if (m_history.count() > 0) {
            if (m_history[0] == wid || firstItemRemoved) {
                WindowInfoWrap history1 = m_windowsTracker->infoFor(m_history[0]);

                //! Check if first found History window is still valid to show its information
                if (history1.isMinimized() || !m_trackedInfo->isTracking(history1)) {
                    windowChanged(m_history[0]);
                } else {
                    setInformation(history1);
                }
            }
        } else {
            //! History is empty so any demonstrated information are invalid
            setIsValid(false);
        }

        //qDebug() << " HISTORY ::: " << m_history;
    } else {
        //qDebug() << " LastActiveWindow : window is not in history";
    }
}

void LastActiveWindow::windowRemoved(const WindowId &wid)
{
    if (m_history.contains(wid)) {
        bool firstItemRemoved{false};

        if (m_history.count() > 0 && m_history[0] == wid) {
            firstItemRemoved = true;
        }

        m_history.removeAll(wid);
        m_history.removeAll(wid);

        if (m_history.count() > 0 && firstItemRemoved) {
            windowChanged(m_history[0]);
        } else {
            setIsValid(false);
        }
    }
}

void LastActiveWindow::cleanHistory()
{
    if (m_history.count() > MAXHISTORY) {
        int size = m_history.count();
        for(int i=0; i<(size-PREFHISTORY); ++i) {
            if (!m_history.isEmpty()) {
                m_history.removeLast();
            }
        }
    }
}

void LastActiveWindow::updateColorScheme()
{
    auto scheme = m_wm->schemesTracker()->schemeForWindow(m_winId);
    if (scheme) {
        setColorScheme(scheme->schemeFile());
    }
}


//! FUNCTIONALITY
void LastActiveWindow::requestActivate()
{
    m_wm->requestActivate(m_winId);
}

void LastActiveWindow::requestClose()
{
    m_wm->requestClose(m_winId);
}

void LastActiveWindow::requestMove(Latte::View *fromView, int localX, int localY)
{
    if (!fromView || !canBeDragged()) {
        return;
    }

    QPoint globalPoint{fromView->x() + localX, fromView->y() + localY};
    m_wm->requestMoveWindow(m_winId, globalPoint);

    fromView->unblockMouse(localX, localY);
}

void LastActiveWindow::requestToggleIsOnAllDesktops()
{
    m_wm->requestToggleIsOnAllDesktops(m_winId);
}

void LastActiveWindow::requestToggleKeepAbove()
{
    m_wm->requestToggleKeepAbove(m_winId);
}

void LastActiveWindow::requestToggleMinimized()
{
    m_wm->requestToggleMinimized(m_winId);
}

void LastActiveWindow::requestToggleMaximized()
{
    m_wm->requestToggleMaximized(m_winId);
}

bool LastActiveWindow::canBeDragged()
{
    return m_wm->windowCanBeDragged(m_winId);
}

}
}
}
