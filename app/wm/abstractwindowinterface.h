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

// local
#include "schemecolors.h"
#include "windowinfowrap.h"
#include "../liblatte2/types.h"
#include "../liblatte2/extras.h"

// C++
#include <unordered_map>
#include <list>

// Qt
#include <QObject>
#include <QWindow>
#include <QDialog>
#include <QMap>
#include <QRect>
#include <QPoint>
#include <QPointer>
#include <QScreen>

// KDE
#include <KActivities/Consumer>

// Plasma
#include <Plasma>

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

    virtual void setViewExtraFlags(QWindow &view) = 0;
    virtual void setViewStruts(QWindow &view, const QRect &rect
                               , Plasma::Types::Location location) = 0;
    virtual void setWindowOnActivities(QWindow &window, const QStringList &activities) = 0;

    virtual void removeViewStruts(QWindow &view) const = 0;

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

    virtual void releaseMouseEventFor(WindowId wid) const = 0;
    virtual void requestToggleMaximized(WindowId wid) const = 0;
    virtual void requestMoveWindow(WindowId wid, QPoint from) const = 0;
    virtual bool windowCanBeDragged(WindowId wid) const = 0;

    void addView(WindowId wid);
    void removeView(WindowId wid);

    SchemeColors *schemeForWindow(WindowId wId);
    void setColorSchemeForWindow(WindowId wId, QString scheme);

signals:
    void activeWindowChanged(WindowId wid);
    void windowChanged(WindowId winfo);
    void windowAdded(WindowId wid);
    void windowRemoved(WindowId wid);
    void currentDesktopChanged();
    void currentActivityChanged();

protected:
    std::list<WindowId> m_windows;
    std::list<WindowId> m_views;
    QPointer<KActivities::Consumer> m_activities;

private slots:
    void updateDefaultScheme();

private:
    //! scheme file and its loaded colors
    QMap<QString, SchemeColors *> m_schemes;

    //! window id and its corresponding scheme file
    QMap<WindowId, QString> m_windowScheme;

};

// namespace alias
using WindowSystem = AbstractWindowInterface;

}
#endif // ABSTRACTWINDOWINTERFACE_H
