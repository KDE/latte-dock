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
#include <QPointer>
#include <QRect>
#include <QQuickView>
#include <QDialog>
#include <QScreen>

#include <Plasma>
#include <KActivities/Consumer>

namespace Latte {

class XWindowInterface;

class AbstractWindowInterface : public QObject {
    Q_OBJECT

public:
    enum class Slide {
        None,
        Top,
        Left,
        Bottom,
        Right,
    };

    explicit AbstractWindowInterface(QObject *parent = nullptr);
    virtual ~AbstractWindowInterface();

    virtual void setDockExtraFlags(QQuickWindow &view) = 0;
    virtual void setDockStruts(WId dockId, const QRect &dockRect
                               , const QScreen &screen, Plasma::Types::Location location) const = 0;

    virtual void removeDockStruts(WId dockId) const = 0;

    virtual WId activeWindow() const = 0;
    virtual WindowInfoWrap requestInfo(WId wid) const = 0;
    virtual WindowInfoWrap requestInfoActive() const = 0;
    virtual bool isOnCurrentDesktop(WId wid) const = 0;
    virtual const std::list<WId> &windows() const = 0;

    virtual void skipTaskBar(const QDialog &dialog) const = 0;
    virtual void slideWindow(QQuickWindow &view, Slide location) const = 0;
    virtual void enableBlurBehind(QQuickWindow &view) const = 0;

    void addDock(WId wid);
    void removeDock(WId wid);

    static AbstractWindowInterface &self();

signals:
    void activeWindowChanged(WId wid);
    void windowChanged(WId winfo);
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void currentDesktopChanged();
    void currentActivityChanged();

protected:
    std::list<WId> m_windows;
    std::list<WId> m_docks;
    QPointer<KActivities::Consumer> m_activities;

    static std::unique_ptr<AbstractWindowInterface> m_wm;
};

// namespace alias
using WindowSystem = AbstractWindowInterface;

}
#endif // ABSTRACTWINDOWINTERFACE_H
