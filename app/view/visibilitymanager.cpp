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
namespace ViewPart {

//! BEGIN: VisiblityManager implementation

VisibilityManager::VisibilityManager(PlasmaQuick::ContainmentView *view)
    : QObject(view)
{
    qDebug() << "VisibilityManager creating...";

    m_latteView = qobject_cast<Latte::View *>(view);
    m_corona = qobject_cast<Latte::Corona *>(view->corona());
    wm = m_corona->wm();

    if (m_latteView) {
        connect(m_latteView, &Latte::View::absGeometryChanged, this, &VisibilityManager::setViewGeometry);
        connect(m_latteView, &Latte::View::eventTriggered, this, &VisibilityManager::viewEventManager);
    }

    if (m_corona) {
        connect(this, &VisibilityManager::modeChanged, m_corona, &Plasma::Corona::availableScreenRectChanged);
    }

    m_timerStartUp.setInterval(5000);
    m_timerStartUp.setSingleShot(true);
    m_timerCheckWindows.setInterval(350);
    m_timerCheckWindows.setSingleShot(true);
    m_timerShow.setSingleShot(true);
    m_timerHide.setSingleShot(true);
    connect(&m_timerCheckWindows, &QTimer::timeout, this, &VisibilityManager::checkAllWindows);
    connect(&m_timerShow, &QTimer::timeout, this, [&]() {
        if (m_isHidden) {
            //   qDebug() << "must be shown";
            emit mustBeShown();
        }
    });
    connect(&m_timerHide, &QTimer::timeout, this, [&]() {
        if (!m_blockHiding && !m_isHidden && !dragEnter) {
            //   qDebug() << "must be hide";
            emit mustBeHide();
        }
    });
    wm->setViewExtraFlags(*m_latteView);
    wm->addView(m_latteView->winId());

    restoreConfig();
}

VisibilityManager::~VisibilityManager()
{
    qDebug() << "VisibilityManager deleting...";
    wm->removeViewStruts(*m_latteView);
    wm->removeView(m_latteView->winId());

    if (edgeGhostWindow) {
        edgeGhostWindow->deleteLater();
    }
}

Types::Visibility VisibilityManager::mode() const
{
    return m_mode;
}

void VisibilityManager::setMode(Latte::Types::Visibility mode)
{
    if (m_mode == mode)
        return;

    Q_ASSERT_X(m_mode != Types::None, staticMetaObject.className(), "set visibility to Types::None");

    // clear mode
    for (auto &c : connections) {
        disconnect(c);
    }

    if (m_mode != Types::DodgeAllWindows && !enabledDynamicBackgroundFlag) {
        windows.clear();
    }

    if (m_mode == Types::AlwaysVisible) {
        wm->removeViewStruts(*m_latteView);
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

    m_timerShow.stop();
    m_timerHide.stop();
    m_timerCheckWindows.stop();
    m_mode = mode;

    switch (m_mode) {
        case Types::AlwaysVisible: {
            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::WindowsGoBelow);
            }

            if (m_latteView->containment() && !m_latteView->inEditMode() && m_latteView->screen()) {
                updateStrutsBasedOnLayoutsAndActivities();
            }

            connections[0] = connect(m_latteView->containment(), &Plasma::Containment::locationChanged
            , this, [&]() {
                if (m_latteView->inEditMode())
                    wm->removeViewStruts(*m_latteView);
            });
            connections[1] = connect(m_latteView, &Latte::View::inEditModeChanged
            , this, [&]() {
                if (!m_latteView->inEditMode() && !m_latteView->positioner()->inLocationChangeAnimation() && m_latteView->screen())
                    wm->setViewStruts(*m_latteView, m_viewGeometry, m_latteView->containment()->location());
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

            raiseView(m_containsMouse);
        }
        break;

        case Types::DodgeActive: {
            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AutoHide);
            }

            connections[0] = connect(wm, &WindowSystem::activeWindowChanged
                                     , this, &VisibilityManager::dodgeActive);
            connections[1] = connect(wm, &WindowSystem::windowChanged
                                     , this, &VisibilityManager::dodgeActive);
            dodgeActive(wm->activeWindow());
        }
        break;

        case Types::DodgeMaximized: {
            //set wayland visibility mode
            if (m_latteView->surface()) {
                m_latteView->surface()->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AutoHide);
            }

            connections[0] = connect(wm, &WindowSystem::activeWindowChanged
                                     , this, &VisibilityManager::dodgeMaximized);
            connections[1] = connect(wm, &WindowSystem::windowChanged
                                     , this, &VisibilityManager::dodgeMaximized);
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
                                     , this, &VisibilityManager::dodgeWindows);
            connections[1] = connect(wm, &WindowSystem::windowRemoved
            , this, [&](WindowId wid) {
                windows.remove(wid);
                m_timerCheckWindows.start();
            });
            connections[2] = connect(wm, &WindowSystem::windowAdded
            , this, [&](WindowId wid) {
                windows.insert(wid, wm->requestInfo(wid));
                m_timerCheckWindows.start();
            });

