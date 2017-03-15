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
#include "dockcorona.h"
#include "../liblattedock/extras.h"

#include <QDebug>

namespace Latte {

//! BEGIN: VisiblityManagerPrivate implementation
VisibilityManagerPrivate::VisibilityManagerPrivate(PlasmaQuick::ContainmentView *view, VisibilityManager *q)
    : QObject(nullptr), q(q), view(view), wm(&WindowSystem::self())
{
    DockView *dockView = qobject_cast<DockView *>(view);

    if (dockView) {
        connect(dockView, &DockView::eventTriggered, this, &VisibilityManagerPrivate::event);
        connect(dockView, &DockView::absGeometryChanged, this, &VisibilityManagerPrivate::setDockGeometry);
    }

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
    wm->removeDockStruts(view->winId());
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

    if (this->mode == Dock::AlwaysVisible) {
        wm->removeDockStruts(view->winId());
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
            if (view->containment() && !view->containment()->isUserConfiguring())
                wm->setDockStruts(view->winId(), dockGeometry, view->location());

            connections[0] = connect(view->containment(), &Plasma::Containment::locationChanged
            , this, [&]() {
                if (view->containment()->isUserConfiguring())
                    wm->removeDockStruts(view->winId());
            });
            connections[1] = connect(view->containment(), &Plasma::Containment::userConfiguringChanged
            , this, [&](bool configuring) {
                if (!configuring)
                    wm->setDockStruts(view->winId(), dockGeometry, view->containment()->location());
            });
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
                windows.insert(std::make_pair(wid, wm->requestInfo(wid)));
            }

            connections[0] = connect(wm, &WindowSystem::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeWindows);
            connections[1] = connect(wm, &WindowSystem::windowRemoved
            , this, [&](WId wid) {
                windows.erase(wid);
                timerCheckWindows.start();
            });
            connections[2] = connect(wm, &WindowSystem::windowAdded
            , this, [&](WId wid) {
                windows.insert(std::make_pair(wid, wm->requestInfo(wid)));
                timerCheckWindows.start();
            });

            timerCheckWindows.start();
        }
        break;
    }


    emit q->modeChanged();
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
    }
}

inline void VisibilityManagerPrivate::setDockGeometry(const QRect &geometry)
{
    if (!view->containment() || this->dockGeometry == geometry)
        return;

    this->dockGeometry = geometry;

    if (mode == Dock::AlwaysVisible && !view->containment()->isUserConfiguring()) {
        wm->setDockStruts(view->winId(), this->dockGeometry, view->containment()->location());
    }
}

void VisibilityManagerPrivate::dodgeActive(WId wid)
{
    if (raiseTemporarily)
        return;

    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid())
        return;

    if (!winfo.isActive()) {
        if (winfo.isPlasmaDesktop())
            raiseDock(true);

        winfo = wm->requestInfo(wm->activeWindow());
    }

    if (wm->isOnCurrentDesktop(wid))
        raiseDock(!intersects(winfo));
}

void VisibilityManagerPrivate::dodgeMaximized(WId wid)
{
    if (raiseTemporarily)
        return;

    auto winfo = wm->requestInfo(wid);

    if (!winfo.isValid())
        return;

    if (!winfo.isActive()) {
        if (winfo.isPlasmaDesktop())
            raiseDock(true);

        winfo = wm->requestInfo(wm->activeWindow());
    }

    if (wm->isOnCurrentDesktop(wid) && !winfo.isMinimized())
        raiseDock(!(view->formFactor() == Plasma::Types::Vertical
                    ? winfo.isMaxHoriz() : winfo.isMaxVert()));
}

void VisibilityManagerPrivate::dodgeWindows(WId wid)
{
    if (raiseTemporarily)
        return;

    if (windows.find(wid) == std::end(windows))
        return;

    windows[wid] = wm->requestInfo(wid);
    auto &winfo = windows[wid];

    if (!winfo.isValid() || !wm->isOnCurrentDesktop(wid))
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
    return (!winfo.isMinimized() && winfo.geometry().intersects(dockGeometry));
}

inline void VisibilityManagerPrivate::saveConfig()
{
    if (!view->containment())
        return;

    auto config = view->containment()->config();
    config.writeEntry("visibility", static_cast<int>(mode));
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
    auto mode = static_cast<Dock::Visibility>(config.readEntry("visibility", static_cast<int>(Dock::DodgeActive)));
    emit q->timerShowChanged();
    emit q->timerHideChanged();

    if (mode == Dock::AlwaysVisible) {
        setMode(mode);
    } else {
        QTimer::singleShot(5000, this, [ &, mode]() {
            setMode(mode);
        });
        setRaiseOnDesktop(config.readEntry("raiseOnDesktopChange", false));
        setRaiseOnActivity(config.readEntry("raiseOnActivityChange", false));
    }

    qDebug() << config.entryMap();
    connect(view->containment(), &Plasma::Containment::userConfiguringChanged
    , this, [&](bool configuring) {
        if (!configuring)
            saveConfig();
    });
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
    }

}
//! END: VisibilityManagerPrivate implementation

//! BEGIN: VisiblityManager implementation
VisibilityManager::VisibilityManager(PlasmaQuick::ContainmentView *view)
    : d(new VisibilityManagerPrivate(view, this))
{
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

//! END: VisibilityManager implementation
}
