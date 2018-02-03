/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "visibilitymanager.h"
#include "visibilitymanager_p.h"

#include "dockview.h"
#include "../dockcorona.h"
#include "../layoutmanager.h"
#include "../windowinfowrap.h"
#include "../../liblattedock/extras.h"

#include <QDebug>

namespace Latte {

//! BEGIN: VisiblityManagerPrivate implementation
VisibilityManagerPrivate::VisibilityManagerPrivate(PlasmaQuick::ContainmentView *view, VisibilityManager *q)
    : QObject(nullptr), q(q), view(view), wm(&WindowSystem::self())
{
    dockView = qobject_cast<DockView *>(view);
    dockCorona = qobject_cast<DockCorona *>(view->corona());

    if (dockView) {
        connect(dockView, &DockView::eventTriggered, this, &VisibilityManagerPrivate::viewEventManager);
        connect(dockView, &DockView::absGeometryChanged, this, &VisibilityManagerPrivate::setDockGeometry);
    }

    timerStartUp.setInterval(5000);
    timerStartUp.setSingleShot(true);
    timerCheckWindows.setInterval(350);
    timerCheckWindows.setSingleShot(true);
    timerShow.setSingleShot(true);
    timerHide.setSingleShot(true);
    connect(&timerCheckWindows, &QTimer::timeout, this, &VisibilityManagerPrivate::checkAllWindows);
    connect(&timerShow, &QTimer::timeout, this, [this]() {
        if (isHidden) {
            //   qDebug() << "must be shown";
            emit this->q->mustBeShown(VisibilityManager::QPrivateSignal{});
        }
    });
    connect(&timerHide, &QTimer::timeout, this, [this]() {
        if (!blockHiding && !isHidden && !dragEnter) {
            //   qDebug() << "must be hide";
            emit this->q->mustBeHide(VisibilityManager::QPrivateSignal{});
        }
    });
    wm->setDockExtraFlags(*view);
    wm->addDock(view->winId());
    restoreConfig();
}

VisibilityManagerPrivate::~VisibilityManagerPrivate()
{
    qDebug() << "VisibilityManagerPrivate deleting...";
    wm->removeDockStruts(*view);
    wm->removeDock(view->winId());
}

inline void VisibilityManagerPrivate::setMode(Dock::Visibility mode)
{
    if (this->mode == mode)
        return;

    Q_ASSERT_X(mode != Dock::None, q->staticMetaObject.className(), "set visibility to Dock::None");

    // clear mode
    for (auto &c : connections) {
        disconnect(c);
    }

    if (mode != Dock::DodgeAllWindows && !enabledDynamicBackgroundFlag) {
        windows.clear();
    }

    if (this->mode == Dock::AlwaysVisible) {
        wm->removeDockStruts(*view);
    } else {
        connections[3] = connect(wm, &WindowSystem::currentDesktopChanged
        , this, [&] {
            if (raiseOnDesktopChange)
                raiseDockTemporarily();
        });
        connections[4] = connect(wm, &WindowSystem::currentActivityChanged
        , this, [&]() {
            if (raiseOnActivityChange)
                raiseDockTemporarily();
            else
                updateHiddenState();
        });
    }

    timerShow.stop();
    timerHide.stop();
    timerCheckWindows.stop();
    this->mode = mode;

    switch (this->mode) {
        case Dock::AlwaysVisible: {
            if (view->containment() && !view->containment()->isUserConfiguring() && view->screen()) {
                updateStrutsBasedOnLayoutsAndActivities();
            }

            connections[0] = connect(view->containment(), &Plasma::Containment::locationChanged
            , this, [&]() {
                if (view->containment()->isUserConfiguring())
                    wm->removeDockStruts(*view);
            });
            connections[1] = connect(view->containment(), &Plasma::Containment::userConfiguringChanged
            , this, [&](bool configuring) {
                if (!configuring && view->screen())
                    wm->setDockStruts(*view, dockGeometry, view->containment()->location());
            });


            if (dockCorona && dockCorona->layoutManager()->memoryUsage() == Dock::MultipleLayouts) {
                connections[2] = connect(dockCorona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, [&]() {
                    updateStrutsBasedOnLayoutsAndActivities();
                });

                connections[3] = connect(dockView, &DockView::activitiesChanged, this, [&]() {
                    updateStrutsBasedOnLayoutsAndActivities();
                });
            }

            raiseDock(true);
        }
        break;

        case Dock::AutoHide: {
            raiseDock(containsMouse);
        }
        break;

        case Dock::DodgeActive: {
            connections[0] = connect(wm, &WindowSystem::activeWindowChanged
                                     , this, &VisibilityManagerPrivate::dodgeActive);
            connections[1] = connect(wm, &WindowSystem::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeActive);
            dodgeActive(wm->activeWindow());
        }
        break;

        case Dock::DodgeMaximized: {
            connections[0] = connect(wm, &WindowSystem::activeWindowChanged
                                     , this, &VisibilityManagerPrivate::dodgeMaximized);
            connections[1] = connect(wm, &WindowSystem::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeMaximized);
            dodgeMaximized(wm->activeWindow());
        }
        break;

        case Dock::DodgeAllWindows: {
            for (const auto &wid : wm->windows()) {
                windows.insert(wid, wm->requestInfo(wid));
            }

            connections[0] = connect(wm, &WindowSystem::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeWindows);
            connections[1] = connect(wm, &WindowSystem::windowRemoved
            , this, [&](WindowId wid) {
                windows.remove(wid);
                timerCheckWindows.start();
            });
            connections[2] = connect(wm, &WindowSystem::windowAdded
            , this, [&](WindowId wid) {
                windows.insert(wid, wm->requestInfo(wid));
                timerCheckWindows.start();
            });

            timerCheckWindows.start();
        }
        break;

        case Dock::WindowsGoBelow:
            break;

        default:
            break;
    }

    view->containment()->config().writeEntry("visibility", static_cast<int>(mode));

    emit q->modeChanged();
}

void VisibilityManagerPrivate::updateStrutsBasedOnLayoutsAndActivities()
{
    bool multipleLayoutsAndCurrent = (dockCorona->layoutManager()->memoryUsage() == Dock::MultipleLayouts
                                      && dockView->managedLayout()
                                      && dockView->managedLayout()->name() == dockCorona->layoutManager()->currentLayoutName());

    if (dockCorona->layoutManager()->memoryUsage() == Dock::SingleLayout || multipleLayoutsAndCurrent) {
        wm->setDockStruts(*view, dockGeometry, view->location());
    } else {
        wm->removeDockStruts(*view);
    }
}

void VisibilityManagerPrivate::setRaiseOnDesktop(bool enable)
{
    if (enable == raiseOnDesktopChange)
        return;

    raiseOnDesktopChange = enable;
    emit q->raiseOnDesktopChanged();
}

void VisibilityManagerPrivate::setRaiseOnActivity(bool enable)
{
    if (enable == raiseOnActivityChange)
        return;

    raiseOnActivityChange = enable;
    emit q->raiseOnActivityChanged();
}

inline void VisibilityManagerPrivate::setIsHidden(bool isHidden)
{
    if (this->isHidden == isHidden)
        return;

    if (blockHiding && isHidden) {
        qWarning() << "isHidden property is blocked, ignoring update";
        return;
    }

    this->isHidden = isHidden;
    emit q->isHiddenChanged();
}

void VisibilityManagerPrivate::setBlockHiding(bool blockHiding)
{
    if (this->blockHiding == blockHiding)
        return;

    this->blockHiding = blockHiding;
    // qDebug() << "blockHiding:" << blockHiding;

    if (this->blockHiding) {
        timerHide.stop();

        if (isHidden) {
            emit q->mustBeShown(VisibilityManager::QPrivateSignal{});
        }
    } else {
        updateHiddenState();
    }

    emit q->blockHidingChanged();
}

inline void VisibilityManagerPrivate::setTimerShow(int msec)
{
    timerShow.setInterval(msec);
    emit q->timerShowChanged();
}

inline void VisibilityManagerPrivate::setTimerHide(int msec)
{
    timerHide.setInterval(msec);
    emit q->timerHideChanged();
}

inline void VisibilityManagerPrivate::raiseDock(bool raise)
{
    if (blockHiding)
        return;

    if (raise) {
        timerHide.stop();

        if (!timerShow.isActive()) {
            timerShow.start();
        }
    } else if (!dragEnter) {
        timerShow.stop();

        if (hideNow) {
            hideNow = false;
            emit q->mustBeHide(VisibilityManager::QPrivateSignal{});
        } else if (!timerHide.isActive())
            timerHide.start();
    }
}

void VisibilityManagerPrivate::raiseDockTemporarily()
{
    if (raiseTemporarily)
        return;

    raiseTemporarily = true;
    timerHide.stop();
    timerShow.stop();

    if (isHidden)
        emit q->mustBeShown(VisibilityManager::QPrivateSignal{});

    QTimer::singleShot(qBound(1800, 2 * timerHide.interval(), 3000), this, [&]() {
        raiseTemporarily = false;
        hideNow = true;
        updateHiddenState();
    });
}

void VisibilityManagerPrivate::updateHiddenState()
{
    if (dragEnter)
        return;

    switch (mode) {
        case Dock::AutoHide:
            raiseDock(containsMouse);
            break;

        case Dock::DodgeActive:
            dodgeActive(wm->activeWindow());
            break;

        case Dock::DodgeMaximized:
            dodgeMaximized(wm->activeWindow());
            break;

        case Dock::DodgeAllWindows:
            dodgeWindows(wm->activeWindow());
            break;

        default:
            break;
    }
}

inline void VisibilityManagerPrivate::setDockGeometry(const QRect &geometry)
{
    if (!view->containment())
        return;

    this->dockGeometry = geometry;

    if (mode == Dock::AlwaysVisible && !view->containment()->isUserConfiguring() && view->screen()) {
        updateStrutsBasedOnLayoutsAndActivities();
    }
}

void VisibilityManagerPrivate::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    wm->setWindowOnActivities(window, activities);
}

