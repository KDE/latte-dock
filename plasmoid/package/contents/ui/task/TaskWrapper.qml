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
    property int maxThickness: addedSpace + root.zoomFactor*root.iconSize

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

    signal runLauncherAnimation();

     /*  Rectangle{
            anchors.fill: parent
            border.width: 1
            border.color: "green"
            color: "transparent"
        }*/

    Behavior on mScale {
        enabled: !root.globalDirectRender
        NumberAnimation { duration: 3 * mainItemContainer.animationTime }
    }

    Behavior on mScale {
        enabled: root.globalDirectRender
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
            visible: active

            sourceComponent: Component{
                TaskGroupItem{}
            }
        }

        //! This is used from bouncing attention animation in order to played correctly
        Loader{
            id: firstPadding

            active: secondIndicator.active && mainItemContainer.inAttentionAnimation
            visible: active

            sourceComponent: Component{
                Item{
                    width: root.vertical ? wrapper.maxThickness-wrapper.width : 1
                    height: !root.vertical ? wrapper.maxThickness-wrapper.height : 1
                }
            }
        }

        TaskIconItem{ id: taskIconItem}

        //! This is used from bouncing attention animation in order to played correctly
        Loader{
            id: secondPadding

            active: firstIndicator.active && mainItemContainer.inAttentionAnimation
            visible: active

            sourceComponent: Component{
                Item{
                    width: root.vertical ? wrapper.maxThickness-wrapper.width : 1
                    height: !root.vertical ? wrapper.maxThickness-wrapper.height : 1
                }
            }
        }

        Loader{
            id: secondIndicator
            active: !firstIndicator.active
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
        if ((distanceFromHovered == 0)&&
                (currentMousePosition  > 0)&&
                (root.dragSource == null) ){

            //use the new parabolicManager in order to handle all parabolic effect messages
            var scales = parabolicManager.applyParabolicEffect(index, currentMousePosition, center);

            //Left hiddenSpacer
            if(((index === 0 )&&(icList.count > 1)) && !root.disableLeftSpacer){
                hiddenSpacerLeft.nScale = scales.leftScale - 1;
            }

            //Right hiddenSpacer
            if(((index === icList.count - 1 )&&(icList.count>1)) && (!root.disableRightSpacer)){
                hiddenSpacerRight.nScale =  scales.rightScale - 1;
            }

            if (!mainItemContainer.inAttentionAnimation)
                mScale = root.zoomFactor;
        }

    } //nScale


    function signalUpdateScale(nIndex, nScale, step){
        if ((index === nIndex)&&(mainItemContainer.hoverEnabled)&&(waitingLaunchers.length===0)){
            /*if (nScale !== 1){
                if (isSeparator){
                    console.log("WRONG TASK SIGNAL for internal separator at pos:"+ index +" and zoom:"+nScale);
                }
            }*/

            if (mainItemContainer.inAttentionAnimation) {
                var subSpacerScale = (nScale-1)/2;
                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            } else if (!inBlockingAnimation) {
                if(nScale >= 0) {
                    mScale = nScale + step;
                } else {
                    mScale = mScale + step;
                }
            }
            //     console.log(index+ ", "+mScale);
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
            root.startEnableDirectRenderTimer();
        }

        if ((mScale > 1) && !mainItemContainer.isZoomed) {
            mainItemContainer.isZoomed = true;
            root.signalAnimationsNeedBothAxis(1);
        } else if ((mScale == 1) && mainItemContainer.isZoomed) {
            sendEndOfNeedBothAxisAnimation();
        }
    }

    Component.onCompleted: {
        root.updateScale.connect(signalUpdateScale);
    }
}// Main task area // id:wrapper
