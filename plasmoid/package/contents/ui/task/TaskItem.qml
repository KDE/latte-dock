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

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.tasks 0.1 as LatteTasks

import "animations" as TaskAnimations
import "indicator" as Indicator

Item {
    id: taskItem

    visible: false //true//(isStartup && animations.speedFactor.current !== 0) ? false : true

    anchors.bottom: (parent && root.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (parent && root.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (parent && root.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (parent && root.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    objectName: "TaskItem"

    width: {
        if (!visible)
            return 0;

        if (isSeparator) {
            if (root.vertical) {
                return taskItem.abilities.metrics.totals.thickness + taskItem.abilities.metrics.margin.screenEdge;
            } else {
                if (root.dragSource || !taskItem.abilities.parabolic.isEnabled) {
                    return LatteCore.Environment.separatorLength+2*taskItem.abilities.metrics.margin.length;
                }
            }

            return 0;
        }

        if (root.vertical) {
            return wrapper.width;
        } else {
            return hiddenSpacerLeft.width+wrapper.width+hiddenSpacerRight.width;
        }
    }

    /*onWidthChanged: {
        console.log("T: " + itemIndex + " - " + launcherUrl + " - " + width + " _ "+ hiddenSpacerLeft.width + " _ " + wrapper.width + " _ " + hiddenSpacerRight.width);
    }*/

    height: {
        if (!visible)
            return 0;

        if (isSeparator) {
            if (!root.vertical) {
                return taskItem.abilities.metrics.totals.thickness + taskItem.abilities.metrics.margin.screenEdge;
            } else {
                if (root.dragSource || !taskItem.abilities.parabolic.isEnabled) {
                    return LatteCore.Environment.separatorLength+2*taskItem.abilities.metrics.margin.length;
                }
            }

            return 0;
        }

        if (root.vertical) {
            return hiddenSpacerLeft.height + wrapper.height + hiddenSpacerRight.height;
        } else {
            return wrapper.height;
        }
    }

    property alias hoverEnabled: taskMouseArea.hoverEnabled
    property alias containsMouse: taskMouseArea.containsMouse
    property alias pressed: taskMouseArea.pressed

    // hoverEnabled: false
    //opacity : isSeparator && (hiddenSpacerLeft.neighbourSeparator || hiddenSpacerRight.neighbourSeparator) ? 0 : 1

    property bool buffersAreReady: false
    property bool delayingRemove: ListView.delayRemove
    property bool scalesUpdatedOnce: false
    //states that exist in windows in a Group of windows
    property bool hasActive: isActive
    property bool hasMinimized: (IsGroupParent === true) ? subWindows.hasMinimized : isMinimized
    property bool hasShown: (IsGroupParent === true) ? subWindows.hasShown : !isMinimized && isWindow
    property bool inAttention: isDemandingAttention && plasmoid.status === PlasmaCore.Types.RequiresAttentionStatus ? true : false
    /*animations flags*/
    property bool inAnimation: true
    property bool inAddRemoveAnimation: true
    property bool inAttentionAnimation: false
    property bool inBlockingAnimation: false
    property bool inBouncingAnimation: false
    property bool inFastRestoreAnimation: false
    property bool inMimicParabolicAnimation: false
    property bool inNewWindowAnimation: false
    property real mimicParabolicScale: -1
    property bool inPopup: false
    property bool inRemoveStage: false

    //! after clicking to show/hide preview enter events are trigerred even though the should not
    property bool showPreviewsIsBlockedFromReleaseEvent: false

    property bool isAbleToShowPreview: true
    property bool isActive: (IsActive === true) ? true : false
    property bool isDemandingAttention: (IsDemandingAttention === true) ? true : false
    property bool isDragged: false
    property bool isGroupable: (IsGroupable === true) ? true : false
    property bool isGroupParent: (IsGroupParent === true) ? true : false
    readonly property bool isHidden: !visible || isForcedHidden
    property bool isForcedHidden: false
    property bool isLauncher: (IsLauncher === true) ? true : false
    property bool hasShownLauncher:  (taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl)
                                     || taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrlWithIcon))
                                     && !root.inActivityChange /*update trigger when changing current activity*/
    property bool isMinimized: (IsMinimized === true) ? true : false
    property bool isSeparator: false
    property bool isStartup: (IsStartup === true) ? true : false
    property bool isWindow: (IsWindow === true) ? true : false
    property bool isZoomed: false

    property bool canPublishGeometries: (isWindow || isStartup || isGroupParent) && visible && width>=taskItem.abilities.metrics.iconSize && height>=taskItem.abilities.metrics.iconSize
                                        && !taskItem.delayingRemove
                                        && (wrapper.mScale===1 || wrapper.mScale===taskItem.abilities.parabolic.factor.zoom) //don't publish during zoom animation

    property bool hoveredFromDragging: (mouseHandler.hoveredItem === taskItem) || (mouseHandler.ignoredItem === taskItem)

    property bool wheelIsBlocked: false

    property int animationTime: (taskItem.abilities.animations.active ? taskItem.abilities.animations.speedFactor.current : 2) * (1.2 *taskItem.abilities.animations.duration.small)
    property int badgeIndicator: 0 //it is used from external apps
    property int itemIndex: index
    property int lastValidIndex: -1 //used for the removal animation
    property int lastButtonClicked: -1;
    property int pressX: -1
    property int pressY: -1
    property int resistanceDelay: 450
    property int spacersMaxSize: Math.max(0,Math.ceil(0.55*taskItem.abilities.metrics.iconSize) - taskItem.abilities.metrics.totals.lengthEdges)
    property int windowsCount: subWindows.windowsCount
    property int windowsMinimizedCount: subWindows.windowsMinimized

    //! are set by the indicator
    property int iconOffsetX: 0
    property int iconOffsetY: 0

    property string activity: tasksModel.activity

    readonly property var m: model
    readonly property int pid: model && model.AppPid ? model.AppPid : -1
    readonly property string appName: model && model.AppName ? model.AppName : ""

    property string modelLauncherUrl: (LauncherUrlWithoutIcon && LauncherUrlWithoutIcon !== null) ? LauncherUrlWithoutIcon : ""
    property string modelLauncherUrlWithIcon: (LauncherUrl && LauncherUrl !== null) ? LauncherUrl : ""
    property string launcherUrl: ""
    property string launcherUrlWithIcon: ""
    property string launcherName: ""

    readonly property alias hoveredTimer: taskMouseArea.hoveredTimer
    readonly property alias mouseArea: taskMouseArea
    readonly property alias tooltipVisualParent: _wrapper.titleTooltipVisualParent
    readonly property alias previewsVisualParent: _wrapper.previewsTooltipVisualParent
    readonly property alias subWindows: subWindows
    readonly property alias wrapper: _wrapper


    readonly property alias showWindowAnimation: _showWindowAnimation
    readonly property alias restoreAnimation: _restoreAnimation

    //abilities
    property Item abilities: null

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

        if (taskItem.abilities.launchers.isSeparator(modelLauncherUrl)){
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

    onHoveredFromDraggingChanged: {
        if (hoveredFromDragging) {
            scrollableList.autoScrollFor(taskItem, true);
        }
    }

    //! separators flags
    readonly property bool tailItemIsSeparator: {
        if (isSeparator || itemIndex < 0 ) {
            return false;
        }

        var tail = index - 1;

        while(tail>=0 && taskItem.abilities.indexer.hidden.indexOf(tail)>=0) {
            tail = tail - 1;
        }

        var hasTailItemSeparator = taskItem.abilities.indexer.separators.indexOf(tail)>=0;

        if (!hasTailItemSeparator && itemIndex === taskItem.abilities.indexer.firstVisibleItemIndex){
            return taskItem.abilities.indexer.tailAppletIsSeparator;
        }

        return hasTailItemSeparator;
    }

    readonly property bool headItemIsSeparator: {
        if (isSeparator || itemIndex < 0 ) {
            return false;
        }

        var head = index + 1;

        while(head>=0 && taskItem.abilities.indexer.hidden.indexOf(head)>=0) {
            head = head + 1;
        }

        var hasHeadItemSeparator = taskItem.abilities.indexer.separators.indexOf(head)>=0;

        if (!hasHeadItemSeparator && itemIndex === taskItem.abilities.indexer.lastVisibleItemIndex){
            return taskItem.abilities.indexer.headAppletIsSeparator;
        }

        return hasHeadItemSeparator;
    }

    ////// Audio streams //////
    property Item audioStreamOverlay
    property var audioStreams: []
    readonly property bool hasAudioStream: root.showAudioBadge && audioStreams.length > 0 && !isLauncher
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

    signal groupWindowAdded();
    signal groupWindowRemoved();
    signal checkWindowsStates();

    Behavior on opacity {
        // NumberAnimation { duration: (IsStartup || (IsLauncher) ) ? 0 : 400 }
        NumberAnimation { duration: taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large }
    }

    Loader{
        anchors.fill: parent
        active: taskItem.abilities.debug.graphicsEnabled

        sourceComponent: Rectangle{
            anchors.fill: parent
            color: "transparent"
            border.color:  "blue"
            border.width: 1
        }
    }


    SubWindows{
        id: subWindows

        property int previousCount: 0

        onWindowsCountChanged: {
            if (root.disableAllWindowsFunctionality) {
                return;
            }

            if ((windowsCount >= 2)
                    && (windowsCount > previousCount)
                    && !(taskItem.containsMouse)
                    && !root.dragSource ){
                taskItem.groupWindowAdded();
            } else if ((windowsCount >= 1)
                       && (windowsCount < previousCount)
                       && !root.dragSource
                       && !taskItem.delayingRemove){
                //sometimes this is triggered in dragging with no reason
                taskItem.groupWindowRemoved();
            }

            if (windowsCount>=1) {
                taskItem.slotPublishGeometries();
            }

            //! workaround in order to update correctly the previousCount
            //! windowsCount can not return to zero because is such case
            //! the window task is removed and the launcher is added from
            //! libtaskmanager
            if (windowsCount>=1) {
                previousCount = windowsCount;
            }
        }
    }

    Loader {
        id: isSeparatorRectangle
        active: (opacityN>0)

        width: taskItem.width
        height: taskItem.height
        anchors.centerIn: separatorItem

        property real opacityN: isSeparator && root.contextMenu && root.contextMenu.visualParent === taskItem ? 1 : 0

        Behavior on opacityN {
            NumberAnimation { duration: taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large }
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

        anchors.bottom: (root.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
        anchors.top: (root.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
        anchors.left: (root.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
        anchors.right: (root.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

        anchors.bottomMargin: (root.location === PlasmaCore.Types.BottomEdge) ? margin : 0
        anchors.topMargin: (root.location === PlasmaCore.Types.TopEdge) ? margin : 0
        anchors.leftMargin: (root.location === PlasmaCore.Types.LeftEdge) ? margin : 0
        anchors.rightMargin: (root.location === PlasmaCore.Types.RightEdge) ? margin : 0

        opacity: (separatorShadow.active) || forceHiddenState ? 0 : 0.4
        visible: taskItem.isSeparator

        width: root.vertical ? taskItem.abilities.metrics.iconSize : ((root.dragSource || root.inEditMode) ? LatteCore.Environment.separatorLength+taskItem.abilities.metrics.totals.lengthEdges: 1)
        height: !root.vertical ? taskItem.abilities.metrics.iconSize : ((root.dragSource || root.inEditMode) ? LatteCore.Environment.separatorLength+taskItem.abilities.metrics.totals.lengthEdges: 1)

        property bool forceHiddenState: false

        readonly property int margin: taskItem.abilities.metrics.margin.screenEdge + taskItem.abilities.metrics.margin.thickness

        Behavior on opacity {
            NumberAnimation { duration: taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large }
        }

        Connections{
            target: root

            onDisableAllWindowsFunctionalityChanged: {
                if (!root.inEditMode) {
                    return;
                }

                taskItem.updateVisibilityBasedOnLaunchers();
            }

            onShowWindowsOnlyFromLaunchersChanged: {
                if (!root.inEditMode) {
                    return;
                }

                taskItem.updateVisibilityBasedOnLaunchers();
            }

            onInActivityChangeChanged: {
                if ((root.showWindowsOnlyFromLaunchers || root.disableAllWindowsFunctionality) && !root.inActivityChange) {
                    taskItem.updateVisibilityBasedOnLaunchers();
                }
            }
        }

        Rectangle {
            anchors.centerIn: parent

            width: root.vertical ? taskItem.abilities.metrics.iconSize - 4  : 1
            height: !root.vertical ? taskItem.abilities.metrics.iconSize - 4 : 1
            color: enforceLattePalette ? latteBridge.palette.textColor : theme.textColor
        }


    }

    ///Shadow in tasks
    Loader{
        id: separatorShadow
        anchors.fill: separatorItem
        active: root.enableShadows && isSeparator && graphicsSystem.isAccelerated
        opacity: separatorItem.forceHiddenState ? 0 : 0.4

        Behavior on opacity {
            NumberAnimation { duration: taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large }
        }

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: root.appShadowColor
            fast: true
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

    TaskMouseArea {
        id: taskMouseArea
    }

    Flow{
        id: taskFlow
        width: parent.width
        height: parent.height

        // a hidden spacer for the first element to add stability
        // IMPORTANT: hidden spacers must be tested on vertical !!!
        HiddenSpacer{ id:hiddenSpacerLeft;}

        Item{
            width: wrapper.width
            height: wrapper.height

            Indicator.Bridge{
                id: indicatorBridge
            }

            Indicator.Loader{
                id: indicatorBackLayer
                level: Indicator.LevelOptions {
                    id: backLevelOptions
                    isBackground: true
                    bridge: indicatorBridge

                    Binding {
                        target: taskItem
                        property: "iconOffsetX"
                        value: backLevelOptions.requested.iconOffsetX
                    }

                    Binding {
                        target: taskItem
                        property: "iconOffsetY"
                        value: backLevelOptions.requested.iconOffsetY
                    }
                }

                Loader{
                    anchors.fill: parent
                    active: taskItem.abilities.debug.graphicsEnabled
                    sourceComponent: Rectangle{
                        color: "transparent"
                        border.width: 1
                        border.color: "purple"
                        opacity: 0.4
                    }
                }
            }

            Wrapper{id: _wrapper}

            Indicator.Loader{
                id: indicatorFrontLayer
                level: Indicator.LevelOptions {
                    isForeground: true
                    bridge: indicatorBridge
                }
            }
        }

        // a hidden spacer on the right for the last item to add stability
        HiddenSpacer{ id:hiddenSpacerRight; rightSpacer: true }
    }// Flow with hidden spacers inside

    Loader {
        id: parabolicAreaLoader
        active: taskItem.abilities.parabolic.isEnabled
        width: root.isHorizontal ? taskItem.width : taskItem.abilities.metrics.mask.thickness.zoomedForItems
        height: root.isHorizontal ? taskItem.abilities.metrics.mask.thickness.zoomedForItems : taskItem.height
        z:10000
        sourceComponent: ParabolicArea{}

        states:[
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: parabolicAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: parabolicAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: parabolicAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: parabolicAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
            }
        ]
    }

    Timer {
        id: publishGeometryTimer
        interval: 800
        repeat: false

        onTriggered: {
            slotPublishGeometries();

            if (taskItem.abilities.debug.timersEnabled) {
                console.log("plasmoid timer: publishGeometryTimer called...");
            }
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

    onCanPublishGeometriesChanged: {
        if (canPublishGeometries) {
            slotPublishGeometries();
            publishGeometryTimer.start();
        }
    }

    onItemIndexChanged: {
        if (itemIndex>=0) {
            lastValidTimer.start();
        }
    }

    onIsDraggedChanged: {
        if (isDragged){
            root.dragSource = taskItem;
            dragHelper.startDrag(taskItem, model.MimeType, model.MimeData,
                                 model.LauncherUrlWithoutIcon, model.decoration);
            pressX = -1;
            pressY = -1;
        }
    }

    onIsMinimizedChanged: {
        checkWindowsStates();
    }

    onIsActiveChanged: {
        checkWindowsStates();
        if (isActive) {
            scrollableList.focusOn(taskItem);
        }
    }

    onIsSeparatorChanged: {
        if (isSeparator) {
            if (tasksExtendedManager.isLauncherToBeMoved(launcherUrl) && itemIndex>=0) {
                tasksExtendedManager.moveLauncherToCorrectPos(launcherUrl, itemIndex);
            }
        }
    }

    onLauncherUrlChanged: updateBadge();

    ////// End of Values Changes /////


    //! A timer is needed in order to handle also touchpads that probably
    //! send too many signals very fast. This way the signals per sec are limited.
    //! The user needs to have a steady normal scroll in order to not
    //! notice a annoying delay
    Timer{
        id: scrollDelayer

        interval: 400

        onTriggered: taskItem.wheelIsBlocked = false;
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
    }

    function sltClearZoom(){
        restoreAnimation.start();
    }

    function handlerDraggingFinished(){
        isDragged = false;
    }
    ///// End of Handlers //////



    ///// Helper functions /////
    function activateNextTask() {
        subWindows.activateNextTask();
    }

    function activateTask() {
        if( taskItem.isLauncher || root.disableAllWindowsFunctionality){
            if (LatteCore.WindowSystem.compositingActive) {
                wrapper.runLauncherAnimation();
            } else {
                launcherAction();
            }
        } else{
            if (model.IsGroupParent) {
                if (LatteCore.WindowSystem.compositingActive && backend.canPresentWindows()) {
                    root.presentWindows(root.plasma515 ? model.WinIdList: model.LegacyWinIdList );
                }
            } else {
                if (windowsPreviewDlg.visible) {
                    forceHidePreview(8.3);
                }

                if (isMinimized) {
                    var i = modelIndex();
                    tasksModel.requestToggleMinimized(i);
                    tasksModel.requestActivate(i);
                } else if (isActive) {
                    tasksModel.requestToggleMinimized(modelIndex());
                } else {
                    tasksModel.requestActivate(modelIndex());
                }
            }
        }
    }

    function forceHidePreview(debugtext) {
        showPreviewsIsBlockedFromReleaseEvent = true;
        hoveredTimer.stop();

        root.forcePreviewsHiding(debugtext);
    }

    function showPreviewWindow() {
        if (root.disableAllWindowsFunctionality || !isAbleToShowPreview) {
            return;
        }

        if(windowsPreviewDlg.activeItem !== taskItem){
            if (!taskItem.abilities.myView.isReady
                    || (taskItem.abilities.myView.isReady && taskItem.abilities.myView.isShownFully)) {
                if (root.latteView && root.titleTooltips) {
                    root.latteView.hideTooltipLabel();
                }

                taskItem.preparePreviewWindow(false);
                windowsPreviewDlg.show(taskItem);
            }
        }
    }

    function showTitleTooltip() {
        if (root.latteView && root.titleTooltips){
            var displayText = isWindow ? model.display : model.AppName;
            var maxCharacters = 80;

            var fixedDisplayText = displayText.length>maxCharacters ? displayText.substring(0,maxCharacters-1) + "..." : displayText;

            root.latteView.showTooltipLabel(taskItem, fixedDisplayText);
        }
    }

    function hidePreviewWindow() {
        if(windowsPreviewDlg.activeItem === taskItem){
            windowsPreviewDlg.hide("14.1");

            if (root.latteView && root.titleTooltips && containsMouse) {
                showTitleTooltip();
            }
        }
    }

    function preparePreviewWindow(hideClose){
        windowsPreviewDlg.visualParent = previewsVisualParent;
        toolTipDelegate.parentTask = taskItem;
        toolTipDelegate.rootIndex = tasksModel.makeModelIndex(itemIndex, -1);

        toolTipDelegate.hideCloseButtons = hideClose;

        toolTipDelegate.appName = Qt.binding(function() {
            return model.AppName;
        });

        if (!isLauncher) {
            toolTipDelegate.pidParent = Qt.binding(function() {
                return model.AppPid;
            });
        } else {
            toolTipDelegate.pidParent = -1;
        }

        toolTipDelegate.windows = Qt.binding(function() {
            return root.plasma515 ? model.WinIdList : model.LegacyWinIdList ;
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
            return (model.VirtualDesktops !== undefined && model.VirtualDesktops.length > 0) ? model.VirtualDesktops : [0];
        });
        toolTipDelegate.isOnAllVirtualDesktopsParent = Qt.binding(function() {
            return model.IsOnAllVirtualDesktops == true;
        });
        toolTipDelegate.activitiesParent = Qt.binding(function() {
            return model.Activities;
        });
    }


    function launcherAction(){
        if (LatteCore.WindowSystem.compositingActive) {
            inBouncingAnimation = true;
            tasksExtendedManager.addWaitingLauncher(taskItem.launcherUrl);
        }

        if (root.disableAllWindowsFunctionality) {
            tasksModel.requestNewInstance(modelIndex());
        } else {
            tasksModel.requestActivate(modelIndex());
        }
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

    function showContextMenu(args) {
        if (isSeparator && !root.inEditMode)
            return;

        if (!root.contextMenu) {
            contextMenu = root.createContextMenu(taskItem, modelIndex(), args);
            contextMenu.show();
        } else {
            //! make sure that context menu isnt deleted multiple times and creates a crash
            //! bug case: 397635
            var cMenu = root.contextMenu;
            root.contextMenu = null;
            cMenu.destroy();
        }
    }

    function modifierAccepted(mouse){
        if (mouse.modifiers & root.modifierQt){
            if ((mouse.button === Qt.LeftButton && root.modifierClick === LatteTasks.Types.LeftClick)
                    || (mouse.button === Qt.MiddleButton && root.modifierClick === LatteTasks.Types.MiddleClick)
                    || (mouse.button === Qt.RightButton && root.modifierClick === LatteTasks.Types.RightClick))
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
                mimicParabolicScale = taskItem.abilities.parabolic.factor.zoom;
            }

            if (parabolicAreaLoader.active) {
                parabolicAreaLoader.item.calculateParabolicScales(icList.currentSpot);
            }
        }
    }

    function slotShowPreviewForTasks(group) {
        if (group === taskItem && !windowsPreviewDlg.visible) {
            preparePreviewWindow(true);
            windowsPreviewDlg.show(taskItem);
        }
    }

    function slotPublishGeometries() {
        //! this way we make sure that layouts that are in different activities that the current layout
        //! don't publish their geometries
        if ( canPublishGeometries && (!latteView || (latteView && root.viewLayout && root.viewLayout.isCurrent()))) {
            var globalChoords = backend.globalRect(wrapper.visualIconItem);
            var limits = backend.globalRect(scrollableList);

            //! Limit the published geometries boundaries at scrolling area boundaries
            var adjX = Math.min(limits.x+limits.width, Math.max(limits.x, globalChoords.x));
            var adjY = Math.min(limits.y+limits.height, Math.max(limits.y, globalChoords.y));

            var length = taskItem.abilities.metrics.iconSize * wrapper.mScale;
            var thickness = length;

            //! Magic Lamp effect doesn't like coordinates outside the screen and
            //! width,heights of zero value... So we now normalize the geometries
            //! sent in order to avoid such circumstances
            if (root.vertical) {
                if (adjY !== globalChoords.y) {
                    if (((globalChoords.y+globalChoords.height) < limits.y) || (globalChoords.y)>(limits.y+limits.height)) {
                        //! totally out of boundaries
                        length = 4;
                    } else {
                        //! semi-part out of boundaries
                        length = Math.max(4, Math.abs(adjY - globalChoords.y));
                    }

                    globalChoords.height = length;
                }
            } else {
                if (adjX !== globalChoords.x) {
                    if (((globalChoords.x+globalChoords.width) < limits.x) || (globalChoords.x)>(limits.x+limits.width)) {
                        //! totally out of boundaries
                        length = 4;
                    } else {
                        //! semi-part out of boundaries
                        length = Math.max(4, Math.abs(adjX - globalChoords.x));
                    }

                    globalChoords.width = length;
                }
            }

            globalChoords.x = adjX;
            globalChoords.y = adjY;

            if (latteView && latteView.dockIsHidden) {
                if (root.location === PlasmaCore.Types.BottomEdge) {
                    globalChoords.y = root.screenGeometry.y+root.screenGeometry.height-1;
                    globalChoords.height = 1;
                } else if (root.location === PlasmaCore.Types.TopEdge) {
                    globalChoords.y = root.screenGeometry.y+1;
                    globalChoords.height = 1;
                } else if (root.location === PlasmaCore.Types.LeftEdge) {
                    globalChoords.x = root.screenGeometry.x+1;
                    globalChoords.width = 1;
                } else if (root.location === PlasmaCore.Types.RightEdge) {
                    globalChoords.x = root.screenGeometry.x+root.screenGeometry.width - 1;
                    globalChoords.width = 1;
                }
            }

            tasksModel.requestPublishDelegateGeometry(taskItem.modelIndex(), globalChoords, taskItem);
        }
    }

    function slotWaitingLauncherRemoved(launch) {
        if ((isWindow || isStartup || isLauncher) && !visible && launch === launcherUrl) {
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

        var streams = pa.streamsForPid(taskItem.pid);
        if (streams.length) {
            pa.registerPidMatch(taskItem.appName);
        } else {
            // We only want to fall back to appName matching if we never managed to map
            // a PID to an audio stream window. Otherwise if you have two instances of
            // an application, one playing and the other not, it will look up appName
            // for the non-playing instance and erroneously show an indicator on both.
            if (!pa.hasPidMatch(taskItem.appName)) {
                var streams_result;
                streams_result = pa.streamsForAppName(taskItem.appName);
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
            taskItem.audioStreams = streams;
        }
    }

    function onLauncherChanged(launcher) {
        if ((root.showWindowsOnlyFromLaunchers || root.disableAllWindowsFunctionality) && launcher === launcherUrl) {
            updateVisibilityBasedOnLaunchers()
        }
    }

    function updateVisibilityBasedOnLaunchers(){
        var launcherExists = !(((tasksModel.launcherPosition(taskItem.launcherUrl) == -1)
                                && (tasksModel.launcherPosition(taskItem.launcherUrlWithIcon) == -1) )
                               || !taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl));

        if (root.showWindowsOnlyFromLaunchers || root.disableAllWindowsFunctionality) {
            var hideWindow =  !launcherExists && (taskItem.isWindow || root.disableAllWindowsFunctionality);

            if (hideWindow) {
                isForcedHidden = true;
                taskRealRemovalAnimation.start();
            } else if (launcherExists && taskItem.isWindow && !taskItem.isVisible) {
                showWindowAnimation.showWindow();
                isForcedHidden = false;
            }
        } else {
            var showWindow =  !launcherExists && taskItem.isWindow;

            if (showWindow) {
                showWindowAnimation.showWindow();
                isForcedHidden = false;
            }
        }
    }

    function toggleMuted() {
        if (muted) {
            taskItem.audioStreams.forEach(function (item) { item.unmute(); });
        } else {
            taskItem.audioStreams.forEach(function (item) { item.mute(); });
        }
    }

    function increaseVolume() {
        taskItem.audioStreams.forEach(function (item) { item.increaseVolume(); });
    }

    function decreaseVolume() {
        taskItem.audioStreams.forEach(function (item) { item.decreaseVolume(); });
    }

    function updateBadge() {
        var badger = root.getBadger(launcherUrl);

        if (badger) {
            badgeIndicator = parseInt(badger.value);
        } else {
            badgeIndicator = 0;
        }
    }

    Connections {
        target: pulseAudio.item
        ignoreUnknownSignals: true // Plasma-PA might not be available
        onStreamsChanged: taskItem.updateAudioStreams()
    }

    Connections {
        target: root
        //trying to fix #440, showing the audio icon indicator to irrelevant tasks
        //after dragging an existent task with audio
        onDragSourceChanged: taskItem.updateAudioStreams()
        onShowAudioBadgeChanged: taskItem.updateAudioStreams()
    }

    Connections {
        target: scrollableList
        onAnimationsFinishedChanged: {
            if (scrollableList.animationsFinished) {
                taskItem.slotPublishGeometries();
            }
        }
    }

    Connections {
        target: taskItem.abilities.shortcuts
        onSglActivateEntryAtIndex: {
            if (!taskItem.abilities.shortcuts.isEnabled) {
                return;
            }

            var shortcutIndex = taskItem.abilities.shortcuts.shortcutIndex(taskItem.itemIndex);

            if (shortcutIndex === entryIndex) {
                if (taskItem.isGroupParent) {
                    taskItem.activateNextTask();
                } else {
                    taskItem.activateTask();
                }
            }
        }

        onSglNewInstanceForEntryAtIndex: {
            if (!taskItem.abilities.shortcuts.isEnabled) {
                return;
            }

            var shortcutIndex = taskItem.abilities.shortcuts.shortcutIndex(taskItem.itemIndex);

            if (shortcutIndex === entryIndex) {
                tasksModel.requestNewInstance(taskItem.modelIndex());
            }
        }
    }

    Connections {
        target: taskItem.abilities.myView
        onIsShownFullyChanged: {
            if (taskItem.abilities.myView.isShownFully) {
                taskItem.slotPublishGeometries();
            }
        }
    }

    Connections {
        target: latteView
        onDockIsHiddenChanged: {
            if (dockIsHidden) {
                taskItem.slotPublishGeometries();
            }
        }
    }


    ///// End of Helper functions ////

    Component.onCompleted: {
        root.draggingFinished.connect(handlerDraggingFinished);
        root.publishTasksGeometries.connect(slotPublishGeometries);
        root.showPreviewForTasks.connect(slotShowPreviewForTasks);
        root.mimicEnterForParabolic.connect(slotMimicEnterForParabolic);

        taskItem.abilities.launchers.launcherChanged.connect(onLauncherChanged);
        taskItem.abilities.launchers.launcherRemoved.connect(onLauncherChanged);
        taskItem.abilities.parabolic.sglClearZoom.connect(sltClearZoom);

        //startup without launcher
        var hideStartup =  ((!hasShownLauncher || !taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl))
                            && taskItem.isStartup);

        if (!LatteCore.WindowSystem.compositingActive) {
            visible = true;
        } else if ( (isWindow || isStartup || isLauncher) && tasksExtendedManager.waitingLauncherExists(launcherUrl)) {
            tasksExtendedManager.waitingLauncherRemoved.connect(slotWaitingLauncherRemoved);
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
        root.draggingFinished.disconnect(handlerDraggingFinished);
        root.publishTasksGeometries.disconnect(slotPublishGeometries);
        root.showPreviewForTasks.disconnect(slotShowPreviewForTasks);
        root.mimicEnterForParabolic.disconnect(slotMimicEnterForParabolic);

        taskItem.abilities.launchers.launcherChanged.disconnect(onLauncherChanged);
        taskItem.abilities.launchers.launcherRemoved.disconnect(onLauncherChanged);
        taskItem.abilities.parabolic.sglClearZoom.disconnect(sltClearZoom);

        tasksExtendedManager.waitingLauncherRemoved.disconnect(slotWaitingLauncherRemoved);

        wrapper.sendEndOfNeedBothAxisAnimation();
    }

    /////Animations

    TaskAnimations.ShowWindowAnimation{ id: _showWindowAnimation }
    TaskAnimations.RestoreAnimation{ id: _restoreAnimation }

    // when changing activities and desktops the index of the tasks
    // is updated immediately to -1, this timer protects this indexing
    // change in order to provide a beautiful removal tasks animation
    Timer {
        id: lastValidTimer
        interval: 100 ///the interval does not follow the animations timing
        repeat: false

        onTriggered: {
            if (taskItem.itemIndex >= 0){
                taskItem.lastValidIndex = taskItem.itemIndex;
            }

            if (taskItem.abilities.debug.timersEnabled) {
                console.log("plasmoid timer: lastValidTimer called...");
            }
        }
    }

    ///Item's Removal Animation
    ListView.onRemove: TaskAnimations.RealRemovalAnimation{ id: taskRealRemovalAnimation }

}// main Item

