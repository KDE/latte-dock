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
#include "../liblattedock/extras.h"

#include <unordered_map>
#include <list>

#include <QObject>
#include <QWindow>
#include <QDialog>
#include <QRect>
#include <QPointer>
#include <QScreen>

#include <Plasma>
#include <KActivities/Consumer>

namespace Latte {

class XWindowInterface;
class WaylandInterface;

class AbstractWindowInterface : public QObject
{
    Q_OBJECT

public:
    enum class Slide
    {
        None,
        Top,
        Left,
        Bottom,
        Right,
    };

    explicit AbstractWindowInterface(QObject *parent = nullptr);
    virtual ~AbstractWindowInterface();

    virtual void setDockExtraFlags(QWindow &view) = 0;
    virtual void setDockStruts(QWindow &view, const QRect &rect
                               , Plasma::Types::Location location) = 0;
    virtual void setWindowOnActivities(QWindow &window, const QStringList &activities) = 0;

    virtual void removeDockStruts(QWindow &view) const = 0;

    virtual WindowId activeWindow() const = 0;
    virtual WindowInfoWrap requestInfo(WindowId wid) const = 0;
    virtual WindowInfoWrap requestInfoActive() const = 0;
    virtual bool isOnCurrentDesktop(WindowId wid) const = 0;
    virtual bool isOnCurrentActivity(WindowId wid) const = 0;
    virtual const std::list<WindowId> &windows() const = 0;

    virtual void setKeepAbove(const QDialog &dialog, bool above = true) const = 0;
    virtual void skipTaskBar(const QDialog &dialog) const = 0;
    virtual void slideWindow(QWindow &view, Slide location) const = 0;
    virtual void enableBlurBehind(QWindow &view) const = 0;
    virtual void setEdgeStateFor(QWindow *view, bool active) const = 0;

    void addDock(WindowId wid);
    void removeDock(WindowId wid);

signals:
    void activeWindowChanged(WindowId wid);
    void windowChanged(WindowId winfo);
    void windowAdded(WindowId wid);
    void windowRemoved(WindowId wid);
    void currentDesktopChanged();
    void currentActivityChanged();

protected:
    std::list<WindowId> m_windows;
    std::list<WindowId> m_docks;
    QPointer<KActivities::Consumer> m_activities;
};

// namespace alias
using WindowSystem = AbstractWindowInterface;

}
#endif // ABSTRACTWINDOWINTERFACE_H
