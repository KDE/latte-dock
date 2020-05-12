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

import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.abilities.applets 0.1 as AppletAbility

AppletAbility.Animations {
    //! Public Local Properties
    local {
        active: speedFactor.current !== 0
        hoverPixelSensitivity: 1

        speedFactor.normal: active ? speedFactor.current : 1.0
        speedFactor.current: plasmoid.configuration.durationTime

        duration.small: LatteCore.Environment.shortDuration
        duration.large: LatteCore.Environment.longDuration
    }

    //! Private Local Properties
    readonly property bool launcherBouncingEnabled: active && plasmoid.configuration.animationLauncherBouncing
    readonly property bool windowInAttentionEnabled: active && plasmoid.configuration.animationWindowInAttention
    readonly property bool newWindowSlidingEnabled: active && plasmoid.configuration.animationNewWindowSliding
    readonly property bool windowAddedInGroupEnabled: active && plasmoid.configuration.animationWindowAddedInGroup
    readonly property bool windowRemovedFromGroupEnabled: active && plasmoid.configuration.animationWindowRemovedFromGroup

}

