/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

////bouncing task, e.g. on launcher activating and when a new window is
////added in a group task
Item{
    id:launcherAnimation
    property bool launchedAlready: false
    property int speed: 0.9 * taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large

    readonly property string needThicknessEvent: launcherAnimation + "_launcher"

    Loader {
        id: launcherAnimationLoader
        source: "launcher/BounceAnimation.qml"
    }

    Connections {
        target: launcherAnimationLoader.item

        onStopped: {
            if (!taskItem.inRemoveStage) {
                taskItem.inBouncingAnimation = false;
                tasksExtendedManager.removeWaitingLauncher(taskItem.launcherUrl);
            }

            taskItem.abilities.parabolic.setDirectRenderingEnabled(false);
            clearAnimationsSignals();

            taskItem.setBlockingAnimation(false);
            taskItem.animationEnded();
        }
    }

    function clearAnimationsSignals() {
        if ( launchedAlready && root.noTasksInAnimation>0 ) {
            root.noTasksInAnimation--;
        }

        if ( launchedAlready ) {
            taskItem.abilities.animations.needThickness.removeEvent(needThicknessEvent);
        }

        launchedAlready = false;
    }

    function init(){
        //console.log ("Nooo 1 : "+root.noTasksInAnimation);
        if(!launchedAlready) {
            taskItem.abilities.parabolic.invkClearZoom();
            launchedAlready = true;
            taskItem.abilities.animations.needThickness.addEvent(needThicknessEvent);

            taskItem.abilities.parabolic.setDirectRenderingEnabled(false);

            root.noTasksInAnimation++;
            taskItem.inBouncingAnimation = true;
            taskItem.setBlockingAnimation(true);

            //trying to fix the ListView nasty behavior
            //during the removal the anchoring for ListView children changes a lot
            var previousTask = icList.childAtIndex(taskItem.lastValidIndex-1);
            if (previousTask !== undefined && !previousTask.isStartup && !previousTask.inBouncingAnimation){
                if (root.vertical) {
                    taskItem.anchors.top = previousTask.bottom;
                } else {
                    taskItem.anchors.left = previousTask.right;
                }
            }
        }

        taskItem.parabolicItem.zoomLength = taskItem.parabolicItem.zoom;
        taskItem.parabolicItem.zoomThickness = taskItem.parabolicItem.zoom;
    }

    function startLauncherAnimation(){
        if(root.launcherBouncingEnabled){
            taskItem.animationStarted();
            init();
            taskItem.launcherAction();
            launcherAnimationLoader.item.start();
        } else {
            launcherAnimationLoader.item.stop();
            taskItem.launcherAction();
        }
    }


    Component.onCompleted: {
        taskItem.launcherAnimationRequested.connect(startLauncherAnimation);
    }

    Component.onDestruction: {
        clearAnimationsSignals();
        taskItem.launcherAnimationRequested.disconnect(startLauncherAnimation);
    }
}
/////////////////// end of launcher animation