            m_timerCheckWindows.start();
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

    m_latteView->containment()->config().writeEntry("visibility", static_cast<int>(m_mode));

    updateKWinEdgesSupport();

    emit modeChanged();
}

void VisibilityManager::updateStrutsBasedOnLayoutsAndActivities()
{
    bool multipleLayoutsAndCurrent = (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts
                                      && m_latteView->managedLayout() && !m_latteView->positioner()->inLocationChangeAnimation()
                                      && m_latteView->managedLayout()->name() == m_corona->layoutManager()->currentLayoutName());

    if (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout || multipleLayoutsAndCurrent) {
        wm->setViewStruts(*m_latteView, m_viewGeometry, m_latteView->location());
    } else {
        wm->removeViewStruts(*m_latteView);
    }
}

bool VisibilityManager::raiseOnDesktop() const
{
    return raiseOnDesktopChange;
}

void VisibilityManager::setRaiseOnDesktop(bool enable)
{
    if (enable == raiseOnDesktopChange)
        return;

    raiseOnDesktopChange = enable;
    emit raiseOnDesktopChanged();
}

bool VisibilityManager::raiseOnActivity() const
{
    return raiseOnActivityChange;
}

void VisibilityManager::setRaiseOnActivity(bool enable)
{
    if (enable == raiseOnActivityChange)
        return;

    raiseOnActivityChange = enable;
    emit raiseOnActivityChanged();
}

bool VisibilityManager::isHidden() const
{
    return m_isHidden;
}

void VisibilityManager::setIsHidden(bool isHidden)
{
    if (m_isHidden == isHidden)
        return;

    if (m_blockHiding && isHidden) {
        qWarning() << "isHidden property is blocked, ignoring update";
        return;
    }

    m_isHidden = isHidden;

    if (supportsKWinEdges()) {
        bool inCurrentLayout = (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout ||
                                (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts
                                 && m_latteView->managedLayout() && !m_latteView->positioner()->inLocationChangeAnimation()
                                 && m_latteView->managedLayout()->name() == m_corona->layoutManager()->currentLayoutName()));

        if (inCurrentLayout) {
            wm->setEdgeStateFor(edgeGhostWindow, m_isHidden);
        } else {
            wm->setEdgeStateFor(edgeGhostWindow, false);
        }
    }

    emit isHiddenChanged();
}

bool VisibilityManager::blockHiding() const
{
    return m_blockHiding;
}

void VisibilityManager::setBlockHiding(bool blockHiding)
{
    if (m_blockHiding == blockHiding) {
        return;
    }

    m_blockHiding = blockHiding;
    // qDebug() << "blockHiding:" << blockHiding;

    if (m_blockHiding) {
        m_timerHide.stop();

        if (m_isHidden) {
            emit mustBeShown();
        }
    } else {
        updateHiddenState();
    }

    emit blockHidingChanged();
}

int VisibilityManager::timerShow() const
{
    return m_timerShow.interval();
}

void VisibilityManager::setTimerShow(int msec)
{
    m_timerShow.setInterval(msec);
    emit timerShowChanged();
}

int VisibilityManager::timerHide() const
{
    return m_timerHide.interval();
}

