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
    readonly property bool active: latteBridge ? latteBridge.animations.active : /?/

    readonly property bool hasThicknessAnimation: latteBridge ? latteBridge.animations.hasThicknessAnimation : false

    //! animations tracking
    readonly property Item needBothAxis: latteBridge ? latteBridge.animations.needBothAxis : /?/
    readonly property Item needLength: latteBridge ? latteBridge.animations.needLength : /?/
    readonly property Item needThickness: latteBridge ? latteBridge.animations.needThickness : /?/

    //! animations properties
    readonly property int shortDuration: latteBridge ? latteBridge.animations.shortDuration : /?/
    readonly property int longDuration: latteBridge ? latteBridge.animations.longDuration : /?/

    readonly property alias hasThicknessAnimation:  latteBridge ? latteBridge.animations.hasThicknessAnimation : /?/

    readonly property Item duration: latteBridge ? latteBridge.animations.duration : /?/
    readonly property Item speedFactor: latteBridge ? latteBridge.animations.speedFactor : /?/

    //! parabolic effect animations
    readonly property real minZoomFactor: latteBridge ? latteBridge.animations.minZoomFactor : /?/
    readonly property alias hoverPixelSensitivity: latteBridge ? latteBridge.animations.hoverPixelSensitivity : /?/
}
