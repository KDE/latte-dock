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

// local
#include "positioner.h"
#include "screenedgeghostwindow.h"
#include "view.h"
#include "../lattecorona.h"
#include "../layoutmanager.h"
#include "../screenpool.h"
#include "../wm/windowinfowrap.h"
#include "../../liblatte2/extras.h"

// Qt
#include <QDebug>

// KDE
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

namespace Latte {

//! BEGIN: VisiblityManagerPrivate implementation
VisibilityManagerPrivate::VisibilityManagerPrivate(PlasmaQuick::ContainmentView *view, VisibilityManager *q)
    : QObject(nullptr), q(q), view(view)
{
    m_latteView = qobject_cast<Latte::View *>(view);
    m_corona = qobject_cast<Latte::Corona *>(view->corona());
    wm = m_corona->wm();

    if (m_latteView) {
        connect(m_latteView, &Latte::View::eventTriggered, this, &VisibilityManagerPrivate::viewEventManager);
        connect(m_latteView, &Latte::View::absGeometryChanged, this, &VisibilityManagerPrivate::setViewGeometry);
    }

    timerStartUp.setInterval(5000);
    timerStartUp.setSingleShot(true);
    timerCheckWindows.setInterval(350);
    timerCheckWindows.setSingleShot(true);
    timerShow.setSingleShot(true);
    timerHide.setSingleShot(true);
    connect(&timerCheckWindows, &QTimer::timeout, this, &VisibilityManagerPrivate::checkAllWindows);
    connect(&timerShow, &QTimer::timeout, this, [this, q]() {
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
    wm->setViewExtraFlags(*view);
    wm->addView(view->winId());
    restoreConfig();
}

VisibilityManagerPrivate::~VisibilityManagerPrivate()
{
    qDebug() << "VisibilityManagerPrivate deleting...";
    wm->removeViewStruts(*view);
    wm->removeView(view->winId());

    if (edgeGhostWindow) {
        edgeGhostWindow->deleteLater();
    }
}

inline void VisibilityManagerPrivate::setMode(Types::Visibility mode)
{
    if (this->mode == mode)
        return;

    Q_ASSERT_X(mode != Types::None, q->staticMetaObject.className(), "set visibility to Types::None");

    // clear mode
    for (auto &c : connections) {
        disconnect(c);
    }

    if (mode != Types::DodgeAllWindows && !enabledDynamicBackgroundFlag) {
        windows.clear();
    }

    if (this->mode == Types::AlwaysVisible) {
        wm->removeViewStruts(*view);
    } else {
        connections[3] = connect(wm, &WindowSystem::currentDesktopChanged
        , this, [&] {
            if (raiseOnDesktopChange)
                raiseViewTemporarily();
        });
        connections[4] = connect(wm, &WindowSystem::currentActivityChanged
        , this, [&]() {
            if (raiseOnActivityChange)
                raiseViewTemporarily();
            else
                updateHiddenState();
        });
    }

    timerShow.stop();
    timerHide.stop();
    timerCheckWindows.stop();
    this->mode = mode;

    switch (this->mode) {
        case Types::AlwaysVisible: {
            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::WindowsGoBelow);
            }

            if (view->containment() && !m_latteView->inEditMode() && view->screen()) {
                updateStrutsBasedOnLayoutsAndActivities();
            }

            connections[0] = connect(view->containment(), &Plasma::Containment::locationChanged
            , this, [&]() {
                if (m_latteView->inEditMode())
                    wm->removeViewStruts(*view);
            });
            connections[1] = connect(m_latteView, &Latte::View::inEditModeChanged
            , this, [&]() {
                if (!m_latteView->inEditMode() && !m_latteView->positioner()->inLocationChangeAnimation() && view->screen())
                    wm->setViewStruts(*view, m_viewGeometry, view->containment()->location());
            });

            if (m_corona && m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
                connections[2] = connect(m_corona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, [&]() {
                    updateStrutsBasedOnLayoutsAndActivities();
                });

                connections[3] = connect(m_latteView, &Latte::View::activitiesChanged, this, [&]() {
                    updateStrutsBasedOnLayoutsAndActivities();
                });
            }

            raiseView(true);
        }
        break;

        case Types::AutoHide: {
            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AutoHide);
            }

            raiseView(containsMouse);
        }
        break;

