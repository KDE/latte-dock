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
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import org.kde.latte 0.1 as Latte

MouseArea{
    id: mainItemContainer

    visible: (isStartup && root.durationTime !== 0) ? false : true

    anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
    anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
    anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
    anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

    width: root.vertical ? wrapper.width :
                           hiddenSpacerLeft.width+wrapper.width+hiddenSpacerRight.width

    height: root.vertical ? hiddenSpacerLeft.height + wrapper.height + hiddenSpacerRight.height :
                            wrapper.height

    acceptedButtons: Qt.LeftButton | Qt.MidButton | Qt.RightButton
    hoverEnabled: (inAnimation !== true)&& (!IsStartup)&&(!root.taskInAnimation)&&(!root.editMode || root.debugLocation)
    // hoverEnabled: false

    property bool buffersAreReady: false
    property bool delayingRemove: ListView.delayRemove
    //states that exist in windows in a Group of windows
    property bool hasActive: isActive
    property bool hasMinimized: (IsGroupParent === true) ? tasksWindows.hasMinimized : isMinimized
    property bool hasShown: (IsGroupParent === true) ? tasksWindows.hasShown : !isMinimized
    property bool inAddRemoveAnimation: true
    property bool inAnimation: true
    property bool inBlockingAnimation: false
    property bool inPopup: false

    property bool isActive: (IsActive === true) ? true : false
    property bool isDemandingAttention: (IsDemandingAttention === true) ? true : false
    property bool isDragged: false
    property bool isGroupParent: (IsGroupParent === true) ? true : false
    property bool isLauncher: (IsLauncher === true) ? true : false
    property bool isMinimized: (IsMinimized === true) ? true : false
    property bool isStartup: (IsStartup === true) ? true : false
    property bool isWindow: (IsWindow === true) ? true : false
    property bool isZoomed: false

    property bool mouseEntered: false
    property bool pressed: false

    property int animationTime: root.durationTime * 1.2 * units.shortDuration
    property int directAnimationTime: 0
    property int hoveredIndex: icList.hoveredIndex
    property int itemIndex: index
    property int lastButtonClicked: -1;
    property int pressX: -1
    property int pressY: -1
    property int resistanceDelay: 750

    property real animationStep: 1 //root.iconSize / 12

    property string activity: tasksModel.activity

    readonly property var m: model

    property QtObject contextMenu: null
    property QtObject draggingResistaner: null
    property QtObject hoveredTimerObj: null

    signal groupWindowAdded();
    signal groupWindowRemoved();
    signal checkWindowsStates();

    /*    onHasMinimizedChanged:{
            console.log(AppId);
            if(AppId == "org.kde.dolphin"){
            console.log("1. Minimized:"+hasMinimized);
            console.log("2. Active:"+hasActive);
            console.log("3. Shown:"+hasShown);
            }
        }
        onHasShownChanged:{
            console.log(AppId);
            if(AppId == "org.kde.dolphin"){
            console.log("1. Minimized:"+hasMinimized);
            console.log("2. Active:"+hasActive);
            console.log("3. Shown:"+hasShown);
            }
        }
        onHasActiveChanged:{
            console.log(AppId);
            if(AppId == "org.kde.dolphin"){
            console.log("1. Minimized:"+hasMinimized);
            console.log("2. Active:"+hasActive);
            console.log("3. Shown:"+hasShown);
            }
        }*/

    /*  Rectangle{
            anchors.fill: parent
            border.width: 1
            border.color: "blue"
            color: "transparent"
           // visible: IsStartup ? true : false
        } */

    Behavior on opacity {
        // NumberAnimation { duration: (IsStartup || (IsLauncher) ) ? 0 : 400 }
        NumberAnimation { duration: root.durationTime*units.longDuration }
    }


    /*        PlasmaCore.ToolTipArea {
             id: toolTip

             anchors.fill: parent

            // active: !inPopup && !groupDialog.visible && plasmoid.configuration.showToolTips
             active: plasmoid.configuration.showToolTips && mainItemContainer.isWindow && mainItemContainer.containsMouse
             enabled: mainItemContainer.isWindow

             interactive: true
             location: plasmoid.location

             mainItem: toolTipDelegate

             onContainsMouseChanged:  {

             }

         }*/


    TaskWindows{
        id: tasksWindows

        property int previousCount: 0

        onWindowsCountChanged: {
            if ((windowsCount >= 2) && (windowsCount > previousCount)){
                if(root.dragSource == null)
                    mainItemContainer.groupWindowAdded();
            }
            else if ((windowsCount >=1) &&(windowsCount < previousCount)){
                //sometimes this is triggered in dragging with no reason
                if(root.dragSource == null)
                    mainItemContainer.groupWindowRemoved();
            }

            if (windowsCount>=1) {
                mainItemContainer.slotPublishGeometries();
            }

            previousCount = windowsCount;
        }
    }


    Flow{
        width: parent.width
        height: parent.height

        // a hidden spacer for the first element to add stability
        // IMPORTANT: hidden spacers must be tested on vertical !!!
        Item{
            id: hiddenSpacerLeft
            //we add one missing pixel from calculations
            width: root.vertical ? wrapper.width : nHiddenSize
            height: root.vertical ? nHiddenSize : wrapper.height

            visible: (index === 0)

            property real nHiddenSize: (nScale > 0) ? (root.realSize * nScale) : 0
            property real nScale: 0

            Behavior on nScale {
                enabled: !root.globalDirectRender
                NumberAnimation { duration: 3 * mainItemContainer.animationTime }
            }

            Behavior on nScale {
                enabled: root.globalDirectRender
                NumberAnimation { duration: mainItemContainer.directAnimationTime }
            }

            /*   Rectangle{
                    width:parent.width
                    height:1
                    y:parent.height / 2
                    border.width: 1
                    border.color: "red"
                    color: "transparent"
                }*/
        }

        Item{
            id: wrapper

            opacity: 0
            width: (mainItemContainer.isStartup && root.durationTime !==0 ) ? 0 : showDelegateWidth
            height: (mainItemContainer.isStartup && root.durationTime !==0) ? 0 : showDelegateheight

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
            property real tempScaleWidth: 0
            property real tempScaleHeight: 0

            property real scaleWidth: (inTempScaling == true) ? tempScaleWidth : mScale
            property real scaleHeight: (inTempScaling == true) ? tempScaleHeight : mScale

            ///Dont use Math.floor it adds one pixel in animations and creates glitches
            property int widthMargins: root.vertical ? root.thickMargin : root.iconMargin
            property int heightMargins: !root.vertical ? root.thickMargin : root.iconMargin

            property real cleanScalingWidth: (root.iconSize + widthMargins) * mScale
            property real cleanScalingHeight: (root.iconSize + heightMargins) * mScale

            property real basicScalingWidth : (inTempScaling == true) ? ((root.iconSize + widthMargins) * scaleWidth) : cleanScalingWidth
            property real basicScalingHeight : (inTempScaling == true) ? ((root.iconSize + heightMargins) * scaleHeight) : cleanScalingHeight

            property real regulatorWidth: basicScalingWidth;//-2;
            property real regulatorHeight: basicScalingHeight;//-2;
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
                NumberAnimation { duration: mainItemContainer.directAnimationTime }
            }

            Flow{
                anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
                anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
                anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
                anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

                anchors.horizontalCenter: !parent.vertical ? parent.horizontalCenter : undefined
                anchors.verticalCenter: parent.vertical ? parent.verticalCenter : undefined

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

            function calculateScales( currentMousePosition ){
                if (root.editMode) {
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


                    //finding the zoom center e.g. for zoom:1.7, calculates 0.35
                    var zoomCenter = (root.zoomFactor - 1) / 2

                    //computes the in the scale e.g. 0...0.35 according to the mouse distance
                    //0.35 on the edge and 0 in the center
                    var firstComputation = (rDistance / center) * zoomCenter;

                    //calculates the scaling for the neighbour tasks
                    var bigNeighbourZoom = Math.min(1 + zoomCenter + firstComputation, root.zoomFactor);
                    var smallNeighbourZoom = Math.max(1 + zoomCenter - firstComputation, 1);

                    bigNeighbourZoom = Number(bigNeighbourZoom.toFixed(4));
                    smallNeighbourZoom = Number(smallNeighbourZoom.toFixed(4));

                    var leftScale;
                    var rightScale;

                    if(positiveDirection === true){
                        rightScale = bigNeighbourZoom;
                        leftScale = smallNeighbourZoom;
                    }
                    else {
                        rightScale = smallNeighbourZoom;
                        leftScale = bigNeighbourZoom;
                    }

                    // console.debug(leftScale + "  " + rightScale + " " + index);


                    //activate messages to update the the neighbour scales
                    root.updateScale(index+1, rightScale, 0);
                    root.updateScale(index-1, leftScale, 0);
                    root.updateScale(index+2, 1, 0);
                    root.updateScale(index-2, 1, 0);

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
                if ((index === nIndex)&&(!mainItemContainer.inAnimation)){
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
                if ((mScale === root.zoomFactor) && !enableDirectRenderTimer.running && !icList.directRender) {
                        enableDirectRenderTimer.start();
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

        // a hidden spacer on the right for the last item to add stability
        Item{
            id: hiddenSpacerRight
            //we add one missing pixel from calculations
            width: root.vertical ? wrapper.width : nHiddenSize
            height: root.vertical ? nHiddenSize : wrapper.height

            visible: (index === icList.count - 1)

            property real nHiddenSize: (nScale > 0) ? (root.realSize * nScale) : 0
            property real nScale: 0

            Behavior on nScale {
                enabled: !root.globalDirectRender
                NumberAnimation { duration: 3 * mainItemContainer.animationTime }
            }

            Behavior on nScale {
                enabled: root.globalDirectRender
                NumberAnimation { duration: mainItemContainer.directAnimationTime }
            }

            /*     Rectangle{
                         width:parent.width
                         height:1
                         y:parent.height / 2
                         border.width: 1
                         border.color: "red"
                         color: "transparent"
                     }*/
        }

    }// Flow with hidden spacers inside

    Component {
        id: taskInitComponent
        Timer {
            id: timer

            interval: 800
            repeat: false

            onTriggered: {
                //      mainItemContainer.hoverEnabled = true;
                tasksModel.requestPublishDelegateGeometry(mainItemContainer.modelIndex(),
                                                          backend.globalRect(mainItemContainer), mainItemContainer);
                timer.destroy();
            }

            Component.onCompleted: timer.start()
        }
    }

    ////// Values Changes /////
    //restore scales when there is no zoom factor for that item or
    //the mouse is out of the ListView
    // onItemIndexChanged: {
    //  }

    onHoveredIndexChanged: {
        var distanceFromHovered = Math.abs(index - icList.hoveredIndex);

        if( (distanceFromHovered > 1) && (hoveredIndex !== -1)){
            if(!isDragged)
                wrapper.mScale = 1;
        }

        if (distanceFromHovered >= 1) {
            hiddenSpacerLeft.nScale = 0;
            hiddenSpacerRight.nScale = 0;
        }
    }


    onIsDraggedChanged: {
        if(isDragged && (!root.editMode)){
            root.dragSource = mainItemContainer;
            dragHelper.startDrag(mainItemContainer, model.MimeType, model.MimeData,
                                 model.LauncherUrlWithoutIcon, model.decoration);
            pressX = -1;
            pressY = -1;
        }
    }

    onDelayingRemoveChanged: {
        if(delayingRemove && isWindow)
            groupWindowRemoved();
    }

    onIsWindowChanged: {
        if (isWindow) {
            taskInitComponent.createObject(mainItemContainer);
        }
    }

    onIsMinimizedChanged: {
        checkWindowsStates();
    }

    onIsActiveChanged: {
        checkWindowsStates();
    }

    ////// End of Values Changes /////


    ///////////////// Mouse Area Events ///////////////////
    onEntered: {
        if ((icList.hoveredIndex !== itemIndex) && isLauncher && windowsPreviewDlg.visible) {
            windowsPreviewDlg.hide(1);
        }

        checkListHovered.stop();

        if((!inAnimation)&&(root.dragSource == null)&&(!root.taskInAnimation)){
            icList.hoveredIndex = index;
            mouseEntered = true;
            root.mouseWasEntered(index-2, false);
            root.mouseWasEntered(index+2, false);
            root.mouseWasEntered(index-1, true);
            root.mouseWasEntered(index+1, true);

            if (icList.orientation == Qt.Horizontal){
                icList.currentSpot = mouseX;
                wrapper.calculateScales(mouseX);
            }
            else{
                icList.currentSpot = mouseY;
                wrapper.calculateScales(mouseY);
            }
        }
    }

    // IMPORTANT: This must be improved ! even for small miliseconds  it reduces performance
    onExited: {
        mouseEntered = false;
        if(mainItemContainer.contextMenu && mainItemContainer.contextMenu.status == PlasmaComponents.DialogStatus.Open){
            ///dont check to restore zooms
        }
        else{
            if(!inAnimation){
                checkListHovered.startNormal();
            }
        }

        /* if(draggingResistaner != null){
                draggingResistaner.destroy();
                draggingResistaner = null;
                isDragged = false;
            }*/
    }

    onPositionChanged: {
        checkListHovered.stop();

        if((inAnimation == false)&&(!root.taskInAnimation)&&(!root.disableRestoreZoom)){
            if(root.dragSource == null){
                if (icList.orientation == Qt.Horizontal){
                    var step = Math.abs(icList.currentSpot-mouse.x);
                    if (step >= animationStep){
                        icList.hoveredIndex = index;
                        icList.currentSpot = mouse.x;

                        wrapper.calculateScales(mouse.x);
                    }
                }
                else{
                    var step = Math.abs(icList.currentSpot-mouse.y);
                    if (step >= animationStep){
                        icList.hoveredIndex = index;
                        icList.currentSpot = mouse.y;

                        wrapper.calculateScales(mouse.y);
                    }
                }
            }

            // mouse.button is always 0 here, hence checking with mouse.buttons
            if (pressX != -1 && mouse.buttons == Qt.LeftButton
                    && isDragged
                    && !root.editMode
                    && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y) ) {
                root.dragSource = mainItemContainer;
                dragHelper.startDrag(mainItemContainer, model.MimeType, model.MimeData,
                                     model.LauncherUrlWithoutIcon, model.decoration);
                pressX = -1;
                pressY = -1;
            }
            else{
                /*    if(draggingResistaner != null){
                        draggingResistaner.destroy();
                        draggingResistaner = null;
                    }
                    isDragged = false;*/
            }
        }
    }

    onContainsMouseChanged:{
        if(!containsMouse){
          //  hiddenSpacerLeft.nScale = 0;
          //  hiddenSpacerRight.nScale = 0;

            if(!inAnimation)
                pressed=false;
        }

        if (isWindow) {
            root.windowsHovered(model.LegacyWinIdList, containsMouse);
        }


        ////window previews/////////
        if (isWindow) {
            if(containsMouse && root.showPreviews && Latte.WindowSystem.compositingActive){
                hoveredTimerObj = hoveredTimerComponent.createObject(mainItemContainer);
                //  preparePreviewWindow();
            }
            else{
                if (hoveredTimerObj){
                    hoveredTimerObj.stop();
                    hoveredTimerObj.destroy();
                }
            }
        }
    }

    onPressed: {
        //console.log("Pressed Task Delegate..");
        if (Latte.WindowSystem.compositingActive) {
            windowsPreviewDlg.hide(2);
        }

        if ((mouse.button == Qt.LeftButton)||(mouse.button == Qt.MidButton)) {
            lastButtonClicked = mouse.button;
            pressed = true;
            pressX = mouse.x;
            pressY = mouse.y;

            if(draggingResistaner == null)
                draggingResistaner = resistanerTimerComponent.createObject(mainItemContainer);
        }
        else if (mouse.button == Qt.RightButton){
            root.createContextMenu(mainItemContainer).show();
        }

        if (hoveredTimerObj){
            hoveredTimerObj.restart();
            /*hoveredTimerObj.stop();
            hoveredTimerObj.destroy();*/
        }
    }

    onReleased: {
        //console.log("Released Task Delegate...");
        if (draggingResistaner != null){
            draggingResistaner.destroy();
            draggingResistaner = null;
        }

        if(pressed && !inBlockingAnimation){
            if (mouse.button == Qt.MidButton){
                if( !mainItemContainer.isLauncher){
                    if (plasmoid.configuration.middleClickAction == TaskManagerApplet.Backend.NewInstance) {
                        tasksModel.requestNewInstance(modelIndex());
                    } else if (plasmoid.configuration.middleClickAction == TaskManagerApplet.Backend.Close) {
                        tasksModel.requestClose(modelIndex());
                    } else if (plasmoid.configuration.middleClickAction == TaskManagerApplet.Backend.ToggleMinimized) {
                        tasksModel.requestToggleMinimized(modelIndex());
                    }
                }
                else {
                    mouseEntered = false;
                    wrapper.runLauncherAnimation();
                }
            }
            else if (mouse.button == Qt.LeftButton){
                if( mainItemContainer.isLauncher){
                    mouseEntered = false;
                    wrapper.runLauncherAnimation();
                }
                else{
                    if (model.IsGroupParent) {
                        if (Latte.WindowSystem.compositingActive) {
                            root.presentWindows(model.LegacyWinIdList);
                        } else {
                            if ((windowsPreviewDlg.visualParent === mainItemContainer)&&(windowsPreviewDlg.visible)) {
                                windowsPreviewDlg.hide(3);
                            } else {
                                preparePreviewWindow(false);
                                windowsPreviewDlg.show(mainItemContainer);
                            }
                        }
                    } else {
                        if (IsMinimized === true) {
                            var i = modelIndex();
                            tasksModel.requestToggleMinimized(i);
                            tasksModel.requestActivate(i);
                        } else if (IsActive === true) {
                            tasksModel.requestToggleMinimized(modelIndex());
                        } else {
                            tasksModel.requestActivate(modelIndex());
                        }
                    }

                }
            }
        }

        pressed = false;

        if(!inAnimation)
            checkListHovered.startDuration(3*units.longDuration);
    }
    ///////////////// End Of Mouse Area Events ///////////////////

    ///// Handlers for Signals /////
    function signalMouseWasEntered(nIndex, value){
        if( index === nIndex)
            mouseEntered = value;
    }

    function animationStarted(){
        //    console.log("Animation started: " + index);
        inAnimation = true;
    }

    function animationEnded(){
        //   console.log("Animation ended: " + index);
        inAnimation = false;
        // checkListHovered.startDuration(3*units.longDuration);
    }

    function clearZoom(){
        if (restoreAnimation)
            restoreAnimation.start();
        //if (wrapper)
        //    wrapper.mScale=1;
    }

    function handlerDraggingFinished(){
        isDragged = false;
    }
    ///// End of Handlers //////



    ///// Helper functions /////
    function preparePreviewWindow(hideClose){
        windowsPreviewDlg.visualParent = mainItemContainer;

        toolTipDelegate.parentTask = mainItemContainer;
        toolTipDelegate.parentIndex = itemIndex;

        toolTipDelegate.hideCloseButtons = hideClose;

        toolTipDelegate.appName = Qt.binding(function() {
            return model.AppName;
        });
        toolTipDelegate.pidParent = Qt.binding(function() {
            return model.AppPid;
        });
        toolTipDelegate.windows = Qt.binding(function() {
            return model.LegacyWinIdList;
        });
        toolTipDelegate.isGroup = Qt.binding(function() {
            return model.IsGroupParent == true;
        });
        toolTipDelegate.icon = Qt.binding(function() {
            return model.decoration;
        });
        toolTipDelegate.launcherUrl = Qt.binding(function() {
            return model.LauncherUrlWithoutIcon;
        });
        toolTipDelegate.isLauncher = Qt.binding(function() {
            return model.IsLauncher == true;
        });
        toolTipDelegate.isMinimizedParent = Qt.binding(function() {
            return model.IsMinimized == true;
        });
        toolTipDelegate.displayParent = Qt.binding(function() {
            return model.display;
        });
        toolTipDelegate.genericName = Qt.binding(function() {
            return model.GenericName;
        });
        toolTipDelegate.virtualDesktopParent = Qt.binding(function() {
            return model.VirtualDesktop != undefined ? model.VirtualDesktop : 0;
        });
        toolTipDelegate.isOnAllVirtualDesktopsParent = Qt.binding(function() {
            return model.IsOnAllVirtualDesktops == true;
        });
        toolTipDelegate.activitiesParent = Qt.binding(function() {
            return model.Activities;
        });

        /*
        toolTipDelegate.parentIndex = index;

        toolTipDelegate.windows = Qt.binding(function() {
            return model.LegacyWinIdList;
        });
        toolTipDelegate.mainText = Qt.binding(function() {
            return model.display;
        });
        toolTipDelegate.icon = Qt.binding(function() {
            return model.decoration;
        });
        toolTipDelegate.subText = Qt.binding(function() {
            return model.IsLauncher === true ? model.GenericName : generateSubText(model);
        });
        toolTipDelegate.launcherUrl = Qt.binding(function() {
            return model.LauncherUrlWithoutIcon;
        });

        toolTipDelegate.titles = tasksWindows.windowsTitles();*/
    }


    function launcherAction(){
        // if ((lastButtonClicked == Qt.LeftButton)||(lastButtonClicked == Qt.MidButton)){
        tasksModel.requestActivate(modelIndex());
        // }
    }

    ///window previews///
    function generateSubText(task) {
        var subTextEntries = new Array();

        if (!plasmoid.configuration.showOnlyCurrentDesktop
                && virtualDesktopInfo.numberOfDesktops > 1
                && model.IsOnAllVirtualDesktops !== true
                && model.VirtualDesktop != -1
                && model.VirtualDesktop != undefined) {
            subTextEntries.push(i18n("On %1", virtualDesktopInfo.desktopNames[model.VirtualDesktop - 1]));
        }

        if (model.Activities == undefined) {
            return subTextEntries.join("\n");
        }

        if (model.Activities.length == 0 && activityInfo.numberOfRunningActivities > 1) {
            subTextEntries.push(i18nc("Which virtual desktop a window is currently on",
                                      "Available on all activities"));
        } else if (model.Activities.length > 0) {
            var activityNames = new Array();

            for (var i = 0; i < model.Activities.length; i++) {
                var activity = model.Activities[i];

                if (plasmoid.configuration.showOnlyCurrentActivity) {
                    if (activity != activityInfo.currentActivity) {
                        activityNames.push(activityInfo.activityName(model.Activities[i]));
                    }
                } else if (activity != activityInfo.currentActivity) {
                    activityNames.push(activityInfo.activityName(model.Activities[i]));
                }
            }

            if (plasmoid.configuration.showOnlyCurrentActivity) {
                if (activityNames.length > 0) {
                    subTextEntries.push(i18nc("Activities a window is currently on (apart from the current one)",
                                              "Also available on %1", activityNames.join(", ")));
                }
            } else if (activityNames.length > 0) {
                subTextEntries.push(i18nc("Which activities a window is currently on",
                                          "Available on %1", activityNames.join(", ")));
            }
        }

        return subTextEntries.join("\n");
    }
    ///window previews////

    function modelIndex(){
        return tasksModel.makeModelIndex(index);
    }

    function setBlockingAnimation(value){
        inBlockingAnimation = value;
    }

    function slotShowPreviewForTasks(group) {
        if (group === mainItemContainer) {
            preparePreviewWindow(true);
            windowsPreviewDlg.show(mainItemContainer);
        }
    }

    function slotPublishGeometries() {
        if ((isWindow || isStartup || isGroupParent) && !icList.delayingRemoval) {
            tasksModel.requestPublishDelegateGeometry(mainItemContainer.modelIndex(),
                                                      backend.globalRect(mainItemContainer), mainItemContainer);
        }
    }

    //fix wrong positioning of launchers....
    onActivityChanged:{
        for(var i=0; i<tasksModel.launcherList.length; ++i){
            if ((tasksModel.launcherList[i] == LauncherUrlWithoutIcon) && (i != index)){
                updatePosition.restart();
            }
        }
    }

    ///// End of Helper functions ////

    Component.onCompleted: {
        root.mouseWasEntered.connect(signalMouseWasEntered);
        root.draggingFinished.connect(handlerDraggingFinished);
        root.clearZoomSignal.connect(clearZoom);
        root.publishTasksGeometries.connect(slotPublishGeometries);
        root.showPreviewForTasks.connect(slotShowPreviewForTasks);

        //fix wrong positioning of launchers....
        for(var i=0; i<tasksModel.launcherList.length; ++i){
            if ((tasksModel.launcherList[i] == LauncherUrlWithoutIcon) && (i != index)){
                updatePosition.restart();
            }
        }

        showWindowAnimation.showWindow();
    }

    Component.onDestruction: {
        wrapper.sendEndOfNeedBothAxisAnimation();
    }

    Timer{
        id:updatePosition
        interval: 800

        onTriggered: {
            for(var i=0; i<tasksModel.launcherList.length; ++i){
                if ((tasksModel.launcherList[i] == LauncherUrlWithoutIcon) && (i != index)){
                    //   console.log("Launch List:"+tasksModel.launcherList);
                    //    console.log("Move from timer "+AppId+" - from:"+ index + " to:" + i + " - total:"+tasksModel.count);
                    tasksModel.move(index, i);
                }
            }
        }
    }


    /////Animations

    ///item's added Animation
    SequentialAnimation{
        id:showWindowAnimation
        property int speed: Latte.WindowSystem.compositingActive ? root.durationTime* (1.2*units.longDuration) : 0
        property bool animationSent: false

        ScriptAction{
            script:{
                if (!showWindowAnimation.animationSent) {
                    showWindowAnimation.animationSent = true;
                    root.signalAnimationsNeedLength(1);
                }
            }
        }

        PropertyAnimation {
            target: wrapper
            property: (icList.orientation == Qt.Vertical) ? "tempScaleHeight" : "tempScaleWidth"
            to: 1
            duration: showWindowAnimation.speed
            easing.type: Easing.OutQuad
        }

        ParallelAnimation{

            PropertyAnimation {
                target: wrapper
                property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                to: 1
                duration: showWindowAnimation.speed
                easing.type: Easing.OutQuad
            }


            PropertyAnimation {
                target: wrapper
                property: "opacity"
                from: 0
                to: 1
                duration: showWindowAnimation.speed
                easing.type: Easing.OutQuad
            }
        }

        onStopped: {
            mainItemContainer.inAddRemoveAnimation = false;

            if(mainItemContainer.isWindow || mainItemContainer.isStartup){
                taskInitComponent.createObject(wrapper);
                if (mainItemContainer.isDemandingAttention){
                    mainItemContainer.groupWindowAdded();
                }
            }
            mainItemContainer.inAnimation = false;

            if (showWindowAnimation.animationSent) {
                root.signalAnimationsNeedLength(-1);
                showWindowAnimation.animationSent = false;
            }
        }

        function init(){
            if (root.durationTime !== 0) {
                wrapper.tempScaleWidth = 0;
                wrapper.tempScaleHeight = 0;
            } else {
                wrapper.tempScaleWidth = 1;
                wrapper.tempScaleHeight = 1;
            }
        }

        function showWindow(){
            if((mainItemContainer.isLauncher || mainItemContainer.isStartup
                    || icList.delayingRemoval || (!mainItemContainer.buffersAreReady && !root.initializatedBuffers)) && root.durationTime !== 0){
                delayShowWindow.createObject(mainItemContainer);
            }
            else{
                execute();
            }
        }

        function execute(){
            init();
            start();
        }

        Component.onDestruction: {
            if (animationSent){
                //console.log("SAFETY REMOVAL 2: animation removing ended");
                animationSent = false;
                root.signalAnimationsNeedLength(-1);
            }
        }
    }

    ///////Restore Zoom Animation/////
    ParallelAnimation{
        id: restoreAnimation

        PropertyAnimation {
            target: wrapper
            property: "mScale"
            to: 1
            duration: 3 * mainItemContainer.animationTime
            easing.type: Easing.Linear
        }

        PropertyAnimation {
            target: hiddenSpacerLeft
            property: "nScale"
            to: 0
            duration: 3 * mainItemContainer.animationTime
            easing.type: Easing.Linear
        }

        PropertyAnimation {
            target: hiddenSpacerRight
            property: "nScale"
            to: 0
            duration: 3 * mainItemContainer.animationTime
            easing.type: Easing.Linear
        }
    }

    //A Timer to check how much time the task is hovered in order to check if we must
    //show window previews
    Component {
        id: hoveredTimerComponent
        Timer {
            id: hoveredTimer

            interval: 2*root.durationTime*units.longDuration

            repeat: false

            onTriggered: {
                if(mainItemContainer.containsMouse && windowsPreviewDlg.activeItem !== mainItemContainer){
                    //console.log("Hovered Timer....");
                    mainItemContainer.preparePreviewWindow(false);
                    windowsPreviewDlg.show(mainItemContainer);
                }

                hoveredTimer.destroy();
            }

            Component.onCompleted: hoveredTimer.start()
        }
    }


    //A Timer to help in resist a bit to dragging, the user must try
    //to press a little first before dragging Started
    Component {
        id: resistanerTimerComponent
        Timer {
            id: resistanerTimer
            interval: mainItemContainer.resistanceDelay
            repeat: false

            onTriggered: {
                if (!mainItemContainer.inBlockingAnimation){
                    mainItemContainer.isDragged = true;
                }
                resistanerTimer.destroy();
            }

            Component.onCompleted: resistanerTimer.start()
        }
    }


    ///trying to compete with the crazy situation in the tasksModel
    ///with launchers and startups... There are windows that stay
    ///startup mode e.g. chrome, libreoffice... not showing startups
    ///the user can lose windows...
    ///Based on the animations, windows are shown directly, startups
    ///are shown after 5secs of existence, and launchers after 200ms
    ///for launchers this is set in order to give to a window the time
    ///to desappear and then show the launcher...


    //   property int mainDelay: IsLauncher ? 800 : 400
    //   property int mainDelay: icList.delayingRemoval ? 2*showWindowAnimation.speed : 450

    //BE CAREFUL: this interval (e.g. 700ms) must be lower from the removal animation
    //duration e.g.(800ms) because there are situattions that because of this some
    //launchers delay A LOT to reappear, e.g google-chrome
    //I will blacklist google-chrome as I have not found any other case for this bug
    //to appear, but even this way there are cases that still appears...
    property int mainDelay: (AppId == "google-chrome") ? 0 : 2*root.durationTime*showWindowAnimation.speed
    property int windowDelay: mainItemContainer.isStartup ? 3*root.durationTime*units.longDuration : mainDelay

    Component {
        id: delayShowWindow
        Timer {
            id: timerWindow

            interval: windowDelay

            repeat: false

            onTriggered: {
                //console.log("I am in here: "+mainItemContainer.windowDelay);
                // showWindowAnimation.execute();
                if(!mainItemContainer.buffersAreReady && !root.initializatedBuffers)
                    showWindowAnimation.showWindow();
                else
                    showWindowAnimation.execute();

                timerWindow.destroy();
            }

            Component.onCompleted: timerWindow.start()
        }
    }

    ///Item's Removal Animation

    ListView.onRemove: SequentialAnimation {
        ScriptAction{
            script:{
                mainItemContainer.inAddRemoveAnimation = true;
                root.signalAnimationsNeedLength(1)
            }
        }

        PropertyAction { target: mainItemContainer; property: "ListView.delayRemove"; value: true }
        PropertyAction { target: mainItemContainer; property: "inAnimation"; value: true }
        PropertyAction { target: icList; property: "delayingRemoval"; value: true }
        //PropertyAction { target: wrapper; property: "opacity"; value: isWindow ? 0 : 1 }
        //animation mainly for launchers removal and startups
        ParallelAnimation{
            id: removalAnimation

            // property int speed: (IsStartup && !mainItemContainer.visible)? 0 : 400
            //property int speed: 400

            NumberAnimation { target: wrapper; property: "opacity"; to: 0; duration: showWindowAnimation.speed; easing.type: Easing.InQuad }

            PropertyAnimation {
                target: wrapper
                property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                to: 0
                duration: showWindowAnimation.speed
                easing.type: Easing.InQuad
            }
        }

        //smooth move into place the surrounding tasks
        PropertyAnimation {
            target: wrapper
            property: (icList.orientation == Qt.Vertical) ? "tempScaleHeight" : "tempScaleWidth"
            to: 0
            duration: showWindowAnimation.speed
            easing.type: Easing.InQuad
        }

        PropertyAction { target: mainItemContainer; property: "inAnimation"; value: false }
        ScriptAction{
            script:{
                if (showWindowAnimation.animationSent){
                    //console.log("SAFETY REMOVAL 1: animation removing ended");
                    showWindowAnimation.animationSent = false;
                    root.signalAnimationsNeedLength(-1);
                }

                root.signalAnimationsNeedLength(-1)
            }
        }

        PropertyAction { target: mainItemContainer; property: "ListView.delayRemove"; value: false }
        PropertyAction { target: icList; property: "delayingRemoval"; value: false }
    }

}// main Item

