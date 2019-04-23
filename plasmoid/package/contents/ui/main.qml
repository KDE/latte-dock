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

import org.kde.latte 0.2 as Latte

import "previews" as Previews
import "task" as Task
import "taskslayout" as TasksLayout
import "../code/tools.js" as TaskTools
import "../code/activitiesTools.js" as ActivitiesTools
import "../code/ColorizerTools.js" as ColorizerTools

Item {
    id:root

    Layout.fillWidth: scrollingEnabled && !root.vertical ? true : false
    Layout.fillHeight: scrollingEnabled && root.vertical ? true : false

    ///IMPORTANT: These values must be tested when the Now Dock Panel support
    ///also the four new anchors. A small issue is shown between the animation
    /// of the now dock plasmoid and the neighbour widgets...
    Layout.minimumWidth: -1 //(userPanelPosition !== 0)&&(!latteView) ? clearWidth : -1
    Layout.minimumHeight: -1 //(userPanelPosition !== 0)&&(!latteView) ? clearHeight : -1
    Layout.preferredWidth: tasksWidth   //(userPanelPosition !== 0)&&(!latteView) ? tasksWidth : tasksWidth
    Layout.preferredHeight: tasksHeight //(userPanelPosition !== 0)&&(!latteView) ? tasksHeight : tasksHeight
    Layout.maximumWidth: -1
    Layout.maximumHeight: -1

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft && !root.vertical
    LayoutMirroring.childrenInherit: true

    property bool debugLocation: false

    //it is used to check both the applet and the containment for direct render
    property bool globalDirectRender: latteView ? latteView.globalDirectRender : icList.directRender

    property bool plasma515: Latte.WindowSystem.plasmaDesktopVersion >= Latte.WindowSystem.makeVersion(5,14,90)

    property bool editMode: latteView ? latteView.editMode : plasmoid.userConfiguring
    property bool inConfigureAppletsMode: latteView ? latteView.inConfigureAppletsMode : true
    property bool disableRestoreZoom: false //blocks restore animation in rightClick
    property bool disableAllWindowsFunctionality: root.showWindowsOnlyFromLaunchers && !indicators.isEnabled
    property bool dropNewLauncher: false
    readonly property bool hasInternalSeparator: parabolicManager.hasInternalSeparator
    property bool inActivityChange: false
    property bool inDraggingPhase: false
    property bool initializationStep: false //true
    property bool isHovered: false
    property bool showBarLine: plasmoid.configuration.showBarLine
    property bool showTaskShortcutBadges: false
    property int tasksBaseIndex: 0
    property bool useThemePanel: plasmoid.configuration.useThemePanel
    property bool taskInAnimation: noTasksInAnimation > 0 ? true : false
    property bool transparentPanel: plasmoid.configuration.transparentPanel
    property bool vertical: plasmoid.formFactor === PlasmaCore.Types.Vertical ? true : false

    property int clearWidth
    property int clearHeight

    property int newLocationDebugUse: PlasmaCore.Types.BottomPositioned
    property int newDroppedPosition: -1
    property int noInitCreatedBuffers: 0
    property int noTasksInAnimation: 0
    property int themePanelSize: plasmoid.configuration.panelSize

    property int position : PlasmaCore.Types.BottomPositioned
    property int tasksStarting: 0

    ///Don't use Math.floor it adds one pixel in animations and creates glitches
    property int widthMargins: root.vertical ? thickMargins : lengthMargins
    property int heightMargins: !root.vertical ? thickMargins : lengthMargins

    property int internalWidthMargins: root.vertical ? thickMargins : 2 * lengthIntMargin
    property int internalHeightMargins: !root.vertical ? thickMargins : 2 * lengthIntMargin

    property real textColorBrightness: ColorizerTools.colorBrightness(themeTextColor)
    property color minimizedDotColor: {
        if (latteView) {
            return latteView.minimizedDotColor;
        }

        return textColorBrightness > 127.5 ? Qt.darker(themeTextColor, 1.7) : Qt.lighter(themeBackgroundColor, 7)
    }

    property color themeTextColor: theme.textColor
    property color themeBackgroundColor: theme.backgroundColor

    property color lightTextColor: textColorBrightness > 127.5 ? themeTextColor : themeBackgroundColor

    //a small badgers record (id,value)
    //in order to track badgers when there are changes
    //in launcher reference from libtaskmanager
    property variant badgers:[]
    property variant launchersOnActivities: []
    property variant waitingLaunchers: []

    //! launchers that are shown after a window removal and must be shown
    //! immediately because they are already present!
    property variant immediateLaunchers: []

    //global plasmoid reference to the context menu
    property QtObject contextMenu: null
    property QtObject contextMenuComponent: Qt.createComponent("ContextMenu.qml");
    property Item dragSource: null
    property Item parabolicManager: _parabolicManager


    //separator calculations based on audoban's design
    property int maxSeparatorLength: {
        if (root.vertical)
            return 5 + heightMargins;
        else
            return 5 + widthMargins;
    }

    property real missingSeparatorLength: {
        if (!root.isVertical)
            return ((iconSize + widthMargins) * zoomFactor) - maxSeparatorLength;
        else
            return ((iconSize + heightMargins) * zoomFactor) - maxSeparatorLength;
    }


    //! it is used to play the animation correct when the user removes a launcher
    property string launcherForRemoval: ""

    //BEGIN Latte Dock properties
    property bool dockIsShownCompletely: latteView ? latteView.dockIsShownCompletely : true
    property bool enableShadows: latteView ? latteView.enableShadows > 0 : plasmoid.configuration.showShadows
    property bool forceHidePanel: false
    property bool directRenderDelayerIsRunning: latteView ? latteView.directRenderDelayerIsRunning : directRenderDelayerForEnteringTimer.running
    property bool disableLeftSpacer: false
    property bool disableRightSpacer: false
    property bool dockIsHidden: latteView ? latteView.dockIsHidden : false
    property bool groupTasksByDefault: latteView ? latteView.groupTasksByDefault: true
    property bool highlightWindows: latteView ? latteView.hoverAction === Latte.Types.HighlightWindows || latteView.hoverAction === Latte.Types.PreviewAndHighlightWindows :
                                                plasmoid.configuration.highlightWindows
    property bool mouseWheelActions: latteView ? latteView.mouseWheelActions : true
    property bool parabolicEffectEnabled: latteView ? latteView.parabolicEffectEnabled : zoomFactor>1 && !root.editMode

    property bool scrollingEnabled: latteView ? latteView.scrollingTasksEnabled : false

    property bool showInfoBadge: latteView ? latteView.showInfoBadge : plasmoid.configuration.showInfoBadge
    property bool showProgressBadge: latteView ? latteView.showProgressBadge : plasmoid.configuration.showInfoBadge
    property bool showAudioBadge: latteView ? latteView.showAudioBadge : plasmoid.configuration.showAudioBadge
    property bool audioBadgeActionsEnabled: latteView ? latteView.audioBadgeActionsEnabled : true
    property bool showOnlyCurrentScreen: latteView ? latteView.showOnlyCurrentScreen : plasmoid.configuration.showOnlyCurrentScreen
    property bool showOnlyCurrentDesktop: latteView ? latteView.showOnlyCurrentDesktop : plasmoid.configuration.showOnlyCurrentDesktop
    property bool showOnlyCurrentActivity: latteView ? latteView.showOnlyCurrentActivity : plasmoid.configuration.showOnlyCurrentActivity
    property bool showPreviews:  latteView ? latteView.hoverAction === Latte.Types.PreviewWindows || latteView.hoverAction === Latte.Types.PreviewAndHighlightWindows :
                                             plasmoid.configuration.showToolTips
    property bool showWindowActions: latteView ? latteView.showWindowActions : plasmoid.configuration.showWindowActions
    property bool showWindowsOnlyFromLaunchers: latteView ? latteView.showWindowsOnlyFromLaunchers : false

    property bool titleTooltips: latteView ? latteView.titleTooltips : false
    property alias windowPreviewIsShown: windowsPreviewDlg.visible

    property int animationStep: latteView ? latteView.animationStep : 1
    property int directRenderAnimationTime: latteView ? latteView.directRenderAnimationTime : 0
    property int dockHoveredIndex : latteView ? latteView.hoveredIndex : -1

    property int iconSize: latteView ? latteView.iconSize : Math.max(plasmoid.configuration.iconSize, 16)
    property int maxIconSize: latteView ? latteView.maxIconSize : iconSize

    property int leftClickAction: latteView ? latteView.leftClickAction : Latte.Types.PresentWindows
    property int middleClickAction: latteView ? latteView.middleClickAction : plasmoid.configuration.middleClickAction
    property int hoverAction: latteView ? latteView.hoverAction : Latte.Types.NoneAction
    property int modifier: latteView ? latteView.modifier : -1
    property int modifierClickAction: latteView ? latteView.modifierClickAction : -1
    property int modifierClick: latteView ? latteView.modifierClick : -1
    property int modifierQt:{
        if (modifier === Latte.Types.Shift)
            return Qt.ShiftModifier;
        else if (modifier === Latte.Types.Ctrl)
            return Qt.ControlModifier;
        else if (modifier === Latte.Types.Alt)
            return Qt.AltModifier;
        else if (modifier === Latte.Types.Meta)
            return Qt.MetaModifier;
        else return -1;
    }

    property int thickMargin: latteView ? latteView.thickMargin : 0.16*iconSize
    property int thickMargins: 2 * thickMargin
    property int lengthIntMargin: latteView ? latteView.lengthIntMargin : 0.04*iconSize
    property int lengthExtMargin: latteView ? latteView.lengthExtMargin : 0.1 * iconSize
    property int lengthMargin: lengthIntMargin + lengthExtMargin
    property int lengthMargins: 2 * lengthMargin

    property int tasksHeight: mouseHandler.height
    property int tasksWidth: mouseHandler.width
    //updated from Binding
    property int userPanelPosition

    readonly property real currentPanelOpacity: latteView ? latteView.currentPanelTransparency / 100 : 1

    //! Animations
    property bool animationsEnabled: latteView ? latteView.animationsEnabled : durationTime !== 0
    property bool animationLauncherBouncing: latteView ? latteView.animationLauncherBouncing : durationTime !== 0
    property bool animationWindowInAttention: latteView ? latteView.animationWindowInAttention : durationTime !== 0
    property bool animationNewWindowSliding: latteView ? latteView.animationNewWindowSliding : durationTime !== 0
    property bool animationWindowAddedInGroup: latteView ? latteView.animationWindowAddedInGroup : durationTime !== 0
    property bool animationWindowRemovedFromGroup: latteView ? latteView.animationWindowRemovedFromGroup : durationTime !== 0

    property real animationsZoomFactor: latteView ? latteView.animationsZoomFactor : durationTime === 0 ? 1 : 1.65
    property real maxZoomFactor: latteView ? latteView.maxZoomFactor : Math.max(zoomFactor, animationsZoomFactor)

    property real appliedDurationTime: animationsEnabled ? durationTime : 2
    property real durationTime: latteView ? latteView.durationTime : plasmoid.configuration.durationTime
    property real zoomFactor: latteView ? latteView.zoomFactor : ( 1 + (plasmoid.configuration.zoomLevel / 20) )

    property int appShadowSize: latteView ? latteView.appShadowSize : Math.ceil(0.12*iconSize)
    property string appShadowColor: latteView ? latteView.appShadowColor : "#ff080808"
    property string appShadowColorSolid: latteView ? latteView.appShadowColorSolid : "#ff080808"

    property alias tasksCount: tasksModel.count
    property alias hoveredIndex: icList.hoveredIndex

    property QtObject currentLayout : latteView && latteView.managedLayout ? latteView.managedLayout : null

    property var badgesForActivate: latteView ? latteView.badgesForActivate : []
    property var managedLayoutName: currentLayout ? currentLayout.name : ""

    property Item latteView: null
    readonly property Item indicators: latteView ? latteView.indicatorsManager : indicatorsStandaloneLoader.item
    //END Latte Dock Panel properties

    //BEGIN Latte Dock Communicator
    property QtObject latteBridge: null
    onLatteBridgeChanged: {
        if (latteBridge) {
            latteBridge.actions.setProperty(plasmoid.id, "latteSideColoringEnabled", false);
        }
    }
    //END  Latte Dock Communicator
    //BEGIN Latte based properties
    readonly property bool enforceLattePalette: latteBridge && latteBridge.applyPalette && latteBridge.palette
    readonly property bool latteInEditMode: latteBridge && latteBridge.inEditMode
    //END Latte based properties

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    signal clearZoomSignal();
    signal draggingFinished();
    signal hiddenTasksUpdated();
    signal launchersUpdatedFor(string launcher);
    signal presentWindows(variant winIds);
    signal requestLayout;
    signal separatorsUpdated();
    signal signalActionsBlockHiding(int value);
    signal signalAnimationsNeedBothAxis(int value);
    signal signalAnimationsNeedLength(int value);
    signal signalAnimationsNeedThickness(int value);
    signal signalPreviewsShown();
    //signal signalDraggingState(bool value);
    signal showPreviewForTasks(QtObject group);
    //trigger updating scaling of neighbour delegates of zoomed delegate
    signal updateScale(int delegateIndex, real newScale, real step)
    signal mimicEnterForParabolic();
    signal publishTasksGeometries();
    signal waitingLauncherRemoved(string launch);
    signal windowsHovered(variant winIds, bool hovered)

    //onAnimationsChanged: console.log(animations);
    /* Rectangle{
                anchors.fill: parent
                border.width: 1
                border.color: "red"
                color: "white"
            } */
    onLatteViewChanged: {
        if (latteView) {
            plasmoid.action("configure").visible = false;
            plasmoid.action("remove").visible = false;
            plasmoid.configuration.isInLatteDock = true;
        } else {
            plasmoid.configuration.isInLatteDock = false;
        }
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


    Connections{
        target: latteView
        onDockIsHiddenChanged:{
            if (latteView.dockIsHidden) {
                windowsPreviewDlg.hide("3.3");
            }
        }

        onLaunchersGroupChanged:{
            if( latteView && latteView.editMode) {
                tasksModel.updateLaunchersList();
            }
        }
    }


    Connections{
        target: latteView && latteView.universalLayoutManager ? latteView.universalLayoutManager : null

        onCurrentLayoutNameChanged: root.publishTasksGeometries();
    }

    Connections{
        target: icList

        onHoveredIndexChanged:{
            if (latteView && icList.hoveredIndex>-1){
                latteView.setHoveredIndex(-1);
            }
        }
    }


    /////
    PlasmaCore.ColorScope{
        id: colorScopePalette
    }

    Loader {
        id: indicatorsStandaloneLoader
        active: !latteView && !plasmoid.configuration.isInLatteDock
        source: "indicators/Manager.qml"
    }

    Binding {
        target: root
        property: "userPanelPosition"
        value: {
            if (latteView) {
                if (latteView.panelUserSetAlignment === -1) {
                    return;
                }

                if (inConfigureAppletsMode) {
                    return Latte.Types.Center;
                } else if (latteView.panelUserSetAlignment === Latte.Types.Justify) {
                        if (latteView.latteAppletPos>=0 && latteView.latteAppletPos<100) {
                            return plasmoid.formFactor === PlasmaCore.Types.Horizontal ? Latte.Types.Left : Latte.Types.Top;
                        } else if (latteView.latteAppletPos>=100 && latteView.latteAppletPos<200) {
                            return Latte.Types.Center;
                        } else if (latteView.latteAppletPos>=200) {
                            return plasmoid.formFactor === PlasmaCore.Types.Horizontal ? Latte.Types.Right : Latte.Types.Bottom;
                        }

                        return Latte.Types.Center;
                }

                return latteView.panelUserSetAlignment;
            }

            return plasmoid.configuration.plasmoidPosition;
        }
    }

    /////

    function initializeHoveredIndex() {
        icList.hoveredIndex = -1;
        icList.currentSpot = -1000;
    }

    function launchersDropped(urls){
        mouseHandler.urlsDroppedOnArea(urls);
    }

    ///UPDATE
    function launcherExists(url) {
        return (ActivitiesTools.getIndex(url, tasksModel.launcherList)>=0);
    }

    function taskExists(url) {
        var tasks = icList.contentItem.children;
        for(var i=0; i<tasks.length; ++i){
            var task = tasks[i];

            if (task.launcherUrl===url && task.isWindow) {
                return true;
            }
        }
        return false;
    }

    function launchersDrop(event) {
        var createLaunchers = false;

        if (event.mimeData.hasUrls) {
            createLaunchers = event.mimeData.urls.every(function (item) {
                return backend.isApplication(item)
            });
        }

        return createLaunchers;
    }

    function currentLauncherList() {
        var launch = [];
        var launchersList = [];

        if (currentLayout) {
            if (latteView && latteView.universalLayoutManager
                    && latteView.managedLayout && latteView.universalSettings
                    && (latteView.launchersGroup === Latte.Types.LayoutLaunchers
                        || latteView.launchersGroup === Latte.Types.GlobalLaunchers)) {

                if (latteView.launchersGroup === Latte.Types.LayoutLaunchers) {
                    launchersList = latteView.managedLayout.launchers;
                } else if (latteView.launchersGroup === Latte.Types.GlobalLaunchers) {
                    launchersList = latteView.universalSettings.launchers;
                }
            }
        } else {
            launchersList = plasmoid.configuration.launchers59;
        }


        for(var i=0; i<launchersList.length; ++i){
            var launcherRecord = launchersList[i];

            if (launcherRecord.indexOf("[") === -1) {
                //global launcher
                launch.push(launcherRecord);
            } else {
                //launcher assigned to activities
                var end = launcherRecord.indexOf("\n");
                var explicitLauncher = launcherRecord.substring(end+1,launcherRecord.length);
                if (explicitLauncher !== "" && launcherRecord.indexOf(activityInfo.currentActivity) > -1) {
                    launch.push(explicitLauncher);
                }
            }
        }

        return launch;
    }

    function currentListViewLauncherList() {
        var launch = [];

        var tasks = icList.contentItem.children;
        for(var i=0; i<tasks.length; ++i){
            var task = icList.childAtIndex(i);

            if (task!==undefined && task.launcherUrl!=="" && tasksModel.launcherInCurrentActivity(task.launcherUrl)) {
                launch.push(task.launcherUrl);
            }
        }

        return launch;
    }

    /// waiting launchers... this is used in order to check
    /// a window or startup if its launcher is playing its animation
    function addWaitingLauncher(launch){
        for(var i=0; i<waitingLaunchers.length; ++i){
            if (waitingLaunchers[i]===launch) {
                return;
            }
        }

        waitingLaunchers.push(launch);
    }

    function removeWaitingLauncher(launch){
        for(var i=0; i<waitingLaunchers.length; ++i){
            if (waitingLaunchers[i]===launch) {
                waitingLaunchers.splice(i,1);
                waitingLauncherRemoved(launch);
                return;
            }
        }
    }

    function waitingLauncherExists(launch){
        for(var i=0; i<waitingLaunchers.length; ++i){
            if (waitingLaunchers[i]===launch) {
                return true;
            }
        }

        return false;
    }

    //! ImmediateLaunchers
    function addImmediateLauncher(launch){
        if (!immediateLauncherExists(launch)) {
            //console.log("Immediate Launcher Added::: "+launch);
            immediateLaunchers.push(launch);
        }
    }

    function removeImmediateLauncher(launch){
        for(var i=0; i<immediateLaunchers.length; ++i){
            if (immediateLaunchers[i]===launch) {
                immediateLaunchers.splice(i,1);
                //console.log("Immediate Launcher Removed::: "+launch);
                return;
            }
        }
    }

    function immediateLauncherExists(launch){
        for(var i=0; i<immediateLaunchers.length; ++i){
            if (immediateLaunchers[i]===launch) {
                return true;
            }
        }

        return false;
    }

    //!

    function hidePreview(){
        windowsPreviewDlg.hide(11);
    }

    onDragSourceChanged: {
        if (dragSource == null) {
            root.draggingFinished();
            root.signalActionsBlockHiding(-1);
            tasksModel.syncLaunchers();

            restoreDraggingPhaseTimer.start();
        } else {
            inDraggingPhase = true;
            root.signalActionsBlockHiding(1);
        }
    }

    /////Window previews///////////

    Previews.ToolTipDelegate2 {
        id: toolTipDelegate
        visible: false
    }

    ////BEGIN interfaces

    PlasmaCore.Dialog{
        id: windowsPreviewDlg
        // hideOnWindowDeactivate: false
        type: PlasmaCore.Dialog.Tooltip
        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.ToolTip

        location: plasmoid.location
        mainItem: toolTipDelegate
        visible: false

        property bool signalSent: false
        property Item activeItem: null

        Component.onCompleted: mainItem.visible = true;

        function hide(debug){
            // console.log("on hide previews event called: "+debug);

            if (latteView && signalSent) {
                //it is used to unblock dock hiding
                root.signalActionsBlockHiding(-1);
                signalSent = false;
            }

            if (!root.contextMenu)
                root.disableRestoreZoom = false;

            hidePreviewWinTimer.start();
        }

        function show(taskItem){
            if (root.disableAllWindowsFunctionality) {
                return;
            }

            hidePreviewWinTimer.stop();

            // console.log("preview show called...");
            if ((!activeItem || (activeItem !== taskItem)) && !root.contextMenu) {
                //console.log("preview show called: accepted...");

                //this can be used from others to hide their appearance
                //e.g but applets from the dock to hide themselves
                if (!visible) {
                    root.signalPreviewsShown();
                }

                activeItem = taskItem;
                toolTipDelegate.parentTask = taskItem;

                if (latteView && !signalSent) {
                    //it is used to block dock hiding
                    root.signalActionsBlockHiding(1);
                    signalSent = true;
                }

                //! Workaround in order to update properly the previews thumbnails
                //! when switching between single thumbnail to another single thumbnail
                mainItem.visible = false;
                visible = true;
                mainItem.visible = true;
            }
        }
    }

    //! Delay windows previews hiding
    Timer {
        id: hidePreviewWinTimer
        interval: 350
        onTriggered: {
            windowsPreviewDlg.visible = false;
            windowsPreviewDlg.mainItem.visible = false;
            windowsPreviewDlg.activeItem = null;
        }
    }

    //! Timer to fix #811, rare cases that both a window preview and context menu are
    //! shown. It is mostly used under wayland in order to avoid crashes. When the context
    //! menu will be shown there is a chance that previews window has already appeared in that
    //! case the previews window must become hidden
    Timer {
        id: windowsPreviewCheckerToNotShowTimer
        interval: 250

        onTriggered: {
            if (windowsPreviewDlg.visible && root.contextMenu) {
                windowsPreviewDlg.hide("8.2");
            }
        }
    }

    //! Timer to delay the removal of the window through the context menu in case the
    //! the window is zoomed
    Timer{
        id: delayWindowRemovalTimer
        //this is the animation time needed in order for tasks to restore their zoom first
        interval: 7 * (root.durationTime * units.shortDuration)

        property var modelIndex

        onTriggered: {
            tasksModel.requestClose(delayWindowRemovalTimer.modelIndex)

            if (latteView && latteView.debugModeTimers) {
                console.log("plasmoid timer: delayWindowRemovalTimer called...");
            }
        }
    }

    Timer {
        id: activityChangeDelayer
        interval: 150
        onTriggered: {
            root.inActivityChange = false;
            activityInfo.previousActivity = activityInfo.currentActivity;

            if (latteView && latteView.debugModeTimers) {
                console.log("plasmoid timer: activityChangeDelayer called...");
            }
        }
    }

    /////Window Previews/////////


    property bool loadLaunchersFirstTime: false;

    onCurrentLayoutChanged: {
        if (currentLayout && !loadLaunchersFirstTime) {
            tasksModel.updateLaunchersList();
            loadLaunchersFirstTime = true;
        }
    }

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
        separateLaunchers: true
        groupInline: false

        groupMode: groupTasksByDefault ? TaskManager.TasksModel.GroupApplications : TaskManager.TasksModel.GroupDisabled
        sortMode: TaskManager.TasksModel.SortManual

        function updateLaunchersList(){
            if (latteView.universalSettings
                    && (latteView.launchersGroup === Latte.Types.LayoutLaunchers
                        || latteView.launchersGroup === Latte.Types.GlobalLaunchers)) {
                if (latteView.launchersGroup === Latte.Types.LayoutLaunchers) {
                    tasksModel.launcherList = latteView.managedLayout.launchers;
                } else if (latteView.launchersGroup === Latte.Types.GlobalLaunchers) {
                    tasksModel.launcherList = latteView.universalSettings.launchers;
                }
            } else {
                tasksModel.launcherList = plasmoid.configuration.launchers59;
            }
        }

        function launcherInCurrentActivity(url) {
            var activities = tasksModel.launcherActivities(url);

            var NULL_UUID = "00000000-0000-0000-0000-000000000000";

            if (activities.indexOf(NULL_UUID) !== -1 || activities.indexOf(activityInfo.currentActivity) !== -1)
                return true;

            return false;
        }

        onActivityChanged: {
            ActivitiesTools.currentActivity = String(activity);
        }

        onLauncherListChanged: {
            if (currentLayout) {
                if (latteView && latteView.universalLayoutManager
                        && latteView.managedLayout && latteView.universalSettings
                        && (latteView.launchersGroup === Latte.Types.LayoutLaunchers
                            || latteView.launchersGroup === Latte.Types.GlobalLaunchers)) {

                    if (latteView.launchersGroup === Latte.Types.LayoutLaunchers) {
                        latteView.managedLayout.launchers = launcherList;
                    } else if (latteView.launchersGroup === Latte.Types.GlobalLaunchers) {
                        latteView.universalSettings.launchers = launcherList;
                    }

                    if (inDraggingPhase) {
                        if (latteView && latteView.launchersGroup >= Latte.Types.LayoutLaunchers) {
                            latteView.universalLayoutManager.launchersSignals.validateLaunchersOrder(root.managedLayoutName,
                                                                                                     plasmoid.id,
                                                                                                     latteView.launchersGroup,
                                                                                                     currentLauncherList());
                        }
                    }
                } else {
                    plasmoid.configuration.launchers59 = launcherList;
                }
            } else {
                plasmoid.configuration.launchers59 = launcherList;
            }
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
            ActivitiesTools.currentActivity = String(activityInfo.currentActivity);
            ActivitiesTools.plasmoid = plasmoid;

            //var loadedLaunchers = ActivitiesTools.restoreLaunchers();
            ActivitiesTools.importLaunchersToNewArchitecture();

            if (currentLayout && latteView.universalSettings
                    && (latteView.launchersGroup === Latte.Types.LayoutLaunchers
                        || latteView.launchersGroup === Latte.Types.GlobalLaunchers)) {

                if (latteView.launchersGroup === Latte.Types.LayoutLaunchers) {
                    launcherList = latteView.managedLayout.launchers;
                } else if (latteView.launchersGroup === Latte.Types.GlobalLaunchers) {
                    launcherList = latteView.universalSettings.launchers;
                }
            } else {
                launcherList = plasmoid.configuration.launchers59;
            }

            groupingAppIdBlacklist = plasmoid.configuration.groupingAppIdBlacklist;
            groupingLauncherUrlBlacklist = plasmoid.configuration.groupingLauncherUrlBlacklist;

            icList.model = tasksModel;
            tasksStarting = count;

            ///Plasma 5.9 enforce grouping at all cases
            if (Latte.WindowSystem.plasmaDesktopVersion >= Latte.WindowSystem.makeVersion(5,9,0)) {
                groupingWindowTasksThreshold = -1;
            }
        }
    }

    //! TaskManagerBackend required a groupDialog setting otherwise it crashes. This patch
    //! sets one just in order not to crash TaskManagerBackend
    PlasmaCore.Dialog {
        //ghost group Dialog to not crash TaskManagerBackend
        id: groupDialogGhost
        visible: false

        type: PlasmaCore.Dialog.PopupMenu
        flags: Qt.WindowStaysOnTopHint
        hideOnWindowDeactivate: true
        location: plasmoid.location
    }


    TaskManagerApplet.Backend {
        id: backend

        taskManagerItem: root
        toolTipItem: toolTipDelegate
        highlightWindows: root.highlightWindows

        onAddLauncher: {
            tasksModel.requestAddLauncher(url);
        }

        Component.onCompleted: {
            //! In Plasma 5.9 TaskManagerBackend required a groupDialog setting
            //! otherwise it crashes.
            //! frameworks 5.29.0 provide id 335104
            //! work only after Plasma 5.9 and frameworks 5.29
            //! + added a check for groupDialog also when it is present
            //!   in plasma 5.8 (that was introduced after 5.8.5)
            if (Latte.WindowSystem.frameworksVersion >= 335104 || (groupDialog !== undefined)) {
                groupDialog = groupDialogGhost;
            }
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

        property string previousActivity: ""
        onCurrentActivityChanged: {
            root.inActivityChange = true;
            activityChangeDelayer.start();
        }

        Component.onCompleted: previousActivity = currentActivity;
    }

    PlasmaCore.DataSource {
        id: mpris2Source
        engine: "mpris2"
        connectedSources: sources
        function sourceNameForLauncherUrl(launcherUrl, pid) {
            if (!launcherUrl || launcherUrl == "") {
                return "";
            }

            // MPRIS spec explicitly mentions that "DesktopEntry" is with .desktop extension trimmed
            // Moreover, remove URL parameters, like wmClass (part after the question mark)
            var desktopFileName = launcherUrl.toString().split('/').pop().split('?')[0].replace(".desktop", "")
            if (desktopFileName.indexOf("applications:") === 0) {
                desktopFileName = desktopFileName.substr(13)
            }

            for (var i = 0, length = connectedSources.length; i < length; ++i) {
                var source = connectedSources[i];
                // we intend to connect directly, otherwise the multiplexer steals the connection away
                if (source === "@multiplex") {
                    continue;
                }

                var sourceData = data[source];
                if (!sourceData || sourceData.DesktopEntry !== desktopFileName) {
                    continue;
                }

                if (pid === undefined || sourceData.InstancePid === pid) {
                    return source;
                }

                var metadata = sourceData.Metadata;
                if (metadata) {
                    var kdePid = metadata["kde:pid"];
                    if (kdePid && pid === kdePid) {
                        return source;
                    }
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

    Loader {
        id: pulseAudio
        source: "PulseAudio.qml"
        active: root.showAudioBadge
    }

    ParabolicManager{
        id: _parabolicManager
    }

    /*  IconsModel{
        id: iconsmdl
    }*/

    Component{
        id: attentionTimerComponent
        Timer{
            id: attentionTimer
            interval:8500
            onTriggered: {
                plasmoid.status = PlasmaCore.Types.PassiveStatus;
                destroy();

                if (latteView && latteView.debugModeTimers) {
                    console.log("plasmoid timer: attentionTimer called...");
                }
            }
            Component.onCompleted: {
                start();
            }
        }
    }

    //Timer to check if the mouse is still inside the ListView
    //IMPORTANT ::: This timer should be used only when the Latte plasmoid
    //is not inside a Latte dock
    Timer{
        id:checkListHovered
        repeat:false;
        interval: 120

        property int normalInterval: Math.max(120, 2 * (root.durationTime * 1.2 * units.shortDuration) + 50)

        onTriggered: {
            if(root.latteView)
                console.log("Plasmoid, checkListHoveredTimer was called, even though it shouldn't...");

            if (!root.containsMouse()) {

                icList.directRender = false;

                root.clearZoom();
            }

            interval = normalInterval;

            if (latteView && latteView.debugModeTimers) {
                console.log("plasmoid timer: checkListHovered called...");
            }
        }

        function startNormal(){
            interval = normalInterval;

            start();
        }

        function startDuration( duration){
            interval = duration;

            start();
        }
    }

    //! Delayer in order to not activate directRendering when the mouse
    //! enters until the timer has ended. This way we make sure that the
    //! zoom-in animations will have ended.
    Timer{
        id:directRenderDelayerForEnteringTimer
        interval: 3.2 * root.durationTime * units.shortDuration
    }

    //this timer restores the draggingPhase flag to false
    //after a dragging has finished... This delay is needed
    //in order to not animate any tasks are added after a
    //dragging
    Timer {
        id: restoreDraggingPhaseTimer
        interval: 150
        onTriggered: inDraggingPhase = false;
    }

    ///Red Liner!!! show the upper needed limit for animations
    Rectangle{
        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

        width: root.vertical ? 1 : 2 * root.iconSize
        height: root.vertical ? 2 * root.iconSize : 1
        color: "red"
        x: (root.position === PlasmaCore.Types.LeftPositioned) ? neededSpace : parent.width - neededSpace
        y: (root.position === PlasmaCore.Types.TopPositioned) ? neededSpace : parent.height - neededSpace

        visible: plasmoid.configuration.zoomHelper

        property int neededSpace: zoomFactor*(iconSize+lengthMargins)
    }

    Item{
        id:barLine

        width: ( icList.orientation === Qt.Horizontal ) ? icList.width + spacing : smallSize
        height: ( icList.orientation === Qt.Vertical ) ? icList.height + spacing : smallSize

        property int spacing: latteView ? 0 : root.iconSize / 2
        property int smallSize: Math.max(0.10 * root.iconSize, 16)

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


        TasksLayout.MouseHandler {
            id: mouseHandler
            anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? scrollableList.bottom : undefined
            anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? scrollableList.top : undefined
            anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? scrollableList.left : undefined
            anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? scrollableList.right : undefined

            anchors.horizontalCenter: !root.vertical ? scrollableList.horizontalCenter : undefined
            anchors.verticalCenter: root.vertical ? scrollableList.verticalCenter : undefined

            width: root.vertical ? maxSize : icList.width
            height: root.vertical ? icList.height : maxSize

            target: icList

            property int maxSize: (((root.hoveredIndex>=0 || dockHoveredIndex>=0 ) || windowPreviewIsShown) && !root.dragSource) ?
                                      root.zoomFactor * (root.iconSize + root.thickMargins) :
                                      root.iconSize + root.thickMargins

            function onlyLaunchersInList(list){
                return list.every(function (item) {
                    return backend.isApplication(item)
                });
            }

            function urlsDroppedOnArea(urls){
                // If all dropped URLs point to application desktop files, we'll add a launcher for each of them.
                if (onlyLaunchersInList(urls)) {
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

            onUrlsDropped: {
                //! inform synced docks for new dropped launchers
                if (latteView && latteView.launchersGroup >= Latte.Types.LayoutLaunchers && onlyLaunchersInList(urls)) {
                    latteView.universalLayoutManager.launchersSignals.urlsDropped(root.managedLayoutName,
                                                                                  latteView.launchersGroup, urls);
                    return;
                }

                //! if the list does not contain only launchers then just open the corresponding
                //! urls with the relevant app
                urlsDroppedOnArea(urls);
            }
        }

       /* Rectangle {
            anchors.fill: scrollableList
            color: "transparent"
            border.width: 1
            border.color: "blue"
        } */

        TasksLayout.ScrollableList {
            id: scrollableList
            width: !root.vertical ? Math.min(root.width, icList.width) : thickness
            height: root.vertical ? Math.min(root.height, icList.height) : thickness
            contentWidth: icList.width
            contentHeight: icList.height

            property int thickness: {
                if (latteView) {
                    return !latteView.thickAnimated ? latteView.maskManager.thicknessNormal : latteView.maskManager.thicknessZoom;
                }

                return (root.thickMargins + root.iconSize) * root.zoomFactor;
            }

            //onCurrentPosChanged: console.log("CP :: "+ currentPos + " icW:"+icList.width + " rw: "+root.width + " w:" +width);

            layer.enabled: contentsExceed && root.scrollingEnabled
            layer.effect: OpacityMask {
                maskSource: TasksLayout.ScrollOpacityMask{
                    width: scrollableList.width + root.lengthMargins
                    height: scrollableList.height
                }
            }

            TasksLayout.ScrollPositioner {
                id: listViewBase

                ListView {
                    id:icList
                    width: !root.vertical ? contentWidth : mouseHandler.maxSize
                    height: root.vertical ? contentHeight : mouseHandler.maxSize
                    boundsBehavior: Flickable.StopAtBounds
                    orientation: Qt.Horizontal
                    delegate: Task.TaskItem{}

                    property int currentSpot : -1000
                    property int hoveredIndex : -1
                    property int previousCount : 0

                    property int tasksCount: tasksModel.count

                    property bool directRender: false

                    //the duration of this animation should be as small as possible
                    //it fixes a small issue with the dragging an item to change it's
                    //position, if the duration is too big there is a point in the
                    //list that an item is going back and forth too fast

                    //more of a trouble
                    moveDisplaced: Transition {
                        NumberAnimation { properties: "x,y"; duration: root.durationTime*units.longDuration; easing.type: Easing.Linear }
                    }

                    ///this transition can not be used with dragging !!!! I breaks
                    ///the lists indexes !!!!!
                    ///move: Transition {
                    ///    NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.Linear }
                    ///}

                    function childAtPos(x, y){
                        var tasks = icList.contentItem.children;

                        for(var i=0; i<tasks.length; ++i){
                            var task = tasks[i];

                            var choords = mapFromItem(task,0, 0);

                            if( (task.objectName==="TaskItem") && (x>=choords.x) && (x<=choords.x+task.width)
                                    && (y>=choords.y) && (y<=choords.y+task.height)){
                                return task;
                            }
                        }

                        return null;
                    }

                    function childAtIndex(position) {
                        var tasks = icList.contentItem.children;

                        if (position < 0)
                            return;

                        for(var i=0; i<tasks.length; ++i){
                            var task = tasks[i];

                            if (task.lastValidIndex === position
                                    || (task.lastValidIndex === -1 && task.itemIndex === position )) {
                                return task;
                            }
                        }

                        return undefined;
                    }
                }
            } // ScrollPositioner
        } // ScrollableList

        TasksLayout.ScrollEdgeShadows {
            id: scrollShadows
            width: !root.vertical ? scrollableList.width : thickness
            height: !root.vertical ? thickness : scrollableList.height
            visible: scrollableList.contentsExceed

            flickable: scrollableList
        } // ScrollEdgeShadows

        Task.VisualAddItem{
            id: newDroppedLauncherVisual
            anchors.fill: mouseHandler

            visible: opacity == 0 ? false : true
            opacity: root.dropNewLauncher && mouseHandler.onlyLaunchers && (root.dragSource == null)? 1 : 0
        }

    }

    //// helpers

    Timer {
        id: iconGeometryTimer
        // INVESTIGATE: such big interval but unfortunately it does not work otherwise
        interval: 500
        repeat: false

        onTriggered: {
            root.publishTasksGeometries();

            if (latteView && latteView.debugModeTimers) {
                console.log("plasmoid timer: iconGeometryTimer called...");
            }
        }
    }


    ///REMOVE
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

        ///REMOVE
        onCountChanged: {
            /*  if(activityInfo.currentActivity != "00000000-0000-0000-0000-000000000000"){
                console.log("----------- Latte Plasmoid Signal: Activities number was changed ---------");
                var allActivities = activities();
                ActivitiesTools.cleanupRecords(allActivities);
                console.log("----------- Latte Plasmoid Signal End ---------");
            }*/
        }
    }

    Timer{
        id:launchersOrderValidatorTimer
        interval: 200

        property var launchers: []

        function launchersAreInSync() {
            return arraysAreEqual(currentListViewLauncherList(), launchers);
        }

        function launcherValidPos(url) {
            for (var i=0; i<launchers.length; ++i) {
                if (launchers[i] === url) {
                    return i;
                }
            }

            return -1;
        }

        function arraysAreEqual(list1, list2) {
            if (list1.length !== list2.length) {
                console.log("  arrays have different size...")
                return false;
            }

            for (var i=0; i<list1.length; ++i) {
                if (list1[i] !== list2[i]) {
                    return false;
                }
            }

            return true;
        }

        //! true if upward is the best way to iterate through current
        //! in order to make it equal with goal
        function upwardIsBetter(current, goal)
        {
            var tCurrent = current.slice();

            if (!arraysAreEqual(tCurrent, goal)) {
                for (var i=0; i<tCurrent.length; ++i) {
                    if (tCurrent[i] !== goal[i]) {
                        var val = tCurrent[i];
                        tCurrent.splice(i, 1);
                        tCurrent.splice(goal.indexOf(val), 0, val);

                        if (arraysAreEqual(tCurrent, goal)){
                            return true;
                        } else {
                            return false;
                        }
                    }
                }
            }

            return false;
        }


        onTriggered: {
            if (launchersAreInSync()) {
                stop();
                console.log("launchers synced at:" + launchers);
                launchers.length = 0;
                parabolicManager.updateTasksEdgesIndexes();
                root.separatorsUpdated();
                tasksModel.syncLaunchers();
            } else {
                var currentLaunchers = currentListViewLauncherList();

                if (upwardIsBetter(currentLaunchers, launchers)) {
                    console.log("UPWARD....");
                    for (var i=0; i<currentLaunchers.length; ++i) {
                        if (currentLaunchers[i] !== launchers[i]) {
                            var p = launcherValidPos(currentLaunchers[i]);
                            if (p === -1) {
                                console.log("No pos found for :"+currentLaunchers[i] + " at: "+launchers);
                                restart();
                                return;
                            }

                            console.log(" moving:" +i + " _ " + p );
                            tasksModel.move(i, p);
                            restart();
                            return;
                        }
                    }
                } else {
                    console.log("DOWNWARD....");
                    for (var i=currentLaunchers.length-1; i>=0; --i) {
                        if (currentLaunchers[i] !== launchers[i]) {
                            var p = launcherValidPos(currentLaunchers[i]);
                            if (p === -1) {
                                console.log("No pos found for :"+currentLaunchers[i] + " at: "+launchers);
                                restart();
                                return;
                            }

                            console.log(" moving:" +i + " _ " + p );
                            tasksModel.move(i, p);
                            restart();
                            return;
                        }
                    }
                }

                console.log("why we reached ??? ");
                console.log("CURRENT ::: " + currentLaunchers);
                console.log("VALID   ::: " + launchers);
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

            var zoomedLength = Math.floor( 1.2 * (iconSize+thickMargins) * (root.zoomFactor));
            var bigAxis = (tasksModel.count-1) * (iconSize+thickMargins) + zoomedLength;
            var smallAxis = zoomedLength;

            var clearBigAxis = tasksModel.count * (iconSize+thickMargins) + (barLine.spacing/2);
            var clearSmallAxis = iconSize+thickMargins;

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

    function addInternalSeparatorAtPos(pos) {
        var separatorName = parabolicManager.freeAvailableSeparatorName();

        if (separatorName !== "") {
            parabolicManager.addLauncherToBeMoved(separatorName, Math.max(0,pos));

            if (latteView && latteView.launchersGroup >= Latte.Types.LayoutLaunchers) {
                latteView.universalLayoutManager.launchersSignals.addLauncher(root.managedLayoutName,
                                                                              latteView.launchersGroup, separatorName);
            } else {
                tasksModel.requestAddLauncher(separatorName);
            }
        }
    }

    // This is called by dockcorona in response to a Meta+number shortcut.
    function activateTaskAtIndex(index) {
        if (typeof index !== "number") {
            return;
        }

        var tasks = icList.contentItem.children;

        //! this is used to bypass the internal separators if they exist
        var confirmedIndex = parabolicManager.realTaskIndex(index - 1);

        for(var i=0; i<tasks.length; ++i){
            var task = tasks[i];

            if (task.itemIndex === confirmedIndex) {
                if (task.isGroupParent) {
                    task.activateNextTask();
                } else {
                    task.activateTask();
                }
                break;
            }
        }
    }

    // This is called by dockcorona in response to a Meta+Alt+number shortcut.
    function newInstanceForTaskAtIndex(index) {
        if (typeof index !== "number") {
            return;
        }

        var tasks = icList.contentItem.children;

        //! this is used to bypass the internal separators if they exist
        var confirmedIndex = parabolicManager.realTaskIndex(index - 1);

        for(var i=0; i<tasks.length; ++i){
            var task = tasks[i];

            if (task.itemIndex === confirmedIndex) {
                TaskTools.activateTask(task.modelIndex(), task.m, Qt.ControlModifier , task);
                break;
            }
        }
    }

    function setHoveredIndex(ind) {
        icList.hoveredIndex = ind;
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

    function getBadger(identifier) {
        var ident1 = identifier;
        var n = ident1.lastIndexOf('/');

        var result = n>=0 ? ident1.substring(n + 1) : identifier;

        for(var i=0; i<badgers.length; ++i) {
            if (result.indexOf(badgers[i].id) >= 0) {
                return badgers[i];
            }
        }
    }

    function updateBadge(identifier, value) {
        var tasks = icList.contentItem.children;
        var identifierF = identifier.concat(".desktop");

        for(var i=0; i<tasks.length; ++i){
            var task = tasks[i];

            if (task && task.launcherUrl && task.launcherUrl.indexOf(identifierF) >= 0) {
                task.badgeIndicator = value === "" ? 0 : Number(value);
                var badge = getBadger(identifierF);
                if (badge) {
                    badge.value = value;
                } else {
                    badgers.push({id: identifierF, value: value});
                }
            }
        }
    }

    function getLauncherList() {
        return plasmoid.configuration.launchers59;
    }

    //! BEGIN ::: external launchers signals in order to update the tasks model
    function extSignalAddLauncher(group, launcher) {
        if (group === latteView.launchersGroup) {
            tasksModel.requestAddLauncher(launcher);
            launchersUpdatedFor(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function extSignalRemoveLauncher(group, launcher) {
        if (group === latteView.launchersGroup) {
            root.launcherForRemoval = launcher;
            tasksModel.requestRemoveLauncher(launcher);
            launchersUpdatedFor(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function extSignalAddLauncherToActivity(group, launcher, activity) {
        if (group === latteView.launchersGroup) {
            var launcherActivities = tasksModel.launcherActivities(launcher);

            if (activity !== tasksModel.activity && (launcherActivities[0] === "00000000-0000-0000-0000-000000000000")) {
                root.launcherForRemoval = launcher;
            }

            tasksModel.requestAddLauncherToActivity(launcher, activity);
            launchersUpdatedFor(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function extSignalRemoveLauncherFromActivity(group, launcher, activity) {
        if (group === latteView.launchersGroup) {
            if (activity === tasksModel.activity) {
                root.launcherForRemoval = launcher;
            }

            tasksModel.requestRemoveLauncherFromActivity(launcher, activity);
            launchersUpdatedFor(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function extSignalUrlsDropped(group, urls) {
        if (group === latteView.launchersGroup) {
            mouseHandler.urlsDroppedOnArea(urls);
        }
    }

    function extSignalMoveTask(group, from, to) {
        if (group === latteView.launchersGroup && !root.dragSource) {
            tasksModel.move(from, to);
            parabolicManager.updateTasksEdgesIndexes();
            root.separatorsUpdated();
            tasksModel.syncLaunchers();
        }
    }

    function extSignalValidateLaunchersOrder(group, launchers) {
        if (group === latteView.launchersGroup && !root.dragSource) {
            launchersOrderValidatorTimer.stop();
            launchersOrderValidatorTimer.launchers = launchers;
            launchersOrderValidatorTimer.start();
        }
    }

    //! END ::: external launchers signals in order to update the tasks model


    //! it is used to add the fake desktop file which represents
    //! the separator (fake launcher)
    function addSeparator(pos){
        var separatorName = parabolicManager.freeAvailableSeparatorName();

        if (separatorName !== "") {
            parabolicManager.addLauncherToBeMoved(separatorName, Math.max(0,pos));

            if (latteView && latteView.launchersGroup >= Latte.Types.LayoutLaunchers) {
                latteView.universalLayoutManager.launchersSignals.addLauncher(latteView.launchersGroup, separatorName);
            } else {
                tasksModel.requestAddLauncher(separatorName);
            }
        }
    }

    function removeLastSeparator(){
        var separatorName = parabolicManager.lastPresentSeparatorName();

        if (separatorName !== "") {
            if (latteView && latteView.launchersGroup >= Latte.Types.LayoutLaunchers) {
                latteView.universalLayoutManager.launchersSignals.removeLauncher(root.managedLayoutName,
                                                                                 latteView.launchersGroup, separatorName);
            } else {
                root.launcherForRemoval = separatorName;
                tasksModel.requestRemoveLauncher(separatorName);
            }
        }
    }

    //! show/hide tasks numbered badges e.g. from global shortcuts
    function setShowTaskShortcutBadges(showBadges){
        showTaskShortcutBadges = showBadges;
    }

    //! setup the tasks first index based on the fact that this is a plasmoid
    //! and applets could exist before it
    function setTasksBaseIndex(base){
        tasksBaseIndex = base;
    }

    function previewContainsMouse() {
        if(toolTipDelegate && toolTipDelegate.containsMouse && toolTipDelegate.parentTask) {
            return true;
        } else {
            return false;
        }
    }

    function containsMouse(){
        //console.log("s1...");
        if (disableRestoreZoom && (root.contextMenu || windowsPreviewDlg.visible)) {
            return;
        } else {
            disableRestoreZoom = false;
        }

        //if (previewContainsMouse())
        //  windowsPreviewDlg.hide(4);

        if (previewContainsMouse())
            return true;

        //console.log("s3...");
        var tasks = icList.contentItem.children;

        for(var i=0; i<tasks.length; ++i){
            var task = tasks[i];

            if(task && task.containsMouse){
                // console.log("Checking "+i+" - "+task.index+" - "+task.containsMouse);
                return true;
            }
        }

        return false;
    }

    function clearZoom(){
        //console.log("Plasmoid clear...");
        if (disableRestoreZoom && (root.contextMenu || windowsPreviewDlg.visible)) {
            return;
        } else {
            disableRestoreZoom = false;
        }

        if (!previewContainsMouse())
            windowsPreviewDlg.hide(4.2);

        if (!latteView) {
            initializeHoveredIndex();
        }

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
        launchersUpdatedFor(url);
        tasksModel.syncLaunchers();
    }

    function resetDragSource() {
        dragSource.z = 0;
        dragSource = null;
    }

    function setGlobalDirectRender(value) {
        if (waitingLaunchers.length > 0)
            return;

        if (latteView) {
            latteView.setGlobalDirectRender(value);
        } else {
            if (value === true) {
                if (root.containsMouse()) {
                    icList.directRender = true;
                } else {
                    //    console.log("direct render true ignored...");
                }
            } else {
                icList.directRender = false;
            }
        }
    }

    function startCheckRestoreZoomTimer(duration) {
        if (latteView) {
            latteView.startCheckRestoreZoomTimer();
        } else {
            if (duration > 0) {
                checkListHovered.startDuration(duration);
            } else {
                checkListHovered.startNormal();
            }
        }
    }

    function stopCheckRestoreZoomTimer() {
        if (latteView) {
            latteView.stopCheckRestoreZoomTimer();
        } else {
            checkListHovered.stop();
        }
    }

    function startDirectRenderDelayerDuringEntering(){
        if (latteView) {
            latteView.startDirectRenderDelayerDuringEntering();
        } else {
            directRenderDelayerForEnteringTimer.start();
        }
    }

    ///REMOVE
    /*function createContextMenu(task) {
        var menu = root.contextMenuComponent.createObject(task);
        menu.visualParent = task;
        menu.mpris2Source = mpris2Source;
        menu.activitiesCount = activityModelInstance.count;
        return menu;
    }*/

    function createContextMenu(rootTask, modelIndex, args) {
        var initialArgs = args || {}
        initialArgs.visualParent = rootTask;
        initialArgs.modelIndex = modelIndex;
        initialArgs.mpris2Source = mpris2Source;
        initialArgs.backend = backend;

        root.contextMenu = root.contextMenuComponent.createObject(rootTask, initialArgs);

        return root.contextMenu;
    }

    Component.onCompleted:  {
        updatePosition();

        root.presentWindows.connect(backend.presentWindows);
        root.windowsHovered.connect(backend.windowsHovered);
        dragHelper.dropped.connect(resetDragSource);
    }

    Component.onDestruction: {
        root.presentWindows.disconnect(backend.presentWindows);
        root.windowsHovered.disconnect(backend.windowsHovered);
        dragHelper.dropped.disconnect(resetDragSource);
    }

    //BEGIN states
    //user set Panel Positions
    // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Bottom Edge
        State {
            name: "bottomCenter"
            when: ((plasmoid.location===PlasmaCore.Types.BottomEdge || plasmoid.location===PlasmaCore.Types.Floating)
                   && root.userPanelPosition===Latte.Types.Center)

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
            when: ((plasmoid.location===PlasmaCore.Types.BottomEdge || plasmoid.location===PlasmaCore.Types.Floating)
                   && root.userPanelPosition===Latte.Types.Left)

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
            when: ((plasmoid.location===PlasmaCore.Types.BottomEdge || plasmoid.location===PlasmaCore.Types.Floating)
                   && root.userPanelPosition===Latte.Types.Right)

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
            when: (plasmoid.location===PlasmaCore.Types.TopEdge && root.userPanelPosition===Latte.Types.Center)

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
            when: (plasmoid.location===PlasmaCore.Types.TopEdge && root.userPanelPosition===Latte.Types.Left)

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
            when: (plasmoid.location===PlasmaCore.Types.TopEdge && root.userPanelPosition===Latte.Types.Right)

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
            when: (plasmoid.location===PlasmaCore.Types.LeftEdge && root.userPanelPosition===Latte.Types.Center)

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
            when: (plasmoid.location===PlasmaCore.Types.LeftEdge && root.userPanelPosition===Latte.Types.Top)

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
            when: (plasmoid.location===PlasmaCore.Types.LeftEdge && root.userPanelPosition===Latte.Types.Bottom)

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
            when: (plasmoid.location===PlasmaCore.Types.RightEdge && root.userPanelPosition===Latte.Types.Center)

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
            when: (plasmoid.location===PlasmaCore.Types.RightEdge && root.userPanelPosition===Latte.Types.Top)

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
            when: (plasmoid.location===PlasmaCore.Types.RightEdge && root.userPanelPosition===Latte.Types.Bottom)

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