        case Types::DodgeActive: {
            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AutoHide);
            }

            connections[0] = connect(wm, &WindowSystem::activeWindowChanged
                                     , this, &VisibilityManagerPrivate::dodgeActive);
            connections[1] = connect(wm, &WindowSystem::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeActive);
            dodgeActive(wm->activeWindow());
        }
        break;

        case Types::DodgeMaximized: {
            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AutoHide);
            }

            connections[0] = connect(wm, &WindowSystem::activeWindowChanged
                                     , this, &VisibilityManagerPrivate::dodgeMaximized);
            connections[1] = connect(wm, &WindowSystem::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeMaximized);
            dodgeMaximized(wm->activeWindow());
        }
        break;

        case Types::DodgeAllWindows: {
            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AutoHide);
            }

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

        case Types::WindowsGoBelow:

            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::WindowsGoBelow);
            }

            break;

        default:
            break;
    }

    view->containment()->config().writeEntry("visibility", static_cast<int>(mode));

    updateKWinEdgesSupport();

    emit q->modeChanged();
}

void VisibilityManagerPrivate::updateStrutsBasedOnLayoutsAndActivities()
{
    bool multipleLayoutsAndCurrent = (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts
                                      && m_latteView->managedLayout() && !m_latteView->positioner()->inLocationChangeAnimation()
                                      && m_latteView->managedLayout()->name() == m_corona->layoutManager()->currentLayoutName());

    if (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout || multipleLayoutsAndCurrent) {
        wm->setViewStruts(*view, m_viewGeometry, view->location());
    } else {
        wm->removeViewStruts(*view);
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

    if (q->supportsKWinEdges()) {
        bool inCurrentLayout = (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout ||
                                (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts
                                 && m_latteView->managedLayout() && !m_latteView->positioner()->inLocationChangeAnimation()
                                 && m_latteView->managedLayout()->name() == m_corona->layoutManager()->currentLayoutName()));

        if (inCurrentLayout) {
            wm->setEdgeStateFor(edgeGhostWindow, isHidden);
        } else {
            wm->setEdgeStateFor(edgeGhostWindow, false);
        }
    }

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

inline void VisibilityManagerPrivate::raiseView(bool raise)
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
        } else if (!timerHide.isActive()) {
            timerHide.start();
        }
    }
}

void VisibilityManagerPrivate::raiseViewTemporarily()
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
        case Types::AutoHide:
            raiseView(containsMouse);
            break;

        case Types::DodgeActive:
            dodgeActive(wm->activeWindow());
            break;

        case Types::DodgeMaximized:
            dodgeMaximized(wm->activeWindow());
            break;

        case Types::DodgeAllWindows:
            dodgeWindows(wm->activeWindow());
            break;

        default:
            break;
    }
}

inline void VisibilityManagerPrivate::setViewGeometry(const QRect &geometry)
{
    if (!view->containment())
        return;

    m_viewGeometry = geometry;

    if (mode == Types::AlwaysVisible && !m_latteView->inEditMode() && view->screen()) {
        updateStrutsBasedOnLayoutsAndActivities();
    }
}

void VisibilityManagerPrivate::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    wm->setWindowOnActivities(window, activities);
}

void VisibilityManagerPrivate::applyActivitiesToHiddenWindows(const QStringList &activities)
{
    if (edgeGhostWindow) {
        wm->setWindowOnActivities(*edgeGhostWindow, activities);
    }
}

void VisibilityManagerPrivate::dodgeActive(WindowId wid)
{
    if (raiseTemporarily)
        return;

    //!don't send false raiseView signal when containing mouse
    if (containsMouse) {
        raiseView(true);
        return;
    }

    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid() || !winfo.isActive()) {
        winfo = wm->requestInfo(wm->activeWindow());

        if (!winfo.isValid()) {
            //! very rare case that window manager doesnt have any active window at all
            raiseView(true);
            return;
        }
    }

    //! don't send false raiseView signal when containing mouse, // Johan comment
    //! I dont know why that wasnt winfo.wid() //active window, but just wid//the window that made the call
    if (wm->isOnCurrentDesktop(winfo.wid()) && wm->isOnCurrentActivity(winfo.wid())) {
        bool overlaps{intersects(winfo)};
        raiseView(!overlaps);
    }
}

