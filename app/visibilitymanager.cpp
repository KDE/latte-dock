#include "visibilitymanager.h"
#include "visibilitymanager_p.h"
#include "plasmaquick/containmentview.h"
#include "abstractwindowinterface.h"
#include "windowinfowrap.h"
#include "dockview.h"
#include "../liblattedock/extras.h"

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
    
    restoreConfig();
    
    connect(&timerCheckWindows, &QTimer::timeout, this, &VisibilityManagerPrivate::checkAllWindows);
    connect(&timerShow, &QTimer::timeout, q, &VisibilityManager::mustBeShown);
    connect(&timerHide, &QTimer::timeout, q, &VisibilityManager::mustBeHide);
    
    wm->setDockDefaultFlags();
}

VisibilityManagerPrivate::~VisibilityManagerPrivate()
{
    wm->removeDockStruts();
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
            wm->setDockStruts(dockRect, view->location());
            raiseDock(true);
        }
        break;

        case Dock::AutoHide: {
            raiseDock(!containsMouse);
        }
        break;

        case Dock::DodgeActive: {
            connections[0] = connect(wm.get(), &AbstractWindowInterface::activeWindowChanged
                                     , this, &VisibilityManagerPrivate::dodgeActive);
            connections[1] = connect(wm.get(), &AbstractWindowInterface::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeActive);

            dodgeActive(wm->activeWindow());
        }
        break;

        case Dock::DodgeMaximized: {
            connections[0] = connect(wm.get(), &AbstractWindowInterface::windowChanged
                                     , this, &VisibilityManagerPrivate::dodgeMaximized);
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
        }
    }
    
    saveConfig();

    emit q->modeChanged();
}

inline void VisibilityManagerPrivate::setIsHidden(bool isHidden)
{
    if (this->isHidden == isHidden)
        return;

    this->isHidden = isHidden;
    emit q->isHiddenChanged();
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
    // possible optimization
    /* if (!isHidden == raise) {
        return;
    } */

    if (raise) {
        timerHide.stop();
        
        if (!timerShow.isActive()) {
            timerShow.start();
        }
    } else {
        timerShow.stop();

        if (!timerHide.isActive() && view->containment()->immutability() != Plasma::Types::Mutable)
            timerHide.start();
    }
}

inline void VisibilityManagerPrivate::setDockRect(const QRect &dockRect)
{
    if (!view->containment() || this->dockRect == dockRect)
        return;

    this->dockRect = dockRect;
    
    if (mode == Dock::AlwaysVisible) {
        wm->setDockStruts(this->dockRect, view->containment()->location());
    }
}

void VisibilityManagerPrivate::dodgeActive(WId wid)
{
    if (wid != wm->activeWindow())
        return;

    auto winfo = wm->requestInfo(wid);
    
    if (!winfo.isValid() || !winfo.isOnCurrentDesktop() || winfo.isMinimized())
        return;

    raiseDock(!intersects(winfo));
}

void VisibilityManagerPrivate::dodgeMaximized(WId wid)
{
    if (wid != wm->activeWindow())
        return;

    auto winfo = wm->requestInfo(wid);
    
    if (!winfo.isValid() || !winfo.isOnCurrentDesktop() || winfo.isMinimized())
        return;

    raiseDock(!winfo.isMaximized());
}

void VisibilityManagerPrivate::dodgeWindows(WId wid)
{
    auto winfo = wm->requestInfo(wid);
    
    if (!winfo.isValid() || !winfo.isOnCurrentDesktop() || winfo.isMinimized())
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
        if (!std::get<1>(winfo).isValid() || !std::get<1>(winfo).isOnCurrentDesktop())
            continue;

        if (std::get<1>(winfo).isFullscreen()) {
            raise = false;
            break;
            
        } else if (std::get<1>(winfo).isMinimized()) {
            continue;
            
        } else if (intersects(std::get<1>(winfo))) {
            raise = false;
            break;
        }
    }
    
    raiseDock(raise);
}

inline bool VisibilityManagerPrivate::intersects(const WindowInfoWrap &winfo)
{
    return winfo.geometry().intersects(dockRect);
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
    
    mode = static_cast<Dock::Visibility>(config.readEntry("visibility", static_cast<int>(Dock::DodgeActive)));
    timerShow.setInterval(config.readEntry("timerShow", 0));
    timerHide.setInterval(config.readEntry("timerHide", 0));
    
    emit q->timerShowChanged();
    emit q->timerHideChanged();
}

bool VisibilityManagerPrivate::event(QEvent *ev)
{
    if (ev->type() == QEvent::Enter && !containsMouse) {
        containsMouse = true;
        emit q->containsMouseChanged();
        
        raiseDock(true);
    } else if (ev->type() == QEvent::Leave && containsMouse) {
        containsMouse = false;
        emit q->containsMouseChanged();

        if (mode == Dock::AutoHide)
            raiseDock(false);
    } else if (ev->type() == QEvent::Show) {
        wm->setDockDefaultFlags();
    }
    
    return QObject::event(ev);
}

//! END: VisibilityManager implementation

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


