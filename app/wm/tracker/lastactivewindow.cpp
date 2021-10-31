/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
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
const int PREFHISTORY = 14;
const int MAXHISTORY = 22;

LastActiveWindow::LastActiveWindow(TrackedGeneralInfo *trackedInfo)
    : QObject(trackedInfo),
      m_trackedInfo(trackedInfo),
      m_windowsTracker(trackedInfo->wm()->windowsTracker()),
      m_wm(trackedInfo->wm())
{
    connect(m_wm->schemesTracker(), &Schemes::colorSchemeChanged, this, [&](WindowId wid) {
        if (wid == m_currentWinId) {
            updateColorScheme();
        }
    });

    connect(m_windowsTracker, &Windows::applicationDataChanged, this, &LastActiveWindow::applicationDataChanged);
    connect(m_windowsTracker, &Windows::windowChanged, this, &LastActiveWindow::windowChanged);
    connect(m_windowsTracker, &Windows::windowRemoved, this, &LastActiveWindow::windowRemoved);

    connect(m_wm, &AbstractWindowInterface::currentActivityChanged, this, &LastActiveWindow::updateInformationFromHistory);
    connect(m_wm, &AbstractWindowInterface::currentDesktopChanged, this, &LastActiveWindow::updateInformationFromHistory);
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

QVariant LastActiveWindow::currentWinId() const
{
    return m_currentWinId;
}

void LastActiveWindow::setCurrentWinId(QVariant winId)
{
    if (m_currentWinId == winId) {
        return;
    }

    m_currentWinId = winId;
    emit currentWinIdChanged();
}

void LastActiveWindow::setInformation(const WindowInfoWrap &info)
{
    if (!m_trackedInfo->isTracking(info)) {
        removeFromHistory(info.wid());
        if (m_currentWinId == info.wid()) {
            updateInformationFromHistory();
        }

        return;
    }

    if (!m_trackedInfo->isShown(info)) {
        if (m_currentWinId == info.wid()) {
            updateInformationFromHistory();
        }

        return;
    }

    bool firstActiveness{false};

    if (m_currentWinId != info.wid()) {
        firstActiveness = true;
    }

    setCurrentWinId(info.wid());

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

    appendInHistory(info.wid());
    emit printRequested();
}

//! PRIVATE SLOTS
void LastActiveWindow::applicationDataChanged(const WindowId &wid)
{
    if (m_currentWinId == wid) {
        setAppName(m_windowsTracker->appNameFor(wid));
        setIcon(m_windowsTracker->iconFor(wid));
    }
}


void LastActiveWindow::windowChanged(const WindowId &wid)
{
    if (!m_trackedInfo->enabled()) {
        // qDebug() << " Last Active Window, Window Changed : TrackedInfo is disabled...";
        setIsValid(false);
        return;
    }

    if (m_history.contains(wid)) {
        WindowInfoWrap historyitem = m_windowsTracker->infoFor(wid);

        if (!m_trackedInfo->isTracking(historyitem)) {
            removeFromHistory(wid);
            if (m_currentWinId == wid) {
                updateInformationFromHistory();
            }
        } else if (!m_trackedInfo->isShown(historyitem)) {
            if (m_currentWinId == wid) {
                updateInformationFromHistory();
            }
        } else if (m_history.indexOf(wid) == 0) {
            setInformation(historyitem);
        }
    } else {
        //qDebug() << " LastActiveWindow : window is not in history";
    }
}

void LastActiveWindow::windowRemoved(const WindowId &wid)
{
    if (m_history.contains(wid)) {
        removeFromHistory(wid);
        updateInformationFromHistory();
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

void LastActiveWindow::printHistory() {
    for(int i=0; i<m_history.count(); ++i) {
        WindowInfoWrap historyitem = m_windowsTracker->infoFor(m_history[i]);
        qDebug() << "  " << i << ". " << historyitem.wid() << " -- " << historyitem.display();
    }
}

void LastActiveWindow::appendInHistory(const QVariant &wid)
{
    if (!m_history.contains(wid)) {
        m_history.prepend(wid);
        cleanHistory();
    } else {
        int windex = m_history.indexOf(wid);
        m_history.move(windex, 0); //! move to start
    }
}

void LastActiveWindow::removeFromHistory(const QVariant &wid)
{
    m_history.removeAll(wid);
}

void LastActiveWindow::updateInformationFromHistory()
{
    for(int i=0; i<m_history.count(); ++i) {
        WindowInfoWrap historyitem = m_windowsTracker->infoFor(m_history[i]);

        if (m_trackedInfo->isTracking(historyitem) && m_trackedInfo->isShown(historyitem)) {
            setInformation(historyitem);
            return;
        }
    }

    setIsValid(false);
}

void LastActiveWindow::updateColorScheme()
{
    auto scheme = m_wm->schemesTracker()->schemeForWindow(m_currentWinId);
    if (scheme) {
        setColorScheme(scheme->schemeFile());
    }
}


//! FUNCTIONALITY
void LastActiveWindow::requestActivate()
{
    m_wm->requestActivate(m_currentWinId);
}

void LastActiveWindow::requestClose()
{
    m_wm->requestClose(m_currentWinId);
}

void LastActiveWindow::requestMove(Latte::View *fromView, int localX, int localY)
{
    if (!fromView || !canBeDragged()) {
        return;
    }

    QPoint globalPoint{fromView->x() + localX, fromView->y() + localY};

    //! fixes bug #437679, Dragged windows do not become active and during drag they go behind active window
    m_wm->requestActivate(m_currentWinId);

    m_wm->requestMoveWindow(m_currentWinId, globalPoint);
    fromView->unblockMouse(localX, localY);
}

void LastActiveWindow::requestToggleIsOnAllDesktops()
{
    m_wm->requestToggleIsOnAllDesktops(m_currentWinId);
}

void LastActiveWindow::requestToggleKeepAbove()
{
    m_wm->requestToggleKeepAbove(m_currentWinId);
}

void LastActiveWindow::requestToggleMinimized()
{
    m_wm->requestToggleMinimized(m_currentWinId);
}

void LastActiveWindow::requestToggleMaximized()
{
    m_wm->requestToggleMaximized(m_currentWinId);
}

bool LastActiveWindow::canBeDragged()
{
    return m_wm->windowCanBeDragged(m_currentWinId);
}

}
}
}
