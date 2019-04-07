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
    m_timerShow.setSingleShot(true);
    m_timerHide.setSingleShot(true);

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

            connections[0] = connect(this, &VisibilityManager::containsMouseChanged
                                     , this, &VisibilityManager::dodgeAllWindows);

            connections[1] = connect(m_latteView->windowsTracker(), &WindowsTracker::existsWindowTouchingChanged
                                     , this, &VisibilityManager::dodgeAllWindows);
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
                                      && m_latteView->managedLayout()->isCurrent());

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

    updateGhostWindowState();

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

void VisibilityManager::updateGhostWindowState()
{
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
            dodgeAllWindows();
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

void VisibilityManager::applyActivitiesToHiddenWindows(const QStringList &activities)
{
    if (edgeGhostWindow) {
        wm->setWindowOnActivities(*edgeGhostWindow, activities);
    }
}

void VisibilityManager::activeWindowDraggingStarted()
{
    setContainsMouse(false);
    updateHiddenState();
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

void VisibilityManager::dodgeAllWindows()
{
    if (raiseTemporarily)
        return;

    if (m_containsMouse) {
        raiseView(true);
    }

    bool windowIntersects{m_latteView->windowsTracker()->activeWindowTouching() || m_latteView->windowsTracker()->existsWindowTouching()};

    raiseView(!windowIntersects);
}

bool VisibilityManager::intersects(const WindowInfoWrap &winfo)
{
    return (!winfo.isMinimized()
            && wm->isOnCurrentDesktop(winfo.wid())
            && wm->isOnCurrentActivity(winfo.wid())
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
                raiseView(true);
            } else {
                m_timerShow.stop();
                updateGhostWindowState();
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

//! END: VisibilityManager implementation

}
}
