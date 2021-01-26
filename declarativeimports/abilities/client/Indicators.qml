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

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

import "./indicators" as LocalIndicator

AbilityDefinition.Indicators {
    property Item bridge: null

    isEnabled: ref.indicators.isEnabled
    type: ref.indicators.type
    info: ref.indicators.info
    configuration: ref.indicators.configuration
    resources: ref.indicators.resources
    indicatorComponent: ref.indicators.indicatorComponent

    Item {
        id: ref
        readonly property Item indicators: bridge ? bridge.indicators : local
    }

    readonly property AbilityDefinition.Indicators local: AbilityDefinition.Indicators {
        isEnabled: true
        type: "org.kde.latte.default"

        info.needsIconColors: false
        info.needsMouseEventCoordinates: false
        info.providesFrontLayer: false
        info.providesHoveredAnimation: false
        info.providesClickedAnimation: false
        info.extraMaskThickness: 0
        info.lengthPadding: 0.08
        info.minThicknessPadding: 0
        info.minLengthPadding:0

        configuration: localLatteConfiguration
        resources: null

        indicatorComponent: latteLocalIndicator

        //! Local Latte Indicator Configuration
        LocalIndicator.LatteConfiguration{
            id: localLatteConfiguration
        }

        //! Local Latte Indicator implementation
        Component {
            id: latteLocalIndicator
            LocalIndicator.LatteIndicator{}
        }
    }
}