void VisibilityManagerPrivate::dodgeMaximized(WindowId wid)
{
    if (raiseTemporarily)
        return;

    //!don't send false raiseView signal when containing mouse
    if (containsMouse) {
        raiseView(true);
        return;
    }

    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid() || !winfo.isActive()) {
        winfo = wm->requestInfo(wm->activeWindow());

        if (!winfo.isValid()) {
            //! very rare case that window manager doesnt have any active window at all
            raiseView(true);
            return;
        }
    }

    auto intersectsMaxVert = [&]() noexcept -> bool {
        return ((winfo.isMaxVert()
                 || (view->screen() && view->screen()->availableSize().height() <= winfo.geometry().height()))
                && intersects(winfo));
    };

    auto intersectsMaxHoriz = [&]() noexcept -> bool {
        return ((winfo.isMaxHoriz()
                 || (view->screen() && view->screen()->availableSize().width() <= winfo.geometry().width()))
                && intersects(winfo));
    };

    //! don't send false raiseView signal when containing mouse, // Johan comment
    //! I dont know why that wasnt winfo.wid() //active window, but just wid//the window that made the call
    if (wm->isOnCurrentDesktop(winfo.wid()) && wm->isOnCurrentActivity(winfo.wid())) {
        bool overlapsMaximized{view->formFactor() == Plasma::Types::Vertical ? intersectsMaxHoriz() : intersectsMaxVert()};
        raiseView(!overlapsMaximized);
    }
}

void VisibilityManagerPrivate::dodgeWindows(WindowId wid)
{
    if (raiseTemporarily)
        return;

    if (windows.find(wid) == std::end(windows))
        return;

    //!don't send false raiseView signal when containing mouse
    if (containsMouse) {
        raiseView(true);
        return;
    }

    windows[wid] = wm->requestInfo(wid);
    auto &winfo = windows[wid];

    if (!winfo.isValid() || !wm->isOnCurrentDesktop(wid) || !wm->isOnCurrentActivity(wid))
        return;

    if (intersects(winfo))
        raiseView(false);
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
    raiseView(raise);
}

inline bool VisibilityManagerPrivate::intersects(const WindowInfoWrap &winfo)
{
    return (!winfo.isMinimized()
            && winfo.geometry().intersects(m_viewGeometry)
            && !winfo.isShaded());
}

