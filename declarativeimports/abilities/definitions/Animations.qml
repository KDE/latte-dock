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

    property bool hasThicknessAnimation: false

    //! animations properties
    property int shortDuration: 250
    property int longDuration: 500   

    readonly property alias duration: _duration

    readonly property alias speedFactor: _speedFactor

    //! animations tracking
    readonly property alias needBothAxis: _bothAxis
    readonly property alias needLength: _needLength
    readonly property alias needThickness: _needThickness

    //! animations related to parabolic effect
    property int hoverPixelSensitivity: 1
    property real minZoomFactor : 1.0

    //! Grouped Properties Structures
    AnimationsTypes.SpeedFactor {
        id: _speedFactor
        normal: 1.0
        current: 1.0
    }

    AnimationsTypes.Duration {
        id: _duration
        large: 500
        proposed: 1000
        small: 250
    }

    AnimationsTypes.Tracker {
        id: _bothAxis
    }

    AnimationsTypes.Tracker {
        id: _needLength
    }

    AnimationsTypes.Tracker {
        id: _needThickness
    }
}
