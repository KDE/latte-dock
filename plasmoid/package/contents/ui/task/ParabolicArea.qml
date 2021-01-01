/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: _parabolicArea
    signal parabolicEntered(real mouseX, real mouseY);
    signal parabolicMove(real mouseX, real mouseY);
    signal parabolicExited();

    property int lastMouseX: 0
    property int lastMouseY: 0

    readonly property bool containsMouse: (taskItem.parabolic.currentParabolicItem === _parabolicArea) || parabolicMouseArea.containsMouse

    readonly property real center: wrapper.center

    MouseArea {
        id: parabolicMouseArea
        anchors.fill: parent
        enabled: visible
        hoverEnabled: true
        visible: taskItem.parabolic.currentParabolicItem !== _parabolicArea

        onEntered: {
            taskItem.parabolic.setCurrentParabolicItem(_parabolicArea);

            var vIndex = taskItem.shortcuts.shortcutIndex(taskItem.itemIndex);
            taskItem.parabolic.setCurrentParabolicItemIndex(vIndex);

            // mouseX/Y can not be trusted at this point
            //_parabolicArea.parabolicEntered(mouseX, mouseY);
        }
    }

    Connections{
        target: root

        //! During dock sliding-in because the parabolic effect isnt trigerred
        //! immediately but we wait first the dock to go to its final normal
        //! place we might miss the activation of the parabolic effect.
        //! By catching that signal we are trying to solve this.
        onDockIsShownCompletelyChanged: {
            if (dockIsShownCompletely && _parabolicArea.containsMouse) {
                _parabolicArea.parabolicMove(lastMouseX, lastMouseY);
            }
        }
    }

    onParabolicEntered: {
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        restoreAnimation.stop();

        //! mouseX/Y can NOW be trusted because ParabolicEnterd event is triggered from View::Parabolic class
        var current = root.isHorizontal ? mouseX : mouseY;
        icList.currentSpot = current;
        calculateParabolicScales(current);
    }

    onParabolicMove: {
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        var mousePos = root.isHorizontal ? mouseX : mouseY;

        if (mousePos<0 || (inBlockingAnimation && !(inAttentionAnimation||inFastRestoreAnimation||inMimicParabolicAnimation)))
            return;

        if (root.latteView && root.latteView.isHalfShown) {
            return;
        }

        if((inAnimation == false)&&(!root.taskInAnimation)&&(!root.disableRestoreZoom) && taskItem.hoverEnabled){
            if( ((wrapper.mScale === 1 || wrapper.mScale === taskItem.parabolic.factor.zoom) && !taskItem.parabolic.directRenderingEnabled)
                    || taskItem.parabolic.directRenderingEnabled || !taskItem.scalesUpdatedOnce) {
                if(root.dragSource == null){
                    var step = Math.abs(icList.currentSpot-mousePos);
                    if (step >= taskItem.animations.hoverPixelSensitivity){
                        icList.currentSpot = mousePos;

                        calculateParabolicScales(mousePos);
                    }
                }
            }
        }
    }

    onParabolicExited: {
        taskItem.scalesUpdatedOnce = false;
    }

    function calculateParabolicScales( currentMousePosition ){
        if (taskItem.parabolic.factor.zoom===1 || parabolic.restoreZoomIsBlocked) {
            return;
        }

        if (root.dragSource === null) {
            //use the new parabolic ability in order to handle all parabolic effect messages
            var scales = taskItem.parabolic.applyParabolicEffect(index, currentMousePosition, center);

            //Left hiddenSpacer for first task
            if(((index === taskItem.indexer.firstVisibleItemIndex)&&(root.tasksCount>0)) && !root.disableLeftSpacer
                    && !inMimicParabolicAnimation && !inFastRestoreAnimation && !inAttentionAnimation){
                hiddenSpacerLeft.nScale = scales.leftScale - 1;
            }

            //Right hiddenSpacer for last task
            if(((index === taskItem.indexer.lastVisibleItemIndex )&&(root.tasksCount>0)) && !root.disableRightSpacer
                    && !inMimicParabolicAnimation && !inFastRestoreAnimation && !inAttentionAnimation){
                hiddenSpacerRight.nScale =  scales.rightScale - 1;
            }

            if (!taskItem.inAttentionAnimation) {
                wrapper.mScale = taskItem.parabolic.factor.zoom;
            } else {
                var subSpacerScale = (taskItem.parabolic.factor.zoom-1)/2;

                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            }

            taskItem.scalesUpdatedOnce = false;
        }
    } //nScale

    function updateScale(nIndex, nScale, step){
        if (!_parabolicArea.containsMouse && (index === nIndex)
                && (taskItem.hoverEnabled || inMimicParabolicAnimation)&&(tasksExtendedManager.waitingLaunchersLength()===0)){
            if (taskItem.inAttentionAnimation) {
                var subSpacerScale = (nScale-1)/2;

                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            } else if (!inBlockingAnimation || taskItem.inMimicParabolicAnimation) {
                var newScale = 1;

                if(nScale >= 0) {
                    newScale = nScale + step;
                } else {
                    newScale = wrapper.mScale + step;
                }

                if (inMimicParabolicAnimation && mimicParabolicScale === -1) {
                    mimicParabolicScale = newScale;
                }

                wrapper.mScale = newScale;
            }
        }
    }

    function sltUpdateLowerItemScale(delegateIndex, newScale, step) {
        if (delegateIndex === index) {
            if (!taskItem.isSeparator && !taskItem.isHidden) {
                //! when accepted
                updateScale(delegateIndex, newScale, step);

                if (newScale > 1) { // clear lower items
                    taskItem.parabolic.sglUpdateLowerItemScale(delegateIndex-1, 1, 0);
                }
            } else {
                taskItem.parabolic.sglUpdateLowerItemScale(delegateIndex-1, newScale, step);
            }
        } else if ((newScale === 1) && (index < delegateIndex)) {
            updateScale(index, 1, 0);
        }
    }

    function sltUpdateHigherItemScale(delegateIndex, newScale, step) {
        if (delegateIndex === index) {
            if (!taskItem.isSeparator && !taskItem.isHidden) {
                //! when accepted
                updateScale(delegateIndex, newScale, step);

                if (newScale > 1) { // clear lower items
                    taskItem.parabolic.sglUpdateHigherItemScale(delegateIndex+1, 1, 0); // clear higher items
                }
            } else {
                taskItem.parabolic.sglUpdateHigherItemScale(delegateIndex+1, newScale, step);
            }
        } else if ((newScale === 1) && (index > delegateIndex)) {
            updateScale(index, 1, 0);
        }
    }

    Component.onCompleted: {
        taskItem.parabolic.sglUpdateLowerItemScale.connect(sltUpdateLowerItemScale);
        taskItem.parabolic.sglUpdateHigherItemScale.connect(sltUpdateHigherItemScale);
    }

    Component.onDestruction: {
        taskItem.parabolic.sglUpdateLowerItemScale.disconnect(sltUpdateLowerItemScale);
        taskItem.parabolic.sglUpdateHigherItemScale.disconnect(sltUpdateHigherItemScale);
    }
}
