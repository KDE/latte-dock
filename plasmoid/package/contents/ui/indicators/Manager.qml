/*
*  Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

import "options" as IndicatorOptions
import "styles" as IndicatorStyles
import "../task/indicator" as TaskIndicator

Item{
    id: managerIndicator

    readonly property Item configuration: explicitOptions

    readonly property bool isEnabled: true
    readonly property bool reversed: plasmoid.configuration.reverseLinesPosition
    readonly property string type: "org.kde.latte.default"

    readonly property Component indicatorComponent: latteStyleIndicator

    readonly property Item info: Item{
        readonly property bool needsIconColors: false
        readonly property bool needsMouseEventCoordinates: false
        readonly property bool providesFrontLayer: false
        readonly property bool providesHoveredAnimation: false
        readonly property bool providesClickedAnimation: false
        readonly property int extraMaskThickness: 0
        readonly property real minThicknessPadding: 0
        readonly property real minLengthPadding:0
    }

    IndicatorOptions.Latte {
        id : explicitOptions
    }

    //! Indicators Components
    Component {
        id: latteStyleIndicator
        IndicatorStyles.LatteIndicator{}
    }
}

