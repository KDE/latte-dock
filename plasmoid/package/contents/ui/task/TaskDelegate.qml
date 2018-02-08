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

import "animations" as TaskAnimations

MouseArea{
    id: mainItemContainer

    visible: false //true//(isStartup && root.durationTime !== 0) ? false : true

    anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
    anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
    anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
    anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

    objectName: "TaskDelegate"

    width: {
        if (!visible)
            return 0;

        if (isSeparator)
            return root.vertical ? separatorItem.width : (root.dragSource || root.editMode ? 5+root.iconMargin : 0);

        if (root.vertical) {
            if (!inAttentionAnimation)
                return wrapper.width;
            else
                return wrapper.maxThickness;
        } else {
            return hiddenSpacerLeft.width+wrapper.width+hiddenSpacerRight.width;
        }
    }

    height: {
        if (!visible)
            return 0;

        if (isSeparator)
            return !root.vertical ? separatorItem.height : (root.dragSource || root.editMode ? 5+root.iconMargin: 0);

        if (root.vertical) {
            return hiddenSpacerLeft.height + wrapper.height + hiddenSpacerRight.height;
        } else {
            if (!inAttentionAnimation)
                return wrapper.height;
            else
                return wrapper.maxThickness;
        }
    }

    acceptedButtons: Qt.LeftButton | Qt.MidButton | Qt.RightButton
    hoverEnabled: visible && (!inAnimation) && (!IsStartup) && (!root.taskInAnimation)
                  && (!root.editMode || root.debugLocation)&&(!inBouncingAnimation) && !isSeparator
    // hoverEnabled: false
    //opacity : isSeparator && (hiddenSpacerLeft.neighbourSeparator || hiddenSpacerRight.neighbourSeparator) ? 0 : 1

    property bool buffersAreReady: false
    property bool delayingRemove: ListView.delayRemove
    //states that exist in windows in a Group of windows
    property bool hasActive: isActive
    property bool hasMinimized: (IsGroupParent === true) ? tasksWindows.hasMinimized : isMinimized
    property bool hasShown: (IsGroupParent === true) ? tasksWindows.hasShown : !isMinimized
    property bool inAddRemoveAnimation: true
    property bool inAnimation: true
    property bool inAttentionAnimation: false
    property bool inBlockingAnimation: false
    property bool inBouncingAnimation: false
    property bool inFastRestoreAnimation: false
    property bool inMimicParabolicAnimation: false
    property real mimicParabolicScale: -1
    property bool inPopup: false
    property bool inRemoveStage: false
    property bool inWheelAction: false

    property bool isActive: (IsActive === true) ? true : false
    property bool isDemandingAttention: (IsDemandingAttention === true) ? true : false
    property bool isDragged: false
    property bool isGroupParent: (IsGroupParent === true) ? true : false
    property bool isLauncher: (IsLauncher === true) ? true : false
    property bool isMinimized: (IsMinimized === true) ? true : false
    property bool isSeparator: false
    property bool isStartup: (IsStartup === true) ? true : false
    property bool isWindow: (IsWindow === true) ? true : false
    property bool isZoomed: false

    property bool pressed: false
    readonly property bool showAttention: isDemandingAttention && plasmoid.status === PlasmaCore.Types.RequiresAttentionStatus ?
                                              true : false

    property int animationTime: root.durationTime * 1.2 * units.shortDuration
    property int badgeIndicator: 0 //it is used from external apps
    property int hoveredIndex: icList.hoveredIndex
    property int itemIndex: index
    property int lastValidIndex: -1 //used for the removal animation
    property int lastButtonClicked: -1;
    property int pressX: -1
    property int pressY: -1
    property int resistanceDelay: 450
    property int spacersMaxSize: Math.max(0,Math.ceil(0.55*root.iconSize) - root.iconMargin)

    property string activity: tasksModel.activity

    readonly property var m: model
    readonly property int pid: model && model.AppPid ? model.AppPid : -1
    readonly property string appName: model && model.AppName ? model.AppName : ""

    property string modelLauncherUrl: (LauncherUrlWithoutIcon !== null) ? LauncherUrlWithoutIcon : ""
    property string modelLauncherUrlWithIcon: (LauncherUrl !== null) ? LauncherUrl : ""
    property string launcherUrl: ""
    property string launcherUrlWithIcon: ""
    property string launcherName: ""

    property Item tooltipVisualParent: wrapper.titleTooltipVisualParent
    property Item previewsVisualParent: wrapper.previewsTooltipVisualParent

    onModelLauncherUrlChanged: {
        if (modelLauncherUrl !== ""){
            launcherUrl = modelLauncherUrl;

            //!extract the launcherName if possible
            var nameStarts = launcherUrl.lastIndexOf("/");
            if (nameStarts === -1){
                nameStarts = launcherUrl.lastIndexOf(":");
            }

            var nameEnds = launcherUrl.lastIndexOf(".desktop");

            if (nameStarts!==-1 && nameEnds!==-1 && nameStarts<nameEnds) {
                launcherName = launcherUrl.substring(nameStarts+1,nameEnds);
            }
        }

        if (parabolicManager.isSeparator(modelLauncherUrl)){
            isSeparator = true;
        } else {
            isSeparator = false;
        }
    }

    onModelLauncherUrlWithIconChanged: {
        if (modelLauncherUrlWithIcon !== ""){
            launcherUrlWithIcon = modelLauncherUrlWithIcon;
        }
    }

    ////// Audio streams //////
    property Item audioStreamOverlay
    property var audioStreams: []
    readonly property bool hasAudioStream: root.indicateAudioStreams && audioStreams.length > 0 && !isLauncher
    readonly property bool playingAudio: hasAudioStream && audioStreams.some(function (item) {
        return !item.corked
    })
    readonly property bool muted: hasAudioStream && audioStreams.every(function (item) {
        return item.muted
    })

    readonly property int volume: {
        if (!hasAudioStream){
            return 0;
        }

        var maxVolume = 0;
        for (var i=0; i<audioStreams.length; ++i){
            if (audioStreams[i].volume > maxVolume)
                maxVolume = audioStreams[i].volume;
        }

        return maxVolume;
    }

    //////

    property QtObject contextMenu: null
    property QtObject draggingResistaner: null
    property QtObject hoveredTimerObj: null

    signal groupWindowAdded();
    signal groupWindowRemoved();
    signal checkWindowsStates();

    Behavior on opacity {
        // NumberAnimation { duration: (IsStartup || (IsLauncher) ) ? 0 : 400 }
        NumberAnimation { duration: root.durationTime*units.longDuration }
    }

    TaskWindows{
        id: tasksWindows

        property int previousCount: 0

        onWindowsCountChanged: {
            if ((windowsCount >= 2) && (windowsCount > previousCount)
                    && !(mainItemContainer.containsMouse || parabolicManager.neighbourIsHovered(itemIndex)) ){
                if(root.dragSource == null)
                    mainItemContainer.groupWindowAdded();
            }
            else if ((windowsCount >=1) &&(windowsCount < previousCount)){
                //sometimes this is triggered in dragging with no reason
                if(root.dragSource == null && !mainItemContainer.delayingRemove)
                    mainItemContainer.groupWindowRemoved();
            }

            if (windowsCount>=1) {
                mainItemContainer.slotPublishGeometries();
            }

            previousCount = windowsCount;
        }
    }

    Loader {
        id: isSeparatorRectangle
        active: (opacityN>0)

        width: mainItemContainer.width
        height: mainItemContainer.height
        anchors.centerIn: separatorItem

        property real opacityN: isSeparator && root.contextMenu && root.contextMenu.visualParent === mainItemContainer ? 1 : 0

        Behavior on opacityN {
            NumberAnimation { duration: root.durationTime*units.longDuration }
        }

        sourceComponent: Rectangle{
            anchors.fill: parent
            opacity: isSeparatorRectangle.opacityN
            radius: 3

            property color tempColor: theme.highlightColor
            color: tempColor
            border.width: 1
            border.color: theme.highlightColor

            onTempColorChanged: tempColor.a = 0.35;

        }
    }

    Item{
        id:separatorItem
        anchors.rightMargin: root.position === PlasmaCore.Types.RightPositioned ? localThickMargin : 0
        anchors.leftMargin: root.position === PlasmaCore.Types.LeftPositioned ? localThickMargin : 0
        anchors.bottomMargin: root.position === PlasmaCore.Types.BottomPositioned ? localThickMargin : 0
        anchors.topMargin: root.position === PlasmaCore.Types.TopPositioned ? localThickMargin : 0

        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined
        anchors.right: root.position === PlasmaCore.Types.RightPositioned ? parent.right : undefined;
        anchors.left: root.position === PlasmaCore.Types.LeftPositioned ? parent.left : undefined;
        anchors.top: root.position === PlasmaCore.Types.TopPositioned ? parent.top : undefined;
        anchors.bottom: root.position === PlasmaCore.Types.BottomPositioned ? parent.bottom : undefined;

        //opacity: separatorShadow.active || root.internalSeparatorHidden ? 0 : 0.4
        opacity: separatorShadow.active || forceHiddenState ? 0 : 0.4

        visible: mainItemContainer.isSeparator

        width: root.vertical ? root.iconSize : (root.dragSource || root.editMode) ? 5+root.iconMargin: 1
        height: !root.vertical ? root.iconSize : (root.dragSource || root.editMode) ? 5+root.iconMargin: 1

        property int localThickMargin: root.statesLineSize + root.thickMarginBase + 4

        property bool forceHiddenState: false

        Behavior on opacity {
            NumberAnimation { duration: root.durationTime*units.longDuration }
        }

        function updateForceHiddenState() {
            if (!isSeparator || root.editMode || root.dragSource) {
                forceHiddenState = false;
            } else {
                var firstPosition = (index>=0) && (index < parabolicManager.firstRealTaskIndex);
                var sepNeighbour = parabolicManager.taskIsSeparator(index-1);
                var firstSepFromLastSeparatorsGroup = (index>=0) && (index > parabolicManager.lastRealTaskIndex);

                forceHiddenState = (firstPosition || sepNeighbour || firstSepFromLastSeparatorsGroup);
            }
        }

        Component.onCompleted: updateForceHiddenState();

        onForceHiddenStateChanged: root.separatorsUpdated();

        Connections{
            target: root
            onEditModeChanged: separatorItem.updateForceHiddenState();
            onDragSourceChanged: {
                separatorItem.updateForceHiddenState();

                if (isSeparator && !root.dragSource) {
                    parabolicManager.setSeparator(launcherUrl, itemIndex);
                }
            }
            onSeparatorsUpdated: separatorItem.updateForceHiddenState();

            onGlobalDirectRenderChanged:{
                if (root.globalDirectRender && restoreAnimation.running) {
                    // console.log("Cleat Task Scale !!!!");
                    restoreAnimation.stop();
                }
            }
        }

        Rectangle {
            anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
            anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined
            anchors.right: root.position === PlasmaCore.Types.RightPositioned ? parent.right : undefined;
            anchors.left: root.position === PlasmaCore.Types.LeftPositioned ? parent.left : undefined;
            anchors.top: root.position === PlasmaCore.Types.TopPositioned ? parent.top : undefined;
            anchors.bottom: root.position === PlasmaCore.Types.BottomPositioned ? parent.bottom : undefined;

            radius: 2

            width: root.vertical ? root.iconSize - 8  : 1
            height: !root.vertical ? root.iconSize - 8 : 1
            color: theme.textColor
        }
    }

    ///Shadow in tasks
    Loader{
        id: separatorShadow
        anchors.fill: separatorItem
        active: root.enableShadows && isSeparator
        opacity: separatorItem.forceHiddenState ? 0 : 0.4

        Behavior on opacity {
            NumberAnimation { duration: root.durationTime*units.longDuration }
        }

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: root.appShadowColor
            samples: 2 * radius
            source: separatorItem
            radius: root.appShadowSize
            verticalOffset: 2
        }
    }

    /* Rectangle{
            anchors.fill: parent
            color: "transparent"
            border.width: 1
            border.color: "blue"
        } */

    Flow{
        id: taskFlow
        width: parent.width
        height: parent.height

        // a hidden spacer for the first element to add stability
        // IMPORTANT: hidden spacers must be tested on vertical !!!
        TaskHiddenSpacer{ id:hiddenSpacerLeft;}

        TaskWrapper{ id: wrapper }

        // a hidden spacer on the right for the last item to add stability
        TaskHiddenSpacer{ id:hiddenSpacerRight; rightSpacer: true }
    }// Flow with hidden spacers inside

    /*Rectangle{
        anchors.fill: taskFlow
        color: "transparent"
        border.width: 1
        border.color: "blue"
    }*/

    Component {
        id: taskInitComponent
        Timer {
            id: timer

            interval: 800
            repeat: false

            onTriggered: {
                //      mainItemContainer.hoverEnabled = true;
                slotPublishGeometries();

                if (latteDock && latteDock.debugModeTimers) {
                    console.log("plasmoid timer: taskInitComponentTimer called...");
                }

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

    onAppNameChanged: updateAudioStreams()
    onPidChanged: updateAudioStreams()
    onHasAudioStreamChanged: updateAudioStreams()

    onHoveredIndexChanged: {
        var distanceFromHovered = Math.abs(index - icList.hoveredIndex);

        /*if( (distanceFromHovered > 1) && (hoveredIndex !== -1)){
            if(!isDragged)
                wrapper.mScale = 1;
        }*/

        if (distanceFromHovered >= 1 && !inAttentionAnimation && !inFastRestoreAnimation && !inMimicParabolicAnimation) {
            hiddenSpacerLeft.nScale = 0;
            hiddenSpacerRight.nScale = 0;
        }
    }

    onItemIndexChanged: {
        if (itemIndex>=0)
            lastValidTimer.start();

        if (isSeparator){
            parabolicManager.setSeparator(launcherUrl, itemIndex);
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

    onIsSeparatorChanged: {
        if (isSeparator) {
            parabolicManager.setSeparator(launcherUrl, itemIndex);

            if (parabolicManager.isLauncherToBeMoved(launcherUrl) && itemIndex>=0) {
                parabolicManager.moveLauncherToCorrectPos(launcherUrl, itemIndex);
            }
        } else {
            parabolicManager.setSeparator(launcherUrl, -1);
        }
    }

    onLauncherUrlChanged: updateBadge();

    ////// End of Values Changes /////


    ///////////////// Mouse Area Events ///////////////////
    onEntered: {
        if (root.editMode)
            return;

        // console.log("entered task:" + icList.hoveredIndex);

        if ((wrapper.mScale>=root.midZoomFactor) && (icList.hoveredIndex!==-1 || icList.hoveredIndex===index)
                && !root.directRenderTimerIsRunning && !root.globalDirectRender) {
            root.startEnableDirectRenderTimer();
        }

        if ((icList.hoveredIndex !== itemIndex) && isLauncher && windowsPreviewDlg.visible) {
            windowsPreviewDlg.hide(1);
        }

        if (!latteDock || (latteDock && !(latteDock.dockIsHidden || latteDock.inSlidingIn || latteDock.inSlidingOut))){
            icList.hoveredIndex = index;
        }

        if(!root.latteDock)
            checkListHovered.stop();

        if (root.latteDock && (!root.showPreviews || (root.showPreviews && isLauncher))){
            root.latteDock.showTooltipLabel(mainItemContainer, model.AppName);
        }

        if (root.latteDock && root.latteDock.isHalfShown) {
            return;
        }

        if((!inAnimation)&&(root.dragSource == null)&&(!root.taskInAnimation) && hoverEnabled){
            if (inAttentionAnimation) {
                var subSpacerScale = (root.zoomFactor-1)/2;
                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            }

            if (!inBlockingAnimation || inAttentionAnimation) {
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
    }

    // IMPORTANT: This must be improved ! even for small miliseconds  it reduces performance
    onExited: {
        if (root.latteDock && (!root.showPreviews || (root.showPreviews && isLauncher))){
            root.latteDock.hideTooltipLabel();
        }

        if(mainItemContainer.contextMenu && mainItemContainer.contextMenu.status == PlasmaComponents.DialogStatus.Open){
            ///dont check to restore zooms
        }
        else{
            if(!inAnimation){
                root.startCheckRestoreZoomTimer();
            }
        }

        /* if(draggingResistaner != null){
                draggingResistaner.destroy();
                draggingResistaner = null;
                isDragged = false;
            }*/
    }

    onPositionChanged: {
        if (root.editMode || (inBlockingAnimation && !(inAttentionAnimation||inFastRestoreAnimation||inMimicParabolicAnimation)))
            return;

        if(!root.latteDock)
            checkListHovered.stop();

        if (root.latteDock && root.latteDock.isHalfShown) {
            return;
        }

        if((inAnimation == false)&&(!root.taskInAnimation)&&(!root.disableRestoreZoom) && hoverEnabled){
            if (!latteDock || (latteDock && !(latteDock.dockIsHidden || latteDock.inSlidingIn || latteDock.inSlidingOut))){
                icList.hoveredIndex = index;
            }

            if( ((wrapper.mScale == 1 || wrapper.mScale === root.zoomFactor) && !root.globalDirectRender) || root.globalDirectRender) {
                if(root.dragSource == null){
                    if (icList.orientation == Qt.Horizontal){
                        var step = Math.abs(icList.currentSpot-mouse.x);
                        if (step >= root.animationStep){
                            icList.currentSpot = mouse.x;

                            wrapper.calculateScales(mouse.x);
                        }
                    }
                    else{
                        var step = Math.abs(icList.currentSpot-mouse.y);
                        if (step >= root.animationStep){
                            icList.currentSpot = mouse.y;

                            wrapper.calculateScales(mouse.y);
                        }
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

        ////window previews/////////
        if (isWindow) {
            if(containsMouse && (root.showPreviews || (!root.showPreviews && root.highlightWindows)) && Latte.WindowSystem.compositingActive){
                hoveredTimerObj = hoveredTimerComponent.createObject(mainItemContainer);
            }
            else{
                if (hoveredTimerObj){
                    hoveredTimerObj.stop();
                    hoveredTimerObj.destroy();
                }
            }
        }

        ////disable hover effect///
        if (isWindow && root.highlightWindows && !containsMouse) {
            root.windowsHovered(model.LegacyWinIdList, false);
        }
    }

    onPressed: {
        //console.log("Pressed Task Delegate..");
        if (Latte.WindowSystem.compositingActive) {
            windowsPreviewDlg.hide(2);
        }

        var modAccepted = modifierAccepted(mouse);

        if ((mouse.button == Qt.LeftButton)||(mouse.button == Qt.MidButton) || modAccepted) {
            lastButtonClicked = mouse.button;
            pressed = true;
            pressX = mouse.x;
            pressY = mouse.y;

            if(draggingResistaner == null && !modAccepted)
                draggingResistaner = resistanerTimerComponent.createObject(mainItemContainer);
        }
        else if (mouse.button == Qt.RightButton && !modAccepted){
            // When we're a launcher, there's no window controls, so we can show all
            // places without the menu getting super huge.
            if (model.IsLauncher === true && !isSeparator) {
                showContextMenu({showAllPlaces: true})
            } else {
                showContextMenu();
            }

            //root.createContextMenu(mainItemContainer).show();
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

        if(pressed && (!inBlockingAnimation || inAttentionAnimation) && !isSeparator){

            if (modifierAccepted(mouse)){
                if( !mainItemContainer.isLauncher){
                    if (root.modifierClickAction == Latte.Dock.NewInstance) {
                        tasksModel.requestNewInstance(modelIndex());
                    } else if (root.modifierClickAction == Latte.Dock.Close) {
                        tasksModel.requestClose(modelIndex());
                    } else if (root.modifierClickAction == Latte.Dock.ToggleMinimized) {
                        tasksModel.requestToggleMinimized(modelIndex());
                    } else if ( root.modifierClickAction == Latte.Dock.CycleThroughTasks) {
                        if (isGroupParent)
                            tasksWindows.activateNextTask();
                        else
                            activateTask();
                    } else if (root.modifierClickAction == Latte.Dock.ToggleGrouping) {
                        tasksModel.requestToggleGrouping(modelIndex());
                    }
                } else {
                    activateTask();
                }
            } else if (mouse.button == Qt.MidButton){
                if( !mainItemContainer.isLauncher){
                    if (root.middleClickAction == Latte.Dock.NewInstance) {
                        tasksModel.requestNewInstance(modelIndex());
                    } else if (root.middleClickAction == Latte.Dock.Close) {
                        tasksModel.requestClose(modelIndex());
                    } else if (root.middleClickAction == Latte.Dock.ToggleMinimized) {
                        tasksModel.requestToggleMinimized(modelIndex());
                    } else if ( root.middleClickAction == Latte.Dock.CycleThroughTasks) {
                        if (isGroupParent)
                            tasksWindows.activateNextTask();
                        else
                            activateTask();
                    } else if (root.middleClickAction == Latte.Dock.ToggleGrouping) {
                        tasksModel.requestToggleGrouping(modelIndex());
                    }
                } else {
                    activateTask();
                }
            } else if (mouse.button == Qt.LeftButton){
                activateTask();
            }

            backend.cancelHighlightWindows();
        }

        pressed = false;

        if(!inAnimation && !root.latteDock)
            checkListHovered.startDuration(3*units.longDuration);
    }

    onWheel: {
        if (isSeparator || !root.mouseWheelActions || inWheelAction || inBouncingAnimation
                || (latteDock && (latteDock.dockIsHidden || latteDock.inSlidingIn || latteDock.inSlidingOut))){
            return;
        }

        var angle = wheel.angleDelta.y / 8;

        //positive direction
        if (angle > 12) {
            if (isLauncher) {
                inWheelAction = true;
                wrapper.runLauncherAnimation();
            } else if (isGroupParent) {
                tasksWindows.activateNextTask();
            } else {
                var taskIndex = modelIndex();

                if (isMinimized) {
                    inWheelAction = true;
                    tasksModel.requestToggleMinimized(taskIndex);
                    wheelActionDelayer.start();
                }

                tasksModel.requestActivate(taskIndex);
            }
            //negative direction
        } else if (angle < -12) {
            if (isLauncher) {
                // do nothing
            } else if (isGroupParent) {
                tasksWindows.activatePreviousTask();
            } else {
                var taskIndex = modelIndex();

                if (isMinimized) {
                    inWheelAction = true;
                    tasksModel.requestToggleMinimized(taskIndex);
                    wheelActionDelayer.start();
                }

                tasksModel.requestActivate(taskIndex);
            }
        }
    }

    ///////////////// End Of Mouse Area Events ///////////////////

    ///// Handlers for Signals /////
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
        if(!root)
            return;

        if (root.globalDirectRender)
            wrapper.mScale = 1;
        else if (!inAttentionAnimation && !inFastRestoreAnimation && !inMimicParabolicAnimation){
            if (icList.hoveredIndex === -1) {
                restoreAnimation.start();
            } else {
                console.log("Clear zoom signal ignored...");
            }
        }
    }

    function handlerDraggingFinished(){
        isDragged = false;
    }
    ///// End of Handlers //////



    ///// Helper functions /////
    function activateNextTask() {
        tasksWindows.activateNextTask();
    }

    function activateTask() {
        if( mainItemContainer.isLauncher){
            if (Latte.WindowSystem.compositingActive) {
                wrapper.runLauncherAnimation();
            } else {
                launcherAction();
            }
        }
        else{
            if (model.IsGroupParent) {
                if (Latte.WindowSystem.compositingActive && backend.canPresentWindows()) {
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

    function preparePreviewWindow(hideClose){
        windowsPreviewDlg.visualParent = previewsVisualParent;

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
        if (Latte.WindowSystem.compositingActive) {
            inBouncingAnimation = true;
            root.addWaitingLauncher(mainItemContainer.launcherUrl);
        }
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

    function launcherIsPresent(url) {
        var activities = tasksModel.launcherActivities(url);

        var NULL_UUID = "00000000-0000-0000-0000-000000000000";

        if (activities.indexOf(NULL_UUID) !== -1 || activities.indexOf(activityInfo.currentActivity) !== -1)
            return true;

        return false;
    }

    function modelIndex(){
        return tasksModel.makeModelIndex(index);
    }

    function showContextMenu(args) {
        if (isSeparator && !root.editMode)
            return;

        contextMenu = root.createContextMenu(mainItemContainer, modelIndex(), args);
        contextMenu.show();
    }

    function modifierAccepted(mouse){
        if (mouse.modifiers & root.modifierQt){
            if ((mouse.button === Qt.LeftButton && root.modifierClick === Latte.Dock.LeftClick)
                    || (mouse.button === Qt.MiddleButton && root.modifierClick === Latte.Dock.MiddleClick)
                    || (mouse.button === Qt.RightButton && root.modifierClick === Latte.Dock.RightClick))
                return true;
        }

        return false;
    }

    function setBlockingAnimation(value){
        inBlockingAnimation = value;
    }

    function slotMimicEnterForParabolic(){
        if (containsMouse) {
            if (inMimicParabolicAnimation) {
                mimicParabolicScale = root.zoomFactor;
            }

            wrapper.calculateScales(icList.currentSpot);
        }
    }

    function slotShowPreviewForTasks(group) {
        if (group === mainItemContainer) {
            preparePreviewWindow(true);
            windowsPreviewDlg.show(mainItemContainer);
        }
    }

    function slotPublishGeometries() {
        //! this way we make sure that layouts that are in different activities that the current layout
        //! dont publish their geometries
        if ((isWindow || isStartup || isGroupParent) && icList && !icList.delayingRemoval
                && (!latteDock
                    || (latteDock && currentLayout && latteDock.universalLayoutManager &&
                        currentLayout.name === latteDock.universalLayoutManager.currentLayoutName))) {
            var globalChoords = backend.globalRect(mainItemContainer);

            //! Magic Lamp effect doesnt like coordinates outside the screen and
            //! width,heights of zero value... So we now normalize the geometries
            //! sent in order to avoid such circumstances
            if (root.vertical) {
                globalChoords.width = 1;
                globalChoords.height = Math.max(root.iconSize, mainItemContainer.height);
            } else {
                globalChoords.height = 1;
                globalChoords.width = Math.max(root.iconSize, mainItemContainer.width);
            }

            if (root.position === PlasmaCore.Types.BottomPositioned) {
                globalChoords.y = plasmoid.screenGeometry.y+plasmoid.screenGeometry.height-1;
            } else if (root.position === PlasmaCore.Types.TopPositioned) {
                globalChoords.y = plasmoid.screenGeometry.y+1;
            } else if (root.position === PlasmaCore.Types.LeftPositioned) {
                globalChoords.x = plasmoid.screenGeometry.x+1;
            } else if (root.position === PlasmaCore.Types.RightPositioned) {
                globalChoords.x = plasmoid.screenGeometry.x+plasmoid.screenGeometry.width - 1;
            }

            tasksModel.requestPublishDelegateGeometry(mainItemContainer.modelIndex(), globalChoords, mainItemContainer);
        }
    }

    function slotWaitingLauncherRemoved(launch) {
        if ((isWindow || isStartup) && !visible && launch === launcherUrl) {
            wrapper.mScale = 1;
            visible = true;
        }
    }


    function updateAudioStreams() {
        if (root.dragSource !== null) {
            audioStreams = [];
            return;
        }

        var pa = pulseAudio.item;
        if (!pa) {
            audioStreams = [];
            return;
        }

        var streams = pa.streamsForPid(mainItemContainer.pid);
        if (streams.length) {
            pa.registerPidMatch(mainItemContainer.appName);
        } else {
            // We only want to fall back to appName matching if we never managed to map
            // a PID to an audio stream window. Otherwise if you have two instances of
            // an application, one playing and the other not, it will look up appName
            // for the non-playing instance and erroneously show an indicator on both.
            if (!pa.hasPidMatch(mainItemContainer.appName)) {
                var streams_result;
                streams_result = pa.streamsForAppName(mainItemContainer.appName);
                if (streams_result.length===0 && launcherName !== "") {
                    streams_result = pa.streamsForAppName(launcherName);
                }
                streams = streams_result;
            }
        }

        // fix a binding loop concerning audiostreams, the audiostreams
        // should be updated only when they have changed
        var changed = false;

        if (streams.length !== audioStreams.length) {
            changed = true;
        } else {
            for(var i=0; i<streams.length; ++i) {
                if (streams[i] !== audioStreams[i]) {
                    changed = true;
                    break;
                }
            }
        }

        if (changed) {
            mainItemContainer.audioStreams = streams;
        }
    }

    function toggleMuted() {
        if (muted) {
            mainItemContainer.audioStreams.forEach(function (item) { item.unmute(); });
        } else {
            mainItemContainer.audioStreams.forEach(function (item) { item.mute(); });
        }
    }

    function increaseVolume() {
        mainItemContainer.audioStreams.forEach(function (item) { item.increaseVolume(); });
    }

    function decreaseVolume() {
        mainItemContainer.audioStreams.forEach(function (item) { item.decreaseVolume(); });
    }

    function updateBadge() {
        var badger = root.getBadger(launcherUrl);

        if (badger && !isLauncher) {
            badgeIndicator = parseInt(badger.value);
        } else {
            badgeIndicator = 0;
        }
    }

    Connections {
        target: pulseAudio.item
        ignoreUnknownSignals: true // Plasma-PA might not be available
        onStreamsChanged: mainItemContainer.updateAudioStreams()
    }

    Connections {
        target: root
        //trying to fix #440, showing the audio icon indicator to irrelevant tasks
        //after dragging an existent task with audio
        onDragSourceChanged: mainItemContainer.updateAudioStreams()
        onIndicateAudioStreamsChanged: mainItemContainer.updateAudioStreams()
    }
    ///// End of Helper functions ////

    Component.onCompleted: {
        root.draggingFinished.connect(handlerDraggingFinished);
        root.clearZoomSignal.connect(clearZoom);
        root.publishTasksGeometries.connect(slotPublishGeometries);
        root.showPreviewForTasks.connect(slotShowPreviewForTasks);
        root.mimicEnterForParabolic.connect(slotMimicEnterForParabolic);

        //startup without launcher
        var hideStartup =  ((((tasksModel.launcherPosition(mainItemContainer.launcherUrl) == -1)
                              && (tasksModel.launcherPosition(mainItemContainer.launcherUrlWithIcon) == -1) )
                             || !launcherIsPresent(mainItemContainer.launcherUrl))
                            && mainItemContainer.isStartup);

        if (!Latte.WindowSystem.compositingActive) {
            visible = true;
        } else if ( (isWindow || isStartup) && root.waitingLauncherExists(launcherUrl)) {
            root.waitingLauncherRemoved.connect(slotWaitingLauncherRemoved);
            visible = false;
        } else if (hideStartup){
            visible = false;
        } else {
            visible = true;
        }

        showWindowAnimation.showWindow();
        updateAudioStreams();
    }

    Component.onDestruction: {
        root.waitingLauncherRemoved.disconnect(slotWaitingLauncherRemoved);
        root.draggingFinished.disconnect(handlerDraggingFinished);
        root.clearZoomSignal.disconnect(clearZoom);
        root.publishTasksGeometries.disconnect(slotPublishGeometries);
        root.showPreviewForTasks.disconnect(slotShowPreviewForTasks);
        root.mimicEnterForParabolic.disconnect(slotMimicEnterForParabolic);

        wrapper.sendEndOfNeedBothAxisAnimation();
    }

    /////Animations

    TaskAnimations.TaskShowWindowAnimation{ id: showWindowAnimation }

    TaskAnimations.TaskRestoreAnimation{ id: restoreAnimation}

    //A Timer to check how much time the task is hovered in order to check if we must
    //show window previews
    Component {
        id: hoveredTimerComponent
        Timer {
            id: hoveredTimer

            interval: Math.max(150,plasmoid.configuration.previewsDelay)

            repeat: false

            onTriggered: {
                if(mainItemContainer.containsMouse && windowsPreviewDlg.activeItem !== mainItemContainer){
                    //console.log("Hovered Timer....");
                    if (root.showPreviews) {
                        mainItemContainer.preparePreviewWindow(false);
                        windowsPreviewDlg.show(mainItemContainer);
                    } else if (mainItemContainer.isWindow && root.highlightWindows) {
                        root.windowsHovered(model.LegacyWinIdList, mainItemContainer.containsMouse);
                    }
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

                if (latteDock && latteDock.debugModeTimers) {
                    console.log("plasmoid timer: resistanerTimer called...");
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
                if(!mainItemContainer.buffersAreReady)
                    showWindowAnimation.showWindow();
                else
                    showWindowAnimation.execute();

                if (latteDock && latteDock.debugModeTimers) {
                    console.log("plasmoid timer: timerWindow called...");
                }

                timerWindow.destroy();
            }

            Component.onCompleted: timerWindow.start()
        }
    }

    // when changing activities and desktops the index of the tasks
    // is updated immediately to -1, this timer protects this indexing
    // change in order to provide a beautiful removal tasks animation
    Timer {
        id: lastValidTimer
        interval: 100 ///the interval does not follow the animations timing
        repeat: false

        onTriggered: {
            if (mainItemContainer.itemIndex >= 0)
                mainItemContainer.lastValidIndex = mainItemContainer.itemIndex;

            if (latteDock && latteDock.debugModeTimers) {
                console.log("plasmoid timer: lastValidTimer called...");
            }
        }
    }

    // The best solution in order to catch when the wheel action ended is to
    // track the isMinimized state, but when the user has enabled window previews
    // at all times that flag doesnt work
    Timer {
        id: wheelActionDelayer
        interval: 200
        onTriggered: {
            mainItemContainer.inWheelAction = false;

            if (latteDock && latteDock.debugModeTimers) {
                console.log("plasmoid timer: wheelActionDelayer called...");
            }
        }
    }

    ///Item's Removal Animation
    ListView.onRemove: TaskAnimations.TaskRealRemovalAnimation{ id: taskRealRemovalAnimation }

}// main Item

