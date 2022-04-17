/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

SequentialAnimation{
    readonly property string bouncePropertyName: taskItem.isVertical ? "iconAnimatedOffsetX" : "iconAnimatedOffsetY"

    Component.onDestruction: {
        //! make sure to return on initial position even when the animation is destroyed in the middle
        if (taskItem.isVertical) {
            taskItem.iconAnimatedOffsetX = 0;
        } else {
            taskItem.iconAnimatedOffsetY = 0;
        }
    }

    //Ghost animation that acts as a delayer
    PropertyAnimation {
        target: taskItem.parabolicItem
        property: "opacity"
        to: 1
        duration:  50
        easing.type: Easing.InQuad
    }
    //end of ghost animation

    ParallelAnimation {
        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoom"
            to: 1
            duration: launcherAnimation.speed
            easing.type: Easing.OutQuad
        }

        PropertyAnimation {
            target: taskItem
            property: bouncePropertyName
            to: taskItem.abilities.metrics.iconSize
            duration: launcherAnimation.speed
            easing.type: Easing.OutQuad
        }
    }

    PropertyAnimation {
        target: taskItem
        property: bouncePropertyName
        to: 0
        duration: 4*launcherAnimation.speed
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
