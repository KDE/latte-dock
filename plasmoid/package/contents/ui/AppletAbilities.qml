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

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.abilities.client 0.1 as AbilityClient

import "abilities" as Ability

AbilityClient.AppletAbilities {
    id: _abilityContainer
    property QtObject tasksModel: null

    readonly property alias launchers: _launchers

    Ability.Launchers {
        id: _launchers
        bridge: _abilityContainer.bridge
        layout: _abilityContainer.layout
        tasksModel: _abilityContainer.tasksModel
    }

    //! local properties
    readonly property int maxIconSizeInPlasma: Math.max(plasmoid.configuration.iconSize, 16)
    readonly property int panelThickness: (root.vertical ? root.width : root.height)

    //! Connections
    Connections {
        target: _abilityContainer.shortcuts
        onDisabledIsStealingGlobalPositionShortcuts: {
            plasmoid.configuration.isPreferredForPositionShortcuts = false;
        }
    }

    Connections {
        target: _abilityContainer.launchers
        onDisabledIsStealingDroppedLaunchers: {
            plasmoid.configuration.isPreferredForDroppedLaunchers = false;
        }
    }

    Connections {
        target: _abilityContainer.userRequests
        onSglViewType: {
            if (viewType === LatteCore.Types.DockView) {
                plasmoid.configuration.animationLauncherBouncing = true;
                plasmoid.configuration.animationWindowInAttention = true;
                plasmoid.configuration.animationWindowAddedInGroup = true;
            } else if (viewType === LatteCore.Types.PanelView) {
                plasmoid.configuration.animationLauncherBouncing = false;
                plasmoid.configuration.animationWindowInAttention = false;
                plasmoid.configuration.animationWindowAddedInGroup = false;
            }
        }
    }
}
