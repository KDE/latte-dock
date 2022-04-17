/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

////////////////// new window and needs attention animation
Item{
    id:newWindowAnimation

    property int speed: 1.2 * taskItem.abilities.animations.speedFactor.normal * taskItem.abilities.animations.duration.large
    property bool isDemandingAttention: taskItem.inAttention
    property bool containsMouse: taskItem.containsMouse

    property bool inDelayedStartup: false

    readonly property bool running: newWindowAnimationLoader.active ? newWindowAnimationLoader.item.running : false
    readonly property bool paused: newWindowAnimationLoader.active ? newWindowAnimationLoader.item.paused : false
    readonly property string needThicknessEvent: newWindowAnimation + "_newwindow"

    Loader {
        id: newWindowAnimationLoader
        source: "newwindow/BounceAnimation.qml"
    }

    Connections {
        target: newWindowAnimationLoader.item

        onStopped: {
            taskItem.abilities.animations.needThickness.removeEvent(needThicknessEvent);
            newWindowAnimation.clear();
        }
    }

    Connections{
        target: taskItem

        onInAttentionChanged:{
            if (!taskItem.inAttention && newWindowAnimation.running && taskItem.inAttentionBuiltinAnimation) {
                clear();
            }
        }
    }

    function clear(){
        newWindowAnimationLoader.item.stop();
        taskItem.setBlockingAnimation(false);
        taskItem.inAttentionBuiltinAnimation = false;
        taskItem.inNewWindowBuiltinAnimation = false;
    }

    function stop() {
        if (running) {
            clear();
        }
    }

    onIsDemandingAttentionChanged: {
        if(isDemandingAttention){
            startNewWindowAnimation();
        }
    }

    function init(){
        taskItem.setBlockingAnimation(true);
        taskItem.inNewWindowBuiltinAnimation = true;
        taskItem.inAttentionBuiltinAnimation = isDemandingAttention;
        taskItem.abilities.animations.needThickness.addEvent(needThicknessEvent);
    }

    function startNewWindowAnimation(){
        if (isDemandingAttention && taskItem.abilities.indicators.info.providesInAttentionAnimation) {
            return;
        } else if (!isDemandingAttention && taskItem.abilities.indicators.info.providesGroupedWindowAddedAnimation) {
            return;
        }

        if (!taskItem.abilities.myView.isHidden
                && ((root.windowInAttentionEnabled && isDemandingAttention)
                    || root.windowAddedInGroupEnabled)){
            if (newWindowAnimationLoader.status !== Loader.Ready) {
                inDelayedStartup = true;
            } else {
                newWindowAnimation.init();
                newWindowAnimationLoader.item.start();
            }
        }
    }

    Component.onCompleted: {
        taskItem.taskGroupedWindowAdded.connect(startNewWindowAnimation);
    }

    Component.onDestruction: {
        taskItem.taskGroupedWindowAdded.disconnect(startNewWindowAnimation);
        taskItem.abilities.animations.needThickness.removeEvent(needThicknessEvent);
    }
}
