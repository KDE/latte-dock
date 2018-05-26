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

import "task" as Task
import "../code/tools.js" as TaskTools
import "../code/activitiesTools.js" as ActivitiesTools

Item {
    id:root

    // Layout.fillHeight: userPanelPosition === 0 ? true : false
    // Layout.fillWidth: userPanelPosition === 0 ? true : false

    ///IMPORTANT: These values must be tested when the Now Dock Panel support
    ///also the four new anchors. A small issue is shown between the animation
    /// of the now dock plasmoid and the neighbour widgets...
    Layout.minimumWidth: (userPanelPosition !== 0)&&(!latteDock) ? clearWidth : -1
    Layout.minimumHeight: (userPanelPosition !== 0)&&(!latteDock) ? clearHeight : -1
    Layout.preferredWidth: (userPanelPosition !== 0)&&(!latteDock) ? tasksWidth : -1
    Layout.preferredHeight: (userPanelPosition !== 0)&&(!latteDock) ? tasksHeight : -1
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft && !root.vertical
    LayoutMirroring.childrenInherit: true

    property bool debugLocation: false

    //it is used to check both the applet and the containment for direct render
    property bool globalDirectRender: latteDock ? latteDock.globalDirectRender : icList.directRender

    property bool editMode: latteDock ? latteDock.editMode : plasmoid.userConfiguring
    property bool disableRestoreZoom: false //blocks restore animation in rightClick
    property bool disableAllWindowsFunctionality: root.showWindowsOnlyFromLaunchers && root.activeIndicator === Latte.Dock.NoneIndicator
    property bool dropNewLauncher: false
    readonly property bool hasInternalSeparator: parabolicManager.hasInternalSeparator
    property bool inActivityChange: false
    property bool inDraggingPhase: false
    property bool initializationStep: false //true
    property bool isHovered: false
    property bool showBarLine: plasmoid.configuration.showBarLine
    property bool showTasksNumbers: false
    property int tasksNumbersBase: 0
    property bool useThemePanel: plasmoid.configuration.useThemePanel
    property bool taskInAnimation: noTasksInAnimation > 0 ? true : false
    property bool transparentPanel: plasmoid.configuration.transparentPanel
    property bool vertical: ((root.position === PlasmaCore.Types.LeftPositioned) ||
                             (root.position === PlasmaCore.Types.RightPositioned)) ? true : false

    property int clearWidth
    property int clearHeight

    property int newLocationDebugUse: PlasmaCore.Types.BottomPositioned
    property int newDroppedPosition: -1
    property int noInitCreatedBuffers: 0
    property int noTasksInAnimation: 0
    property int themePanelSize: plasmoid.configuration.panelSize

    property int position : PlasmaCore.Types.BottomPositioned
    property int tasksStarting: 0
    property int realSize: iconSize + iconMargin

    ///Dont use Math.floor it adds one pixel in animations and creates glitches
    property int widthMargins: root.vertical ? thickMargin : iconMargin
    property int heightMargins: !root.vertical ? thickMargin : iconMargin


    // formula for luminance according to:
    // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
    property real textColorRs: {
        var color = theme.textColor.r;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    property real textColorGs: {
        var color = theme.textColor.g;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    property real textColorBs: {
        var color = theme.textColor.b;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    property real textColorLuma: 0.2126*textColorRs + 0.7152*textColorGs + 0.0722*textColorBs
    property color minimizedDotColor: textColorLuma > 0.6 ? Qt.darker(theme.textColor, 1.7) : Qt.lighter(theme.textColor, 7)

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
    property bool dockIsShownCompletely: latteDock ? latteDock.dockIsShownCompletely : true
    property bool enableShadows: latteDock ? latteDock.enableShadows > 0 : plasmoid.configuration.showShadows
    property bool forceHidePanel: false
    property bool directRenderDelayerIsRunning: latteDock ? latteDock.directRenderDelayerIsRunning : directRenderDelayerForEnteringTimer.running
    property bool disableLeftSpacer: false
    property bool disableRightSpacer: false
    property bool dockIsHidden: latteDock ? latteDock.dockIsHidden : false
    property bool groupTasksByDefault: latteDock ? latteDock.groupTasksByDefault: true
    property bool highlightWindows: latteDock ? latteDock.highlightWindows: plasmoid.configuration.highlightWindows
    property bool indicateAudioStreams: latteDock ? latteDock.indicateAudioStreams : plasmoid.configuration.indicateAudioStreams
    property bool mouseWheelActions: latteDock ? latteDock.mouseWheelActions : true
    property bool reverseLinesPosition: latteDock ? latteDock.reverseLinesPosition : plasmoid.configuration.reverseLinesPosition
    property bool dotsOnActive: latteDock ? latteDock.dotsOnActive : plasmoid.configuration.dotsOnActive
    property bool showGlow: latteDock ? latteDock.showGlow : plasmoid.configuration.showGlow
    property bool glow3D: latteDock ? latteDock.glow3D : false
    property bool showOnlyCurrentScreen: latteDock ? latteDock.showOnlyCurrentScreen : plasmoid.configuration.showOnlyCurrentScreen
    property bool showOnlyCurrentDesktop: latteDock ? latteDock.showOnlyCurrentDesktop : plasmoid.configuration.showOnlyCurrentDesktop
    property bool showOnlyCurrentActivity: latteDock ? latteDock.showOnlyCurrentActivity : plasmoid.configuration.showOnlyCurrentActivity
    property bool showPreviews:  latteDock ? latteDock.showToolTips : plasmoid.configuration.showToolTips
    property bool showWindowActions: latteDock ? latteDock.showWindowActions : plasmoid.configuration.showWindowActions
    property bool showWindowsOnlyFromLaunchers: latteDock ? latteDock.showWindowsOnlyFromLaunchers : false
    property bool smartLaunchersEnabled: latteDock ? latteDock.smartLaunchersEnabled : plasmoid.configuration.smartLaunchersEnabled
    property bool threeColorsWindows: latteDock ? latteDock.threeColorsWindows : plasmoid.configuration.threeColorsWindows
    property bool titleTooltips: latteDock ? latteDock.titleTooltips : false
    property alias windowPreviewIsShown: windowsPreviewDlg.visible

    property int activeIndicator: latteDock ? latteDock.activeIndicator : Latte.Dock.AllIndicator
    property int activeIndicatorType: latteDock ? latteDock.activeIndicatorType : Latte.Dock.LineIndicator
    property int animationStep: latteDock ? latteDock.animationStep : 1
    property int directRenderAnimationTime: latteDock ? latteDock.directRenderAnimationTime : 0
    property int dockHoveredIndex : latteDock ? latteDock.hoveredIndex : -1
    property int iconMargin: latteDock ? latteDock.iconMargin : 0.12*iconSize
    property int iconSize: latteDock ? latteDock.iconSize : Math.max(plasmoid.configuration.iconSize, 16)
    property int glowOption: latteDock ? latteDock.glowOption : Latte.Dock.GlowAll
    property real glowOpacity: latteDock ? latteDock.glowOpacity : 0.35

    property int middleClickAction: latteDock ? latteDock.middleClickAction : plasmoid.configuration.middleClickAction
    property int modifier: latteDock ? latteDock.modifier : -1
    property int modifierClickAction: latteDock ? latteDock.modifierClickAction : -1
    property int modifierClick: latteDock ? latteDock.modifierClick : -1
    property int modifierQt:{
        if (modifier === Latte.Dock.Shift)
            return Qt.ShiftModifier;
        else if (modifier === Latte.Dock.Ctrl)
            return Qt.ControlModifier;
        else if (modifier === Latte.Dock.Alt)
            return Qt.AltModifier;
        else if (modifier === Latte.Dock.Meta)
            return Qt.MetaModifier;
        else return -1;
    }

    //decouple iconMargin which now is used only for length calculations with thickMargins
    //which are used for thickness calculations
    property int thickMarginBase: latteDock ? latteDock.thickMarginBase : Math.ceil(iconMargin/2)
    property int thickMarginHigh: latteDock ? latteDock.thickMarginHigh : Math.ceil(iconMargin/2)
    property int thickMargin: thickMarginBase + thickMarginHigh

    property int statesLineSize: latteDock ? latteDock.statesLineSize : Math.ceil( root.iconSize/13 )
    property int tasksHeight: mouseHandler.height
    property int tasksWidth: mouseHandler.width
    property int userPanelPosition: latteDock ? latteDock.panelAlignment : plasmoid.configuration.plasmoidPosition

    property real durationTime: latteDock ? latteDock.durationTime : plasmoid.configuration.durationTime
    property real zoomFactor: latteDock ? latteDock.zoomFactor : ( 1 + (plasmoid.configuration.zoomLevel / 20) )

    property int appShadowSize: latteDock ? latteDock.appShadowSize : Math.ceil(0.12*iconSize)
    property string appShadowColor: latteDock ? latteDock.appShadowColor : "#ff080808"
    property string appShadowColorSolid: latteDock ? latteDock.appShadowColorSolid : "#ff080808"

    property alias tasksCount: tasksModel.count
    property alias hoveredIndex: icList.hoveredIndex

    property QtObject currentLayout : latteDock && latteDock.dockManagedLayout ? latteDock.dockManagedLayout : null
    property var managedLayoutName: currentLayout ? currentLayout.name : ""

    property Item latteDock: null
    //END Now Dock Panel properties


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
    onLatteDockChanged: {
        if (latteDock)
            plasmoid.configuration.isInLatteDock = true;
        else
            plasmoid.configuration.isInLatteDock = false;
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
        target: latteDock
        onDockIsHiddenChanged:{
            if (latteDock.dockIsHidden) {
                windowsPreviewDlg.hide("3.3");
            }
        }

        onLaunchersGroupChanged:{
            if( latteDock && latteDock.editMode) {
                tasksModel.updateLaunchersList();
            }
        }
    }


    Connections{
        target: latteDock && latteDock.universalLayoutManager ? latteDock.universalLayoutManager : null

        onCurrentLayoutNameChanged: root.publishTasksGeometries();
    }

    Connections{
        target: icList

        onHoveredIndexChanged:{
            if (latteDock && icList.hoveredIndex>-1){
                latteDock.setHoveredIndex(-1);
            }
        }
    }


    /////
    PlasmaCore.ColorScope{
        id: colorScopePalette
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
            restoreDraggingPhaseTimer.start();
            root.draggingFinished();
            root.signalActionsBlockHiding(-1);

            tasksModel.syncLaunchers();
        } else {
            inDraggingPhase = true;
            root.signalActionsBlockHiding(1);
        }
    }

    /////Window previews///////////

    ToolTipDelegate2 {
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

        property bool contextMenu: false
        property bool signalSent: false
        property Item activeItem: null

        onVisibleChanged: {
            if (visible) {
                windowsPreviewCheckerToNotShowTimer.start();
            }
        }

        function hide(debug){
            //console.log("on hide previews event called: "+debug);

            if (latteDock && signalSent) {
                //it is used to unblock dock hiding
                root.signalActionsBlockHiding(-1);
                signalSent = false;
                //root.signalDraggingState(false);
            }

            windowsPreviewDlg.activeItem = null;

            toolTipDelegate.containsMouse = false;
            toolTipDelegate.parentTask = null;
            toolTipDelegate.parentIndex = -1;
            toolTipDelegate.windows = [];
            toolTipDelegate.isGroup = false;

            if (!contextMenu)
                root.disableRestoreZoom = false;

            visible = false;
        }

        function show(taskItem){
            if (root.disableAllWindowsFunctionality) {
                return;
            }

            //console.log("preview show called...");
            if ((!activeItem || (activeItem !== taskItem)) && !root.contextMenu) {
                //console.log("preview show called: accepted...");

                //this can be used from others to hide their appearance
                //e.g but applets from the dock to hide themselves
                if (!visible) {
                    root.signalPreviewsShown();
                }

                //used to initialize windows previews buffers from task to task
                if (!Latte.WindowSystem.isPlatformWayland) {
                    visible = false;
                }
                activeItem = taskItem;
                toolTipDelegate.parentTask = taskItem;

                if (latteDock && !signalSent) {
                    //it is used to block dock hiding
                    root.signalActionsBlockHiding(1);
                    signalSent = true;
                    //root.signalDraggingState(true);
                }

                if (!Latte.WindowSystem.isPlatformWayland) {
                    showPreviewWinTimer.start();
                } else {
                    visible = true;
                }
            }
        }
    }

    //! I cant find another way to fix the issue with window thumbnails
    //! there are many cases that not correct previews are shown are
    //! no previews in cases that they should (X11 related)
    Timer {
        id: showPreviewWinTimer
        interval: 50
        onTriggered: {
            windowsPreviewDlg.visible = true;

            if (latteDock && latteDock.debugModeTimers) {
                console.log("plasmoid timer: showPreviewTimer called...");
            }
        }
    }

    //! Timer to fix #811, rare cases that both a window preview and context menu are
    //! shown
    Timer {
        id: windowsPreviewCheckerToNotShowTimer
        interval: 250

        onTriggered: {
            if (windowsPreviewDlg.visible && root.contextMenu) {
                windowsPreviewDlg.hide();
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

            if (latteDock && latteDock.debugModeTimers) {
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

            if (latteDock && latteDock.debugModeTimers) {
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
        screenGeometry: latteDock ? latteDock.screenGeometry : plasmoid.screenGeometry
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
            if (latteDock.universalSettings
                    && (latteDock.launchersGroup === Latte.Dock.LayoutLaunchers
                        || latteDock.launchersGroup === Latte.Dock.GlobalLaunchers)) {
                if (latteDock.launchersGroup === Latte.Dock.LayoutLaunchers) {
                    tasksModel.launcherList = latteDock.dockManagedLayout.launchers;
                } else if (latteDock.launchersGroup === Latte.Dock.GlobalLaunchers) {
                    tasksModel.launcherList = latteDock.universalSettings.launchers;
                }
            } else {
                tasksModel.launcherList = plasmoid.configuration.launchers59;
            }
        }

        onActivityChanged: {
            ActivitiesTools.currentActivity = String(activity);
        }

        onLauncherListChanged: {
            if (currentLayout) {
                if (latteDock && latteDock.universalLayoutManager
                        && latteDock.dockManagedLayout && latteDock.universalSettings
                        && (latteDock.launchersGroup === Latte.Dock.LayoutLaunchers
                            || latteDock.launchersGroup === Latte.Dock.GlobalLaunchers)) {

                    if (latteDock.launchersGroup === Latte.Dock.LayoutLaunchers) {
                        latteDock.dockManagedLayout.launchers = launcherList;
                    } else if (latteDock.launchersGroup === Latte.Dock.GlobalLaunchers) {
                        latteDock.universalSettings.launchers = launcherList;
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

            if (currentLayout && latteDock.universalSettings
                    && (latteDock.launchersGroup === Latte.Dock.LayoutLaunchers
                        || latteDock.launchersGroup === Latte.Dock.GlobalLaunchers)) {

                if (latteDock.launchersGroup === Latte.Dock.LayoutLaunchers) {
                    launcherList = latteDock.dockManagedLayout.launchers;
                } else if (latteDock.launchersGroup === Latte.Dock.GlobalLaunchers) {
                    launcherList = latteDock.universalSettings.launchers;
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
        active: root.indicateAudioStreams
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

                if (latteDock && latteDock.debugModeTimers) {
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
            if(root.latteDock)
                console.log("Plasmoid, checkListHoveredTimer was called, even though it shouldnt...");

            if (!root.containsMouse()) {

                icList.directRender = false;

                root.clearZoom();
            }

            interval = normalInterval;

            if (latteDock && latteDock.debugModeTimers) {
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

            property int maxSize: (((root.hoveredIndex>=0 || dockHoveredIndex>=0 ) || windowPreviewIsShown) && !root.dragSource) ?
                                      root.statesLineSize + root.zoomFactor * (root.iconSize + root.thickMargin) :
                                      root.statesLineSize + root.iconSize + root.thickMargin

            function urlsDroppedOnArea(urls){
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

            onUrlsDropped: {
                if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                    latteDock.universalLayoutManager.launchersSignals.urlsDropped(root.managedLayoutName,
                                                                                  latteDock.launchersGroup, urls);
                } else {
                    urlsDroppedOnArea(urls);
                }
            }
        }


        ListView {
            id:icList
            boundsBehavior: Flickable.StopAtBounds

            property int currentSpot : -1000
            property int hoveredIndex : -1
            property int previousCount : 0

            property int tasksCount: tasksModel.count

            property bool directRender: false

            //   onTasksCountChanged: updateImplicits();

            //  property int count: children ? children.length : 0
            /*   anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
            anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
            anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
            anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

            anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
            anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined  */

            width: !root.vertical ? contentWidth : mouseHandler.maxSize
            height: root.vertical ? contentHeight : mouseHandler.maxSize

            orientation: Qt.Horizontal

            delegate: Task.TaskDelegate{}

            /* Rectangle{
                anchors.fill: parent
                border.width: 1
                border.color: "red"
                color: "transparent"
            } */

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
            /* move:  Transition {
                NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.Linear }
            } */

            function childAtPos(x, y){
                var tasks = icList.contentItem.children;

                for(var i=0; i<tasks.length; ++i){
                    var task = tasks[i];

                    var choords = mapFromItem(task,0, 0);

                    if( (task.objectName==="TaskDelegate") && (x>=choords.x) && (x<=choords.x+task.width)
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
        // INVESTIGATE: such big interval but unfortunately it doesnot work otherwise
        interval: 500
        repeat: false

        onTriggered: {
            root.publishTasksGeometries();

            if (latteDock && latteDock.debugModeTimers) {
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

            var zoomedLength = Math.floor( 1.2 * (iconSize+thickMargin) * (root.zoomFactor));
            var bigAxis = (tasksModel.count-1) * (iconSize+thickMargin) + zoomedLength;
            var smallAxis = zoomedLength + statesLineSize;

            var clearBigAxis = tasksModel.count * (iconSize+thickMargin) + (barLine.spacing/2);
            var clearSmallAxis = (iconSize+thickMargin)+statesLineSize;

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

            if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                latteDock.universalLayoutManager.launchersSignals.addLauncher(root.managedLayoutName,
                                                                              latteDock.launchersGroup, separatorName);
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
        if (group === latteDock.launchersGroup) {
            tasksModel.requestAddLauncher(launcher);
            launchersUpdatedFor(launcher);
        }
    }

    function extSignalRemoveLauncher(group, launcher) {
        if (group === latteDock.launchersGroup) {
            root.launcherForRemoval = launcher;
            tasksModel.requestRemoveLauncher(launcher);
            launchersUpdatedFor(launcher);
        }
    }

    function extSignalAddLauncherToActivity(group, launcher, activity) {
        if (group === latteDock.launchersGroup) {
            var launcherActivities = tasksModel.launcherActivities(launcher);

            if (activity !== tasksModel.activity && (launcherActivities[0] === "00000000-0000-0000-0000-000000000000")) {
                root.launcherForRemoval = launcher;
            }

            tasksModel.requestAddLauncherToActivity(launcher, activity);
            launchersUpdatedFor(launcher);
        }
    }

    function extSignalRemoveLauncherFromActivity(group, launcher, activity) {
        if (group === latteDock.launchersGroup) {
            if (activity === tasksModel.activity) {
                root.launcherForRemoval = launcher;
            }

            tasksModel.requestRemoveLauncherFromActivity(launcher, activity);
            launchersUpdatedFor(launcher);
        }
    }

    function extSignalUrlsDropped(group, urls) {
        if (group === latteDock.launchersGroup) {
            mouseHandler.urlsDroppedOnArea(urls);
        }
    }

    function extSignalMoveTask(group, from, to) {
        if (group === latteDock.launchersGroup && !root.dragSource) {
            tasksModel.move(from, to);
            parabolicManager.updateTasksEdgesIndexes();
            root.separatorsUpdated();
        }
    }

    //! END ::: external launchers signals in order to update the tasks model


    //! it is used to add the fake desktop file which represents
    //! the separator (fake launcher)
    function addSeparator(pos){
        var separatorName = parabolicManager.freeAvailableSeparatorName();

        if (separatorName !== "") {
            parabolicManager.addLauncherToBeMoved(separatorName, Math.max(0,pos));

            if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                latteDock.universalLayoutManager.launchersSignals.addLauncher(latteDock.launchersGroup, separatorName);
            } else {
                tasksModel.requestAddLauncher(separatorName);
            }
        }
    }

    function removeLastSeparator(){
        var separatorName = parabolicManager.lastPresentSeparatorName();

        if (separatorName !== "") {
            if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                latteDock.universalLayoutManager.launchersSignals.removeLauncher(root.managedLayoutName,
                                                                                 latteDock.launchersGroup, separatorName);
            } else {
                root.launcherForRemoval = separatorName;
                tasksModel.requestRemoveLauncher(separatorName);
            }
        }
    }

    //! show/hide tasks numbered badges e.g. from global shortcuts
    function setShowTasksNumbers(showNumbers){
        showTasksNumbers = showNumbers;
    }

    //! setup the tasks first index based on the fact that this is a plasmoid
    //! and applets could exist before it
    function setTasksNumbersBase(base){
        tasksNumbersBase = base;
    }

    function previewContainsMouse() {
        if(toolTipDelegate && toolTipDelegate.containsMouse && toolTipDelegate.parentTask)
            return true;
        else
            return false;
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

        if (!latteDock) {
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
    }

    function resetDragSource() {
        dragSource.z = 0;
        dragSource = null;
    }

    function setGlobalDirectRender(value) {
        if (waitingLaunchers.length > 0)
            return;

        if (latteDock) {
            latteDock.setGlobalDirectRender(value);
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
        if (latteDock) {
            latteDock.startCheckRestoreZoomTimer();
        } else {
            if (duration > 0) {
                checkListHovered.startDuration(duration);
            } else {
                checkListHovered.startNormal();
            }
        }
    }

    function stopCheckRestoreZoomTimer() {
        if (latteDock) {
            latteDock.stopCheckRestoreZoomTimer();
        } else {
            checkListHovered.stop();
        }
    }

    function startDirectRenderDelayerDuringEntering(){
        if (latteDock) {
            latteDock.startDirectRenderDelayerDuringEntering();
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
