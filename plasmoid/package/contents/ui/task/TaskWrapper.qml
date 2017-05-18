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
                return 5 + root.widthMargins;
            else
                return (root.iconSize + root.widthMargins) * mScale + root.statesLineSize;
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
                return 5 + root.heightMargins;
            else
                return (root.iconSize + root.heightMargins) * mScale + root.statesLineSize;
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
    property real showDelegateWidth: root.vertical ? basicScalingWidth+addedSpace :
                                                     basicScalingWidth
    property real showDelegateheight: root.vertical ? basicScalingHeight :
                                                      basicScalingHeight + addedSpace

    //scales which are used mainly for activating InLauncher
    ////Scalers///////
    property bool inTempScaling: (((tempScaleWidth !== 1) || (tempScaleHeight !== 1) ) && (!mainItemContainer.mouseEntered) )

    property real mScale: 1
    property real tempScaleWidth: 1
    property real tempScaleHeight: 1

    property real scaleWidth: (inTempScaling == true) ? tempScaleWidth : mScale
    property real scaleHeight: (inTempScaling == true) ? tempScaleHeight : mScale

    property real cleanScalingWidth: (root.iconSize + root.widthMargins) * mScale
    property real cleanScalingHeight: (root.iconSize + root.heightMargins) * mScale

    property real basicScalingWidth : (inTempScaling == true) ? ((root.iconSize + root.widthMargins) * scaleWidth) : cleanScalingWidth
    property real basicScalingHeight : (inTempScaling == true) ? ((root.iconSize + root.heightMargins) * scaleHeight) : cleanScalingHeight

    property real regulatorWidth: mainItemContainer.isSeparator ? separatorRegWidth : basicScalingWidth;
    property real regulatorHeight: mainItemContainer.isSeparator ? separatorRegHeight : basicScalingHeight;

    property int separatorRegLength: root.vertical ? separatorRegWidth : separatorRegHeight

    property real separatorRegWidth: {
        if (!mainItemContainer.isSeparator)
            return;

        if (!root.vertical)
            return 5 + root.widthMargins;
        else
            return (root.iconSize + root.thickMargin) * wrapper.mScale;
    }

    property real separatorRegHeight: {
        if (!mainItemContainer.isSeparator)
            return;

        if (root.vertical)
            return 5 + root.heightMargins;
        else
            return (root.iconSize + root.thickMargin) * wrapper.mScale;
    }
    /// end of Scalers///////

    //property int curIndex: icList.hoveredIndex
    //  property int index: mainItemContainer.Positioner.index
    property real center: width / 2

    signal runLauncherAnimation();

    /*   Rectangle{
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

        TaskIconItem{}

        Loader{
            id: secondIndicator
            active: !firstIndicator.active
            visible: active

            sourceComponent: Component{
                TaskGroupItem{}
            }
        }

    }//Flow

    //!this is used in order to update the index when the signal is for applets
    //!outside the latte plasmoid
    function updateIdSendScale(indx, zScale, zStep){
        if ((indx>=0 && indx<=root.tasksCount-1) || (!root.latteDock)){
            root.updateScale(indx, zScale, zStep);
        } else{
            var appletId = latteDock.latteAppletPos;
            if (indx<0)
                appletId = latteDock.latteAppletPos + indx;
            else if (indx>root.tasksCount-1){
                var step=indx-root.tasksCount+1;
                appletId = latteDock.latteAppletPos + step;
            }

            latteDock.updateScale(appletId, zScale, zStep);
        }
    }

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

            var rDistance = Math.abs(currentMousePosition  - center);

            //check if the mouse goes right or down according to the center
            var positiveDirection =  ((currentMousePosition  - center) >= 0 );

            var minimumZoom = 1;

            if(mainItemContainer.isSeparator){
                //minimumZoom for separator item
                var tempZoomDifference = (root.missingSeparatorLength / (root.maxSeparatorLength+root.missingSeparatorLength)) * root.zoomFactor;
                minimumZoom = Math.max(tempZoomDifference, 1);
            }

            //finding the zoom center e.g. for zoom:1.7, calculates 0.35
            var zoomCenter = ((root.zoomFactor + minimumZoom)/2) - 1;

            //computes the in the scale e.g. 0...0.35 according to the mouse distance
            //0.35 on the edge and 0 in the center
            var firstComputation = (rDistance / center) * (zoomCenter-minimumZoom+1);

            //calculates the scaling for the neighbour tasks
            var bigNeighbourZoom = Math.min(1 + zoomCenter + firstComputation, root.zoomFactor);
            var smallNeighbourZoom = Math.max(1 + zoomCenter - firstComputation, minimumZoom);

            //bigNeighbourZoom = Number(bigNeighbourZoom.toFixed(4));
            //smallNeighbourZoom = Number(smallNeighbourZoom.toFixed(4));

            var leftScale;
            var rightScale;

            if(positiveDirection === true){
                rightScale = bigNeighbourZoom;
                leftScale = smallNeighbourZoom;
            } else {
                rightScale = smallNeighbourZoom;
                leftScale = bigNeighbourZoom;
            }

            //! compute the neighbour separator scales
            var bsNeighbourZoom = 1;
            var ssNeighbourZoom = 1;

            if(root.internalSeparatorPos>=0) {
                if((root.internalSeparatorPos === index+1) || (root.internalSeparatorPos === index-1) ){
                    var sepZoomDifference = (root.maxSeparatorLength / (root.maxSeparatorLength+root.missingSeparatorLength)) * root.zoomFactor;

                    bsNeighbourZoom = Math.max(1,bigNeighbourZoom - sepZoomDifference);
                    ssNeighbourZoom = Math.max(1,smallNeighbourZoom - sepZoomDifference);
                }
            }

            // console.debug(leftScale + "  " + rightScale + " " + index);

            if(!root.hasInternalSeparator || Math.abs(index-root.internalSeparatorPos)>=2
                    || mainItemContainer.isSeparator){
                //activate messages to update the the neighbour scales
                    updateIdSendScale(index+1, rightScale, 0);
                    updateIdSendScale(index-1, leftScale, 0);

                    updateIdSendScale(index+2, 1, 0);
                    updateIdSendScale(index-2, 1, 0);
            } else if(root.internalSeparatorPos>=0) {
                if(root.internalSeparatorPos === index+1){
                    if (!positiveDirection) {
                        updateIdSendScale(index+2, ssNeighbourZoom, 0);
                    } else {
                        updateIdSendScale(index+2, bsNeighbourZoom, 0);
                    }

                    updateIdSendScale(index-1, leftScale, 0);
                    updateIdSendScale(index+1, rightScale, 0);

                    updateIdSendScale(index+3, 1, 0);
                    updateIdSendScale(index-2, 1, 0);
                } else if(root.internalSeparatorPos === index-1) {
                    if (!positiveDirection) {
                        updateIdSendScale(index-2, bsNeighbourZoom, 0);
                    } else {
                        updateIdSendScale(index-2, ssNeighbourZoom, 0);
                    }

                    updateIdSendScale(index-1, leftScale, 0);
                    updateIdSendScale(index+1, rightScale, 0);

                    updateIdSendScale(index+2, 1, 0);
                    updateIdSendScale(index-3, 1, 0);
                }
            }

            //Left hiddenSpacer
            if(((index === 0 )&&(icList.count > 1)) && !root.disableLeftSpacer){
                hiddenSpacerLeft.nScale = leftScale - 1;
            }

            //Right hiddenSpacer
            if(((index === icList.count - 1 )&&(icList.count>1)) && (!root.disableRightSpacer)){
                hiddenSpacerRight.nScale =  rightScale - 1;
            }

            mScale = root.zoomFactor;
        }

    } //nScale


    function signalUpdateScale(nIndex, nScale, step){
        if ((index === nIndex)&&(mainItemContainer.hoverEnabled)&&(waitingLaunchers.length===0)){
            if(nScale >= 0) {
                mScale = nScale + step;
            } else {
                mScale = mScale + step;
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