void VisibilityManagerPrivate::dodgeActive(WindowId wid)
{
    if (raiseTemporarily)
        return;

    //!dont send false raiseDock signal when containing mouse
    if (containsMouse) {
        raiseDock(true);
        return;
    }

    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid())
        return;

    if (!winfo.isActive()) {
        if (winfo.isPlasmaDesktop())
            raiseDock(true);

        winfo = wm->requestInfo(wm->activeWindow());
    }

    //!dont send false raiseDock signal when containing mouse
    if (wm->isOnCurrentDesktop(wid) && wm->isOnCurrentActivity(wid)) {
        raiseDock(!intersects(winfo));
    }
}

void VisibilityManagerPrivate::dodgeMaximized(WindowId wid)
{
    if (raiseTemporarily)
        return;

    //!dont send false raiseDock signal when containing mouse
    if (containsMouse) {
        raiseDock(true);
        return;
    }

    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid())
        return;

    if (!winfo.isActive()) {
        if (winfo.isPlasmaDesktop())
            raiseDock(true);

        winfo = wm->requestInfo(wm->activeWindow());
    }

    auto isMaxVert = [&]() noexcept -> bool {
        return winfo.isMaxVert()
               || (view->screen() && view->screen()->availableSize().height() <= winfo.geometry().height()
                   && intersects(winfo));
    };

    auto isMaxHoriz = [&]() noexcept -> bool {
        return winfo.isMaxHoriz()
               || (view->screen() && view->screen()->availableSize().width() <= winfo.geometry().width()
                   && intersects(winfo));
    };

    //!dont send false raiseDock signal when containing mouse
    if (wm->isOnCurrentDesktop(wid) && wm->isOnCurrentActivity(wid))
        raiseDock(view->formFactor() == Plasma::Types::Vertical
                  ? !isMaxHoriz() : !isMaxVert());
}

