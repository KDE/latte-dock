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

import QtQuick 2.8
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

import org.kde.taskmanager 0.1 as TaskManager
import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import org.kde.activities 0.1 as Activities

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

import org.kde.latte.abilities.applets 0.1 as AppletAbility

import org.kde.latte.private.tasks 0.1 as LatteTasks

import "abilities" as Ability
import "previews" as Previews
import "task" as Task
import "taskslayout" as TasksLayout
import "../code/tools.js" as TaskTools
import "../code/activitiesTools.js" as ActivitiesTools
import "../code/ColorizerTools.js" as ColorizerTools

Item {
    id:root
    Layout.fillWidth: scrollingEnabled && !root.vertical
    Layout.fillHeight: scrollingEnabled && root.vertical

    Layout.minimumWidth: -1
    Layout.minimumHeight: -1
    Layout.preferredWidth: tasksWidth
    Layout.preferredHeight: tasksHeight
    Layout.maximumWidth: -1
    Layout.maximumHeight: -1

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft && !root.vertical
    LayoutMirroring.childrenInherit: true

    property bool plasma515: latteView ? latteView.plasma515 : LatteCore.Environment.plasmaDesktopVersion >= LatteCore.Environment.makeVersion(5,15,0)
    property bool plasma518: latteView ? latteView.plasma518 : LatteCore.Environment.plasmaDesktopVersion >= LatteCore.Environment.makeVersion(5,18,0)

    property bool editMode: latteView ? latteView.editMode : plasmoid.userConfiguring
    property bool inConfigureAppletsMode: latteView ? latteView.inConfigureAppletsMode : true
    property bool disableRestoreZoom: false //blocks restore animation in rightClick
    property bool disableAllWindowsFunctionality: plasmoid.configuration.hideAllTasks
    property bool dropNewLauncher: false
    property bool inActivityChange: false
    property bool inDraggingPhase: false
    property bool initializationStep: false //true
    property bool isHovered: false
    property bool showBarLine: plasmoid.configuration.showBarLine
    property bool useThemePanel: plasmoid.configuration.useThemePanel
    property bool taskInAnimation: noTasksInAnimation > 0 ? true : false
    property bool transparentPanel: plasmoid.configuration.transparentPanel
    property bool vertical: plasmoid.formFactor === PlasmaCore.Types.Vertical ? true : false

    property int clearWidth
    property int clearHeight

    property int newDroppedPosition: -1
    property int noInitCreatedBuffers: 0
    property int noTasksInAnimation: 0
    property int themePanelSize: plasmoid.configuration.panelSize

    property int location : {
        if (plasmoid.location === PlasmaCore.Types.LeftEdge
                || plasmoid.location === PlasmaCore.Types.RightEdge
                || plasmoid.location === PlasmaCore.Types.TopEdge) {
            return plasmoid.location;
        }

        return PlasmaCore.Types.BottomEdge;
    }

    property int tasksStarting: 0

    ///Don't use Math.floor it adds one pixel in animations and creates glitches
    property int widthMargins: root.vertical ? metrics.totals.thicknessEdges : metrics.totals.lengthEdges
    property int heightMargins: !root.vertical ? metrics.totals.thicknessEdges : metrics.totals.lengthEdges

    property int internalWidthMargins: root.vertical ? metrics.totals.thicknessEdges : metrics.totals.lengthPaddings
    property int internalHeightMargins: !root.vertical ? metrics.totals.thicknessEdges : metrics.totals.lengthPaddings

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

    //global plasmoid reference to the context menu
    property QtObject contextMenu: null
    property QtObject contextMenuComponent: Qt.createComponent("ContextMenu.qml");
    property Item dragSource: null

    property Item tasksExtendedManager: _tasksExtendedManager
    readonly property alias animations: _animations
    readonly property alias indexer: _indexer
    readonly property alias launchers: _launchers
    readonly property alias metrics: _metrics
    readonly property alias parabolic: _parabolic
    readonly property alias shortcuts: _shortcuts

    readonly property alias containsDrag: mouseHandler.containsDrag
    readonly property bool dragAreaEnabled: latteView ? (root.dragSource !== null
                                                         || latteView.dragInfo.isSeparator
                                                         || latteView.dragInfo.isTask
                                                         || !latteView.dragInfo.isPlasmoid)
                                                      : true

    //! it is used to play the animation correct when the user removes a launcher
    property string launcherForRemoval: ""

    //BEGIN Latte Dock properties
    property bool badges3DStyle: latteView ? latteView.badges3DStyle : true
    property bool dockIsShownCompletely: latteView ? latteView.dockIsShownCompletely : true
    property bool enableShadows: latteView ? latteView.enableShadows > 0 : plasmoid.configuration.showShadows
    property bool forceHidePanel: false

    property bool disableLeftSpacer: false
    property bool disableRightSpacer: false
    property bool dockIsHidden: latteView ? latteView.dockIsHidden : false
    property bool groupTasksByDefault: plasmoid.configuration.groupTasksByDefault
    property bool highlightWindows: hoverAction === LatteTasks.Types.HighlightWindows || hoverAction === LatteTasks.Types.PreviewAndHighlightWindows
    property bool parabolicEffectEnabled: latteView ? latteView.parabolicEffectEnabled : parabolic.factor.zoom && !root.editMode

    property bool scrollingEnabled: plasmoid.configuration.scrollTasksEnabled
    property bool autoScrollTasksEnabled: scrollingEnabled && plasmoid.configuration.autoScrollTasksEnabled
    property bool manualScrollTasksEnabled: scrollingEnabled &&  manualScrollTasksType !== LatteTasks.Types.ManualScrollDisabled
    property int manualScrollTasksType: plasmoid.configuration.manualScrollTasksType

    property bool showInfoBadge: plasmoid.configuration.showInfoBadge
    property bool showProgressBadge: plasmoid.configuration.showProgressBadge
    property bool showAudioBadge: plasmoid.configuration.showAudioBadge
    property bool infoBadgeProminentColorEnabled: plasmoid.configuration.infoBadgeProminentColorEnabled
    property bool audioBadgeActionsEnabled: plasmoid.configuration.audioBadgeActionsEnabled
    property bool showOnlyCurrentScreen: plasmoid.configuration.showOnlyCurrentScreen
    property bool showOnlyCurrentDesktop: plasmoid.configuration.showOnlyCurrentDesktop
    property bool showOnlyCurrentActivity: plasmoid.configuration.showOnlyCurrentActivity
    property bool showPreviews:  hoverAction === LatteTasks.Types.PreviewWindows || hoverAction === LatteTasks.Types.PreviewAndHighlightWindows
    property bool showWindowActions: plasmoid.configuration.showWindowActions && !disableAllWindowsFunctionality
    property bool showWindowsOnlyFromLaunchers: plasmoid.configuration.showWindowsOnlyFromLaunchers && !disableAllWindowsFunctionality

    property bool titleTooltips: latteView ? latteView.titleTooltips : false
    property alias windowPreviewIsShown: windowsPreviewDlg.visible

    property int launchersGroup: plasmoid.configuration.launchersGroup

    property int leftClickAction: plasmoid.configuration.leftClickAction
    property int middleClickAction: plasmoid.configuration.middleClickAction
    property int hoverAction: plasmoid.configuration.hoverAction
    property int modifier: plasmoid.configuration.modifier
    property int modifierClickAction: plasmoid.configuration.modifierClickAction
    property int modifierClick: plasmoid.configuration.modifierClick
    property int modifierQt:{
        if (modifier === LatteTasks.Types.Shift)
            return Qt.ShiftModifier;
        else if (modifier === LatteTasks.Types.Ctrl)
            return Qt.ControlModifier;
        else if (modifier === LatteTasks.Types.Alt)
            return Qt.AltModifier;
        else if (modifier === LatteTasks.Types.Meta)
            return Qt.MetaModifier;
        else return -1;
    }
    property int taskScrollAction: plasmoid.configuration.taskScrollAction

    onTaskScrollActionChanged: {
        if (taskScrollAction > LatteTasks.Types.ScrollToggleMinimized) {
            //! migrating scroll action to LatteTasks.Types.ScrollAction
            plasmoid.configuration.taskScrollAction = plasmoid.configuration.taskScrollAction-LatteTasks.Types.ScrollToggleMinimized;
        }
    }

    //! Real properties are need in order for parabolic effect to be 1px precise perfect.
    //! This way moving from Tasks to Applets and vice versa is pretty stable when hovering with parabolic effect.
    property real tasksHeight:  mouseHandler.height
    property real tasksWidth: mouseHandler.width

    //updated from Binding
    property int alignment

    readonly property real currentPanelOpacity: latteView ? latteView.currentPanelTransparency / 100 : 1

    property int appShadowSize: latteView ? latteView.appShadowSize : Math.ceil(0.12*metrics.iconSize)
    property string appShadowColor: latteView ? latteView.appShadowColor : "#ff080808"
    property string appShadowColorSolid: latteView ? latteView.appShadowColorSolid : "#ff080808"

    property alias tasksCount: tasksModel.count

    readonly property real screenGeometryHeightRatio: screenGeometry.height / screenGeometry.width
    readonly property rect screenGeometry: latteView ? latteView.screenGeometry : plasmoid.screenGeometry

    readonly property bool viewLayoutIsCurrent: latteView && viewLayout && latteView.layoutsManager
                                                && viewLayout.name === latteView.layoutsManager.currentLayoutName
    readonly property string viewLayoutName: viewLayout ? viewLayout.name : ""
    readonly property QtObject viewLayout : latteView && latteView.viewLayout ? latteView.viewLayout : null

    property Item latteView: null
    readonly property Item indicators: latteView ? latteView.indicatorsManager : indicatorsStandaloneLoader.item
    //END Latte Dock Panel properties

    readonly property bool inEditMode: latteInEditMode || plasmoid.userConfiguring

    //BEGIN Latte Dock Communicator
    property QtObject latteBridge: null

    readonly property bool enforceLattePalette: latteBridge && latteBridge.applyPalette && latteBridge.palette
    readonly property bool latteInEditMode: latteBridge && latteBridge.inEditMode
    //END  Latte Dock Communicator

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    signal draggingFinished();
    signal hiddenTasksUpdated();
    signal launchersUpdatedFor(string launcher);
    signal presentWindows(variant winIds);
    signal requestLayout;
    signal signalPreviewsShown();
    //signal signalDraggingState(bool value);
    signal showPreviewForTasks(QtObject group);
    //trigger updating scaling of neighbour delegates of zoomed delegate
    signal updateScale(int delegateIndex, real newScale, real step)
    signal mimicEnterForParabolic();
    signal publishTasksGeometries();
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
            plasmoid.configuration.isInLatteDock = true;

            if (root.launchersGroup === LatteCore.Types.LayoutLaunchers
                    || root.launchersGroup === LatteCore.Types.GlobalLaunchers) {
                tasksModel.updateLaunchersList();
            }
        } else {
            plasmoid.configuration.isInLatteDock = false;
        }
    }

    onLaunchersGroupChanged:{
        if(latteView) {
            tasksModel.updateLaunchersList();
        }
    }

    Connections {
        target: plasmoid
        onLocationChanged: {
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
    }


    Connections{
        target: latteView && latteView.layoutsManager ? latteView.layoutsManager : null

        onCurrentLayoutNameChanged: root.publishTasksGeometries();
    }

    Binding {
        target: plasmoid
        property: "status"
        value: {
            if (tasksModel.anyTaskDemandsAttentionInValidTime || root.dragSource) {
                return PlasmaCore.Types.NeedsAttentionStatus;
            }

            return PlasmaCore.Types.PassiveStatus;
        }
    }

    /////
    PlasmaCore.ColorScope{
        id: colorScopePalette
    }

    //!
    Item {
        id: graphicsSystem
        readonly property bool isAccelerated: (GraphicsInfo.api !== GraphicsInfo.Software)
                                              && (GraphicsInfo.api !== GraphicsInfo.Unknown)
    }

    Loader {
        id: indicatorsStandaloneLoader
        active: !latteView && !plasmoid.configuration.isInLatteDock
        source: "indicators/Manager.qml"
    }

    Binding {
        target: root
        property: "alignment"
        value: {
            if (latteView) {
                if (latteView.panelAlignment === -1) {
                    return;
                }

                if (latteView.panelAlignment === LatteCore.Types.Justify) {
                    if (latteView.latteAppletPos>=0 && latteView.latteAppletPos<100) {
                        return plasmoid.formFactor === PlasmaCore.Types.Horizontal ? LatteCore.Types.Left : LatteCore.Types.Top;
                    } else if (latteView.latteAppletPos>=100 && latteView.latteAppletPos<200) {
                        return LatteCore.Types.Center;
                    } else if (latteView.latteAppletPos>=200) {
                        return plasmoid.formFactor === PlasmaCore.Types.Horizontal ? LatteCore.Types.Right : LatteCore.Types.Bottom;
                    }

                    return LatteCore.Types.Center;
                }

                return latteView.panelAlignment;
            }

            return LatteCore.Types.Center;
        }
    }

    /////

    function launchersDropped(urls){
        mouseHandler.urlsDropped(urls);
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

        if (viewLayout) {
            if (latteView && latteView.layoutsManager
                    && latteView.viewLayout && latteView.universalSettings
                    && (root.launchersGroup === LatteCore.Types.LayoutLaunchers
                        || root.launchersGroup === LatteCore.Types.GlobalLaunchers)) {

                if (root.launchersGroup === LatteCore.Types.LayoutLaunchers) {
                    launchersList = latteView.viewLayout.launchers;
                } else if (root.launchersGroup === LatteCore.Types.GlobalLaunchers) {
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

    function forcePreviewsHiding(debug) {
        // console.log(" org.kde.latte   Tasks: Force hide previews event called: "+debug);
        windowsPreviewDlg.activeItem = null;
        windowsPreviewDlg.visible = false;
    }

    function hidePreview(){
        windowsPreviewDlg.hide(11);
    }

    onDragSourceChanged: {
        if (dragSource == null) {
            root.draggingFinished();
            tasksModel.syncLaunchers();

            restoreDraggingPhaseTimer.start();
        } else {
            inDraggingPhase = true;
        }
    }

    /////Window previews///////////

    Previews.ToolTipDelegate2 {
        id: toolTipDelegate
        visible: false
    }

    ////BEGIN interfaces

    LatteTasks.Dialog{
        id: windowsPreviewDlg
        type: plasmoid.configuration.previewWindowAsPopup ? PlasmaCore.Dialog.PopupMenu : PlasmaCore.Dialog.Tooltip
        flags: plasmoid.configuration.previewWindowAsPopup ? Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.Popup :
                                                             Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.ToolTip

        location: root.location
        mainItem: toolTipDelegate
        visible: false

        property bool signalSent: false
        property Item activeItem: null

        Component.onCompleted: mainItem.visible = true;

        onContainsMouseChanged: {
            //! Orchestrate restore zoom and previews window hiding. Both should be
            //! triggered together.
            if (containsMouse) {
                hidePreviewWinTimer.stop();
                parabolic.stopRestoreZoomTimer();
                parabolic.setDirectRenderingEnabled(false);
            } else {
                hide(7.3);
                parabolic.startRestoreZoomTimer();
            }
        }

        function hide(debug){
            //console.log("   Tasks: hide previews event called: "+debug);
            if (containsMouse || !visible) {
                return;
            }

            if (latteView && signalSent) {
                //it is used to unblock dock hiding
                signalSent = false;
            }

            if (!root.contextMenu) {
                root.disableRestoreZoom = false;
            }

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
                    signalSent = true;
                }

                //! Workaround in order to update properly the previews thumbnails
                //! when switching between single thumbnail to another single thumbnail
                //! maybe is not needed any more, let's disable it
                //mainItem.visible = false;
                visible = true;
                //mainItem.visible = true;
            }
        }
    }

    //! Delay windows previews hiding
    Timer {
        id: hidePreviewWinTimer
        interval: 300
        onTriggered: {
            //! Orchestrate restore zoom and previews window hiding. Both should be
            //! triggered together.
            var contains = (windowsPreviewDlg.containsMouse
                            || (windowsPreviewDlg.activeItem && windowsPreviewDlg.activeItem.containsMouse) /*main task*/
                            || (windowsPreviewDlg.activeItem /*dragging file(s) from outside*/
                                && mouseHandler.hoveredItem
                                && !root.dragSource
                                && mouseHandler.hoveredItem === windowsPreviewDlg.activeItem));

            if (!contains) {
                root.forcePreviewsHiding(9.9);
            }
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
        interval: 7 * (animations.speedFactor.current * animations.duration.small)

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

    onViewLayoutChanged: {
        if (viewLayout && !loadLaunchersFirstTime) {
            tasksModel.updateLaunchersList();
            loadLaunchersFirstTime = true;
        }
    }

    TaskManager.TasksModel {
        id: tasksModel

        virtualDesktop: virtualDesktopInfo.currentDesktop
        screenGeometry: root.screenGeometry
        // comment in order to support LTS Plasma 5.8
        // screen: plasmoid.screen
        activity: viewLayout ? viewLayout.lastUsedActivity : activityInfo.currentActivity

        filterByVirtualDesktop: root.showOnlyCurrentDesktop
        filterByScreen: root.showOnlyCurrentScreen
        filterByActivity: root.showOnlyCurrentActivity

        launchInPlace: true
        separateLaunchers: false
        groupInline: false

        groupMode: groupTasksByDefault ? TaskManager.TasksModel.GroupApplications : TaskManager.TasksModel.GroupDisabled
        sortMode: TaskManager.TasksModel.SortManual

        property bool anyTaskDemandsAttentionInValidTime: false

        function updateLaunchersList(){
            if (latteView
                    && (root.launchersGroup === LatteCore.Types.LayoutLaunchers
                        || root.launchersGroup === LatteCore.Types.GlobalLaunchers)) {
                if (root.launchersGroup === LatteCore.Types.LayoutLaunchers) {
                    console.log("Tasks: Applying LAYOUT Launchers List...");
                    tasksModel.launcherList = latteView.viewLayout.launchers;
                } else if (root.launchersGroup === LatteCore.Types.GlobalLaunchers) {
                    console.log("Tasks: Applying GLOBAL Launchers List...");
                    tasksModel.launcherList = latteView.universalSettings.launchers;
                }
            } else {
                console.log("Tasks: Applying UNIQUE Launchers List...");
                tasksModel.launcherList = plasmoid.configuration.launchers59;
            }
        }

        function launcherInCurrentActivity(url) {
            var activities = tasksModel.launcherActivities(url);

            var NULL_UUID = "00000000-0000-0000-0000-000000000000";

            if (activities.length === 0 || activities.indexOf(NULL_UUID) !== -1 || activities.indexOf(activityInfo.currentActivity) !== -1) {
                return true;
            }

            return false;
        }

        onActivityChanged: {
            ActivitiesTools.currentActivity = String(activity);
        }

        onLauncherListChanged: {
            if (viewLayout) {
                if (latteView && latteView.layoutsManager
                        && latteView.viewLayout && latteView.universalSettings
                        && (root.launchersGroup === LatteCore.Types.LayoutLaunchers
                            || root.launchersGroup === LatteCore.Types.GlobalLaunchers)) {

                    if (root.launchersGroup === LatteCore.Types.LayoutLaunchers) {
                        latteView.viewLayout.launchers = launcherList;
                    } else if (root.launchersGroup === LatteCore.Types.GlobalLaunchers) {
                        latteView.universalSettings.launchers = launcherList;
                    }

                    if (inDraggingPhase) {
                        if (latteView && root.launchersGroup >= LatteCore.Types.LayoutLaunchers) {
                            latteView.layoutsManager.launchersSignals.validateLaunchersOrder(root.viewLayoutName,
                                                                                             plasmoid.id,
                                                                                             root.launchersGroup,
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
                anyTaskDemandsAttentionInValidTime = true;
                attentionTimerComponent.createObject(root);
            }
        }


        Component.onCompleted: {
            ActivitiesTools.launchersOnActivities = root.launchersOnActivities
            ActivitiesTools.currentActivity = String(activityInfo.currentActivity);
            ActivitiesTools.plasmoid = plasmoid;

            //var loadedLaunchers = ActivitiesTools.restoreLaunchers();
            ActivitiesTools.importLaunchersToNewArchitecture();

            if (viewLayout && latteView.universalSettings
                    && (root.launchersGroup === LatteCore.Types.LayoutLaunchers
                        || root.launchersGroup === LatteCore.Types.GlobalLaunchers)) {

                if (root.launchersGroup === LatteCore.Types.LayoutLaunchers) {
                    launcherList = latteView.viewLayout.launchers;
                } else if (root.launchersGroup === LatteCore.Types.GlobalLaunchers) {
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
            if (LatteCore.Environment.plasmaDesktopVersion >= LatteCore.Environment.makeVersion(5,9,0)) {
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
        location: root.location
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
            if (LatteCore.Environment.frameworksVersion >= 335104 || (groupDialog !== undefined)) {
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
            if (!launcherUrl || launcherUrl === "") {
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
                if (!sourceData) {
                    continue;
                }

                if (sourceData.DesktopEntry === desktopFileName || (pid && sourceData.InstancePid === pid)) {
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
        function play(source) {
            startOperation(source, "Play");
        }
        function pause(source) {
            startOperation(source, "Pause");
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

    TasksExtendedManager {
        id: _tasksExtendedManager
    }

    Ability.Animations {
        id: _animations
        bridge: latteBridge
    }

    Ability.Indexer {
        id: _indexer
        bridge: latteBridge
        layout: icList.contentItem

        allItemsCount: tasksModel.count
    }

    Ability.Launchers {
        id: _launchers
    }

    Ability.Metrics {
        id: _metrics
        bridge: latteBridge
    }

    Ability.ParabolicEffect {
        id: _parabolic
        bridge: latteBridge
        local.restoreZoomIsBlocked: root.contextMenu || windowsPreviewDlg.containsMouse
    }

    Ability.PositionShortcuts {
        id: _shortcuts
        bridge: latteBridge
        isStealingGlobalPositionShortcuts: plasmoid.configuration.isPreferredForPositionShortcuts

        onDisabledIsStealingGlobalPositionShortcuts: {
            plasmoid.configuration.isPreferredForPositionShortcuts = false;
        }
    }

    AppletAbility.Requirements{
        id: _requires
        bridge: latteBridge

        activeIndicatorEnabled: false
        latteIconOverlayEnabled: false
        lengthMarginsEnabled: false
        latteSideColoringEnabled: false
        screenEdgeMarginSupported: true        
        innerZoomFactor: {
            if (!_animations.active || !LatteCore.WindowSystem.compositingActive) {
                return 1;
            }

            var hasHighThicknessAnimation = _animations.launcherBouncingEnabled || _animations.windowInAttentionEnabled || _animations.windowAddedInGroupEnabled;

            return hasHighThicknessAnimation ? 1.65 : 1.0;
        }
    }

    Component{
        id: attentionTimerComponent
        Timer{
            id: attentionTimer
            interval:8500
            onTriggered: {
                tasksModel.anyTaskDemandsAttentionInValidTime = false;
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

        width: root.vertical ? 1 : 2 * metrics.iconSize
        height: root.vertical ? 2 * metrics.iconSize : 1
        color: "red"
        x: (root.location === PlasmaCore.Types.LeftEdge) ? neededSpace : parent.width - neededSpace
        y: (root.location === PlasmaCore.Types.TopEdge) ? neededSpace : parent.height - neededSpace

        visible: plasmoid.configuration.zoomHelper

        property int neededSpace: parabolic.factor.zoom*metrics.totals.length
    }

    Item{
        id:barLine
        anchors.bottom: (root.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
        anchors.top: (root.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
        anchors.left: (root.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
        anchors.right: (root.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

        width: ( icList.orientation === Qt.Horizontal ) ? icList.width + spacing : smallSize
        height: ( icList.orientation === Qt.Vertical ) ? icList.height + spacing : smallSize

        property int spacing: latteBridge ? 0 : metrics.iconSize / 2
        property int smallSize: Math.max(0.10 * metrics.iconSize, 16)

        Behavior on opacity{
            NumberAnimation { duration: animations.speedFactor.current*animations.duration.large }
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
                NumberAnimation { duration: animations.speedFactor.current*animations.duration.large }
            }
        }


        /// item which is used as anchors for the plasma's theme
        Item{
            id:belower

            width: (root.location === PlasmaCore.Types.LeftEdge) ? shadowsSvgItem.margins.left : shadowsSvgItem.margins.right
            height: (root.location === PlasmaCore.Types.BottomEdge)? shadowsSvgItem.margins.bottom : shadowsSvgItem.margins.top

            anchors.top: (root.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
            anchors.bottom: (root.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
            anchors.right: (root.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
            anchors.left: (root.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined
        }


        /// the current theme's panel
        PlasmaCore.FrameSvgItem{
            id: shadowsSvgItem

            anchors.bottom: (root.location === PlasmaCore.Types.BottomEdge) ? belower.bottom : undefined
            anchors.top: (root.location === PlasmaCore.Types.TopEdge) ? belower.top : undefined
            anchors.left: (root.location === PlasmaCore.Types.LeftEdge) ? belower.left : undefined
            anchors.right: (root.location === PlasmaCore.Types.RightEdge) ? belower.right : undefined

            anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
            anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

            width: root.vertical ? panelSize + margins.left + margins.right: parent.width
            height: root.vertical ? parent.height : panelSize + margins.top + margins.bottom

            imagePath: "translucent/widgets/panel-background"
            prefix:"shadow"

            opacity: (plasmoid.configuration.showBarLine && plasmoid.configuration.useThemePanel && !root.forceHidePanel) ? 1 : 0
            visible: (opacity == 0) ? false : true

            property int panelSize: ((root.location === PlasmaCore.Types.BottomEdge) ||
                                     (root.location === PlasmaCore.Types.TopEdge)) ?
                                        plasmoid.configuration.panelSize + belower.height:
                                        plasmoid.configuration.panelSize + belower.width

            Behavior on opacity{
                NumberAnimation { duration: animations.speedFactor.current*animations.duration.large }
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
            anchors.bottom: (root.location === PlasmaCore.Types.BottomEdge) ? scrollableList.bottom : undefined
            anchors.top: (root.location === PlasmaCore.Types.TopEdge) ? scrollableList.top : undefined
            anchors.left: (root.location === PlasmaCore.Types.LeftEdge) ? scrollableList.left : undefined
            anchors.right: (root.location === PlasmaCore.Types.RightEdge) ? scrollableList.right : undefined

            anchors.horizontalCenter: !root.vertical ? scrollableList.horizontalCenter : undefined
            anchors.verticalCenter: root.vertical ? scrollableList.verticalCenter : undefined

            width: root.vertical ? maxSize : icList.width
            height: root.vertical ? icList.height : maxSize

            target: icList

            visible: root.dragAreaEnabled

            property int maxSize: (parabolic.local.lastIndex>=0 || windowPreviewIsShown || animations.hasThicknessAnimation) ?
                                      (parabolic.factor.maxZoom * metrics.totals.thickness) + metrics.margin.screenEdge :
                                      metrics.totals.thickness + metrics.margin.screenEdge

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
                if (latteView && root.launchersGroup >= LatteCore.Types.LayoutLaunchers && onlyLaunchersInList(urls)) {
                    latteView.layoutsManager.launchersSignals.urlsDropped(root.viewLayoutName,
                                                                          root.launchersGroup, urls);
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
            width: !root.vertical ? length : thickness
            height: root.vertical ? length : thickness
            contentWidth: icList.width
            contentHeight: icList.height

            property int thickness:0 // through Binding to avoid binding loops
            property int length:0 // through Binding to avoid binding loops

            //onCurrentPosChanged: console.log("CP :: "+ currentPos + " icW:"+icList.width + " rw: "+root.width + " w:" +width);

            layer.enabled: contentsExceed && root.scrollingEnabled
            layer.effect: OpacityMask {
                maskSource: TasksLayout.ScrollOpacityMask{
                    width: scrollableList.width
                    height: scrollableList.height
                }
            }

            Binding {
                target: scrollableList
                property: "thickness"
                when: latteView && !latteView.maskManager.inTempHiding
                value: {
                    if (latteView) {
                        return animations.hasThicknessAnimation ? latteView.maskManager.thicknessZoom : latteView.maskManager.thicknessNormal;
                    }

                    return metrics.totals.thickness * parabolic.factor.zoom;
                }
            }

            Binding {
                target: scrollableList
                property: "length"
                when: latteView && !latteView.maskManager.inTempHiding
                value: {
                    if (root.vertical) {
                        return Math.min(root.height, icList.height)
                    }

                    return Math.min(root.width, icList.width);
                }
            }

            TasksLayout.ScrollPositioner {
                id: listViewBase

                ListView {
                    id:icList
                    anchors.bottom: (root.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
                    anchors.top: (root.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
                    anchors.left: (root.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
                    anchors.right: (root.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

                    anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
                    anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

                    width: !root.vertical ? contentWidth : mouseHandler.maxSize
                    height: root.vertical ? contentHeight : mouseHandler.maxSize
                    boundsBehavior: Flickable.StopAtBounds
                    orientation: plasmoid.formFactor === PlasmaCore.Types.Vertical ? Qt.Vertical : Qt.Horizontal
                    delegate: Task.TaskItem{
                        animations: _animations
                        indexer: _indexer
                        launchers: _launchers
                        metrics: _metrics
                        parabolic: _parabolic
                        requires: _requires
                        shortcuts: _shortcuts
                    }

                    property int currentSpot : -1000
                    property int previousCount : 0

                    property int tasksCount: tasksModel.count

                    //the duration of this animation should be as small as possible
                    //it fixes a small issue with the dragging an item to change it's
                    //position, if the duration is too big there is a point in the
                    //list that an item is going back and forth too fast

                    //more of a trouble
                    moveDisplaced: Transition {
                        NumberAnimation { properties: "x,y"; duration: animations.speedFactor.current*animations.duration.large; easing.type: Easing.Linear }
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

                    function launcherModelIndex(url) {
                        var tasks = icList.contentItem.children;

                        for(var i=0; i<tasks.length; ++i){
                            var task = tasks[i];

                            if (task && (task.launcherUrl===url)) {
                                return task.itemIndex;
                            }
                        }

                        return -1;
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

        LatteComponents.AddingArea {
            id: newDroppedLauncherVisual
            width: root.vertical ? metrics.totals.thickness : scrollableList.width
            height: root.vertical ? scrollableList.height : metrics.totals.thickness

            visible: backgroundOpacity > 0
            radius: metrics.iconSize/10
            backgroundOpacity: root.dropNewLauncher && mouseHandler.onlyLaunchers && (root.dragSource == null)? 0.75 : 0
            duration: animations.speedFactor.current

            title: i18n("Tasks Area")

            states: [
                ///Bottom Edge
                State {
                    name: "left"
                    when: root.location===PlasmaCore.Types.LeftEdge

                    AnchorChanges {
                        target: newDroppedLauncherVisual
                        anchors{ top:undefined; bottom:undefined; left:scrollableList.left; right:undefined;
                            horizontalCenter:undefined; verticalCenter:scrollableList.verticalCenter}
                    }

                    PropertyChanges {
                        target: newDroppedLauncherVisual
                        anchors{ topMargin:0; bottomMargin:0; leftMargin:metrics.margin.screenEdge; rightMargin:0;}
                    }
                },
                State {
                    name: "right"
                    when: root.location===PlasmaCore.Types.RightEdge

                    AnchorChanges {
                        target: newDroppedLauncherVisual
                        anchors{ top:undefined; bottom:undefined; left:undefined; right:scrollableList.right;
                            horizontalCenter:undefined; verticalCenter:scrollableList.verticalCenter}
                    }

                    PropertyChanges {
                        target: newDroppedLauncherVisual
                        anchors{ topMargin:0; bottomMargin:0; leftMargin:0; rightMargin:metrics.margin.screenEdge;}
                    }
                },
                State {
                    name: "top"
                    when: root.location===PlasmaCore.Types.TopEdge

                    AnchorChanges {
                        target: newDroppedLauncherVisual
                        anchors{ top:scrollableList.top; bottom:undefined; left:undefined; right:undefined;
                            horizontalCenter:scrollableList.horizontalCenter; verticalCenter:undefined}
                    }

                    PropertyChanges {
                        target: newDroppedLauncherVisual
                        anchors{ topMargin:metrics.margin.screenEdge; bottomMargin:0; leftMargin:0; rightMargin:0;}
                    }
                },
                State {
                    name: "bottom"
                    when: root.location!==PlasmaCore.Types.TopEdge
                          && root.location !== PlasmaCore.Types.LeftEdge
                          && root.location !== PlasmaCore.Types.RightEdge

                    AnchorChanges {
                        target: newDroppedLauncherVisual
                        anchors{ top:undefined; bottom:scrollableList.bottom; left:undefined; right:undefined;
                            horizontalCenter:scrollableList.horizontalCenter; verticalCenter:undefined}
                    }

                    PropertyChanges {
                        target: newDroppedLauncherVisual
                        anchors{ topMargin:0; bottomMargin:metrics.margin.screenEdge; leftMargin:0; rightMargin:0;}
                    }
                }
            ]
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
        interval: 400

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
                            var launcherModelIndex = icList.launcherModelIndex(currentLaunchers[i]);

                            if (launcherModelIndex === -1) {
                                console.log(" launcher was not found in model, syncing stopped...");
                                stop();
                                return;
                            }

                            console.log(" moving:" +launcherModelIndex + " _ " + p );
                            tasksModel.move(launcherModelIndex, p);
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
                            var launcherModelIndex = icList.launcherModelIndex(currentLaunchers[i]);

                            if (launcherModelIndex === -1) {
                                console.log(" launcher was not found in model, syncing stopped...");
                                stop();
                                return;
                            }

                            console.log(" moving:" +launcherModelIndex + " _ " + p );
                            tasksModel.move(launcherModelIndex, p);
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
    function addInternalSeparatorAtPos(pos) {
        var separatorName = launchers.freeAvailableSeparatorName();

        if (separatorName !== "") {
            tasksExtendedManager.addLauncherToBeMoved(separatorName, Math.max(0,pos));

            if (latteView && root.launchersGroup >= LatteCore.Types.LayoutLaunchers) {
                latteView.layoutsManager.launchersSignals.addLauncher(root.viewLayoutName,
                                                                      root.launchersGroup, separatorName);
            } else {
                tasksModel.requestAddLauncher(separatorName);
            }
        }
    }

    function activateTaskAtIndex(index) {
        // This is called with Meta+number shortcuts by plasmashell when Tasks are in a plasma panel.
        shortcuts.sglActivateEntryAtIndex(index);
    }

    function newInstanceForTaskAtIndex(index) {
        // This is called with Meta+Alt+number shortcuts by plasmashell when Tasks are in a plasma panel.
        shortcuts.sglNewInstanceForEntryAtIndex(index);
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
        if (group === root.launchersGroup) {
            tasksModel.requestAddLauncher(launcher);
            launchersUpdatedFor(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function extSignalRemoveLauncher(group, launcher) {
        if (group === root.launchersGroup) {
            root.launcherForRemoval = launcher;
            tasksModel.requestRemoveLauncher(launcher);
            launchersUpdatedFor(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function extSignalAddLauncherToActivity(group, launcher, activity) {
        if (group === root.launchersGroup) {
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
        if (group === root.launchersGroup) {
            if (activity === tasksModel.activity) {
                root.launcherForRemoval = launcher;
            }

            tasksModel.requestRemoveLauncherFromActivity(launcher, activity);
            launchersUpdatedFor(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function extSignalUrlsDropped(group, urls) {
        if (group === root.launchersGroup) {
            mouseHandler.urlsDroppedOnArea(urls);
        }
    }

    function extSignalMoveTask(group, from, to) {
        if (group === root.launchersGroup && !root.dragSource) {
            tasksModel.move(from, to);
            tasksModel.syncLaunchers();
        }
    }

    function extSignalValidateLaunchersOrder(group, launchers) {
        if (group === root.launchersGroup && !root.dragSource) {
            launchersOrderValidatorTimer.stop();
            launchersOrderValidatorTimer.launchers = launchers;
            launchersOrderValidatorTimer.start();
        }
    }

    //! END ::: external launchers signals in order to update the tasks model


    //! it is used to add the fake desktop file which represents
    //! the separator (fake launcher)
    function addSeparator(pos){
        var separatorName = launchers.freeAvailableSeparatorName();

        if (separatorName !== "") {
            tasksExtendedManager.addLauncherToBeMoved(separatorName, Math.max(0,pos));

            if (latteView && root.launchersGroup >= LatteCore.Types.LayoutLaunchers) {
                latteView.layoutsManager.launchersSignals.addLauncher(root.launchersGroup, separatorName);
            } else {
                tasksModel.requestAddLauncher(separatorName);
            }
        }
    }

    function previewContainsMouse() {
        return windowsPreviewDlg.containsMouse;
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

    function hasLauncher(url) {
        return tasksModel.launcherPosition(url) != -1;
    }

    function addLauncher(url) {
        //workaround to protect in case the launcher contains the iconData
        var pos = url.indexOf("?iconData=");

        if (pos>0) {
            url = url.substring( 0, url.indexOf("?iconData=" ) );
        }

        var path = url;
        var filename = path.split("/").pop();
        tasksExtendedManager.addToBeAddedLauncher(filename);

        tasksModel.requestAddLauncher(url);
        launchersUpdatedFor(url);
        tasksModel.syncLaunchers();
    }

    function resetDragSource() {
        dragSource.z = 0;
        dragSource = null;
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
    // Alignments
    // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Bottom Edge
        State {
            name: "bottomCenter"
            when: (root.location===PlasmaCore.Types.BottomEdge && root.alignment===LatteCore.Types.Center)

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
            when: (root.location===PlasmaCore.Types.BottomEdge && root.alignment===LatteCore.Types.Left)

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
            when: (root.location===PlasmaCore.Types.BottomEdge && root.alignment===LatteCore.Types.Right)

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
            when: (root.location===PlasmaCore.Types.TopEdge && root.alignment===LatteCore.Types.Center)

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
            when: (root.location===PlasmaCore.Types.TopEdge && root.alignment===LatteCore.Types.Left)

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
            when: (root.location===PlasmaCore.Types.TopEdge && root.alignment===LatteCore.Types.Right)

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
            when: (root.location===PlasmaCore.Types.LeftEdge && root.alignment===LatteCore.Types.Center)

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
            when: (root.location===PlasmaCore.Types.LeftEdge && root.alignment===LatteCore.Types.Top)

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
            when: (root.location===PlasmaCore.Types.LeftEdge && root.alignment===LatteCore.Types.Bottom)

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
            when: (root.location===PlasmaCore.Types.RightEdge && root.alignment===LatteCore.Types.Center)

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
            when: (root.location===PlasmaCore.Types.RightEdge && root.alignment===LatteCore.Types.Top)

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
            when: (root.location===PlasmaCore.Types.RightEdge && root.alignment===LatteCore.Types.Bottom)

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
