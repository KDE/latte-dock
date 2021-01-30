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

import org.kde.latte.core 0.2 as LatteCore

///item's added Animation
SequentialAnimation{
    id:showWindowAnimation
    property int speed: taskItem.abilities.animations.newWindowSlidingEnabled ? (1.2 * taskItem.abilities.animations.speedFactor.normal * taskItem.abilities.animations.duration.large) : 0
    property bool animationSent: false

    readonly property string needLengthEvent: showWindowAnimation + "_showwindow"

    //Ghost animation that acts as a delayer, in order to fix #342
    PropertyAnimation {
        target: taskItem.parabolicItem
        property: "opacity"
        to: 0
        //it is not depend to durationTime when animations are active
        duration: taskItem.abilities.animations.newWindowSlidingEnabled ? 750 : 0
        easing.type: Easing.InQuad
    }
    //end of ghost animation

    ScriptAction{
        script:{
            if (!showWindowAnimation.animationSent) {
                showWindowAnimation.animationSent = true;
                taskItem.abilities.animations.needLength.addEvent(needLengthEvent);
            }
        }
    }

    PropertyAnimation {
        target: taskItem.parabolicItem
        property: (icList.orientation == Qt.Vertical) ? "tempScaleHeight" : "tempScaleWidth"
        to: 1
        duration: showWindowAnimation.speed
        easing.type: Easing.OutQuad
    }

    ParallelAnimation{

        PropertyAnimation {
            target: taskItem.parabolicItem
            property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
            to: 1
            duration: showWindowAnimation.speed
            easing.type: Easing.OutQuad
        }


        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "opacity"
            from: 0
            to: 1
            duration: showWindowAnimation.speed
            easing.type: Easing.OutQuad
        }
    }

    onStopped: {
        taskItem.inAddRemoveAnimation = false;

        if (tasksExtendedManager.toBeAddedLauncherExists(taskItem.launcherUrl)) {
            tasksExtendedManager.removeToBeAddedLauncher(taskItem.launcherUrl);
        }

        if(taskItem.isWindow || taskItem.isStartup){
            publishGeometryTimer.start();
            if (taskItem.isDemandingAttention){
                taskItem.groupWindowAdded();
            }
        }
        taskItem.inAnimation = false;

        if (showWindowAnimation.animationSent) {
            taskItem.abilities.animations.needLength.removeEvent(needLengthEvent);
            showWindowAnimation.animationSent = false;
        }
    }

    function execute(){
        //trying to fix the ListView nasty behavior
        //during the removal the anchoring for ListView children changes a lot
        if (isWindow){
            var previousTask = icList.childAtIndex(index-1);
            var nextTask = icList.childAtIndex(index+1);
            if (previousTask !== undefined && nextTask !== undefined && nextTask.inBouncingAnimation){
                if (root.vertical) {
                    taskItem.anchors.top = previousTask.bottom;
                } else {
                    taskItem.anchors.left = previousTask.right;
                }
            }
        }

        var hasShownLauncher = ((tasksModel.launcherPosition(taskItem.launcherUrl) !== -1)
                                    || (tasksModel.launcherPosition(taskItem.launcherUrlWithIcon) !== -1) );

        var launcherIsAlreadyShown = hasShownLauncher && isLauncher && !root.inActivityChange && !tasksExtendedManager.toBeAddedLauncherExists(taskItem.launcherUrl) ;

        //Animation Add/Remove (2) - when is window with no launcher, animations enabled
        //Animation Add/Remove (3) - when is launcher with no window, animations enabled
        var animation2 = ((!hasShownLauncher || !taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl))
                          && taskItem.isWindow
                          && LatteCore.WindowSystem.compositingActive);

        var animation3 = (!tasksExtendedManager.immediateLauncherExists(taskItem.launcherUrl)
                          && taskItem.isLauncher
                          && LatteCore.WindowSystem.compositingActive);

        var activities = tasksModel.launcherActivities(taskItem.launcherUrl);
        var animation6 = (root.inActivityChange && taskItem.isWindow
                          && activities.indexOf(activityInfo.currentActivity)>=0
                          && activities.indexOf(activityInfo.previousActivity) === -1
                          && LatteCore.WindowSystem.compositingActive);


        //startup without launcher, animation should be blocked
        var launcherExists = !(!hasShownLauncher || !taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl));

        //var hideStartup =  launcherExists && taskItem.isStartup; //! fix #976
        var hideWindow =  (root.showWindowsOnlyFromLaunchers || root.disableAllWindowsFunctionality) && !launcherExists && taskItem.isWindow;

        if (tasksExtendedManager.immediateLauncherExists(taskItem.launcherUrl) && taskItem.isLauncher) {
            tasksExtendedManager.removeImmediateLauncher(taskItem.launcherUrl);
        }

        //if (hideStartup || hideWindow) { //fix #976
        if (hideWindow) {
            isForcedHidden = true;
            taskItem.visible = false;
            taskItem.parabolicItem.tempScaleWidth = 0;
            taskItem.parabolicItem.tempScaleHeight = 0;
            taskItem.parabolicItem.opacity = 0;
            taskItem.inAnimation = false;
        } else if (!LatteCore.WindowSystem.compositingActive || root.inDraggingPhase
                   || taskItem.isSeparator) {
            isForcedHidden = false;
            taskItem.visible = true;
            taskItem.parabolicItem.tempScaleWidth = 1;
            taskItem.parabolicItem.tempScaleHeight = 1;
            taskItem.parabolicItem.zoom = 1;
            taskItem.parabolicItem.opacity = 1;
            taskItem.inAnimation = false;
        } else if (( animation2 || animation3 || animation6 || isForcedHidden)
                   && (taskItem.abilities.animations.speedFactor.current !== 0) && !launcherIsAlreadyShown){
            isForcedHidden = false;
            taskItem.visible = true;
            taskItem.parabolicItem.tempScaleWidth = 0;
            taskItem.parabolicItem.tempScaleHeight = 0;
            start();
        } else {
            isForcedHidden = false;
            var frozenTask = tasksExtendedManager.getFrozenTask(taskItem.launcherUrl);

            if (frozenTask && frozenTask.zoom>1) {
                taskItem.parabolicItem.zoom = frozenTask.zoom;
                tasksExtendedManager.removeFrozenTask(taskItem.launcherUrl);
            } else {
                taskItem.parabolicItem.tempScaleWidth = 1;
                taskItem.parabolicItem.tempScaleHeight = 1;
            }

            //! by enabling it we break the bouncing animation
            //taskItem.visible = true;
            taskItem.parabolicItem.opacity = 1;
            taskItem.inAnimation = false;
        }
    }

    function showWindow(){
        execute();
    }

    Component.onDestruction: {
        if (animationSent){
            //console.log("SAFETY REMOVAL 2: animation removing ended");
            animationSent = false;
            taskItem.abilities.animations.needLength.removeEvent(needLengthEvent);
        }
    }
}