void VisibilityManagerPrivate::dodgeWindows(WindowId wid)
{
    if (raiseTemporarily)
        return;

    if (windows.find(wid) == std::end(windows))
        return;

    //!dont send false raiseDock signal when containing mouse
    if (containsMouse) {
        raiseDock(true);
        return;
    }

    windows[wid] = wm->requestInfo(wid);
    auto &winfo = windows[wid];

    if (!winfo.isValid() || !wm->isOnCurrentDesktop(wid) || !wm->isOnCurrentActivity(wid))
        return;

    if (intersects(winfo))
        raiseDock(false);
    else
        timerCheckWindows.start();
}

void VisibilityManagerPrivate::checkAllWindows()
{
    if (raiseTemporarily)
        return;

    bool raise{true};
    bool existsFaultyWindow{false};

    for (const auto &winfo : windows) {
        // <WindowId, WindowInfoWrap>
        if (winfo.geometry() == QRect(0, 0, 0, 0)) {
            existsFaultyWindow = true;
        }

        if (!winfo.isValid() || !wm->isOnCurrentDesktop(winfo.wid()) || !wm->isOnCurrentActivity(winfo.wid()))
            continue;

        if (winfo.isFullscreen()) {
            raise = false;
            break;
        } else if (intersects(winfo)) {
            raise = false;
            break;
        }
    }

    cleanupFaultyWindows();
    raiseDock(raise);
}

