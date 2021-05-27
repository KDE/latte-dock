/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.app 0.1 as LatteApp

import "./privates" as Ability

Ability.AnimationsPrivate {
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

    //! do not update during dragging/moving applets inConfigureAppletsMode
    updateIsBlocked: (root.dragOverlay && root.dragOverlay.pressed)
                     || layouter.appletsInParentChange
}
