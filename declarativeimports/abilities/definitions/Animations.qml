/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0

import "./animations" as AnimationsTypes

Item {
    property bool active: false
    readonly property bool hasThicknessAnimation: (needBothAxis.count>0) || (needThickness.count>0)
    //! mouse sensitivity in pixels for parabolic effect hover animation
    property int hoverPixelSensitivity: 1

    //! animations properties
    property AnimationsTypes.Duration duration: AnimationsTypes.Duration {
        large: 500
        proposed: 1000
        small: 250
    }

    property AnimationsTypes.SpeedFactor speedFactor: AnimationsTypes.SpeedFactor {
        id: _speedFactor
        normal: 1.0
        current: 1.0
    }

    property AnimationsTypes.Requirements requirements: AnimationsTypes.Requirements {
        zoomFactor: 1.0
    }

    //! animations tracking
    property AnimationsTypes.Tracker needBothAxis: AnimationsTypes.Tracker{
        count: 0
        events: []

        //function addEvent(event);
        //function removeEvent(event);
    }
    property AnimationsTypes.Tracker needLength: AnimationsTypes.Tracker{
        count: 0
        events: []

        //function addEvent(event);
        //function removeEvent(event);
    }
    property AnimationsTypes.Tracker needThickness: AnimationsTypes.Tracker{
        count: 0
        events: []

        //function addEvent(event);
        //function removeEvent(event);
    }
}
