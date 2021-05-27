/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.abilities.client 0.1 as AbilityClient

Item {
    id: _abilityContainer
    property Item bridge: null
    property Item layout: null

    readonly property bool isLayoutListViewContainer: layout && layout.parent && layout.parent instanceof ListView
    readonly property bool isLayoutGridContainer: layout && layout instanceof Grid

    readonly property bool inDesktop: plasmoid.location === PlasmaCore.Types.Floating
                                      || plasmoid.location === PlasmaCore.Types.Desktop

    readonly property int alignment: _containment.alignment
    readonly property int location: inDesktop ? PlasmaCore.Types.BottomEdge : plasmoid.location
    readonly property int thickness: _animations.hasThicknessAnimation ? _metrics.mask.thickness.maxZoomedForItems : // dont clip bouncing tasks when zoom=1
                                                                         _metrics.mask.thickness.normalForItems

    readonly property real layoutWidth: {
        if (isLayoutListViewContainer) {
            return plasmoid.formFactor !== PlasmaCore.Types.Vertical ? layout.parent.width : thickness;
        } else if (isLayoutGridContainer) {
            return plasmoid.formFactor !== PlasmaCore.Types.Vertical ? layout.width : thickness
        }

        return 0;
    }

    readonly property real layoutHeight: {
        if (isLayoutListViewContainer) {
            return plasmoid.formFactor === PlasmaCore.Types.Vertical ? layout.parent.height : thickness;
        } else if (isLayoutGridContainer) {
            return plasmoid.formFactor !== PlasmaCore.Types.Vertical ? layout.height : thickness
        }

        return 0;
    }

    //! basic
    readonly property alias animations: _animations
    readonly property alias containment: _containment
    readonly property alias debug: _debug
    readonly property alias environment: _environment
    readonly property alias indexer: _indexer
    readonly property alias indicators: _indicators
    readonly property alias metrics: _metrics
    readonly property alias myView: _myView
    readonly property alias parabolic: _parabolic
    readonly property alias shortcuts: _shortcuts
    readonly property alias requires: _requires
    readonly property alias thinTooltip: _thinTooltip
    readonly property alias userRequests: _userRequests

    AbilityClient.Animations {
        id: _animations
        bridge: _abilityContainer.bridge
    }

    AbilityClient.Containment {
        id: _containment
        bridge: _abilityContainer.bridge
    }

    AbilityClient.Debug {
        id: _debug
        bridge: _abilityContainer.bridge
    }

    AbilityClient.Environment {
        id: _environment
        bridge: _abilityContainer.bridge
    }

    AbilityClient.Indexer {
        id: _indexer
        bridge: _abilityContainer.bridge
        layout: _abilityContainer.layout
    }

    AbilityClient.Indicators {
        id: _indicators
        bridge: _abilityContainer.bridge
    }

    AbilityClient.Metrics {
        id: _metrics
        bridge: _abilityContainer.bridge
        parabolic: _parabolic
    }

    AbilityClient.MyView {
        id: _myView
        bridge: _abilityContainer.bridge
    }

    AbilityClient.ParabolicEffect {
        id: _parabolic
        bridge: _abilityContainer.bridge
        indexer: _indexer
        layout: _abilityContainer.layout
    }

    AbilityClient.PositionShortcuts {
        id: _shortcuts
        bridge: _abilityContainer.bridge
        indexer: _indexer
    }

    AbilityClient.ThinTooltip {
        id: _thinTooltip
        bridge: _abilityContainer.bridge
    }

    AbilityClient.Requirements{
        id: _requires
        bridge: _abilityContainer.bridge
    }

    AbilityClient.UserRequests {
        id: _userRequests
        bridge: _abilityContainer.bridge
    }


    Loader {
        id: anchorsLoader
        active: isLayoutListViewContainer || isLayoutGridContainer
        source: "./appletabilities/ContainerAnchorBindings.qml"

        readonly property Item anchoredItem: isLayoutListViewContainer ? layout.parent : layout
    }

    Loader {
        active: isLayoutListViewContainer
        source: "./appletabilities/ContainerListViewBindings.qml"
    }

    Loader {
        active: isLayoutGridContainer
        source: "./appletabilities/ContainerGridBindings.qml"
    }
}
