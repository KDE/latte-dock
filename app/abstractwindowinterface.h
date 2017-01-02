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

#ifndef ABSTRACTWINDOWINTERFACE_H
#define ABSTRACTWINDOWINTERFACE_H

#include "windowinfowrap.h"
#include "../liblattedock/dock.h"

#include <unordered_map>
#include <memory>
#include <list>

#include <QObject>
#include <QRect>
#include <QQuickWindow>

#include <Plasma>

namespace Latte {

class XWindowInterface;

class AbstractWindowInterface : public QObject {
    Q_OBJECT
    
public:
    explicit AbstractWindowInterface(QQuickWindow *const view, QObject *parent = nullptr);
    virtual ~AbstractWindowInterface();
    
    virtual void setDockDefaultFlags() = 0;
    
    virtual WId activeWindow() const = 0;
    virtual WindowInfoWrap requestInfo(WId wid) const = 0;
    virtual WindowInfoWrap requestInfoActive() const = 0;
    virtual bool isOnCurrentDesktop(WId wid) const = 0;
    virtual const std::list<WId> &windows() const = 0;
    
    virtual void setDockStruts(const QRect &dockRect, Plasma::Types::Location location) const = 0;
    virtual void removeDockStruts() const = 0;
    
    static AbstractWindowInterface *getInstance(QQuickWindow *const view, QObject *parent = nullptr);
    
signals:
    void activeWindowChanged(WId wid);
    void windowChanged(WId winfo);
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void currentDesktopChanged(int desktop);
    
protected:
    QQuickWindow *const m_view;
    std::list<WId> m_windows;
};

}

#endif // ABSTRACTWINDOWINTERFACE_H
