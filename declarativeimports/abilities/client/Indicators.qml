/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
        isEnabled: false
        type: "org.kde.latte.default"

        info.needsIconColors: false
        info.needsMouseEventCoordinates: false
        info.providesFrontLayer: false
        info.providesHoveredAnimation: false
        info.providesClickedAnimation: false
        info.providesTaskLauncherAnimation: false
        info.providesInAttentionAnimation: false
        info.providesGroupedWindowAddedAnimation: false
        info.providesGroupedWindowRemovedAnimation: false        
        info.extraMaskThickness: 0
        info.backgroundCornerMargin: 1.00
        info.lengthPadding: 0.08
        info.minThicknessPadding: 0
        info.minLengthPadding:0

        configuration: localLatteConfiguration
        resources: null

        indicatorComponent: isEnabled ? latteLocalIndicator : null

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
