#include "visibilitymanager.h"
#include "visibilitymanager_p.h"

namespace Latte {

//! BEGIN: VisiblityManagerPrivate implementation
VisibilityManagerPrivate::VisibilityManagerPrivate(PlasmaQuick::ContainmentView *view, VisibilityManager *q)
    : QObject(view), q(q)
{

}

VisibilityManagerPrivate::~VisibilityManagerPrivate()
{

}

void VisibilityManagerPrivate::setMode(Dock::Visibility mode)
{

}

void VisibilityManagerPrivate::setIsHidden(bool isHidden)
{

}

void VisibilityManagerPrivate::setTimerShow(int msec)
{

}

void VisibilityManagerPrivate::setTimerHide(int msec)
{

}

void VisibilityManagerPrivate::raiseDock(bool raise)
{
    if (raise) {
        timerHide.stop();
        
        if (!timerShow.isActive())
            timerShow.start();
    } else {
        timerShow.stop();
        
        if (!timerHide.isActive())
            timerHide.start();
    }
}

void VisibilityManagerPrivate::setDockRect(const QRect &rect)
{

}

void VisibilityManagerPrivate::windowAdded(WId id)
{

}

void VisibilityManagerPrivate::dodgeActive(WId id)
{

}

void VisibilityManagerPrivate::dodgeWindows(WId id)
{

}

void VisibilityManagerPrivate::checkAllWindows()
{

}

bool VisibilityManagerPrivate::intersects(const WindowInfoWrap &info)
{

}

void VisibilityManagerPrivate::saveConfig()
{

}

void VisibilityManagerPrivate::restoreConfig()
{

}

bool VisibilityManagerPrivate::event(QEvent *ev)
{
    if (ev->type() == QEvent::Enter && !containsMouse) {
        containsMouse = true;
        emit q->containsMouseChanged();
        
        if (mode == Dock::AutoHide)
            raiseDock(true);
            
    } else if (ev->type() == QEvent::Leave && containsMouse) {
        containsMouse = false;
        emit q->containsMouseChanged();
        
        if (mode == Dock::AutoHide)
            raiseDock(false);
    }
    
    return QObject::event(ev);
}

//! END: VisibilityManager implementation

//! BEGIN: VisiblityManager implementation
VisibilityManager::VisibilityManager(PlasmaQuick::ContainmentView *view)
    : d(new VisibilityManagerPrivate(view, this))
{
    d->restoreConfig();
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

void VisibilityManager::setHidden(bool isHidden)
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

#include "abstractwindowinterface.h"
#include "xwindowinterface.h"
#include "plasmaquick/containmentview.h"




