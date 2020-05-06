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

import org.kde.plasma.core 2.0 as PlasmaCore

////////////////// new window and needs attention animation
SequentialAnimation{
    id:newWindowAnimation

    property int speed: 1.2 * taskItem.animations.speedFactor.normal * taskItem.animations.duration.large
    property bool isDemandingAttention: taskItem.inAttention
    property bool containsMouse: taskItem.containsMouse

    readonly property string needThicknessEvent: newWindowAnimation + "_newwindow"

    SequentialAnimation{
        alwaysRunToEnd: true

        ParallelAnimation{
            PropertyAnimation {
                target: wrapper
                property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                to: 1 + (thickPercentage * 2 * (taskItem.requires.innerZoomFactor-1))
                duration: newWindowAnimation.speed
                easing.type: Easing.OutQuad

                property real thickPercentage: taskItem.inAttentionAnimation ? 0.8 : 0.6
            }

            PropertyAnimation {
                target: wrapper
                property: (icList.orientation == Qt.Horizontal) ? "tempScaleWidth" : "tempScaleHeight"
                to: 1
                duration: newWindowAnimation.speed
                easing.type: Easing.OutQuad
            }
        }

        PropertyAnimation {
            target: wrapper
            property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
            to: 1
            duration: 4.4*newWindowAnimation.speed
            easing.type: Easing.OutBounce
        }
    }

    function clear(){
        loops = 1;
        newWindowAnimation.stop();
        //  iconImageBuffer.anchors.centerIn = iconImageBuffer.parent;

        wrapper.tempScaleWidth = 1;
        wrapper.tempScaleHeight = 1;

        taskItem.setBlockingAnimation(false);
        taskItem.inAttentionAnimation = false;
        taskItem.inNewWindowAnimation = false;
    }

    onStopped: {
        taskItem.animations.needThickness.removeEvent(needThicknessEvent);
        clear();
    }

    onIsDemandingAttentionChanged: {
        if(isDemandingAttention){
            bounceNewWindow();
        }
    }

    function init(){
        taskItem.setBlockingAnimation(true);
        taskItem.inNewWindowAnimation = true;

        wrapper.tempScaleWidth = wrapper.mScale;
        wrapper.tempScaleHeight = wrapper.mScale;

        if(!isDemandingAttention)
            loops = 1;
        else {
            loops = 20;
            taskItem.inAttentionAnimation = true;
        }

        taskItem.animations.needThickness.addEvent(needThicknessEvent);
    }

    function bounceNewWindow(){
        //if (isDemandingAttention && !root.dockIsHidden && (taskItem.parabolic.factor.zoom > 1)){

        if (!root.dockIsHidden && ((taskItem.animations.windowInAttentionEnabled && isDemandingAttention)
                                   || taskItem.animations.windowAddedInGroupEnabled)){
            newWindowAnimation.init();
            start();
        }
    }

    Component.onCompleted: {
        taskItem.groupWindowAdded.connect(bounceNewWindow);
    }

    Component.onDestruction: {
        taskItem.groupWindowAdded.disconnect(bounceNewWindow);
        taskItem.animations.needThickness.removeEvent(needThicknessEvent);
    }
}