void VisibilityManager::setTimerHide(int msec)
{
    m_timerHide.setInterval(msec);
    emit timerHideChanged();
}

bool VisibilityManager::supportsKWinEdges() const
{
    return (edgeGhostWindow != nullptr);
}

void VisibilityManager::raiseView(bool raise)
{
    if (m_blockHiding)
        return;

    if (raise) {
        m_timerHide.stop();

        if (!m_timerShow.isActive()) {
            m_timerShow.start();
        }
    } else if (!dragEnter) {
        m_timerShow.stop();

        if (hideNow) {
            hideNow = false;
            emit mustBeHide();
        } else if (!m_timerHide.isActive()) {
            m_timerHide.start();
        }
    }
}

void VisibilityManager::raiseViewTemporarily()
{
    if (raiseTemporarily)
        return;

    raiseTemporarily = true;
    m_timerHide.stop();
    m_timerShow.stop();

    if (m_isHidden)
        emit mustBeShown();

    QTimer::singleShot(qBound(1800, 2 * m_timerHide.interval(), 3000), this, [&]() {
        raiseTemporarily = false;
        hideNow = true;
        updateHiddenState();
    });
}

void VisibilityManager::updateHiddenState()
{
    if (dragEnter)
        return;

    switch (m_mode) {
        case Types::AutoHide:
            raiseView(m_containsMouse);
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

void VisibilityManager::setViewGeometry(const QRect &geometry)
{
    if (!m_latteView->containment())
        return;

    m_viewGeometry = geometry;

    if (m_mode == Types::AlwaysVisible && !m_latteView->inEditMode() && m_latteView->screen()) {
        updateStrutsBasedOnLayoutsAndActivities();
    }
}

void VisibilityManager::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    wm->setWindowOnActivities(window, activities);
}

void VisibilityManager::applyActivitiesToHiddenWindows(const QStringList &activities)
{
    if (edgeGhostWindow) {
        wm->setWindowOnActivities(*edgeGhostWindow, activities);
    }
}

void VisibilityManager::dodgeActive(WindowId wid)
{
    if (raiseTemporarily)
        return;

    //!don't send false raiseView signal when containing mouse
    if (m_containsMouse) {
        raiseView(true);
        return;
    }

    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid() || !winfo.isActive()) {
        winfo = wm->requestInfo(wm->activeWindow());

        if (!winfo.isValid()) {
            //! very rare case that window manager doesn't have any active window at all
            raiseView(true);
            return;
        }
    }

    //! don't send false raiseView signal when containing mouse, // Johan comment
    //! I don't know why that wasn't winfo.wid() //active window, but just wid//the window that made the call
    if (wm->isOnCurrentDesktop(winfo.wid()) && wm->isOnCurrentActivity(winfo.wid())) {
        bool overlaps{intersects(winfo)};
        raiseView(!overlaps);
    }
}

void VisibilityManager::dodgeMaximized(WindowId wid)
{
    if (raiseTemporarily)
        return;

    //!don't send false raiseView signal when containing mouse
    if (m_containsMouse) {
        raiseView(true);
        return;
    }

    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid() || !winfo.isActive()) {
        winfo = wm->requestInfo(wm->activeWindow());

        if (!winfo.isValid()) {
            //! very rare case that window manager doesn't have any active window at all
            raiseView(true);
            return;
        }
    }

    auto intersectsMaxVert = [&]() noexcept -> bool {
        return ((winfo.isMaxVert()
                 || (m_latteView->screen() && m_latteView->screen()->availableSize().height() <= winfo.geometry().height()))
                && intersects(winfo));
    };

    auto intersectsMaxHoriz = [&]() noexcept -> bool {
        return ((winfo.isMaxHoriz()
                 || (m_latteView->screen() && m_latteView->screen()->availableSize().width() <= winfo.geometry().width()))
                && intersects(winfo));
    };

    //! don't send false raiseView signal when containing mouse, // Johan comment
    //! I don't know why that wasn't winfo.wid() //active window, but just wid//the window that made the call
    if (wm->isOnCurrentDesktop(winfo.wid()) && wm->isOnCurrentActivity(winfo.wid())) {
        bool overlapsMaximized{m_latteView->formFactor() == Plasma::Types::Vertical ? intersectsMaxHoriz() : intersectsMaxVert()};
        raiseView(!overlapsMaximized);
    }
}