inline bool VisibilityManagerPrivate::intersects(const WindowInfoWrap &winfo)
{
    return (!winfo.isMinimized()
            && winfo.geometry().intersects(dockGeometry)
            && !winfo.isShaded());
}

inline void VisibilityManagerPrivate::saveConfig()
{
    if (!view->containment())
        return;

    auto config = view->containment()->config();

    config.writeEntry("timerShow", timerShow.interval());
    config.writeEntry("timerHide", timerHide.interval());
    config.writeEntry("raiseOnDesktopChange", raiseOnDesktopChange);
    config.writeEntry("raiseOnActivityChange", raiseOnActivityChange);
    view->containment()->configNeedsSaving();
}

inline void VisibilityManagerPrivate::restoreConfig()
{
    if (!view->containment())
        return;

    auto config = view->containment()->config();
    timerShow.setInterval(config.readEntry("timerShow", 200));
    timerHide.setInterval(config.readEntry("timerHide", 700));
    emit q->timerShowChanged();
    emit q->timerHideChanged();

    setRaiseOnDesktop(config.readEntry("raiseOnDesktopChange", false));
    setRaiseOnActivity(config.readEntry("raiseOnActivityChange", false));

    auto mode = [&]() {
        return static_cast<Dock::Visibility>(view->containment()->config()
                                             .readEntry("visibility", static_cast<int>(Dock::DodgeActive)));
    };

    if (mode() == Dock::AlwaysVisible) {
        setMode(Dock::AlwaysVisible);
    } else {
        connect(&timerStartUp, &QTimer::timeout, this, [ &, mode]() {
            setMode(mode());
        });
        connect(view->containment(), &Plasma::Containment::userConfiguringChanged
        , this, [&](bool configuring) {
            if (configuring && timerStartUp.isActive())
                timerStartUp.start(100);
        });

        timerStartUp.start();
    }

    connect(view->containment(), &Plasma::Containment::userConfiguringChanged
    , this, [&](bool configuring) {
        if (!configuring)
            saveConfig();
    });
}

void VisibilityManagerPrivate::viewEventManager(QEvent *ev)
{
    switch (ev->type()) {
        case QEvent::Enter:
            if (containsMouse)
                break;

            containsMouse = true;
            emit q->containsMouseChanged();

            if (mode != Dock::AlwaysVisible)
                raiseDock(true);

            break;

        case QEvent::Leave:
            if (!containsMouse)
                break;

            containsMouse = false;
            emit q->containsMouseChanged();
            updateHiddenState();
            break;

        case QEvent::DragEnter:
            dragEnter = true;

            if (isHidden)
                emit q->mustBeShown(VisibilityManager::QPrivateSignal{});

            break;

        case QEvent::DragLeave:
        case QEvent::Drop:
            dragEnter = false;
            updateHiddenState();
            break;

        case QEvent::Show:
            wm->setDockExtraFlags(*view);
            break;

        default:
            break;
    }
}

