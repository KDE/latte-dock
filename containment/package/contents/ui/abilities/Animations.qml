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
import org.kde.latte.private.app 0.1 as LatteApp

import org.kde.latte.abilities.containers 0.1 as ContainerAbility

ContainerAbility.Animations {
    property Item metrics: null
    property QtObject settings: null

    //! Public Properties
    active: plasmoid.configuration.animationsEnabled && LatteCore.WindowSystem.compositingActive

    duration.large: LatteCore.Environment.longDuration
    duration.proposed: speedFactor.current * 2.8 * duration.large
    duration.small: LatteCore.Environment.shortDuration

    speedFactor.normal: 1.0
    speedFactor.current: {
        if (!active || plasmoid.configuration.durationTime === 0) {
            return 0;
        }

        if (plasmoid.configuration.durationTime === 1 ) {
            return 0.75;
        } else if (plasmoid.configuration.durationTime === 2) {
            return speedFactor.normal;
        } else if (plasmoid.configuration.durationTime === 3) {
            return 1.15;
        }

        return speedFactor.normal;
    }

    //! animations related to parabolic effect
    hoverPixelSensitivity: {
        if (!settings || settings.sensitivity === LatteApp.Settings.HighMouseSensitivity) {
            return 1;
        } else if (settings.sensitivity === LatteApp.Settings.MediumMouseSensitivity) {
            return Math.max(3, metrics.iconSize / 18);
        } else if (settings.sensitivity === LatteApp.Settings.LowMouseSensitivity) {
            return Math.max(5, metrics.iconSize / 10);
        }
    }
}