void VisibilityManager::dodgeWindows(WindowId wid)
{
    if (raiseTemporarily)
        return;

    if (windows.find(wid) == std::end(windows))
        return;

    //!don't send false raiseView signal when containing mouse
    if (m_containsMouse) {
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
        m_timerCheckWindows.start();
}

void VisibilityManager::checkAllWindows()
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

bool VisibilityManager::intersects(const WindowInfoWrap &winfo)
{
    return (!winfo.isMinimized()
            && winfo.geometry().intersects(m_viewGeometry)
            && !winfo.isShaded());
}

void VisibilityManager::saveConfig()
{
    if (!m_latteView->containment())
        return;

    auto config = m_latteView->containment()->config();

    config.writeEntry("enableKWinEdges", enableKWinEdgesFromUser);
    config.writeEntry("timerShow", m_timerShow.interval());
    config.writeEntry("timerHide", m_timerHide.interval());
    config.writeEntry("raiseOnDesktopChange", raiseOnDesktopChange);
    config.writeEntry("raiseOnActivityChange", raiseOnActivityChange);

    m_latteView->containment()->configNeedsSaving();
}

void VisibilityManager::restoreConfig()
{
    if (!m_latteView || !m_latteView->containment()){
        return;
    }

    auto config = m_latteView->containment()->config();
    m_timerShow.setInterval(config.readEntry("timerShow", 0));
    m_timerHide.setInterval(config.readEntry("timerHide", 700));
    emit timerShowChanged();
    emit timerHideChanged();

    enableKWinEdgesFromUser = config.readEntry("enableKWinEdges", true);
    emit enableKWinEdgesChanged();

    setRaiseOnDesktop(config.readEntry("raiseOnDesktopChange", false));
    setRaiseOnActivity(config.readEntry("raiseOnActivityChange", false));

    auto storedMode = static_cast<Types::Visibility>(m_latteView->containment()->config().readEntry("visibility", static_cast<int>(Types::DodgeActive)));

    if (storedMode == Types::AlwaysVisible) {
        qDebug() << "Loading visibility mode: Always Visible , on startup...";
        setMode(Types::AlwaysVisible);
    } else {
        connect(&m_timerStartUp, &QTimer::timeout, this, [&]() {
            auto fMode = static_cast<Types::Visibility>(m_latteView->containment()->config().readEntry("visibility", static_cast<int>(Types::DodgeActive)));
            qDebug() << "Loading visibility mode:" << fMode << " on startup...";
            setMode(fMode);
        });
        connect(m_latteView->containment(), &Plasma::Containment::userConfiguringChanged
        , this, [&](bool configuring) {
            if (configuring && m_timerStartUp.isActive())
                m_timerStartUp.start(100);
        });

        m_timerStartUp.start();
    }

    connect(m_latteView->containment(), &Plasma::Containment::userConfiguringChanged
    , this, [&](bool configuring) {
        if (!configuring) {
            saveConfig();
        }
    });
}

bool VisibilityManager::containsMouse() const
{
    return m_containsMouse;
}

void VisibilityManager::setContainsMouse(bool contains)
{
    if (m_containsMouse == contains) {
        return;
    }

    m_containsMouse = contains;
    emit containsMouseChanged();

    if (contains && m_mode != Types::AlwaysVisible) {
        raiseView(true);
    }
}

void VisibilityManager::viewEventManager(QEvent *ev)
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

            if (m_isHidden)
                emit mustBeShown();

            break;

        case QEvent::DragLeave:
        case QEvent::Drop:
            dragEnter = false;
            updateHiddenState();
            break;

        case QEvent::Show:
            wm->setViewExtraFlags(*m_latteView);
            break;

        default:
            break;
    }
}

void VisibilityManager::cleanupFaultyWindows()
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
bool VisibilityManager::enabledDynamicBackground() const
{
    return enabledDynamicBackgroundFlag;
}

