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
    signal parabolicEntered(int mouseX, int mouseY);
    signal parabolicMove(int mouseX, int mouseY);
    signal parabolicExited();

    readonly property bool containsMouse: taskItem.parabolic.currentParabolicItem === _parabolicArea

    property real center:root.isHorizontal ?
                             (wrapper.width + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2 :
                             (wrapper.height + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2

    MouseArea {
        id: parabolicMouseArea
        anchors.fill: parent
        enabled: visible
        hoverEnabled: true
        visible: taskItem.parabolic.currentParabolicItem !== _parabolicArea

        onEntered: {
            _parabolicArea.parabolicEntered(mouseX, mouseY);
            taskItem.parabolic.setCurrentParabolicItem(_parabolicArea);
        }
    }

    Connections {
        target: taskItem.parabolic

        onCurrentParabolicItemChanged: {
            if (taskItem.parabolic.currentParabolicItem !== _parabolicArea) {
                _parabolicArea.parabolicExited();
            }
        }
    }

    onParabolicEntered: {
        taskItem.parabolic.stopRestoreZoomTimer();
        restoreAnimation.stop();

        if ((taskItem.parabolic.local.lastIndex !== itemIndex) && isLauncher && windowsPreviewDlg.visible) {
            windowsPreviewDlg.hide(1);
        }

        if (root.latteView && (!root.showPreviews && root.titleTooltips) || (root.showPreviews && root.titleTooltips && isLauncher)){
            taskItem.showTitleTooltip();
        }

        //! show previews if enabled
        if(isAbleToShowPreview && !showPreviewsIsBlockedFromReleaseEvent && !isLauncher
                && (((root.showPreviews || (windowsPreviewDlg.visible && !isLauncher))
                     && windowsPreviewDlg.activeItem !== taskItem)
                    || root.highlightWindows)){

            if (!root.disableAllWindowsFunctionality) {
                //! don't delay showing preview in normal states,
                //! that is when the dock wasn't hidden
                if (!hoveredTimer.running && !windowsPreviewDlg.visible) {
                    //! first task with no previews shown can trigger the delay
                    hoveredTimer.start();
                } else {
                    //! when the previews are already shown, update them immediately
                    taskItem.showPreviewWindow();
                }
            }
        }

        taskItem.showPreviewsIsBlockedFromReleaseEvent = false;

        if (root.autoScrollTasksEnabled) {
            scrollableList.autoScrollFor(taskItem, false);
        }
    }

    onParabolicMove: {
        var mousePos = root.vertical ? mouseY : mouseX;

        if (mousePos<0 || (inBlockingAnimation && !(inAttentionAnimation||inFastRestoreAnimation||inMimicParabolicAnimation)))
            return;

        if (root.latteView && root.latteView.isHalfShown) {
            return;
        }

        if((inAnimation == false)&&(!root.taskInAnimation)&&(!root.disableRestoreZoom) && taskItem.hoverEnabled){
            var rapidMovement = taskItem.parabolic.local.lastIndex>=0 && Math.abs(taskItem.parabolic.local.lastIndex-itemIndex)>1;

            if (rapidMovement) {
                taskItem.parabolic.setDirectRenderingEnabled(true);
            }

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
        taskItem.isAbleToShowPreview = true;

        if (root.latteView && (!root.showPreviews || (root.showPreviews && isLauncher))){
            root.latteView.hideTooltipLabel();
        }

        if (root.showPreviews) {
            root.hidePreview(17.5);
        }
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
