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

import org.kde.taskmanager 0.1 as TaskManager
import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import org.kde.activities 0.1 as Activities

import org.kde.latte 0.1 as Latte

import "../code/tools.js" as TaskTools
import "../code/activitiesTools.js" as ActivitiesTools

Item {
    id:root

    Layout.fillHeight: userPanelPosition === 0 ? true : false
    Layout.fillWidth: userPanelPosition === 0 ? true : false

    ///IMPORTANT: These values must be tested when the Now Dock Panel support
    ///also the four new anchors. A small issue is shown between the animation
    /// of the now dock plasmoid and the neighbour widgets...
    Layout.minimumWidth: (userPanelPosition !== 0)&&(!nowDockPanel) ? clearWidth : -1
    Layout.minimumHeight: (userPanelPosition !== 0)&&(!nowDockPanel) ? clearHeight : -1
    Layout.preferredWidth: (userPanelPosition !== 0)&&(!nowDockPanel) ? tasksWidth : -1
    Layout.preferredHeight: (userPanelPosition !== 0)&&(!nowDockPanel) ? tasksHeight : -1

    property bool debugLocation: false

    property bool editMode: plasmoid.userConfiguring
    property bool disableRestoreZoom: false //blocks restore animation in rightClick
    property bool dropNewLauncher: false
    property bool initializationStep: true
    property bool initializatedBuffers: noInitCreatedBuffers >= tasksStarting ? true : false
    property bool isHovered: false
    property bool showBarLine: plasmoid.configuration.showBarLine
    property bool useThemePanel: plasmoid.configuration.useThemePanel
    property bool taskInAnimation: noTasksInAnimation > 0 ? true : false
    property bool transparentPanel: plasmoid.configuration.transparentPanel
    property bool vertical: ((root.position === PlasmaCore.Types.LeftPositioned) ||
                             (root.position === PlasmaCore.Types.RightPositioned)) ? true : false

    property int clearWidth
    property int clearHeight

    //property int iconMargin: 5
    property int iconMargin: 0.12*iconSize

    property int newLocationDebugUse: PlasmaCore.Types.BottomPositioned
    property int newDroppedPosition: -1
    property int noInitCreatedBuffers: 0
    property int noTasksInAnimation: 0
    property int themePanelSize: plasmoid.configuration.panelSize

    property int position : PlasmaCore.Types.BottomPositioned
    property int tasksStarting: 0
    property int realSize: iconSize + iconMargin
    property int statesLineSize: Math.ceil( root.iconSize/13 )

    property real textColorLuma: 0.2126*theme.textColor.r + 0.7152*theme.textColor.g + 0.0722*theme.textColor.b

    property variant launchersOnActivities: []

    property QtObject contextMenuComponent: Qt.createComponent("ContextMenu.qml");
    property Item dragSource: null

    property color minimizedDotColor: textColorLuma > 0.5 ? Qt.darker(theme.textColor, 1+ (1-textColorLuma)) : Qt.lighter(theme.textColor, 1+(1-textColorLuma))

    //BEGIN Now Dock Panel properties
    property bool enableShadows: nowDockPanel ? nowDockPanel.enableShadows > 0 : plasmoid.configuration.showShadows
    property bool forceHidePanel: false
    property bool disableLeftSpacer: false
    property bool disableRightSpacer: false
    property bool dockIsHidden: nowDockPanel ? nowDockPanel.dockIsHidden : false
    property bool highlightWindows: nowDockPanel ? nowDockPanel.highlightWindows: plasmoid.configuration.highlightWindows
    property bool reverseLinesPosition: nowDockPanel ? nowDockPanel.reverseLinesPosition : plasmoid.configuration.reverseLinesPosition
    property bool dotsOnActive: nowDockPanel ? nowDockPanel.dotsOnActive : plasmoid.configuration.dotsOnActive
    property bool showGlow: nowDockPanel ? nowDockPanel.showGlow : plasmoid.configuration.showGlow
    property bool showOnlyCurrentScreen: nowDockPanel ? nowDockPanel.showOnlyCurrentScreen : plasmoid.configuration.showOnlyCurrentScreen
    property bool showOnlyCurrentDesktop: nowDockPanel ? nowDockPanel.showOnlyCurrentDesktop : plasmoid.configuration.showOnlyCurrentDesktop
    property bool showOnlyCurrentActivity: nowDockPanel ? nowDockPanel.showOnlyCurrentActivity : plasmoid.configuration.showOnlyCurrentActivity
    property bool showPreviews:  nowDockPanel ? nowDockPanel.showToolTips : plasmoid.configuration.showToolTips
    property bool showWindowActions: nowDockPanel ? nowDockPanel.showWindowActions : plasmoid.configuration.showWindowActions
    property bool smartLaunchersEnabled: nowDockPanel ? nowDockPanel.smartLaunchersEnabled : plasmoid.configuration.smartLaunchersEnabled
    property bool threeColorsWindows: nowDockPanel ? nowDockPanel.threeColorsWindows : plasmoid.configuration.threeColorsWindows

    property int durationTime: nowDockPanel ? nowDockPanel.durationTime : plasmoid.configuration.durationTime
    property int iconSize: nowDockPanel ? nowDockPanel.iconSize : Math.max(plasmoid.configuration.iconSize, 16)
    property int tasksHeight: mouseHandler.height
    property int tasksWidth: mouseHandler.width
    property int userPanelPosition: nowDockPanel ? nowDockPanel.panelAlignment : plasmoid.configuration.plasmoidPosition

    property real zoomFactor: nowDockPanel ? nowDockPanel.zoomFactor : ( 1 + (plasmoid.configuration.zoomLevel / 20) )

    property alias tasksCount: tasksModel.count
    property alias hoveredIndex: icList.hoveredIndex

    property Item nowDockPanel: null
    //END Now Dock Panel properties


    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    signal clearZoomSignal();
    signal draggingFinished();
    signal mouseWasEntered(int delegateIndex, bool value);
    signal presentWindows(variant winIds);
    signal requestLayout;
    signal signalAnimationsNeedBothAxis(int value);
    signal signalAnimationsNeedLength(int value);
    signal signalAnimationsNeedThickness(int value);
    signal signalDraggingState(bool value);
    //trigger updating scaling of neighbour delegates of zoomed delegate
    signal updateScale(int delegateIndex, real newScale, real step)
    signal publishTasksGeometries();
    signal windowsHovered(variant winIds, bool hovered)

    //onAnimationsChanged: console.log(animations);
    /* Rectangle{
                anchors.fill: parent
                border.width: 1
                border.color: "red"
                color: "white"
            } */
    onNowDockPanelChanged: {
        if (nowDockPanel)
            plasmoid.configuration.isInNowDockPanel = true;
        else
            plasmoid.configuration.isInNowDockPanel = false;
    }


    Connections {
        target: plasmoid
        onLocationChanged: {
            root.updatePosition();
            iconGeometryTimer.start();
        }
    }

    Connections {
        target: plasmoid.configuration

        // onLaunchersChanged: tasksModel.launcherList = plasmoid.configuration.launchers
        onGroupingAppIdBlacklistChanged: tasksModel.groupingAppIdBlacklist = plasmoid.configuration.groupingAppIdBlacklist;
        onGroupingLauncherUrlBlacklistChanged: tasksModel.groupingLauncherUrlBlacklist = plasmoid.configuration.groupingLauncherUrlBlacklist;
    }

    /////
    PlasmaCore.ColorScope{
        id: colorScopePalette
    }

    /////

    onDragSourceChanged: {
        if (dragSource == null) {
            root.draggingFinished();
            root.signalDraggingState(false);

            tasksModel.syncLaunchers();
        } else {
            root.signalDraggingState(true);
        }
    }

    /////Winwow previews///////////

    ToolTipDelegate {
        id: toolTipDelegate
        visible: false

        property int currentItem: -1
    }

    // FIXME: at some point this must be dropped with NowDock plugin
    ////BEGIN interfaces
    Latte.WindowSystem{
        id:windowSystem
    }

    PlasmaCore.Dialog{
        id: windowsPreviewDlg
        // hideOnWindowDeactivate: false

        type: PlasmaCore.Dialog.Tooltip
        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus

        location: plasmoid.location

        visible: false

        mainItem: toolTipDelegate

        property Item activeItem: null

        function hide(){
            if (nowDockPanel) {
                //it is used to unblock dock hiding
                root.signalDraggingState(false);
            }
            visible = false;
            //activeItem = null;
            initializePreviewComponent.createObject(windowsPreviewDlg);
        }

        function show(){
            var tasks = icList.contentItem.children;

            for(var i=0; i<tasks.length; ++i){
                var task = tasks[i];

                if(task && task.isActive){
                    activeItem = task;
                    break;
                }
            }

            if (nowDockPanel) {
                //it is used to block dock hiding
                root.signalDraggingState(true);
            }
            visible = true;
        }
    }

    //A Timer to delay the initialization of the active item in order
    //to not break then active item animation
    Component {
        id: initializePreviewComponent
        Timer {
            id: initializePreviewTimer
            interval: 300
            repeat: false

            onTriggered: {
                windowsPreviewDlg.activeItem = null;
                initializePreviewTimer.destroy();
            }

            Component.onCompleted: initializePreviewTimer.start()
        }
    }



    /////Window Previews/////////

    TaskManager.TasksModel {
        id: tasksModel

        virtualDesktop: virtualDesktopInfo.currentDesktop
        screenGeometry: plasmoid.screenGeometry
        // comment in order to support LTS Plasma 5.8
        // screen: plasmoid.screen
        activity: activityInfo.currentActivity

        filterByVirtualDesktop: root.showOnlyCurrentDesktop
        filterByScreen: root.showOnlyCurrentScreen
        filterByActivity: root.showOnlyCurrentActivity

        launchInPlace: true
        separateLaunchers: false
        groupInline: false

        groupMode: TaskManager.TasksModel.GroupApplications
        sortMode: TaskManager.TasksModel.SortManual

        onActivityChanged: {
            ActivitiesTools.currentActivity = activity;
            //  console.log("Updated :"+activity);

            launcherList = ActivitiesTools.restoreLaunchers();
            //root.updateImplicits();
            //panelGeometryTimer.start();
        }

        //    onCountChanged: {
        //   updateImplicits();
        //  }

        onLauncherListChanged: {
            // plasmoid.configuration.launchers = launcherList;
            ActivitiesTools.updateLaunchers(launcherList);
        }

        onGroupingAppIdBlacklistChanged: {
            plasmoid.configuration.groupingAppIdBlacklist = groupingAppIdBlacklist;
        }

        onGroupingLauncherUrlBlacklistChanged: {
            plasmoid.configuration.groupingLauncherUrlBlacklist = groupingLauncherUrlBlacklist;
        }

        onAnyTaskDemandsAttentionChanged: {
            if (anyTaskDemandsAttention){
                plasmoid.status = PlasmaCore.Types.RequiresAttentionStatus;
                attentionTimerComponent.createObject(root);
            }
        }


        Component.onCompleted: {
            ActivitiesTools.launchersOnActivities = root.launchersOnActivities
            ActivitiesTools.currentActivity = activityInfo.currentActivity;
            ActivitiesTools.plasmoid = plasmoid;

            launcherList = ActivitiesTools.restoreLaunchers();
            groupingAppIdBlacklist = plasmoid.configuration.groupingAppIdBlacklist;
            groupingLauncherUrlBlacklist = plasmoid.configuration.groupingLauncherUrlBlacklist;

            icList.model = tasksModel;
            tasksStarting = count;
        }
    }

    TaskManagerApplet.Backend {
        id: backend

        taskManagerItem: root
        toolTipItem: toolTipDelegate
        highlightWindows:root.highlightWindows

        onAddLauncher: {
            tasksModel.requestAddLauncher(url);
        }
    }

    TaskManagerApplet.DragHelper {
        id: dragHelper

        dragIconSize: units.iconSizes.medium
    }

    TaskManager.VirtualDesktopInfo {
        id: virtualDesktopInfo
    }

    TaskManager.ActivityInfo {
        id: activityInfo
    }

    PlasmaCore.DataSource {
        id: mpris2Source
        engine: "mpris2"
        connectedSources: sources

        function sourceNameForLauncherUrl(launcherUrl) {
            if (!launcherUrl) {
                return "";
            }

            // MPRIS spec explicitly mentions that "DesktopEntry" is with .desktop extension trimmed
            // Moreover, remove URL parameters, like wmClass (part after the question mark)
            var desktopFileName = launcherUrl.toString().split('/').pop().split('?')[0].replace(".desktop", "")

            for (var i = 0, length = sources.length; i < length; ++i) {
                var source = sources[i];
                var sourceData = data[source];

                if (sourceData && sourceData.DesktopEntry === desktopFileName) {
                    return source
                }
            }

            return ""
        }

        function startOperation(source, op) {
            var service = serviceForSource(source)
            var operation = service.operationDescription(op)
            return service.startOperationCall(operation)
        }

        function goPrevious(source) {
            startOperation(source, "Previous");
        }
        function goNext(source) {
            startOperation(source, "Next");
        }
        function playPause(source) {
            startOperation(source, "PlayPause");
        }
        function stop(source) {
            startOperation(source, "Stop");
        }
        function raise(source) {
            startOperation(source, "Raise");
        }
        function quit(source) {
            startOperation(source, "Quit");
        }
    }


    /*  IconsModel{
        id: iconsmdl
    }*/

    Component{
        id: attentionTimerComponent
        Timer{
            id: attentionTimer
            interval:6500
            onTriggered: {
                plasmoid.status = PlasmaCore.Types.PassiveStatus;
                destroy();
            }
            Component.onCompleted: {
                start();
            }
        }
    }

    //Timer to check if the mouse is still inside the ListView
    Timer{
        id:checkListHovered
        repeat:false;
        interval: 120;

        onTriggered: {
            if (!root.containsMouse())
                root.clearZoom();

            interval = 120;
        }

        function startDuration( duration){
            interval = duration;

            start();
        }
    }


    ///Red Liner!!! show the upper needed limit for annimations
    Rectangle{
        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

        width: root.vertical ? 1 : 2 * root.iconSize
        height: root.vertical ? 2 * root.iconSize : 1
        color: "red"
        x: (root.position === PlasmaCore.Types.LeftPositioned) ? neededSpace : parent.width - neededSpace
        y: (root.position === PlasmaCore.Types.TopPositioned) ? neededSpace : parent.height - neededSpace

        visible: plasmoid.configuration.zoomHelper

        property int neededSpace: zoomFactor*(iconSize+iconMargin) + statesLineSize
    }

    Item{
        id:barLine

        opacity: (tasksModel.count > 0) && root.initializatedBuffers ? 1 : 0

        /*    anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
        anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
        anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
        anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

        anchors.horizontalCenter: !parent.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: parent.vertical ? parent.verticalCenter : undefined */

        width: ( icList.orientation === Qt.Horizontal ) ? icList.width + spacing : smallSize
        height: ( icList.orientation === Qt.Vertical ) ? icList.height + spacing : smallSize

        property int spacing: root.iconSize / 2
        property int smallSize: Math.max(3.7*root.statesLineSize, 16)

        Behavior on opacity{
            NumberAnimation { duration: root.durationTime*units.longDuration }
        }

        /// plasmoid's default panel
        BorderImage{
            anchors.fill:parent
            source: "../images/panel-west.png"
            border { left:8; right:8; top:8; bottom:8 }

            opacity: (plasmoid.configuration.showBarLine && !plasmoid.configuration.useThemePanel && !root.forceHidePanel) ? 1 : 0

            visible: (opacity == 0) ? false : true

            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch

            Behavior on opacity{
                NumberAnimation { duration: root.durationTime*units.longDuration }
            }
        }


        /// item which is used as anchors for the plasma's theme
        Item{
            id:belower

            width: (root.position === PlasmaCore.Types.LeftPositioned) ? shadowsSvgItem.margins.left : shadowsSvgItem.margins.right
            height: (root.position === PlasmaCore.Types.BottomPositioned)? shadowsSvgItem.margins.bottom : shadowsSvgItem.margins.top

            anchors.top: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
            anchors.bottom: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
            anchors.right: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
            anchors.left: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined
        }


        /// the current theme's panel
        PlasmaCore.FrameSvgItem{
            id: shadowsSvgItem

            anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? belower.bottom : undefined
            anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? belower.top : undefined
            anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? belower.left : undefined
            anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? belower.right : undefined

            anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
            anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

            width: root.vertical ? panelSize + margins.left + margins.right: parent.width
            height: root.vertical ? parent.height : panelSize + margins.top + margins.bottom

            imagePath: "translucent/widgets/panel-background"
            prefix:"shadow"

            opacity: (plasmoid.configuration.showBarLine && plasmoid.configuration.useThemePanel && !root.forceHidePanel) ? 1 : 0
            visible: (opacity == 0) ? false : true

            property int panelSize: ((root.position === PlasmaCore.Types.BottomPositioned) ||
                                     (root.position === PlasmaCore.Types.TopPositioned)) ?
                                        plasmoid.configuration.panelSize + belower.height:
                                        plasmoid.configuration.panelSize + belower.width

            Behavior on opacity{
                NumberAnimation { duration: root.durationTime*units.longDuration }
            }


            PlasmaCore.FrameSvgItem{
                anchors.margins: belower.width-1
                anchors.fill:parent
                imagePath: plasmoid.configuration.transparentPanel ? "translucent/widgets/panel-background" :
                                                                     "widgets/panel-background"
            }
        }


        MouseHandler {
            id: mouseHandler
            anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? icList.bottom : undefined
            anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? icList.top : undefined
            anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? icList.left : undefined
            anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? icList.right : undefined

            anchors.horizontalCenter: !root.vertical ? icList.horizontalCenter : undefined
            anchors.verticalCenter: root.vertical ? icList.verticalCenter : undefined

            width: root.vertical ? maxSize : icList.width
            height: root.vertical ? icList.height : maxSize

            target: icList

            property int maxSize: (root.hoveredIndex>=0 && !root.dragSource) ?
                                      root.statesLineSize + root.zoomFactor * (root.iconSize + root.iconMargin) - 1 :
                                      root.statesLineSize + root.iconSize + root.iconMargin - 1

            onUrlsDropped: {
                // If all dropped URLs point to application desktop files, we'll add a launcher for each of them.
                var createLaunchers = urls.every(function (item) {
                    return backend.isApplication(item)
                });

                if (createLaunchers) {
                    urls.forEach(function (item) {
                        addLauncher(item);
                    });
                    return;
                }

                if (!hoveredItem) {
                    return;
                }

                // DeclarativeMimeData urls is a QJsonArray but requestOpenUrls expects a proper QList<QUrl>.
                var urlsList = backend.jsonArrayToUrlList(urls);

                // Otherwise we'll just start a new instance of the application with the URLs as argument,
                // as you probably don't expect some of your files to open in the app and others to spawn launchers.
                tasksModel.requestOpenUrls(hoveredItem.modelIndex(), urlsList);
            }
        }


        ListView {
            id:icList

            property int currentSpot : -1000
            property int hoveredIndex : -1
            property int previousCount : 0

            property int tasksCount: contentItem.children.length

            property bool delayingRemoval: false

            onTasksCountChanged: updateImplicits();

            //  property int count: children ? children.length : 0
            /*   anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
            anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
            anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
            anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

            anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
            anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined  */

            width: contentWidth
            height: contentHeight

            orientation: Qt.Horizontal

            delegate: TaskDelegate{}

            /*Rectangle{
                anchors.fill: parent
                border.width: 1
                border.color: "red"
                color: "transparent"
            }*/

            //the duration of this animation should be as small as possible
            //it fixes a small issue with the dragging an item to change it's
            //position, if the duration is too big there is a point in the
            //list that an item is going back and forth too fast

            //more of a trouble
            moveDisplaced: Transition {
                NumberAnimation { properties: "x,y"; duration: root.durationTime*units.shortDuration; easing.type: Easing.Linear }
            }

            ///this transition can not be used with dragging !!!! I breaks
            ///the lists indexes !!!!!
            /*  move:  Transition {
                NumberAnimation { properties: "x,y"; duration: units.longDuration; easing.type: Easing.Linear }
            } */

            function childAtPos(x, y){
                var tasks = icList.contentItem.children;

                for(var i=0; i<tasks.length; ++i){
                    var task = tasks[i];

                    var choords = mapFromItem(task,0, 0);

                    if(choords.y < 0)
                        choords.y = 0;
                    if(choords.x < 0)
                        choords.x = 0;

                    if( (x>=choords.x) && (x<=choords.x+task.width)
                            && (y>=choords.y) && (y<=choords.y+task.height)){
                        return task
                    }
                }

                return null;
            }
        }

        VisualAddItem{
            id: newDroppedLauncherVisual
            anchors.fill: mouseHandler

            visible: opacity == 0 ? false : true
            opacity: root.dropNewLauncher && mouseHandler.onlyLaunchers && (root.dragSource == null)? 1 : 0
        }
    }

    //// helpers

    Timer {
        id: iconGeometryTimer
        // INVESTIGATE: such big interval but unfortunately it doesnot work otherwise
        interval: 500
        repeat: false

        onTriggered: {
            //    console.debug("Found children: "+icList.contentItem.children.length);
            TaskTools.publishIconGeometries(icList.contentItem.children);
        }
    }

    ////Activities List
    ////it can be used to cleanup the launchers from garbage-deleted activities....
    Item{
        id: activityModelInstance
        property int count: activityModelRepeater.count

        Repeater {
            id:activityModelRepeater
            model: Activities.ActivityModel {
                id: activityModel
                //  shownStates: "Running"
            }
            delegate: Item {
                visible: false
                property string activityId: model.id
                property string activityName: model.name
            }
        }

        function activities(){
            var activitiesResult = [];

            for(var i=0; i<activityModelInstance.count; ++i){
                console.log(children[i].activityId);
                activitiesResult.push(children[i].activityId);
            }

            return activitiesResult;
        }

        onCountChanged: {
            if(activityInfo.currentActivity != "00000000-0000-0000-0000-000000000000"){
                console.log("----------- Latte Plasmoid Signal: Activities number was changed ---------");
                var allActivities = activities();
                ActivitiesTools.cleanupRecords(allActivities);
                console.log("----------- Latte Plasmoid Signal End ---------");
            }
        }
    }

    /////////

    //// functions
    function movePanel(obj, newPosition){
        var bLine = obj;
        if (newPosition === PlasmaCore.Types.BottomPositioned){
            bLine.anchors.horizontalCenter = bLine.parent.horizontalCenter;
            bLine.anchors.verticalCenter = undefined;
            bLine.anchors.bottom = bLine.parent.bottom;
            bLine.anchors.top = undefined;
            bLine.anchors.left = undefined;
            bLine.anchors.right = undefined;
        }
        else if (newPosition === PlasmaCore.Types.TopPositioned){
            bLine.anchors.horizontalCenter = bLine.parent.horizontalCenter;
            bLine.anchors.verticalCenter = undefined;
            bLine.anchors.bottom = undefined;
            bLine.anchors.top = bLine.parent.top;
            bLine.anchors.left = undefined;
            bLine.anchors.right = undefined;
        }
        else if (newPosition === PlasmaCore.Types.LeftPositioned){
            bLine.anchors.horizontalCenter = undefined;
            bLine.anchors.verticalCenter = bLine.parent.verticalCenter;
            bLine.anchors.bottom = undefined;
            bLine.anchors.top = undefined;
            bLine.anchors.left = bLine.parent.left;
            bLine.anchors.right = undefined;
        }
        else if (newPosition === PlasmaCore.Types.RightPositioned){
            bLine.anchors.horizontalCenter = undefined;
            bLine.anchors.verticalCenter = bLine.parent.verticalCenter;
            bLine.anchors.bottom = undefined;
            bLine.anchors.top = undefined;
            bLine.anchors.left =undefined;
            bLine.anchors.right = bLine.parent.right;
        }
    }

    property int ncounter:0

    function updateImplicits(){
        if(icList.previousCount !== icList.count){
            icList.previousCount = icList.count;

            var zoomedLength = Math.floor( 1.2 * (iconSize+iconMargin) * (root.zoomFactor));
            var bigAxis = (tasksModel.count-1) * (iconSize+iconMargin) + zoomedLength;
            var smallAxis = zoomedLength + statesLineSize;

            var clearBigAxis = tasksModel.count * (iconSize+iconMargin) + (barLine.spacing/2);
            var clearSmallAxis = (iconSize+iconMargin)+statesLineSize;

            //  debugging code
            //     ncounter++;
            //      console.log("Implicits______ "+ncounter+". - "+tasksModel.count);

            if (root.vertical){
                root.implicitWidth = smallAxis;
                root.implicitHeight = bigAxis;
                root.clearWidth = clearSmallAxis;
                root.clearHeight = clearBigAxis;
            }
            else{
                root.implicitWidth = bigAxis;
                root.implicitHeight = smallAxis;
                root.clearWidth = clearBigAxis;
                root.clearHeight = clearSmallAxis;
            }

            iconGeometryTimer.restart();
        }
    }

    PlasmaComponents.Button{
        id: orientationBtn
        text:"Orientation"

        anchors.centerIn: parent
        visible: root.debugLocation

        onClicked:{
            switch(root.position){
            case PlasmaCore.Types.BottomPositioned:
                root.newLocationDebugUse = PlasmaCore.Types.LeftEdge;
                break;
            case PlasmaCore.Types.LeftPositioned:
                root.newLocationDebugUse = PlasmaCore.Types.TopEdge;
                break;
            case PlasmaCore.Types.TopPositioned:
                root.newLocationDebugUse = PlasmaCore.Types.RightEdge;
                break;
            case PlasmaCore.Types.RightPositioned:
                root.newLocationDebugUse = PlasmaCore.Types.BottomEdge;
                break;
            }
            updatePosition();
        }
    }


    function updatePosition(){
        var newPosition;
        var tempVertical=false;

        var positionUsed;


        if (root.debugLocation)
            positionUsed = root.newLocationDebugUse;
        else
            positionUsed = plasmoid.location;

        switch (positionUsed) {
        case PlasmaCore.Types.LeftEdge:
            newPosition = PlasmaCore.Types.LeftPositioned;
            tempVertical = true;
            break;
        case PlasmaCore.Types.RightEdge:
            newPosition = PlasmaCore.Types.RightPositioned;
            tempVertical = true;
            break;
        case PlasmaCore.Types.TopEdge:
            newPosition = PlasmaCore.Types.TopPositioned;
            break;
        default:
            newPosition = PlasmaCore.Types.BottomPositioned;
            break
        }

        movePanel(barLine,newPosition);
        movePanel(icList,newPosition);

        if(tempVertical)
            icList.orientation = Qt.Vertical;
        else
            icList.orientation = Qt.Horizontal;

        root.position = newPosition;
    }

    function outsideContainsMouse(){
        if (disableRestoreZoom) {
            return true;
        }

        var tasks = icList.contentItem.children;

        if(toolTipDelegate.currentItem != -1)
            return true;

        for(var i=0; i<tasks.length; ++i){
            var task = tasks[i];

            //l    console.log("Checking "+i+" - "+task.index+" - "+task.containsMouse);
            if(task && task.containsMouse){
                return true;
            }
        }

        return false;
    }

    function containsMouse(){
        if (disableRestoreZoom) {
            return;
        }

        var result = root.outsideContainsMouse();

        if ((!result || toolTipDelegate.parentIndex != icList.hoveredIndex) && windowSystem.compositingActive) {
            windowsPreviewDlg.hide();
        }

        if (result)
            return true;

        if (!result && nowDockPanel && nowDockPanel.outsideContainsMouse())
            return true;

        if (nowDockPanel)
            nowDockPanel.clearZoom();

        return false;
    }

    function clearZoom(){
        //console.log("Plasmoid clear...");
        if (disableRestoreZoom) {
            return;
        }

        icList.currentSpot = -1000;
        icList.hoveredIndex = -1;
        root.clearZoomSignal();
    }

    function hasLauncher(url) {
        return tasksModel.launcherPosition(url) != -1;
    }

    function addLauncher(url) {
        //workaround to protect in case the launcher contains the iconData
        var pos = url.indexOf("?iconData=");

        if (pos>0) {
            url = url.substring( 0, url.indexOf("?iconData=" ) );
        }

        tasksModel.requestAddLauncher(url);
    }

    function resetDragSource() {
        dragSource = null;
    }

    function createContextMenu(task) {
        var menu = root.contextMenuComponent.createObject(task);
        menu.visualParent = task;
        menu.mpris2Source = mpris2Source;
        menu.activitiesCount = activityModelInstance.count;
        return menu;
    }

    Component.onCompleted:  {
        updatePosition();

        root.presentWindows.connect(backend.presentWindows);
        root.windowsHovered.connect(backend.windowsHovered);
        //    mouseHandler.urlDropped.connect(backend.urlDropped);
        dragHelper.dropped.connect(resetDragSource);
    }

    //BEGIN states
    //user set Panel Positions
    // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [

        ///Bottom Edge
        State {
            name: "bottomCenter"
            when: (root.position === PlasmaCore.Types.BottomPosition && userPanelPosition===Latte.Dock.Center)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomLeft"
            when: (root.position === PlasmaCore.Types.BottomPosition && userPanelPosition===Latte.Dock.Left)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomRight"
            when: (root.position === PlasmaCore.Types.BottomPosition && userPanelPosition===Latte.Dock.Right)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        ///Top Edge
        State {
            name: "topCenter"
            when: (root.position === PlasmaCore.Types.TopPosition && userPanelPosition===Latte.Dock.Center)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "topLeft"
            when: (root.position === PlasmaCore.Types.TopPosition && userPanelPosition===Latte.Dock.Left)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "topRight"
            when: (root.position === PlasmaCore.Types.TopPosition && userPanelPosition===Latte.Dock.Right)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        ////Left Edge
        State {
            name: "leftCenter"
            when: (root.position === PlasmaCore.Types.LeftPosition && userPanelPosition===Latte.Dock.Center)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: icList
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        State {
            name: "leftTop"
            when: (root.position === PlasmaCore.Types.LeftPosition && userPanelPosition===Latte.Dock.Top)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "leftBottom"
            when: (root.position === PlasmaCore.Types.LeftPosition && userPanelPosition===Latte.Dock.Bottom)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        ///Right Edge
        State {
            name: "rightCenter"
            when: (root.position === PlasmaCore.Types.RightPosition && userPanelPosition===Latte.Dock.Center)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: icList
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        State {
            name: "rightTop"
            when: (root.position === PlasmaCore.Types.RightPosition && userPanelPosition===Latte.Dock.Top)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "rightBottom"
            when: (root.position === PlasmaCore.Types.RightPosition && userPanelPosition===Latte.Dock.Bottom)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: icList
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        }

    ]
    //END states

}
