/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

SequentialAnimation{
    //Ghost animation that acts as a delayer
    PropertyAnimation {
        target: taskItem.parabolicItem
        property: "opacity"
        to: 1
        duration:  50
        easing.type: Easing.InQuad
    }
    //end of ghost animation

    ParallelAnimation{
        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoomThickness"
            to: taskItem.containsMouse ? 1+2*(taskItem.abilities.parabolic.factor.maxZoom-1) : 1 + (1.5 * (taskItem.abilities.parabolic.factor.maxZoom-1))
            duration: launcherAnimation.speed
            easing.type: Easing.OutQuad
        }

        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoomLength"
            to: 1
            duration: launcherAnimation.speed
            easing.type: Easing.OutQuad
        }
    }

    PropertyAnimation {
        target: taskItem.parabolicItem
        property: "zoomThickness"
        to: 1
        duration: 4*launcherAnimation.speed
        easing.type: Easing.OutBounce
    }

    ParallelAnimation{
        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoomLength"
            to: 1
            duration: 0.5 * taskItem.abilities.animations.speedFactor.current*launcherAnimation.speed
            easing.type: Easing.OutBounce
        }

        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoom"
            to: 1
            duration:  0.5 * taskItem.abilities.animations.speedFactor.current*launcherAnimation.speed
            easing.type: Easing.OutQuad
        }
    }
}