void VisibilityManager::setEnabledDynamicBackground(bool active)
{
    if (enabledDynamicBackgroundFlag == active) {
        return;
    }

    enabledDynamicBackgroundFlag = active;

    if (active) {
        if (m_mode != Types::DodgeAllWindows) {
            for (const auto &wid : wm->windows()) {
                windows.insert(wid, wm->requestInfo(wid));
            }
        }

        connectionsDynBackground[0] = connect(m_latteView->corona(), &Plasma::Corona::availableScreenRectChanged,
                                              this, &VisibilityManager::updateAvailableScreenGeometry);

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

        if (m_mode != Types::DodgeAllWindows) {
            windows.clear();
        }

       // ATTENTION: this was creating a crash under wayland environment through the blur effect
       // setExistsWindowMaximized(false);
       // setExistsWindowSnapped(false);
    }

    emit enabledDynamicBackgroundChanged();
}

bool VisibilityManager::existsWindowMaximized() const
{
    return windowIsMaximizedFlag;
}

void VisibilityManager::setExistsWindowMaximized(bool windowMaximized)
{
    if (windowIsMaximizedFlag == windowMaximized) {
        return;
    }

    windowIsMaximizedFlag = windowMaximized;

    emit existsWindowMaximizedChanged();
}

bool VisibilityManager::existsWindowSnapped() const
{
    return windowIsSnappedFlag;
}

void VisibilityManager::setExistsWindowSnapped(bool windowSnapped)
{
    if (windowIsSnappedFlag == windowSnapped) {
        return;
    }

    windowIsSnappedFlag = windowSnapped;

    emit existsWindowSnappedChanged();
}

SchemeColors *VisibilityManager::touchingWindowScheme() const
{
    return touchingScheme;
}

void VisibilityManager::setTouchingWindowScheme(SchemeColors *scheme)
{
    if (touchingScheme == scheme) {
        return;
    }

    touchingScheme = scheme;

    emit touchingWindowSchemeChanged();
}

