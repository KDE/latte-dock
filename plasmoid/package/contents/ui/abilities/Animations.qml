/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7

Item {
    readonly property bool hasThicknessAnimation: latteBridge ? latteBridge.animations.hasThicknessAnimation : false


    //! COntainment properties
    readonly property alias active: apis.active

    //! animations tracking
    readonly property alias needBothAxis: apis.needBothAxis
    readonly property alias needLength: apis.needLength
    readonly property alias needThickness: apis.needThickness

    //! animations properties
    readonly property alias shortDuration: apis.shortDuration
    readonly property alias longDuration: apis.longDuration

    readonly property alias hasThicknessAnimation:  apis.hasThicknessAnimation

    readonly property Item duration: apis.duration
    readonly property Item speedFactor: apis.speedFactor

    //! parabolic effect animations
    readonly property alias minZoomFactor: apis.minZoomFactor
    readonly property alias hoverPixelSensitivity: apis.hoverPixelSensitivity
}

