/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

///////Restore Zoom Animation/////
SequentialAnimation{
    id: fastRestoreAnimation

    property int speed: 5*taskItem.animationTime

    ScriptAction {
        script: {
            taskItem.inFastRestoreAnimation = true;
            taskItem.inBlockingAnimation = true;
        }
    }

    ParallelAnimation {
        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoom"
            to: 1
            duration: fastRestoreAnimation.speed
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoomLength"
            to: 1
            duration: fastRestoreAnimation.speed
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "zoomThickness"
            to: 1
            duration: fastRestoreAnimation.speed
            easing.type: Easing.InCubic
        }
    }

    onStopped: {
        newWindowAnimation.stop();

        taskItem.inBlockingAnimation = false;
        taskItem.inFastRestoreAnimation = false;
        // console.log("Fast Restore Animation ended...");
    }

}
