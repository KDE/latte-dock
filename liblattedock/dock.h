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

#ifndef DOCK_H
#define DOCK_H

#include <QObject>
#include <QMetaEnum>
#include <QMetaType>

namespace Latte {

class Dock {
    Q_GADGET

public:
    Dock() = delete;
    ~Dock() {}

    enum Visibility {
        None = -1,
        AlwaysVisible = 0,
        AutoHide,
        DodgeActive,
        DodgeMaximized,
        DodgeAllWindows
    };
    Q_ENUM(Visibility)

    enum Alignment {
        Center = 0,
        Left,
        Right,
        Top,
        Bottom,
        Justify = 10
    };
    Q_ENUM(Alignment)

    enum SessionType {
        DefaultSession = 0,
        OnTheRoadSession
    };
    Q_ENUM(SessionType)

};

}//end of namespace
#endif
