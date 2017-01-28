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
#include "windowinfowrap.h"
#include "dockview.h"
#include "../liblattedock/extras.h"

#include <QDebug>

namespace Latte {

//! BEGIN: VisiblityManagerPrivate implementation
VisibilityManagerPrivate::VisibilityManagerPrivate(PlasmaQuick::ContainmentView *view, VisibilityManager *q)
    : QObject(q), q(q), view(view), wm(AbstractWindowInterface::getInstance(view, nullptr))
{
    DockView *dockView = qobject_cast<DockView *>(view);

    if (dockView) {
        connect(dockView, &DockView::eventTriggered, this, &VisibilityManagerPrivate::event);
    }

    timerCheckWindows.setInterval(350);
    timerCheckWindows.setSingleShot(true);
    timerShow.setSingleShot(true);
    timerHide.setSingleShot(true);
    connect(&timerCheckWindows, &QTimer::timeout, this, &VisibilityManagerPrivate::checkAllWindows);
    connect(&timerShow, &QTimer::timeout, this, [this]() {
        if (isHidden) {
            //   qDebug() << "must be shown";
            emit this->q->mustBeShown();
        }
    });
    connect(&timerHide, &QTimer::timeout, this, [this]() {
        if (!blockHiding && !isHidden && !dragEnter) {
            //   qDebug() << "must be hide";
            emit this->q->mustBeHide();
        }
    });
    wm->setDockDefaultFlags();
}

VisibilityManagerPrivate::~VisibilityManagerPrivate()
{
    wm->removeDockStruts();
    wm->deleteLater();
}

inline void VisibilityManagerPrivate::setMode(Dock::Visibility mode)
{
    if (this->mode == mode)
        return;

    // clear mode
    if (this->mode == Dock::AlwaysVisible)
        wm->removeDockStruts();

    for (auto &c : connections) {
        disconnect(c);
    }

    timerShow.stop();
    timerHide.stop();
    timerCheckWindows.stop();
    this->mode = mode;

    switch (this->mode) {
        case Dock::AlwaysVisible: {
            if (view->containment() && !view->containment()->isUserConfiguring())
                wm->setDockStruts(dockRect, view->location());

            connections[0] = connect(view->containment(), &Plasma::Containment::locationChanged
            , this, [&]() {
                if (view->containment()->isUserConfiguring())
                    wm->removeDockStruts();
            });
            connections[1] = connect(view->containment(), &Plasma::Containment::userConfiguringChanged
            , this, [&](bool configuring) {
                if (!configuring)
                    wm->setDockStruts(dockRect, view->containment()->location());
            });
            raiseDock(true);
        }
        break;

        case Dock::AutoHide: {
            raiseDock(containsMouse);
        }
        break;

        case Dock::DodgeActive: {
            connections[0] = connect(wm.get(), &AbstractWindowInterface::activeWindowChanged
                                     , this, &VisibilityManagerPrivate::dodgeActive);
            connections[1] = connect(wm.get(), &AbstractWindowInterface::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeActive);
            connections[2] = connect(wm.get(), &AbstractWindowInterface::currentDesktopChanged
            , this, [&](int) {
                dodgeActive(wm->activeWindow());
            });
            dodgeActive(wm->activeWindow());
        }
        break;

        case Dock::DodgeMaximized: {
            connections[0] = connect(wm.get(), &AbstractWindowInterface::activeWindowChanged
                                     , this, &VisibilityManagerPrivate::dodgeMaximized);
            connections[1] = connect(wm.get(), &AbstractWindowInterface::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeMaximized);
            connections[2] = connect(wm.get(), &AbstractWindowInterface::currentDesktopChanged
            , this, [&](int) {
                dodgeMaximized(wm->activeWindow());
            });
            dodgeMaximized(wm->activeWindow());
        }
        break;

        case Dock::DodgeAllWindows: {
            for (const auto &wid : wm->windows()) {
                windows.insert({wid, wm->requestInfo(wid)});
            }

            connections[0] = connect(wm.get(), &AbstractWindowInterface::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeWindows);
            connections[1] = connect(wm.get(), &AbstractWindowInterface::windowRemoved
            , this, [&](WId wid) {
                windows.erase(wid);
                timerCheckWindows.start();
            });
            connections[2] = connect(wm.get(), &AbstractWindowInterface::windowAdded
            , this, [&](WId wid) {
                windows.insert({wid, wm->requestInfo(wid)});
                timerCheckWindows.start();
            });
            connections[3] = connect(wm.get(), &AbstractWindowInterface::currentDesktopChanged
            , this, [&](int) {
                timerCheckWindows.start();
            });
            timerCheckWindows.start();
        }
    }

    saveConfig();
    emit q->modeChanged();
}

inline void VisibilityManagerPrivate::setIsHidden(bool isHidden)
{
    if (this->isHidden == isHidden)
        return;

    if (blockHiding) {
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
            isHidden = false;
            emit q->isHiddenChanged();
            emit q->mustBeShown();
        }
    } else {
        updateHiddenState();
    }

