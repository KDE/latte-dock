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

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.1 as Latte

Item{
    id: wrapper

    opacity: 0
    width: {
        if (!mainItemContainer.visible)
            return 0;

        if (mainItemContainer.isSeparator){
            if (!root.vertical)
                return 0; //5 + root.widthMargins;
            else
                return (root.iconSize + root.widthMargins) + root.statesLineSize;
        }

        if (mainItemContainer.isStartup && root.durationTime !==0 ) {
            var moreThickness = root.vertical ? addedSpace : 0;

            return cleanScalingWidth + moreThickness;
        } else {
            return showDelegateWidth;
        }
    }

    height: {
        if (!mainItemContainer.visible)
            return 0;

        if (mainItemContainer.isSeparator){
            if (root.vertical)
                return 0; //5 + root.heightMargins;
            else
                return (root.iconSize + root.heightMargins) + root.statesLineSize;
        }

        if (mainItemContainer.isStartup && root.durationTime !==0){
            var moreThickness = !root.vertical ? addedSpace : 0;

            return cleanScalingHeight + moreThickness;
        } else {
            return showDelegateheight;
        }
    }

    //size needed fom the states below icons
    //property int statesLineSize: root.statesLineSize
    property int addedSpace: root.statesLineSize //7
    property int maxThickness: !root.vertical ? addedSpace + root.zoomFactor*(root.iconSize+root.heightMargins)
                                              : addedSpace + root.zoomFactor*(root.iconSize+root.widthMargins)

    property real showDelegateWidth: root.vertical ? basicScalingWidth+addedSpace :
                                                     basicScalingWidth
    property real showDelegateheight: root.vertical ? basicScalingHeight :
                                                      basicScalingHeight + addedSpace

    //scales which are used mainly for activating InLauncher
    ////Scalers///////
    property bool inTempScaling: ((tempScaleWidth !== 1) || (tempScaleHeight !== 1) )

    property real mScale: 1
    property real tempScaleWidth: 1
    property real tempScaleHeight: 1

    property real scaleWidth: (inTempScaling == true) ? tempScaleWidth : mScale
    property real scaleHeight: (inTempScaling == true) ? tempScaleHeight : mScale

    property real cleanScalingWidth: (root.iconSize + root.widthMargins) * mScale
    property real cleanScalingHeight: (root.iconSize + root.heightMargins) * mScale

    property real basicScalingWidth : (inTempScaling == true) ? ((root.iconSize + root.widthMargins) * scaleWidth) : cleanScalingWidth
    property real basicScalingHeight : (inTempScaling == true) ? ((root.iconSize + root.heightMargins) * scaleHeight) : cleanScalingHeight

    property real regulatorWidth: mainItemContainer.isSeparator ? width : basicScalingWidth;
    property real regulatorHeight: mainItemContainer.isSeparator ? height : basicScalingHeight;
    /// end of Scalers///////

    //property int curIndex: icList.hoveredIndex
    //  property int index: mainItemContainer.Positioner.index
    //property real center: (width + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2
    property real center: (width + hiddenSpacerLeft.nHiddenSize + hiddenSpacerRight.nHiddenSize) / 2

    property Item titleTooltipVisualParent: taskIconItem.titleTooltipVisualParent
    property Item previewsTooltipVisualParent: taskIconItem.previewsTootipVisualParent

    signal runLauncherAnimation();

    /*  Rectangle{
            anchors.fill: parent
            border.width: 1
            border.color: "green"
            color: "transparent"
        }*/

    Behavior on mScale {
        enabled: !root.globalDirectRender || inMimicParabolicAnimation
        NumberAnimation{
            duration: 3 * mainItemContainer.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on mScale {
        enabled: root.globalDirectRender && !inMimicParabolicAnimation && !restoreAnimation.running
        NumberAnimation { duration: root.directRenderAnimationTime }
    }

    Flow{
        anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
        anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
        anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
        anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

        width: wrapper.width
        height: wrapper.height

        flow: root.vertical ? Flow.TopToBottom : Flow.LeftToRight

        Loader{
            id: firstIndicator

            active:( (((root.position === PlasmaCore.Types.TopPositioned) || (root.position === PlasmaCore.Types.LeftPositioned))
                      && !root.reverseLinesPosition)
                    || (((root.position === PlasmaCore.Types.BottomPositioned) || (root.position === PlasmaCore.Types.RightPositioned))
                        && root.reverseLinesPosition) )
                   && !root.disableAllWindowsFunctionality
            visible: active

            sourceComponent: Component{
                TaskGroupItem{}
            }
        }

        //! This is used from bouncing attention animation in order to played correctly
        Loader{
            id: firstPadding

            active: secondIndicator.active && !root.reverseLinesPosition
                    && (mainItemContainer.inAttentionAnimation || mainItemContainer.inFastRestoreAnimation)
                    && !root.disableAllWindowsFunctionality
            visible: active

            sourceComponent: Component{
                Item{
                    width: root.vertical ? wrapper.maxThickness-wrapper.width : 1
                    height: !root.vertical ? wrapper.maxThickness-wrapper.height : 1

                    /* Rectangle{
                        width: !root.vertical ? 1 : parent.width
                        height: !root.vertical ? parent.height : 1
                        x: !root.vertical ? wrapper.width /2 : 0
                        y: root.vertical ? wrapper.height /2 : 0
                        border.width: 1
                        border.color: "blue"
                        color: "transparent"
                    } */
                }
            }
        }

        TaskIconItem{ id: taskIconItem}

        //! This is used from bouncing attention animation in order to played correctly
        Loader{
            id: secondPadding

            active: firstIndicator.active && !root.reverseLinesPosition
                    && (mainItemContainer.inAttentionAnimation || mainItemContainer.inFastRestoreAnimation)
                    && !root.disableAllWindowsFunctionality
            visible: active

            sourceComponent: Component{
                Item{
                    width: root.vertical ? wrapper.maxThickness-wrapper.width : 1
                    height: !root.vertical ? wrapper.maxThickness-wrapper.height : 1

                    /*Rectangle{
                        width: !root.vertical ? 1 : parent.width
                        height: !root.vertical ? parent.height : 1
                        x: !root.vertical ? wrapper.width/2 : 0
                        y: root.vertical ? wrapper.height/2 : 0
                        border.width: 1
                        border.color: "blue"
                        color: "transparent"
                    }*/
                }
            }
        }

        Loader{
            id: secondIndicator
            active: !firstIndicator.active
                    && !root.disableAllWindowsFunctionality
            visible: active

            sourceComponent: Component{
                TaskGroupItem{}
            }
        }

    }//Flow


    function calculateScales( currentMousePosition ){
        if (root.editMode || root.zoomFactor===1 || root.durationTime===0) {
            return;
        }

        var distanceFromHovered = Math.abs(index - icList.hoveredIndex);

        // A new algorithm tryig to make the zoom calculation only once
        // and at the same time fixing glitches
        if ((distanceFromHovered === 0) &&
                //! IMPORTANT: IS FIXING A BUG THAT NEGATIVE VALUES ARE SENT onEntered EVENT OF MOUSEAREA
                // (currentMousePosition>=0) &&
                (root.dragSource === null) ){

            //use the new parabolicManager in order to handle all parabolic effect messages
            var scales = parabolicManager.applyParabolicEffect(index, currentMousePosition, center);

            //Left hiddenSpacer for first task
            if(((index === parabolicManager.firstRealTaskIndex )&&(root.tasksCount>0)) && !root.disableLeftSpacer
                    && !inMimicParabolicAnimation && !inFastRestoreAnimation && !inAttentionAnimation){
                hiddenSpacerLeft.nScale = scales.leftScale - 1;
            }

            //Right hiddenSpacer for last task
            if(((index === parabolicManager.lastRealTaskIndex )&&(root.tasksCount>0)) && !root.disableRightSpacer
                    && !inMimicParabolicAnimation && !inFastRestoreAnimation && !inAttentionAnimation){
                hiddenSpacerRight.nScale =  scales.rightScale - 1;
            }

            if (!mainItemContainer.inAttentionAnimation) {
                mScale = root.zoomFactor;
            } else {
                var subSpacerScale = (root.zoomFactor-1)/2;

                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            }

            mainItemContainer.scalesUpdatedOnce = false;
        }

    } //nScale

    function signalUpdateScale(nIndex, nScale, step){
        if (!mainItemContainer.containsMouse && (index === nIndex)
                && (mainItemContainer.hoverEnabled || inMimicParabolicAnimation)&&(waitingLaunchers.length===0)){
            if (mainItemContainer.inAttentionAnimation) {
                var subSpacerScale = (nScale-1)/2;

                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            } else if (!inBlockingAnimation || mainItemContainer.inMimicParabolicAnimation) {
                var newScale = 1;

                if(nScale >= 0) {
                    newScale = nScale + step;
                } else {
                    newScale = mScale + step;
                }

                if (inMimicParabolicAnimation && mimicParabolicScale === -1) {
                    mimicParabolicScale = newScale;
                }

                mScale = newScale;
            }
        }
    }

    function sendEndOfNeedBothAxisAnimation(){
        if (mainItemContainer.isZoomed) {
            mainItemContainer.isZoomed = false;
            root.signalAnimationsNeedBothAxis(-1);
        }
    }

    onMScaleChanged: {
        if ((mScale === root.zoomFactor) && !root.directRenderTimerIsRunning && !root.globalDirectRender) {
            root.setGlobalDirectRender(true);
        }

        if (inMimicParabolicAnimation){
            if (mScale >= mimicParabolicScale) {
                inMimicParabolicAnimation = false;
                inAnimation = false;
                inBlockingAnimation = false;
                mimicParabolicScale = -1;
            } else {
                var tempScale = (root.zoomFactor - mScale) / 2;

                hiddenSpacerLeft.nScale = tempScale;
                hiddenSpacerRight.nScale = tempScale;
            }
        }

        if ((mScale > 1) && !mainItemContainer.isZoomed) {
            mainItemContainer.isZoomed = true;
            root.signalAnimationsNeedBothAxis(1);
        } else if ((mScale == 1) && mainItemContainer.isZoomed) {
            sendEndOfNeedBothAxisAnimation();
        }
    }

    Component.onCompleted: {
        if (!Latte.WindowSystem.compositingActive) {
            opacity = 1;
        }

        root.updateScale.connect(signalUpdateScale);
    }

    Component.onDestruction: {
        root.updateScale.disconnect(signalUpdateScale);
    }
}// Main task area // id:wrapper
