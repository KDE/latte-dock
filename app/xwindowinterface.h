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

#ifndef XWINDOWINTERFACE_H
#define XWINDOWINTERFACE_H

#include "abstractwindowinterface.h"

#include <QObject>
#include <QPointer>

#include <KWindowInfo>
#include <Plasma>
#include <KActivities/Consumer>

namespace Latte {

class XWindowInterface : public AbstractWindowInterface {
    Q_OBJECT

public:
    XWindowInterface(QQuickWindow *const view, QObject *parent);
    virtual ~XWindowInterface();

    void setDockDefaultFlags() override;

    WId activeWindow() const override;
    WindowInfoWrap requestInfo(WId wid) const override;
    WindowInfoWrap requestInfoActive() const override;
    bool isOnCurrentDesktop(WId wid) const override;
    const std::list<WId> &windows() const override;

    void setDockStruts(const QRect &dockRect, Plasma::Types::Location location) const override;
    void removeDockStruts() const override;


private:
    bool isValidWindow(const KWindowInfo &winfo) const;
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);

    WId m_desktopId;
    QPointer<KActivities::Consumer> activities;

    QList<QMetaObject::Connection> connections;
};

}

#endif // XWINDOWINTERFACE_H