void VisibilityManagerPrivate::cleanupFaultyWindows()
{
    foreach (auto key, windows.keys()) {
        auto winfo = windows[key];

        //! garbage windows removing
        if (winfo.geometry() == QRect(0, 0, 0, 0)) {
            //qDebug() << "Faulty Geometry ::: " << winfo.wid();
            windows.remove(key);
        }
    }
}

//! Dynamic Background functions
void VisibilityManagerPrivate::setEnabledDynamicBackground(bool active)
{
    if (enabledDynamicBackgroundFlag == active) {
        return;
    }

    enabledDynamicBackgroundFlag = active;

    if (active) {
        if (mode != Dock::DodgeAllWindows) {
            for (const auto &wid : wm->windows()) {
                windows.insert(wid, wm->requestInfo(wid));
            }
        }

        connectionsDynBackground[0] = connect(view->corona(), &Plasma::Corona::availableScreenRectChanged,
                                              this, &VisibilityManagerPrivate::updateAvailableScreenGeometry);

        connectionsDynBackground[1] = connect(wm, &WindowSystem::windowChanged, this, [&](WindowId wid) {
            windows[wid] = wm->requestInfo(wid);
            updateDynamicBackgroundWindowFlags();
        });

        connectionsDynBackground[2] = connect(wm, &WindowSystem::windowRemoved, this, [&](WindowId wid) {
            windows.remove(wid);
        });

        connectionsDynBackground[3] = connect(wm, &WindowSystem::windowAdded, this, [&](WindowId wid) {
            windows.insert(wid, wm->requestInfo(wid));
            updateDynamicBackgroundWindowFlags();
        });

        connectionsDynBackground[4] = connect(wm, &WindowSystem::activeWindowChanged, this, [&](WindowId wid) {
            if (windows.contains(lastActiveWindowWid)) {
                windows[lastActiveWindowWid] = wm->requestInfo(lastActiveWindowWid);
            }

            windows[wid] = wm->requestInfo(wid);
            lastActiveWindowWid = wid;

            updateDynamicBackgroundWindowFlags();
        });

        connectionsDynBackground[5] = connect(wm, &WindowSystem::currentDesktopChanged, this, [&] {
            updateDynamicBackgroundWindowFlags();
        });

        connectionsDynBackground[6] = connect(wm, &WindowSystem::currentActivityChanged, this, [&] {
            updateDynamicBackgroundWindowFlags();
        });

        updateAvailableScreenGeometry();
        updateDynamicBackgroundWindowFlags();
    } else {
        // clear mode
        for (auto &c : connectionsDynBackground) {
            disconnect(c);
        }

        if (mode != Dock::DodgeAllWindows) {
            windows.clear();
        }

        setExistsWindowMaximized(false);
        setExistsWindowSnapped(false);
    }

    emit q->enabledDynamicBackgroundChanged();
}

void VisibilityManagerPrivate::setExistsWindowMaximized(bool windowMaximized)
{
    if (windowIsMaximizedFlag == windowMaximized) {
        return;
    }

    windowIsMaximizedFlag = windowMaximized;

    emit q->existsWindowMaximizedChanged();
}

void VisibilityManagerPrivate::setExistsWindowSnapped(bool windowSnapped)
{
    if (windowIsSnappedFlag == windowSnapped) {
        return;
    }

    windowIsSnappedFlag = windowSnapped;

    emit q->existsWindowSnappedChanged();
}

