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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.abilities.client 0.1 as AbilityClient

Item {
    id: _abilityContainer
    property Item bridge: null
    property Item layout: null

    readonly property bool isLayoutListViewContainer: layout && layout.parent && layout.parent instanceof ListView
    readonly property bool isLayoutRowColumnContainer: layout && ((layout instanceof Row) || (layout instanceof Column))
    readonly property bool isLayoutGridContainer: layout && layout instanceof Grid

    readonly property bool inDesktop: plasmoid.location === PlasmaCore.Types.Floating
                                      || plasmoid.location === PlasmaCore.Types.Desktop

    readonly property int alignment: _containment.alignment
    readonly property int location: inDesktop ? PlasmaCore.Types.BottomEdge : plasmoid.location
    readonly property int thickness: _animations.hasThicknessAnimation ? _metrics.mask.thickness.zoomedForItems :
                                                                         _metrics.mask.thickness.normalForItems

    //! basic
    readonly property alias animations: _animations
    readonly property alias containment: _containment
    readonly property alias debug: _debug
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
        active: isLayoutListViewContainer || isLayoutRowColumnContainer || isLayoutGridContainer
        source: "./appletabilities/ContainerAnchorBindings.qml"

        readonly property Item anchoredItem: isLayoutListViewContainer ? layout.parent : layout
    }

    Loader {
        active: isLayoutListViewContainer
        source: "./appletabilities/ContainerListViewBindings.qml"
    }
}