inline void VisibilityManagerPrivate::saveConfig()
{
    if (!view->containment())
        return;

    auto config = view->containment()->config();

    config.writeEntry("enableKWinEdges", enableKWinEdgesFromUser);
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
    timerShow.setInterval(config.readEntry("timerShow", 0));
    timerHide.setInterval(config.readEntry("timerHide", 700));
    emit q->timerShowChanged();
    emit q->timerHideChanged();

    enableKWinEdgesFromUser = config.readEntry("enableKWinEdges", true);
    emit q->enableKWinEdgesChanged();

    setRaiseOnDesktop(config.readEntry("raiseOnDesktopChange", false));
    setRaiseOnActivity(config.readEntry("raiseOnActivityChange", false));

    auto mode = [&]() {
        return static_cast<Types::Visibility>(view->containment()->config()
                                             .readEntry("visibility", static_cast<int>(Types::DodgeActive)));
    };

    if (mode() == Types::AlwaysVisible) {
        setMode(Types::AlwaysVisible);
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

void VisibilityManagerPrivate::setContainsMouse(bool contains)
{
    if (containsMouse == contains) {
        return;
    }

    containsMouse = contains;
    emit q->containsMouseChanged();

    if (contains && mode != Types::AlwaysVisible) {
        raiseView(true);
    }
}

void VisibilityManagerPrivate::viewEventManager(QEvent *ev)
{
    switch (ev->type()) {
        case QEvent::Enter:
            setContainsMouse(true);
            break;

        case QEvent::Leave:
            setContainsMouse(false);
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
            wm->setViewExtraFlags(*view);
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
        if (mode != Types::DodgeAllWindows) {
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

        if (mode != Types::DodgeAllWindows) {
            windows.clear();
        }

       // ATTENTION: this was creating a crash under wayland environment through the blur effect
       // setExistsWindowMaximized(false);
       // setExistsWindowSnapped(false);
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

void VisibilityManagerPrivate::setTouchingWindowScheme(SchemeColors *scheme)
{
    if (touchingScheme == scheme) {
        return;
    }

    touchingScheme = scheme;

    emit q->touchingWindowSchemeChanged();
}

void VisibilityManagerPrivate::updateAvailableScreenGeometry()
{
    if (!view || !view->containment()) {
        return;
    }

    int currentScrId = m_latteView->positioner()->currentScreenId();
    QRect tempAvailableScreenGeometry = m_corona->availableScreenRectWithCriteria(currentScrId, {Types::AlwaysVisible}, {});

    if (tempAvailableScreenGeometry != availableScreenGeometry) {
        availableScreenGeometry = tempAvailableScreenGeometry;

        snappedWindowsGeometries.clear();

        //! for top view the snapped geometries would be
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

bool VisibilityManagerPrivate::isMaximizedInCurrentScreen(const WindowInfoWrap &winfo)
{
    //! updated implementation to identify the screen that the maximized window is present
    //! in order to avoid: https://bugs.kde.org/show_bug.cgi?id=397700

    if (winfo.isValid() && !winfo.isMinimized() && wm->isOnCurrentDesktop(winfo.wid()) && wm->isOnCurrentActivity(winfo.wid())) {
        if (winfo.isMaximized() && availableScreenGeometry.contains(winfo.geometry().center())) {
            return true;
        }
    }

    return false;
}

bool VisibilityManagerPrivate::isTouchingPanelEdge(const WindowInfoWrap &winfo)
{
    if (winfo.isValid() && !winfo.isMinimized() && wm->isOnCurrentDesktop(winfo.wid()) && wm->isOnCurrentActivity(winfo.wid())) {
        bool touchingPanelEdge{false};

        QRect screenGeometry = m_latteView->screenGeometry();
        bool inCurrentScreen{screenGeometry.contains(winfo.geometry().topLeft()) || screenGeometry.contains(winfo.geometry().bottomRight())};

        if (inCurrentScreen) {
            if (view->location() == Plasma::Types::TopEdge) {
                touchingPanelEdge = (winfo.geometry().y() == availableScreenGeometry.y());
            } else if (view->location() == Plasma::Types::BottomEdge) {
                touchingPanelEdge = (winfo.geometry().bottom() == availableScreenGeometry.bottom());
            } else if (view->location() == Plasma::Types::LeftEdge) {
                touchingPanelEdge = (winfo.geometry().x() == availableScreenGeometry.x());
            } else if (view->location() == Plasma::Types::RightEdge) {
                touchingPanelEdge = (winfo.geometry().right() == availableScreenGeometry.right());
            }
        }

        return touchingPanelEdge;
    }

    return false;
}

void VisibilityManagerPrivate::updateDynamicBackgroundWindowFlags()
{
    bool foundSnap{false};
    bool foundMaximized{false};

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! maybe a garbage collector here is a good idea!!!
    bool existsFaultyWindow{false};
    WindowId maxWinId;
    WindowId snapWinId;

    for (const auto &winfo : windows) {
        if (isMaximizedInCurrentScreen(winfo)) {
            foundMaximized = true;
            maxWinId = winfo.wid();
        }

        if (winfo.isActive() && isTouchingPanelEdge(winfo)) {
            foundSnap = true;
            snapWinId = winfo.wid();
        }

        if (!existsFaultyWindow && winfo.geometry() == QRect(0, 0, 0, 0)) {
            existsFaultyWindow = true;
        }

        //qDebug() << "window geometry ::: " << winfo.geometry();
    }

    //! active windows that are touching the panel edge should have a higher priority
    //! this is why are identified first
    if (!foundSnap) {
        for (const auto &winfo : windows) {
            if ((winfo.isKeepAbove() && isTouchingPanelEdge(winfo))
                || (!winfo.isActive() && snappedWindowsGeometries.contains(winfo.geometry()))) {
                foundSnap = true;
                snapWinId = winfo.wid();
                break;
            }
        }
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

    //! update color scheme for touching window

    if (foundSnap) {
        //! first the snap one because that would mean it is active
        setTouchingWindowScheme(wm->schemeForWindow(snapWinId));
    } else if (foundMaximized) {
        setTouchingWindowScheme(wm->schemeForWindow(maxWinId));
    } else {
        setTouchingWindowScheme(nullptr);
    }
}

//! KWin Edges Support functions
void VisibilityManagerPrivate::setEnableKWinEdges(bool enable)
{
    if (enableKWinEdgesFromUser == enable) {
        return;
    }

    enableKWinEdgesFromUser = enable;

    emit q->enableKWinEdgesChanged();

    updateKWinEdgesSupport();
}

void VisibilityManagerPrivate::updateKWinEdgesSupport()
{
    if (mode == Types::AutoHide
        || mode == Types::DodgeActive
        || mode == Types::DodgeAllWindows
        || mode == Types::DodgeMaximized) {
        if (enableKWinEdgesFromUser) {
            createEdgeGhostWindow();
        } else if (!enableKWinEdgesFromUser) {
            deleteEdgeGhostWindow();
        }
    } else if (mode == Types::AlwaysVisible
               || mode == Types::WindowsGoBelow) {
        deleteEdgeGhostWindow();
    }
}

void VisibilityManagerPrivate::createEdgeGhostWindow()
{
    if (!edgeGhostWindow) {
        edgeGhostWindow = new ViewPart::ScreenEdgeGhostWindow(m_latteView);

        wm->setViewExtraFlags(*edgeGhostWindow);

        connect(edgeGhostWindow, &ViewPart::ScreenEdgeGhostWindow::containsMouseChanged, this, [ = ](bool contains) {
            if (contains) {
                emit this->q->mustBeShown(VisibilityManager::QPrivateSignal{});
            }
        });

        connectionsKWinEdges[0] = connect(wm, &WindowSystem::currentActivityChanged,
        this, [&]() {
            bool inCurrentLayout = (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout ||
                                    (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts
                                     && m_latteView->managedLayout() && !m_latteView->positioner()->inLocationChangeAnimation()
                                     && m_latteView->managedLayout()->name() == m_corona->layoutManager()->currentLayoutName()));

            if (edgeGhostWindow) {
                if (inCurrentLayout) {
                    wm->setEdgeStateFor(edgeGhostWindow, isHidden);
                } else {
                    wm->setEdgeStateFor(edgeGhostWindow, false);
                }
            }
        });

        emit q->supportsKWinEdgesChanged();
    }
}

void VisibilityManagerPrivate::deleteEdgeGhostWindow()
{
    if (edgeGhostWindow) {
        edgeGhostWindow->deleteLater();
        edgeGhostWindow = nullptr;

        for (auto &c : connectionsKWinEdges) {
            disconnect(c);
        }

        emit q->supportsKWinEdgesChanged();
    }
}

//! Window Functions
void VisibilityManagerPrivate::requestToggleMaximizeForActiveWindow()
{
    WindowInfoWrap actInfo = wm->requestInfoActive();

    //active window can be toggled only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        wm->requestToggleMaximized(actInfo.wid());
    }
}

void VisibilityManagerPrivate::requestMoveActiveWindow(int localX, int localY)
{
    WindowInfoWrap actInfo = wm->requestInfoActive();

    //active window can be dragged only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        QPoint globalPoint{m_latteView->x() + localX, m_latteView->y() + localY};
        wm->requestMoveWindow(actInfo.wid(), globalPoint);
    }
}

bool VisibilityManagerPrivate::activeWindowCanBeDragged()
{
    WindowInfoWrap actInfo = wm->requestInfoActive();

    //active window can be dragged only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        return wm->windowCanBeDragged(actInfo.wid());
    }

    return false;
}

//! END: VisibilityManagerPrivate implementation


//! BEGIN: VisibilityManager implementation
VisibilityManager::VisibilityManager(PlasmaQuick::ContainmentView *view)
    : d(new VisibilityManagerPrivate(view, this))
{
    Latte::View *m_latteView = qobject_cast<Latte::View *>(view);

    if (m_latteView) {
        connect(this, &VisibilityManager::modeChanged, m_latteView->corona(), &Plasma::Corona::availableScreenRectChanged);
    }
}

VisibilityManager::~VisibilityManager()
{
    qDebug() << "VisibilityManager deleting...";
    delete d;
}

Types::Visibility VisibilityManager::mode() const
{
    return d->mode;
}

void VisibilityManager::setMode(Types::Visibility mode)
{
    d->setMode(mode);
}

void VisibilityManager::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    d->setWindowOnActivities(window, activities);
}

void VisibilityManager::applyActivitiesToHiddenWindows(const QStringList &activities)
{
    d->applyActivitiesToHiddenWindows(activities);
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

bool VisibilityManager::existsWindowSnapped() const
{
    return d->windowIsSnappedFlag;
}

SchemeColors *VisibilityManager::touchingWindowScheme() const
{
    return d->touchingScheme;
}

//! KWin Edges Support functions
bool VisibilityManager::enableKWinEdges() const
{
    return d->enableKWinEdgesFromUser;
}

void VisibilityManager::setEnableKWinEdges(bool enable)
{
    d->setEnableKWinEdges(enable);
}

bool VisibilityManager::supportsKWinEdges() const
{
    return (d->edgeGhostWindow != nullptr);
}

//! Window Functions
void VisibilityManager::requestToggleMaximizeForActiveWindow()
{
    d->requestToggleMaximizeForActiveWindow();
}

void VisibilityManager::requestMoveActiveWindow(int localX, int localY)
{
    d->requestMoveActiveWindow(localX, localY);
}

bool VisibilityManager::activeWindowCanBeDragged()
{
    return d->activeWindowCanBeDragged();
}

//! END: VisibilityManager implementation

}