void VisibilityManagerPrivate::updateAvailableScreenGeometry()
{
    if (!view || !view->containment()) {
        return;
    }

    QRect tempAvailableScreenGeometry = dockCorona->availableScreenRectWithCriteria(view->containment()->screen(), {Dock::AlwaysVisible}, {});

    if (tempAvailableScreenGeometry != availableScreenGeometry) {
        availableScreenGeometry = tempAvailableScreenGeometry;

        snappedWindowsGeometries.clear();

        //! for top dock the snapped geometries would be
        int halfWidth1 = std::floor(availableScreenGeometry.width() / 2);
        int halfWidth2 = availableScreenGeometry.width() - halfWidth1;
        int halfHeight1 = std::floor((availableScreenGeometry.height()) / 2);
        int halfHeight2 = availableScreenGeometry.height() - halfHeight1;

        int x1 = availableScreenGeometry.x();
        int x2 = availableScreenGeometry.x() + halfWidth1;
        int y1 = availableScreenGeometry.y();
        int y2 = availableScreenGeometry.y() + halfHeight1;

        QRect snap1;
        QRect snap2;
        QRect snap3;
        QRect snap4;

        if (view->formFactor() == Plasma::Types::Horizontal) {
            if (view->location() == Plasma::Types::TopEdge) {
                snap1 = QRect(x1, y1, halfWidth1, halfHeight1);
                snap3 = QRect(x2, y1, halfWidth2, halfHeight1);
            } else if ((view->location() == Plasma::Types::BottomEdge)) {
                snap1 = QRect(x1, y2, halfWidth1, halfHeight2);
                snap3 = QRect(x2, y2, halfWidth2, halfHeight2);
            }

            snap2 = QRect(x1, y1, halfWidth1, availableScreenGeometry.height());
            snap4 = QRect(x2, y1, halfWidth2, availableScreenGeometry.height());
        } else if (view->formFactor() == Plasma::Types::Vertical) {
            QRect snap5;

            if (view->location() == Plasma::Types::LeftEdge) {
                snap1 = QRect(x1, y1, halfWidth1, halfHeight1);
                snap3 = QRect(x1, y2, halfWidth1, halfHeight2);
                snap5 = QRect(x1, y1, halfWidth1, availableScreenGeometry.height());
            } else if ((view->location() == Plasma::Types::RightEdge)) {
                snap1 = QRect(x2, y1, halfWidth2, halfHeight1);
                snap3 = QRect(x2, y2, halfWidth2, halfHeight2);
                snap5 = QRect(x2, y1, halfWidth2, availableScreenGeometry.height());
            }

            snap2 = QRect(x1, y1, availableScreenGeometry.width(), halfHeight1);
            snap4 = QRect(x1, y2, availableScreenGeometry.width(), halfHeight2);

            snappedWindowsGeometries.append(snap5);
        }

        snappedWindowsGeometries.append(snap1);
        snappedWindowsGeometries.append(snap2);
        snappedWindowsGeometries.append(snap3);
        snappedWindowsGeometries.append(snap4);

        updateDynamicBackgroundWindowFlags();
    }
}

void VisibilityManagerPrivate::updateDynamicBackgroundWindowFlags()
{
    bool foundSnap{false};
    bool foundMaximized{false};

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! maybe a garbage collector here is a good idea!!!
    bool existsFaultyWindow{false};

    for (const auto &winfo : windows) {
        if (winfo.isValid() && !winfo.isMinimized() && wm->isOnCurrentDesktop(winfo.wid()) && wm->isOnCurrentActivity(winfo.wid())) {
            if (winfo.isMaximized()) {
                foundMaximized = true;
            }

            bool touchingPanelEdge{false};

            if (view->location() == Plasma::Types::TopEdge) {
                touchingPanelEdge = (winfo.geometry().y() == availableScreenGeometry.y());
            } else if (view->location() == Plasma::Types::BottomEdge) {
                touchingPanelEdge = (winfo.geometry().bottom() == availableScreenGeometry.bottom());
            } else if (view->location() == Plasma::Types::LeftEdge) {
                touchingPanelEdge = (winfo.geometry().x() == availableScreenGeometry.x());
            } else if (view->location() == Plasma::Types::RightEdge) {
                touchingPanelEdge = (winfo.geometry().right() == availableScreenGeometry.right());
            }

            if (((winfo.isActive() || winfo.isKeepAbove()) && touchingPanelEdge)
                || (!winfo.isActive() && snappedWindowsGeometries.contains(winfo.geometry()))) {
                foundSnap = true;
            }
        }

        if (winfo.geometry() == QRect(0, 0, 0, 0)) {
            existsFaultyWindow = true;
        }

        //qDebug() << "window geometry ::: " << winfo.geometry();
    }

    if (existsFaultyWindow) {
        cleanupFaultyWindows();
    }

    /*if (!foundMaximized && !foundSnap) {
        qDebug() << "SCREEN GEOMETRY : " << availableScreenGeometry;
        qDebug() << "SNAPS ::: " << snappedWindowsGeometries;
    }

    qDebug() << " FOUND ::: " << foundMaximized << foundSnap;*/

    setExistsWindowMaximized(foundMaximized);
    setExistsWindowSnapped(foundSnap);
}
//! END: VisibilityManagerPrivate implementation

