/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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

Item{
    property bool enabledForApplets: true
    property bool needsIconColors: false
    property bool needsMouseEventCoordinates: false

    property bool providesFrontLayer: false
    property bool providesHoveredAnimation: false
    property bool providesClickedAnimation: false

    property int extraMaskThickness: 0

    property real minThicknessPadding: 0
    property real minLengthPadding: 0

    property real lengthPadding: 0.08
    property real appletLengthPadding: -1

    property variant svgPaths: []
}
