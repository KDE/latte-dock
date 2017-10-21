/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

SequentialAnimation {
    id: taskRealRemovalAnimation
    PropertyAction { target: mainItemContainer; property: "ListView.delayRemove"; value: true }
    PropertyAction { target: mainItemContainer; property: "inRemoveStage"; value: true }

    //Animation Add/Remove (1) - when is window with no launcher, animations enabled
    //Animation Add/Remove (4) - the user removes a launcher, animation enabled
    property bool animation1: ((((tasksModel.launcherPosition(mainItemContainer.launcherUrl) == -1)
                                 && (tasksModel.launcherPosition(mainItemContainer.launcherUrlWithIcon) == -1) )
                                || !launcherIsPresent(mainItemContainer.launcherUrl))
                               && !mainItemContainer.isStartup)

    property bool animation4: ((mainItemContainer.launcherUrl===root.launcherForRemoval
                                || mainItemContainer.launcherUrlWithIcon===root.launcherForRemoval ) && !mainItemContainer.isStartup)

    property bool enabledAnimation: (animation1 || animation4) && (root.durationTime !== 0) && !mainItemContainer.inBouncingAnimation;

    ScriptAction{
        script:{
            root.mouseWasEntered.disconnect(signalMouseWasEntered);
            root.draggingFinished.disconnect(handlerDraggingFinished);
            root.clearZoomSignal.disconnect(clearZoom);
            root.publishTasksGeometries.disconnect(slotPublishGeometries);
            root.showPreviewForTasks.disconnect(slotShowPreviewForTasks);
            root.updateScale.disconnect(wrapper.signalUpdateScale);

            mainItemContainer.inAnimation = true;
            icList.delayingRemoval = true;
            mainItemContainer.inAddRemoveAnimation = true;
            mainItemContainer.inRemoveStage = true;

            //trying to fix the ListView nasty behavior
            //during the removal the anchoring for ListView children changes a lot
            var previousTask = icList.childAtIndex(mainItemContainer.lastValidIndex-1);
            if (previousTask !== undefined && !previousTask.isStartup && !previousTask.inBouncingAnimation){
                if (root.vertical) {
                    mainItemContainer.anchors.top = previousTask.bottom;
                } else {
                    mainItemContainer.anchors.left = previousTask.right;
                }
            }

            // console.log("1." + mainItemContainer.launcherUrl + " - " + taskRealRemovalAnimation.enabledAnimation);
            // console.log("2." + root.launcherForRemoval + " - " + mainItemContainer.isLauncher);

            root.signalAnimationsNeedLength(1);
        }
    }

    //Ghost animation that acts as a delayer in case there is a bouncing animation
    //taking place
    PropertyAnimation {
        target: wrapper
        property: "opacity"
        to: 1

        //this duration must be a bit less than the bouncing animation. Otherwise the
        //smooth trasition between removals is breaking
        duration:  mainItemContainer.inBouncingAnimation ? 4*launcherSpeedStep + 50 : 0
        easing.type: Easing.InQuad

        property int launcherSpeedStep: root.durationTime * 0.8 * units.longDuration
    }
    //end of ghost animation

    PropertyAnimation {
        target: wrapper
        property: "mScale"
        to: 1
        duration:  taskRealRemovalAnimation.enabledAnimation ? showWindowAnimation.speed : 0
        easing.type: Easing.InQuad
    }

    //PropertyAction { target: wrapper; property: "opacity"; value: isWindow ? 0 : 1 }
    //animation mainly for launchers removal and startups
    ParallelAnimation{
        id: removalAnimation

        // property int speed: (IsStartup && !mainItemContainer.visible)? 0 : 400
        //property int speed: 400
        NumberAnimation {
            target: wrapper;
            property: "opacity";
            to: 0;
            duration: taskRealRemovalAnimation.enabledAnimation ? showWindowAnimation.speed : 0
            easing.type: Easing.InQuad }

        PropertyAnimation {
            target: wrapper
            property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
            to: 0
            duration:  taskRealRemovalAnimation.enabledAnimation ? showWindowAnimation.speed : 0
            easing.type: Easing.InQuad
        }
    }

    //smooth move into place the surrounding tasks
    PropertyAnimation {
        target: wrapper
        property: (icList.orientation == Qt.Vertical) ? "tempScaleHeight" : "tempScaleWidth"
        to: 0
        duration:  taskRealRemovalAnimation.enabledAnimation ? showWindowAnimation.speed : 0
        easing.type: Easing.InQuad
    }

    ScriptAction{
        script:{
            if (showWindowAnimation.animationSent){
                //console.log("SAFETY REMOVAL 1: animation removing ended");
                showWindowAnimation.animationSent = false;
                root.signalAnimationsNeedLength(-1);
            }

            root.signalAnimationsNeedLength(-1);

            mainItemContainer.inAnimation = false;
            icList.delayingRemoval = false;

            if(mainItemContainer.launcherUrl===root.launcherForRemoval && mainItemContainer.isLauncher)
                root.launcherForRemoval="";

            //send signal that the launcher is really removing
            if (mainItemContainer.inBouncingAnimation) {
                mainItemContainer.visible = false;
                root.removeWaitingLauncher(mainItemContainer.launcherUrl);
            }
        }
    }

    PropertyAction { target: mainItemContainer; property: "ListView.delayRemove"; value: false }
}
