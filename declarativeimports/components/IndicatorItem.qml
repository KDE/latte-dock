/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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
    readonly property Item level: parent && parent.hasOwnProperty("level") ? parent.level : null
    readonly property Item indicator: parent && parent.hasOwnProperty("level") ? parent.level.bridge : null

    //! indicator is using main colors from icon e.g. unity style
    property bool needsIconColors: false
    //! indicator is using the mouse events coordinates e.g. for animations
    property bool needsMouseEventCoordinates: false
    //! indicator provides also foreground layer that should be drawn on top of the Items icons
    property bool providesFrontLayer: false
    //! indicator provides its own hovering animations. Latte should not provide
    //! any hovering animation in that case
    property bool providesHoveredAnimation: false
    //! indicator provides its own clicked animations. Latte should not provide
    //! any clicked animation in that case
    property bool providesClickedAnimation: false

    //! this indicator visuals go out of the regular mask thickness and this variable sets
    //! the pixels needed outside of the mask thickness e.g. a glow
    property int extraMaskThickness: 0

    //! this indicator specifies a minimum thickness padding in order to be drawn correctly.
    //! Values below the specified value are ignored. This value is a percentage,
    //! e.g 0.06 -> 6%
    property real minThicknessPadding: 0
    //! this indicator specifies a minimum length padding in order to be drawn correctly.
    //! Values below the specified value are ignored. This value is a percentage,
    //! e.g 0.06 -> 6%
    property real minLengthPadding: 0

    //! svg image paths either from plasma theme or local files relevant to indicator "ui" directory
    //! in order to reduce resources usage
    property var svgImagePaths: []
}
