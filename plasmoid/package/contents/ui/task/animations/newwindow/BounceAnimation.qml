/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

SequentialAnimation{
    alwaysRunToEnd: true
    loops: newWindowAnimation.isDemandingAttention ? 20 : 1

    Component.onCompleted: {
        if (newWindowAnimation.inDelayedStartup) {
            newWindowAnimation.inDelayedStartup = false;
            newWindowAnimation.init();
            start();
        }
    }

    ParallelAnimation{
        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoomThickness"
            to: 1 + (thickPercentage * 2 * (taskItem.abilities.animations.requirements.zoomFactor-1))
            duration: newWindowAnimation.speed
            easing.type: Easing.OutQuad

            property real thickPercentage: taskItem.inAttentionBuiltinAnimation ? 0.8 : 0.6
        }

        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoomLength"
            to: 1
            duration: newWindowAnimation.speed
            easing.type: Easing.OutQuad
        }
    }

    PropertyAnimation {
        target: taskItem.parabolicItem
        property: "zoomThickness"
        to: 1
        duration: 4.4*newWindowAnimation.speed
        easing.type: Easing.OutBounce
    }
}
