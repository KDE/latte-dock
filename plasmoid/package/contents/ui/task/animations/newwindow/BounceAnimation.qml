/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

SequentialAnimation{
    loops: newWindowAnimation.isDemandingAttention ? 20 : 1

    readonly property string bouncePropertyName: taskItem.isVertical ? "iconAnimatedOffsetX" : "iconAnimatedOffsetY"

    Component.onCompleted: {
        if (newWindowAnimation.inDelayedStartup) {
            newWindowAnimation.inDelayedStartup = false;
            newWindowAnimation.init();
            start();
        }
    }

    Component.onDestruction: {
        //! make sure to return on initial position even when the animation is destroyed in the middle
        if (taskItem.isVertical) {
            taskItem.iconAnimatedOffsetX = 0;
        } else {
            taskItem.iconAnimatedOffsetY = 0;
        }
    }

    PropertyAnimation {
        target: taskItem
        property: bouncePropertyName
        to: 0.6 * taskItem.abilities.metrics.iconSize
        duration: newWindowAnimation.speed
        easing.type: Easing.OutQuad
    }

    PropertyAnimation {
        target: taskItem
        property: bouncePropertyName
        to: 0
        duration: 4.4*newWindowAnimation.speed
        easing.type: Easing.OutBounce
    }

    onStopped: {
        //! make sure to return on initial position even when the animation is destroyed in the middle
        if (taskItem.isVertical) {
            taskItem.iconAnimatedOffsetX = 0;
        } else {
            taskItem.iconAnimatedOffsetY = 0;
        }
    }
}
