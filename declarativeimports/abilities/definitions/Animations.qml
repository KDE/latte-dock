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

    //! animations tracking
    property AnimationsTypes.Tracker needBothAxis: AnimationsTypes.Tracker{}
    property AnimationsTypes.Tracker needLength: AnimationsTypes.Tracker{}
    property AnimationsTypes.Tracker needThickness: AnimationsTypes.Tracker{}

    //! animations related to parabolic effect
    property int hoverPixelSensitivity: 1
}