void VisibilityManager::updateAvailableScreenGeometry()
{
    if (!m_latteView || !m_latteView->containment()) {
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

        if (m_latteView->formFactor() == Plasma::Types::Horizontal) {
            if (m_latteView->location() == Plasma::Types::TopEdge) {
                snap1 = QRect(x1, y1, halfWidth1, halfHeight1);
                snap3 = QRect(x2, y1, halfWidth2, halfHeight1);
            } else if ((m_latteView->location() == Plasma::Types::BottomEdge)) {
                snap1 = QRect(x1, y2, halfWidth1, halfHeight2);
                snap3 = QRect(x2, y2, halfWidth2, halfHeight2);
            }

            snap2 = QRect(x1, y1, halfWidth1, availableScreenGeometry.height());
            snap4 = QRect(x2, y1, halfWidth2, availableScreenGeometry.height());
        } else if (m_latteView->formFactor() == Plasma::Types::Vertical) {
            QRect snap5;

            if (m_latteView->location() == Plasma::Types::LeftEdge) {
                snap1 = QRect(x1, y1, halfWidth1, halfHeight1);
                snap3 = QRect(x1, y2, halfWidth1, halfHeight2);
                snap5 = QRect(x1, y1, halfWidth1, availableScreenGeometry.height());
            } else if ((m_latteView->location() == Plasma::Types::RightEdge)) {
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

bool VisibilityManager::isMaximizedInCurrentScreen(const WindowInfoWrap &winfo)
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

bool VisibilityManager::isTouchingPanelEdge(const WindowInfoWrap &winfo)
{
    if (winfo.isValid() && !winfo.isMinimized() && wm->isOnCurrentDesktop(winfo.wid()) && wm->isOnCurrentActivity(winfo.wid())) {
        bool touchingPanelEdge{false};

        QRect screenGeometry = m_latteView->screenGeometry();
        bool inCurrentScreen{screenGeometry.contains(winfo.geometry().topLeft()) || screenGeometry.contains(winfo.geometry().bottomRight())};

        if (inCurrentScreen) {
            if (m_latteView->location() == Plasma::Types::TopEdge) {
                touchingPanelEdge = (winfo.geometry().y() == availableScreenGeometry.y());
            } else if (m_latteView->location() == Plasma::Types::BottomEdge) {
                touchingPanelEdge = (winfo.geometry().bottom() == availableScreenGeometry.bottom());
            } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
                touchingPanelEdge = (winfo.geometry().x() == availableScreenGeometry.x());
            } else if (m_latteView->location() == Plasma::Types::RightEdge) {
                touchingPanelEdge = (winfo.geometry().right() == availableScreenGeometry.right());
            }
        }

        return touchingPanelEdge;
    }

    return false;
}

void VisibilityManager::updateDynamicBackgroundWindowFlags()
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
bool VisibilityManager::enableKWinEdges() const
{
    return enableKWinEdgesFromUser;
}

void VisibilityManager::setEnableKWinEdges(bool enable)
{
    if (enableKWinEdgesFromUser == enable) {
        return;
    }

    enableKWinEdgesFromUser = enable;

    emit enableKWinEdgesChanged();

    updateKWinEdgesSupport();
}

void VisibilityManager::updateKWinEdgesSupport()
{
    if (m_mode == Types::AutoHide
        || m_mode == Types::DodgeActive
        || m_mode == Types::DodgeAllWindows
        || m_mode == Types::DodgeMaximized) {
        if (enableKWinEdgesFromUser) {
            createEdgeGhostWindow();
        } else if (!enableKWinEdgesFromUser) {
            deleteEdgeGhostWindow();
        }
    } else if (m_mode == Types::AlwaysVisible
               || m_mode == Types::WindowsGoBelow) {
        deleteEdgeGhostWindow();
    }
}

void VisibilityManager::createEdgeGhostWindow()
{
    if (!edgeGhostWindow) {
        edgeGhostWindow = new ScreenEdgeGhostWindow(m_latteView);

        wm->setViewExtraFlags(*edgeGhostWindow);

        connect(edgeGhostWindow, &ScreenEdgeGhostWindow::containsMouseChanged, this, [ = ](bool contains) {
            if (contains) {
                emit mustBeShown();
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
                    wm->setEdgeStateFor(edgeGhostWindow, m_isHidden);
                } else {
                    wm->setEdgeStateFor(edgeGhostWindow, false);
                }
            }
        });

        emit supportsKWinEdgesChanged();
    }
}

void VisibilityManager::deleteEdgeGhostWindow()
{
    if (edgeGhostWindow) {
        edgeGhostWindow->deleteLater();
        edgeGhostWindow = nullptr;

        for (auto &c : connectionsKWinEdges) {
            disconnect(c);
        }

        emit supportsKWinEdgesChanged();
    }
}

//! Window Functions
void VisibilityManager::requestToggleMaximizeForActiveWindow()
{
    WindowInfoWrap actInfo = wm->requestInfoActive();

    //active window can be toggled only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        wm->requestToggleMaximized(actInfo.wid());
    }
}

void VisibilityManager::requestMoveActiveWindow(int localX, int localY)
{
    WindowInfoWrap actInfo = wm->requestInfoActive();

    //active window can be dragged only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        QPoint globalPoint{m_latteView->x() + localX, m_latteView->y() + localY};

        wm->requestMoveWindow(actInfo.wid(), globalPoint);

        //! This timer is needed because otherwise the mouse position
        //! in the dragged window changes to TopLeft corner
        QTimer::singleShot(250, this, [&, actInfo, globalPoint]() {
            wm->releaseMouseEventFor(m_latteView->winId());
        });

        setContainsMouse(false);
        updateHiddenState();
    }
}

bool VisibilityManager::activeWindowCanBeDragged()
{
    WindowInfoWrap actInfo = wm->requestInfoActive();

    //active window can be dragged only when it is in the same screen
    if (actInfo.isValid() && !actInfo.geometry().isNull() && m_latteView->screenGeometry().contains(actInfo.geometry().center())) {
        return wm->windowCanBeDragged(actInfo.wid());
    }

    return false;
}

//! END: VisibilityManager implementation

}
}
