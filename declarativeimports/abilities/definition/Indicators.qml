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

import "./indicators" as IndicatorTypes

Item{
    property bool isEnabled: false

    readonly property real padding: Math.max(info.minLengthPadding, info.lengthPadding)

    property string type: "org.kde.latte.default"

    property QtObject configuration: null
    property QtObject resources: null   

    property Component indicatorComponent

    property IndicatorTypes.IndicatorInfo info: IndicatorTypes.IndicatorInfo{
        enabledForApplets: true
        needsIconColors: false
        needsMouseEventCoordinates: false

        providesFrontLayer: false
        providesHoveredAnimation: false
        providesClickedAnimation: false

        extraMaskThickness: 0

        minThicknessPadding: 0
        minLengthPadding: 0

        lengthPadding: 0.08
        appletLengthPadding: -1

        svgPaths: []
    }
}

