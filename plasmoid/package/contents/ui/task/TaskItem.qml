/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

import org.kde.latte.abilities.items 0.1 as AbilityItem

import "animations" as TaskAnimations

AbilityItem.BasicItem {
    id: taskItem
    visible: false
    objectName: "TaskItem"

    isHidden: !visible || isForcedHidden

    isHiddenSpacerForcedShow: taskItem.inAttentionBuiltinAnimation
    isHiddenSpacerAnimated: showWindowAnimation.running
                            || root.inActivityChange
                            || taskItem.inRemoveStage
                            || (taskItem.containsMouse && inAttentionBuiltinAnimation && taskItem.parabolicItem.zoom!==taskItem.abilities.parabolic.factor.zoom)

    isMonochromaticForcedContentItem: plasmoid.configuration.forceMonochromaticIcons
    monochromizedItem: taskIcon.monochromizedItem

    isSeparatorHidden: isSeparator && (lastValidIndex > taskItem.abilities.indexer.lastVisibleItemIndex)
    isSeparatorInRealLength: isSeparator && root.dragSource

    containsMouse: taskMouseArea.containsMouse || parabolicAreaContainsMouse
    thinTooltipText: {
        if (root.showPreviews && !isLauncher) {
            return "";
        }

        return isWindow ? model.display : model.AppName;
    }

    preserveIndicatorInInitialPosition: inBouncingAnimation || inAttentionBuiltinAnimation || inNewWindowBuiltinAnimation

    parabolicItem.isParabolicEventBlocked: root.dragSource
                                           || !hoverEnabled
                                           || !taskItem.abilities.myView.isShownFully
                                           || inAnimation
                                           || (inBlockingAnimation && !inAttentionBuiltinAnimation)
    parabolicItem.isUpdatingOnlySpacers: inAttentionBuiltinAnimation || inBouncingAnimation

    property alias hoverEnabled: taskMouseArea.hoverEnabled
    property alias pressed: taskMouseArea.pressed

    property bool delayingRemove: ListView.delayRemove
    //states that exist in windows in a Group of windows
    property bool hasActive: isActive
    property bool hasMinimized: (IsGroupParent === true) ? subWindows.hasMinimized : isMinimized
    property bool hasShown: (IsGroupParent === true) ? subWindows.hasShown : !isMinimized && isWindow
    property bool inAttention: isDemandingAttention && plasmoid.status === PlasmaCore.Types.NeedsAttentionStatus ? true : false

    /*animations flags*/
    property bool inAnimation: true
    property bool inAddRemoveAnimation: true
    property bool inAttentionBuiltinAnimation: false
    property bool inBlockingAnimation: false
    property bool inBouncingAnimation: false
    property bool inNewWindowBuiltinAnimation: false
    property bool inPopup: false
    property bool inRemoveStage: false

    property bool isLauncherBuiltinAnimationRunning: false
    property bool isLauncherAnimationRunning: isLauncherBuiltinAnimationRunning
                                              || (taskItem.abilities.indicators.info.providesTaskLauncherAnimation && isIndicatorTaskLauncherAnimationRunning)

    //! after clicking to show/hide preview enter events are trigerred even though the should not
    property bool showPreviewsIsBlockedFromReleaseEvent: false

    property bool isAbleToShowPreview: true
    property bool isActive: (IsActive === true) ? true : false
    property bool isDemandingAttention: (IsDemandingAttention === true) ? true : false
    property bool isDragged: false
    property bool isGroupable: (IsGroupable === true) ? true : false
    property bool isGroupParent: (IsGroupParent === true) ? true : false
    property bool isForcedHidden: false
    property bool isLauncher: (IsLauncher === true) ? true : false
    property bool hasShownLauncher:  (taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrl)
                                      || taskItem.abilities.launchers.inCurrentActivity(taskItem.launcherUrlWithIcon))
                                     && !root.inActivityChange /*update trigger when changing current activity*/
    property bool isMinimized: (IsMinimized === true) ? true : false
    property bool isStartup: (IsStartup === true) ? true : false
    property bool isWindow: (IsWindow === true) ? true : false

    property bool canPublishGeometries: (isWindow || isStartup || isGroupParent) && visible && width>=taskItem.abilities.metrics.iconSize && height>=taskItem.abilities.metrics.iconSize
                                        && !taskItem.delayingRemove
                                        && (taskItem.parabolicItem.zoom===1 || taskItem.parabolicItem.zoom===taskItem.abilities.parabolic.factor.zoom) //don't publish during zoom animation

    property bool hoveredFromDragging: (mouseHandler.hoveredItem === taskItem) || (mouseHandler.ignoredItem === taskItem)

    property bool wheelIsBlocked: false
    property bool hasAddedWaitingLauncher: false

    property int badgeIndicator: 0 //it is used from external apps
    property int lastValidIndex: -1 //used for the removal animation
    property int lastButtonClicked: -1;
    property int pressX: -1
    property int pressY: -1
    property int resistanceDelay: 450
    property int windowsCount: subWindows.windowsCount
    property int windowsMinimizedCount: subWindows.windowsMinimized

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
    readonly property alias subWindows: subWindows

    readonly property alias showWindowAnimation: _showWindowAnimation

    //! Indicator Properties
    indicator.isTask: true
    indicator.isLauncher: taskItem.isLauncher || root.disableAllWindowsFunctionality
    indicator.isStartup: !root.disableAllWindowsFunctionality && taskItem.isStartup
    indicator.isWindow: !root.disableAllWindowsFunctionality && taskItem.isWindow

    indicator.isActive: !root.disableAllWindowsFunctionality && (taskItem.hasActive
                                                                 || (root.showPreviews
                                                                     && (taskItem.isWindow || taskItem.isGroupParent)
                                                                     && windowsPreviewDlg.activeItem
                                                                     && (windowsPreviewDlg.activeItem === taskItem)) )

    indicator.isGroup: !root.disableAllWindowsFunctionality && taskItem.isGroupParent
    indicator.isHovered: taskItem.containsMouse || (windowsPreviewDlg.containsMouse && (toolTipDelegate.parentTask === taskItem))
    indicator.isMinimized: !root.disableAllWindowsFunctionality && taskItem.isMinimized
    indicator.isPressed: taskItem.pressed
    indicator.inAttention: !root.disableAllWindowsFunctionality && taskItem.inAttention
    indicator.inRemoving: taskItem.inRemoveStage

    indicator.isSquare: true

    indicator.hasActive: !root.disableAllWindowsFunctionality && taskItem.hasActive
    indicator.hasMinimized: !root.disableAllWindowsFunctionality && taskItem.hasMinimized
    indicator.hasShown: !root.disableAllWindowsFunctionality && taskItem.hasShown
    indicator.windowsCount: !root.disableAllWindowsFunctionality ? taskItem.windowsCount : 0
    indicator.windowsMinimizedCount: !root.disableAllWindowsFunctionality ? taskItem.windowsMinimizedCount : 0

    indicator.scaleFactor: taskItem.parabolicItem.zoom
    indicator.panelOpacity: taskItem.abilities.myView.backgroundOpacity
    indicator.shadowColor: taskItem.abilities.myView.itemShadow.shadowSolidColor

    indicator.progressVisible: taskIcon.progressVisible /*since 0.9.2*/
    indicator.progress: taskIcon.progress /*since 0.9.2*/

    indicator.palette: taskItem.abilities.myView.palette

    indicator.iconBackgroundColor: taskIcon.backgroundColor
    indicator.iconGlowColor: taskIcon.glowColor
    //! Indicator Properties

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

    //! Content Item
    contentItem: TaskIcon{
        id:taskIcon
    }
    //////

    property QtObject contextMenu: null

    signal checkWindowsStates();

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
                taskItem.taskGroupedWindowAdded();
            } else if ((windowsCount >= 1)
                       && (windowsCount < previousCount)
                       && !root.dragSource
                       && !taskItem.delayingRemove){
                //sometimes this is triggered in dragging with no reason
                taskItem.taskGroupedWindowRemoved();
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

    TaskMouseArea {
        id: taskMouseArea
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

    onShortcutRequestedActivate: {
        if (taskItem.isGroupParent) {
            taskItem.activateNextTask();
        } else {
            taskItem.activateTask();
        }
    }

    onShortcutRequestedNewInstance: {
        tasksModel.requestNewInstance(taskItem.modelIndex());
    }

    ////// End of Values Changes and Signals /////


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

    function handlerDraggingFinished(){
        isDragged = false;
    }
    ///// End of Handlers //////



    ///// Helper functions /////
    function activateNextTask() {
        subWindows.activateNextTask();
    }

    function activateLauncher() {
        if (LatteCore.WindowSystem.compositingActive) {
            taskItem.taskLauncherActivated();
            hasAddedWaitingLauncher = true;
            tasksExtendedManager.addWaitingLauncher(taskItem.launcherUrl);
        }

        if (root.disableAllWindowsFunctionality) {
            tasksModel.requestNewInstance(modelIndex());
        } else {
            tasksModel.requestActivate(modelIndex());
        }
    }

    function activateTask() {
        if( taskItem.isLauncher || root.disableAllWindowsFunctionality){
            activateLauncher();
        } else{
            if (model.IsGroupParent) {
                if (root.plasmaAtLeast525) {
                    //! At least Plasma 5.25 case
                    var isWindowViewAvailable = LatteCore.WindowSystem.compositingActive && backend.windowViewAvailable;
                    if (isWindowViewAvailable) {
                        root.activateWindowView(model.WinIdList);
                    }
                } else {
                    //! Plasma 5.24 case
                    var isPresentWindowsAvailable = LatteCore.WindowSystem.compositingActive && backend.canPresentWindows;
                    if (isPresentWindowsAvailable) {
                        root.presentWindows(model.WinIdList);
                    }
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
                taskItem.preparePreviewWindow(false);
                windowsPreviewDlg.show(taskItem);
            }
        }
    }

    function hidePreviewWindow() {
        if(windowsPreviewDlg.activeItem === taskItem){
            windowsPreviewDlg.hide("14.1");
        }
    }

    function preparePreviewWindow(hideClose){
        windowsPreviewDlg.visualParent = tooltipVisualParent;
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

    function slotShowPreviewForTasks(group) {
        if (group === taskItem && !windowsPreviewDlg.visible) {
            preparePreviewWindow(true);
            windowsPreviewDlg.show(taskItem);
        }
    }

    function slotPublishGeometries() {
        //! this way we make sure that layouts that are in different activities that the current layout
        //! don't publish their geometries
        if ( canPublishGeometries && (!taskItem.abilities.myView.isReady || (taskItem.abilities.myView.isReady && taskItem.abilities.myView.inCurrentLayout()))) {
            var globalChoords = backend.globalRect(taskItem.parabolicItem.contentItemContainer);
            var limits = backend.globalRect(scrollableList);

            //! Limit the published geometries boundaries at scrolling area boundaries
            var adjX = Math.min(limits.x+limits.width, Math.max(limits.x, globalChoords.x));
            var adjY = Math.min(limits.y+limits.height, Math.max(limits.y, globalChoords.y));

            var length = taskItem.abilities.metrics.iconSize * taskItem.parabolicItem.zoom;
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

            if (taskItem.abilities.myView.isHidden) {
                if (root.location === PlasmaCore.Types.BottomEdge) {
                    globalChoords.y = taskItem.abilities.myView.screenGeometry.y + taskItem.abilities.myView.screenGeometry.height-1;
                    globalChoords.height = 1;
                } else if (root.location === PlasmaCore.Types.TopEdge) {
                    globalChoords.y = taskItem.abilities.myView.screenGeometry.y+1;
                    globalChoords.height = 1;
                } else if (root.location === PlasmaCore.Types.LeftEdge) {
                    globalChoords.x = taskItem.abilities.myView.screenGeometry.x+1;
                    globalChoords.width = 1;
                } else if (root.location === PlasmaCore.Types.RightEdge) {
                    globalChoords.x = taskItem.abilities.myView.screenGeometry.x + taskItem.abilities.myView.screenGeometry.width - 1;
                    globalChoords.width = 1;
                }
            }

            tasksModel.requestPublishDelegateGeometry(taskItem.modelIndex(), globalChoords, taskItem);
        }
    }

    function slotWaitingLauncherRemoved(launch) {
        if ((isWindow || isStartup || isLauncher) && !visible && launch === launcherUrl) {
            if (!taskItem.abilities.indicators.info.providesTaskLauncherAnimation) {
                //! this is needed only from in-built launcher animation to restore zoom smoothly
                taskItem.parabolicItem.zoom = 1;
            }
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
                streams = pa.streamsForAppName(taskItem.appName);
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

    //fix bug #478, when changing form factor sometimes the tasks are not positioned
    //correctly, in such case we make a fast reinitialization for the sizes
    Connections {
        target: plasmoid
        onFormFactorChanged:{
            taskItem.inAddRemoveAnimation = false;
        }
    }

    Connections {
        target: root
        //trying to fix #440, showing the audio icon indicator to irrelevant tasks
        //after dragging an existent task with audio
        onDragSourceChanged: taskItem.updateAudioStreams()
        onShowAudioBadgeChanged: taskItem.updateAudioStreams()

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

    Connections {
        target: scrollableList
        onAnimationsFinishedChanged: {
            if (scrollableList.animationsFinished) {
                taskItem.slotPublishGeometries();
            }
        }
    }

    Connections {
        target: taskItem.abilities.myView
        onIsHiddenChanged: {
            if (taskItem.abilities.myView.isHidden) {
                taskItem.slotPublishGeometries();
            }
        }

        onIsShownFullyChanged: {
            if (taskItem.abilities.myView.isShownFully) {
                taskItem.slotPublishGeometries();
            }
        }
    }

    ///// End of Helper functions ////

    Component.onCompleted: {
        parabolicItem.opacity = 0;

        root.draggingFinished.connect(handlerDraggingFinished);
        root.publishTasksGeometries.connect(slotPublishGeometries);
        root.showPreviewForTasks.connect(slotShowPreviewForTasks);

        taskItem.abilities.launchers.launcherChanged.connect(onLauncherChanged);
        taskItem.abilities.launchers.launcherRemoved.connect(onLauncherChanged);

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

        taskItem.abilities.launchers.launcherChanged.disconnect(onLauncherChanged);
        taskItem.abilities.launchers.launcherRemoved.disconnect(onLauncherChanged);

        tasksExtendedManager.waitingLauncherRemoved.disconnect(slotWaitingLauncherRemoved);

        taskItem.parabolicItem.sendEndOfNeedBothAxisAnimation();
    }

    /////Animations
    TaskAnimations.ShowWindowAnimation{ id: _showWindowAnimation }

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

    onIsLauncherAnimationRunningChanged: {
        if (!isLauncherAnimationRunning && taskRealRemovalAnimation.paused) {
            taskRealRemovalAnimation.resume();
        }
    }
}// main Item

