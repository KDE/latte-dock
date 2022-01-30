/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

SequentialAnimation {
    id: taskRealRemovalAnimation
    PropertyAction { target: taskItem; property: "ListView.delayRemove"; value: !taskItem.isSeparator } //try to delete separators instantly, this way maybe tasks model does not break when switching activities fastly
    PropertyAction { target: taskItem; property: "inAnimation"; value: true }
    PropertyAction { target: taskItem; property: "inAddRemoveAnimation"; value: true }
    PropertyAction { target: taskItem; property: "inRemoveStage"; value: true }

    //Animation Add/Remove (1) - when is window with no launcher in current activity, animations enabled
    //Animation Add/Remove (4) - the user removes a launcher, animation enabled
    property bool animation1: ( (tasksModel.launcherPosition(taskItem.launcherUrl) === -1 /*no-launcher*/
                                 && tasksModel.launcherPosition(taskItem.launcherUrlWithIcon) === -1)
                               || ((!taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl)/*no-launcher-in-current-activity*/
                                    && !taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrlWithIcon)))
                               && !taskItem.isStartup
                               && LatteCore.WindowSystem.compositingActive)

    property bool animation4: (tasksExtendedManager.launchersToBeRemovedCount /*update trigger*/
                               && (tasksExtendedManager.isLauncherToBeRemoved(taskItem.launcherUrl)
                                 || tasksExtendedManager.isLauncherToBeRemoved(taskItem.launcherUrlWithIcon))
                               && !taskItem.isStartup
                               && LatteCore.WindowSystem.compositingActive)

    property bool enabledAnimation: (animation1 || animation4)
                                    && root.newWindowSlidingEnabled
                                    && !taskItem.inBouncingAnimation
                                    && !taskItem.isSeparator
                                    && taskItem.visible

    readonly property string needLengthEvent: taskRealRemovalAnimation + "_realremoval"

    ScriptAction{
        script:{
            //! When a window is removed and afterwards its launcher must be shown immediately!
            if (!enabledAnimation && taskItem.isWindow && !taskItem.isSeparator
                    && (taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl)
                        || taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrlWithIcon))
                    && !tasksExtendedManager.immediateLauncherExists(taskItem.launcherUrl)){
                tasksExtendedManager.addImmediateLauncher(taskItem.launcherUrl);
            }

            //trying to fix the ListView nasty behavior
            //during the removal the anchoring for ListView children changes a lot
            var previousTask = icList.childAtIndex(taskItem.lastValidIndex-1);

            //! When removing a task and there are surrounding separators then the hidden spacers
            //! are updated immediately for the neighbour tasks. In such case in order to not break
            //! the removal animation a small margin must applied
            var spacer = taskItem.headItemIsSeparator ? -(2+taskItem.abilities.metrics.totals.lengthEdge) : ( taskItem.headItemIsSeparator ? (2+taskItem.abilities.metrics.totals.lengthEdge)/2 : 0);

            if (!taskItem.inBouncingAnimation && !animation4) {
                //! Is this really needed ??
                //! That hidden reanchoring was creating issue :#433359
                //! this needs to be reinvestigated - reapproached in case the following
                //! commented lines are really needed

            /*    //! real slide-out case
                var taskInListPos = mapToItem(icList, 0, 0);
                taskItem.parent = icList;

                if (root.vertical) {
                    taskItem.anchors.top = icList.top;
                    taskItem.anchors.topMargin = taskInListPos.y + spacer;

                    if (root.location===PlasmaCore.Types.LeftEdge) {
                        taskItem.anchors.left = icList.left;
                    } else {
                        taskItem.anchors.right = icList.right;
                    }
                } else {
                    taskItem.anchors.left = icList.left;
                    taskItem.anchors.leftMargin = taskInListPos.x + spacer;

                    if (root.location===PlasmaCore.Types.TopEdge) {
                        taskItem.anchors.top = icList.top;
                    } else {
                        taskItem.anchors.bottom = icList.bottom;
                    }
                }*/
            } else if (previousTask !== undefined && !previousTask.isStartup && !previousTask.inBouncingAnimation ) {
                // bouncing case
                if (root.vertical) {
                    taskItem.anchors.top = previousTask.bottom;
                    taskItem.anchors.topMargin = spacer;
                } else {
                    taskItem.anchors.left = previousTask.right;
                    taskItem.anchors.leftMargin = spacer;
                }
            }

            //console.log("1." + taskItem.launcherUrl + " - " + taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl) + "__" +
            //            animation1 + ":" + animation4 + "=>" + taskRealRemovalAnimation.enabledAnimation + "  index:" + taskItem.lastValidIndex);
            //console.log("2." + taskItem.isLauncher);

            taskItem.abilities.animations.needLength.addEvent(needLengthEvent);

            if (taskItem.parabolicItem.zoom > 1 && !taskRealRemovalAnimation.enabledAnimation
                    && !taskItem.inBouncingAnimation && LatteCore.WindowSystem.compositingActive) {
                tasksExtendedManager.setFrozenTask(taskItem.launcherUrl, taskItem.parabolicItem.zoom);
            } else {
                //! remove frozen task if it is deprecated. This fixes the libreoffice launcher issue with custom indicator launcher animations,
                //! steps to reproduce:
                //! 1. set an indicator that provides its own launcher animation such as "Animated Dot"
                //! 2. enable parabolic effect zoom
                //! 3. click on libreoffice writer launcher and keep the mouse inside the launcher
                //! 4. close libreoffice writer window from its decoration close button
                //! 5. libreoffice writer launcher is zoomed even though it should not
                var frozenTask = tasksExtendedManager.getFrozenTask(taskItem.launcherUrl);
                if (frozenTask && frozenTask.zoom>1) {
                    tasksExtendedManager.removeFrozenTask(taskItem.launcherUrl);
                }
            }

            if (taskItem.isLauncherAnimationRunning && !taskItem.isSeparator) {
                taskRealRemovalAnimation.pause();
            }
        }
    }

    //! Wait for launcher animation to finish before continue with real removal
    PauseAnimation {
        duration: taskItem.isLauncherAnimationRunning && !taskItem.isSeparator ? 50 : 0
    }

    PropertyAnimation {
        target: taskItem.parabolicItem
        property: "zoom"
        to: 1
        duration: taskRealRemovalAnimation.enabledAnimation ? showWindowAnimation.speed : 0
        easing.type: Easing.InQuad
    }

    //PropertyAction { target: taskItem.parabolicItem; property: "opacity"; value: isWindow ? 0 : 1 }
    //animation mainly for launchers removal and startups
    ParallelAnimation{
        id: removalAnimation

        // property int speed: (IsStartup && !taskItem.visible)? 0 : 400
        //property int speed: 400
        NumberAnimation {
            target: taskItem.parabolicItem;
            property: "opacity";
            to: 0;
            duration: taskRealRemovalAnimation.enabledAnimation ? 1.35*showWindowAnimation.speed : 0
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: taskItem
            property: taskItem.isVertical ? "iconAnimatedOffsetX" : "iconAnimatedOffsetY"
            to: -0.7*taskItem.abilities.metrics.iconSize
            duration: taskRealRemovalAnimation.enabledAnimation ? 1.35*showWindowAnimation.speed : 0
            easing.type: Easing.InQuad
        }
    }

    //smooth move into place the surrounding tasks
    PropertyAnimation {
        target: taskItem.parabolicItem
        property: "zoomLength"
        to: 0
        duration: taskRealRemovalAnimation.enabledAnimation ? 1.35*showWindowAnimation.speed : 0
        easing.type: Easing.InQuad
    }

    ScriptAction{
        script:{
            if (showWindowAnimation.animationSent){
                //console.log("SAFETY REMOVAL 1: animation removing ended");
                showWindowAnimation.animationSent = false;
                taskItem.abilities.animations.needLength.removeEvent(showWindowAnimation.needLengthEvent);
            }

            taskItem.abilities.animations.needLength.removeEvent(needLengthEvent);

            if(tasksExtendedManager.isLauncherToBeRemoved(taskItem.launcherUrl) && taskItem.isLauncher) {
                tasksExtendedManager.removeToBeRemovedLauncher(taskItem.launcherUrl);
            }

            if (windowsPreviewDlg.visible && windowsPreviewDlg.mainItem.parentTask === taskItem
                    && isWindow && !isGroupParent){
                hidePreview();
            }

            if (root.showWindowsOnlyFromLaunchers || root.disableAllWindowsFunctionality) {
                if (root.vertical) {
                    taskItem.anchors.top = undefined;
                    taskItem.anchors.topMargin = 0;
                } else {
                    taskItem.anchors.left = undefined;
                    taskItem.anchors.leftMargin = 0;
                }
            }

            taskItem.visible = false;

            //send signal that the launcher is really removing
            if (taskItem.hasAddedWaitingLauncher) {
                tasksExtendedManager.removeWaitingLauncher(taskItem.launcherUrl);

                if (!taskItem.abilities.indicators.info.providesTaskLauncherAnimation) {
                    //! this is needed only from in-built launcher animation to restore zoom smoothly to neighbour tasks
                    taskItem.abilities.parabolic.setDirectRenderingEnabled(false);
                }
            }
        }
    }

    PropertyAction { target: taskItem; property: "inAnimation"; value: false }
    PropertyAction { target: taskItem; property: "inAddRemoveAnimation"; value: false }
    PropertyAction { target: taskItem; property: "inRemoveStage"; value: false }

    PropertyAction { target: taskItem; property: "ListView.delayRemove"; value: false }
}