    emit q->blockHidingChanged();
}

inline void VisibilityManagerPrivate::setTimerShow(int msec)
{
    timerShow.setInterval(msec);
    saveConfig();
    emit q->timerShowChanged();
}

inline void VisibilityManagerPrivate::setTimerHide(int msec)
{
    timerHide.setInterval(msec);
    saveConfig();
    emit q->timerHideChanged();
}

inline void VisibilityManagerPrivate::raiseDock(bool raise)
{
    if (raise) {
        timerHide.stop();

        if (!timerShow.isActive()) {
            timerShow.start();
        }
    } else if (!blockHiding && !dragEnter) {
        timerShow.stop();

        if (!timerHide.isActive())
            timerHide.start();
    }
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
    }
}

inline void VisibilityManagerPrivate::setDockRect(const QRect &dockRect)
{
    if (!view->containment() || this->dockRect == dockRect)
        return;

    this->dockRect = dockRect;

    if (mode == Dock::AlwaysVisible && !view->containment()->isUserConfiguring()) {
        wm->setDockStruts(this->dockRect, view->containment()->location());
    }
}

void VisibilityManagerPrivate::dodgeActive(WId wid)
{
    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid())
        return;

    if (!winfo.isActive()) {
        if (winfo.isPlasmaDesktop())
            raiseDock(true);

        return;
    }

    raiseDock(wm->isOnCurrentDesktop(wid) && !intersects(winfo));
}

void VisibilityManagerPrivate::dodgeMaximized(WId wid)
{
    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid())
        return;

    if (!winfo.isActive()) {
        if (winfo.isPlasmaDesktop())
            raiseDock(true);

        return;
    }

    raiseDock(wm->isOnCurrentDesktop(wid) && !winfo.isMaximized());
}

void VisibilityManagerPrivate::dodgeWindows(WId wid)
{
    if (windows.find(wid) == std::end(windows))
        return;

    auto winfo = wm->requestInfo(wid);
    windows[wid] = winfo;

    if (!winfo.isValid() || !wm->isOnCurrentDesktop(wid))
        return;

    if (intersects(winfo))
        raiseDock(false);
    else
        timerCheckWindows.start();
}

void VisibilityManagerPrivate::checkAllWindows()
{
    bool raise{true};

    for (const auto &winfo : windows) {
        //! std::pair<WId, WindowInfoWrap>
        if (!std::get<1>(winfo).isValid() || !wm->isOnCurrentDesktop(std::get<0>(winfo)))
            continue;

        if (std::get<1>(winfo).isFullscreen()) {
            raise = false;
            break;
        } else if (intersects(std::get<1>(winfo))) {
            raise = false;
            break;
        }
    }

    raiseDock(raise);
}

inline bool VisibilityManagerPrivate::intersects(const WindowInfoWrap &winfo)
{
    return !winfo.isMinimized() && winfo.geometry().intersects(dockRect);
}

inline void VisibilityManagerPrivate::saveConfig()
{
    if (!view->containment())
        return;

    auto config = view->containment()->config();
    config.writeEntry("visibility", static_cast<int>(mode));
    config.writeEntry("timerShow", timerShow.interval());
    config.writeEntry("timerHide", timerHide.interval());
    view->containment()->configNeedsSaving();
}

inline void VisibilityManagerPrivate::restoreConfig()
{
    if (!view->containment())
        return;

    auto config = view->containment()->config();
    timerShow.setInterval(config.readEntry("timerShow", 200));
    timerHide.setInterval(config.readEntry("timerHide", 700));
    auto mode = static_cast<Dock::Visibility>(config.readEntry("visibility", static_cast<int>(Dock::DodgeActive)));
    emit q->timerShowChanged();
    emit q->timerHideChanged();
    setMode(mode);
}

bool VisibilityManagerPrivate::event(QEvent *ev)
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
            emit q->mustBeShown();
            break;

        case QEvent::DragLeave:
        case QEvent::Drop:
            dragEnter = false;
            updateHiddenState();
            break;

        case QEvent::Show:
            wm->setDockDefaultFlags();
            restoreConfig();
            break;
    }

    return QObject::event(ev);
}
//! END: VisibilityManagerPrivate implementation

//! BEGIN: VisiblityManager implementation
VisibilityManager::VisibilityManager(PlasmaQuick::ContainmentView *view)
    : d(new VisibilityManagerPrivate(view, this))
{
}

VisibilityManager::~VisibilityManager()
{
}

Dock::Visibility VisibilityManager::mode() const
{
    return d->mode;
}

void VisibilityManager::setMode(Dock::Visibility mode)
{
    d->setMode(mode);
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

void VisibilityManager::updateDockGeometry(const QRect &geometry)
{
    d->setDockRect(geometry);
}
//! END: VisibilityManager implementation
}