//! BEGIN: VisiblityManager implementation
VisibilityManager::VisibilityManager(PlasmaQuick::ContainmentView *view)
    : d(new VisibilityManagerPrivate(view, this))
{
    DockView *dockView = qobject_cast<DockView *>(view);

    if (dockView) {
        connect(this, &VisibilityManager::modeChanged, dockView->corona(), &Plasma::Corona::availableScreenRectChanged);
    }
}

VisibilityManager::~VisibilityManager()
{
    qDebug() << "VisibilityManager deleting...";
    delete d;
}

Dock::Visibility VisibilityManager::mode() const
{
    return d->mode;
}

void VisibilityManager::setMode(Dock::Visibility mode)
{
    d->setMode(mode);
}

void VisibilityManager::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    d->setWindowOnActivities(window, activities);
}

bool VisibilityManager::raiseOnDesktop() const
{
    return d->raiseOnDesktopChange;
}

void VisibilityManager::setRaiseOnDesktop(bool enable)
{
    d->setRaiseOnDesktop(enable);
}

bool VisibilityManager::raiseOnActivity() const
{
    return d->raiseOnActivityChange;
}

void VisibilityManager::setRaiseOnActivity(bool enable)
{
    d->setRaiseOnActivity(enable);
}

bool VisibilityManager::isHidden() const
{
    return d->isHidden;
}

void VisibilityManager::setIsHidden(bool isHidden)
{
    d->setIsHidden(isHidden);
}

bool VisibilityManager::blockHiding() const
{
    return d->blockHiding;
}

void VisibilityManager::setBlockHiding(bool blockHiding)
{
    d->setBlockHiding(blockHiding);
}

bool VisibilityManager::containsMouse() const
{
    return d->containsMouse;
}

int VisibilityManager::timerShow() const
{
    return d->timerShow.interval();
}

void VisibilityManager::setTimerShow(int msec)
{
    d->setTimerShow(msec);
}

int VisibilityManager::timerHide() const
{
    return d->timerHide.interval();
}

void VisibilityManager::setTimerHide(int msec)
{
    d->setTimerHide(msec);
}

//! Dynamic Background functions
bool VisibilityManager::enabledDynamicBackground() const
{
    return d->enabledDynamicBackgroundFlag;
}

void VisibilityManager::setEnabledDynamicBackground(bool active)
{
    d->setEnabledDynamicBackground(active);
}

bool VisibilityManager::existsWindowMaximized() const
{
    return d->windowIsMaximizedFlag;
}

void VisibilityManager::setExistsWindowMaximized(bool windowMaximized)
{
    d->setExistsWindowMaximized(windowMaximized);
}

bool VisibilityManager::existsWindowSnapped() const
{
    return d->windowIsSnappedFlag;
}

void VisibilityManager::setExistsWindowSnapped(bool windowSnapped)
{
    d->setExistsWindowSnapped(windowSnapped);
}

//! END: VisibilityManager implementation
}
