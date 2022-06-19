/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.activities 0.1 as Activities
import org.kde.taskmanager 0.1 as TaskManager

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.tasks 0.1 as LatteTasks

import "../code/activitiesTools.js" as ActivitiesTools

PlasmaComponents.ContextMenu {
    id: menu

    property bool changingLayout: false

    property QtObject mpris2Source
    property QtObject backend

    property var modelIndex
    readonly property var atm: TaskManager.AbstractTasksModel

    placement: {
        if (root.location === PlasmaCore.Types.LeftEdge) {
            return PlasmaCore.Types.RightPosedTopAlignedPopup;
        } else if (root.location === PlasmaCore.Types.TopEdge) {
            return PlasmaCore.Types.BottomPosedLeftAlignedPopup;
        } else if (root.location === PlasmaCore.Types.RightEdge) {
            return PlasmaCore.Types.LeftPosedTopAlignedPopup;
        } else {
            return PlasmaCore.Types.TopPosedLeftAlignedPopup;
        }
    }

    minimumWidth: visualParent ? visualParent.width : 1

    property bool isOnAllActivitiesLauncher: true
    property bool showAllPlaces: false

    property int activitiesCount: 0

    readonly property string tailSeparatorText: plasmoid.formFactor === PlasmaCore.Types.Vertical ? i18n("Top Separator") :
                                                                                                    (!root.LayoutMirroring.enabled ? i18n("Left Separator") : i18n("Right Separator"))
    readonly property string headSeparatorText: plasmoid.formFactor === PlasmaCore.Types.Vertical ? i18n("Bottom Separator") :
                                                                                                    (!root.LayoutMirroring.enabled ? i18n("Right Separator") : i18n("Left Separator"))

    onStatusChanged: {
        if (visualParent && get(atm.LauncherUrlWithoutIcon) != null && status == PlasmaComponents.DialogStatus.Open) {
            launcherToggleAction.checked = (tasksModel.launcherPosition(get(atm.LauncherUrlWithoutIcon)) != -1);
            activitiesDesktopsMenu.refresh();
        } else if (status == PlasmaComponents.DialogStatus.Closed) {
            root.contextMenu = null;
            menu.destroy();
            backend.ungrabMouse(visualParent);
        }
    }

    function get(modelProp) {
        return tasksModel.data(modelIndex, modelProp)
    }

    function show() {
        loadDynamicLaunchActions(visualParent.m.LauncherUrlWithoutIcon);
        loadMyViewActions();
        // backend.ungrabMouse(visualParent);
        openRelative();

        if (LatteCore.WindowSystem.isPlatformWayland){
            //!Hiding previews under wayland it needs a delay otherwise it creates crashes
            windowsPreviewCheckerToNotShowTimer.start();
        } else {
            windowsPreviewDlg.hide("9.4");
        }
    }

    function newMenuItem(parent) {
        return Qt.createQmlObject(
                    "import org.kde.plasma.components 2.0 as PlasmaComponents;" +
                    "PlasmaComponents.MenuItem {}",
                    parent);
    }

    function newSeparator(parent) {
        return Qt.createQmlObject(
                    "import org.kde.plasma.components 2.0 as PlasmaComponents;" +
                    "PlasmaComponents.MenuItem { separator: true }",
                    parent);
    }

    function loadDynamicLaunchActions(launcherUrl) {
        var sections = [
                    {
                        title:   i18n("Places"),
                        group:   "places",
                        actions: backend.placesActions(launcherUrl, showAllPlaces, menu)
                    },
                    {
                        title:   i18n("Recent Files"),
                        group:   "recents",
                        actions: backend.recentDocumentActions(launcherUrl, menu)
                    },
                    {
                        title:   i18n("Actions"),
                        group:   "actions",
                        actions: backend.jumpListActions(launcherUrl, menu)
                    }
                ]

        // C++ can override section heading by returning a QString as first action
        sections.forEach((section) => {
                             if (typeof section.actions[0] === "string") {
                                 section.title = section.actions.shift(); // take first
                             }
                         });

        // QMenu does not limit its width automatically. Even if we set a maximumWidth
        // it would just cut off text rather than eliding. So we do this manually.
        var textMetrics = Qt.createQmlObject("import QtQuick 2.4; TextMetrics {}", menu);
        var maximumWidth = theme.mSize(theme.defaultFont).width * 22;

        sections.forEach(function (section) {
            if (section["actions"].length > 0 || section["group"] == "actions") {
                // Don't add the "Actions" header if the menu has nothing but actions
                // in it, because then it's redundant (all menus have actions)
                if (
                        (section["group"] != "actions") ||
                        (section["group"] == "actions" && (sections[0]["actions"].length > 0 || sections[1]["actions"].length > 0))
                        ) {
                    var sectionHeader = newMenuItem(menu);
                    sectionHeader.text = section["title"];
                    sectionHeader.section = true;
                    menu.addMenuItem(sectionHeader, startNewInstanceItem);
                }
            }

            for (var i = 0; i < section["actions"].length; ++i) {
                var item = newMenuItem(menu);
                item.action = section["actions"][i];

                // Crude way of manually eliding...
                var elided = false;
                textMetrics.text = Qt.binding(function() {
                    return item.action.text;
                });

                while (textMetrics.width > maximumWidth) {
                    item.action.text = item.action.text.slice(0, -1);
                    elided = true;
                }

                if (elided) {
                    item.action.text += "…";
                }

                menu.addMenuItem(item, startNewInstanceItem);
            }
        });

        // Add Media Player control actions
        var sourceName = mpris2Source.sourceNameForLauncherUrl(launcherUrl, get(atm.AppPid));

        var winIdList = atm.WinIdList;

        if (sourceName && !(get(winIdList) !== undefined && get(winIdList).length > 1)) {
            var playerData = mpris2Source.data[sourceName]

            if (playerData.CanControl) {
                var menuItem = menu.newMenuItem(menu);
                menuItem.text = i18nc("Play previous track", "Previous Track");
                menuItem.icon = "media-skip-backward";
                menuItem.enabled = Qt.binding(function() {
                    return playerData.CanGoPrevious;
                });
                menuItem.clicked.connect(function() {
                    mpris2Source.goPrevious(sourceName);
                });
                menu.addMenuItem(menuItem, virtualDesktopsMenuItem);

                menuItem = menu.newMenuItem(menu);
                // PlasmaCore Menu doesn't actually handle icons or labels changing at runtime...
                menuItem.text = Qt.binding(function() {
                    return playerData.PlaybackStatus === "Playing" ? i18nc("Pause playback", "Pause") : i18nc("Start playback", "Play");
                });
                menuItem.icon = Qt.binding(function() {
                    return playerData.PlaybackStatus === "Playing" ? "media-playback-pause" : "media-playback-start";
                });
                menuItem.enabled = Qt.binding(function() {
                    return playerData.PlaybackStatus === "Playing" ? playerData.CanPause : playerData.CanPlay;
                });
                menuItem.clicked.connect(function() {
                    mpris2Source.playPause(sourceName);
                });
                menu.addMenuItem(menuItem, virtualDesktopsMenuItem);

                menuItem = menu.newMenuItem(menu);
                menuItem.text = i18nc("Play next track", "Next Track");
                menuItem.icon = "media-skip-forward";
                menuItem.enabled = Qt.binding(function() {
                    return playerData.CanGoNext;
                });
                menuItem.clicked.connect(function() {
                    mpris2Source.goNext(sourceName);
                });
                menu.addMenuItem(menuItem, virtualDesktopsMenuItem);

                menuItem = menu.newMenuItem(menu);
                menuItem.text = i18nc("Stop playback", "Stop");
                menuItem.icon = "media-playback-stop";
                menuItem.clicked.connect(function() {
                    mpris2Source.stop(sourceName);
                });
                menu.addMenuItem(menuItem, virtualDesktopsMenuItem);

                // Technically media controls and audio streams are separate but for the user they're
                // semantically related, don't add a separator inbetween.
                if (!menu.visualParent.hasAudioStream) {
                    menu.addMenuItem(newSeparator(menu), virtualDesktopsMenuItem);
                }

                // If we don't have a window associated with the player but we can quit
                // it through MPRIS we'll offer a "Quit" option instead of "Close"
                if (!closeWindowItem.visible && playerData.CanQuit) {
                    menuItem = menu.newMenuItem(menu);
                    menuItem.text = i18nc("Quit media player app", "Quit");
                    menuItem.icon = "application-exit";
                    menuItem.visible = Qt.binding(function() {
                        return !closeWindowItem.visible;
                    });
                    menuItem.clicked.connect(function() {
                        mpris2Source.quit(sourceName);
                    });
                    menu.addMenuItem(menuItem);
                }

                // If we don't have a window associated with the player but we can raise
                // it through MPRIS we'll offer a "Restore" option
                if (!startNewInstanceItem.visible && playerData.CanRaise) {
                    menuItem = menu.newMenuItem(menu);
                    menuItem.text = i18nc("Open or bring to the front window of media player app", "Restore");
                    menuItem.icon = playerData["Desktop Icon Name"];
                    menuItem.visible = Qt.binding(function() {
                        return !startNewInstanceItem.visible;
                    });
                    menuItem.clicked.connect(function() {
                        mpris2Source.raise(sourceName);
                    });
                    menu.addMenuItem(menuItem, startNewInstanceItem);
                }
            }
        }

        // We allow mute/unmute whenever an application has a stream, regardless of whether it
        // is actually playing sound.
        // This way you can unmute, e.g. a telephony app, even after the conversation has ended,
        // so you still have it ringing later on.
        if (menu.visualParent.hasAudioStream) {
            var muteItem = menu.newMenuItem(menu);
            muteItem.checkable = true;
            muteItem.checked = Qt.binding(function() {
                return menu.visualParent && menu.visualParent.muted;
            });
            muteItem.clicked.connect(function() {
                menu.visualParent.toggleMuted();
            });
            muteItem.text = i18n("Mute");
            muteItem.icon = "audio-volume-muted";
            menu.addMenuItem(muteItem, virtualDesktopsMenuItem);

            menu.addMenuItem(newSeparator(menu), virtualDesktopsMenuItem);
        }
    }

    function loadMyViewActions() {
        if (!appletAbilities.myView.isReady) {
            return;
        }

        var actionsCount = appletAbilities.myView.containmentActions.length;

        for (var i=0; i<actionsCount; ++i) {
            var item = newMenuItem(menu);
            item.action = appletAbilities.myView.containmentActions[i];
            item.visible = Qt.binding(function() {
                return this.action.visible;
            });
            menu.addMenuItem(item, myViewActions);
        }
    }

    ///REMOVE
    function updateOnAllActivitiesLauncher(){
        //isOnAllActivitiesLauncher = ActivitiesTools.isOnAllActivities(visualParent.m.LauncherUrlWithoutIcon);
    }

    Component.onCompleted: {
        ActivitiesTools.launchersOnActivities = root.launchersOnActivities
        ActivitiesTools.currentActivity = activityInfo.currentActivity;
        ActivitiesTools.plasmoid = plasmoid;

        //From Plasma 5.10 and frameworks 5.34 jumpLists and
        //places are supported
        if (LatteCore.Environment.frameworksVersion >= 336384) {
            // Cannot have "Connections" as child of PlasmaCoponents.ContextMenu.
            backend.showAllPlaces.connect(function() {
                visualParent.showContextMenu({showAllPlaces: true});
            });
        }
        //  updateOnAllActivitiesLauncher();
    }


    Component.onDestruction: {
        if (!changingLayout) {
            root.contextMenu = null;
            backend.ungrabMouse(visualParent);
        }
    }

    /// Sub Items

    PlasmaComponents.MenuItem {
        id: startNewInstanceItem
        visible: (visualParent && visualParent.m.IsLauncher !== true && visualParent.m.IsStartup !== true)

        enabled: visualParent && visualParent.m.LauncherUrlWithoutIcon != null

        text: i18n("Start New Instance")
        icon: "list-add-symbolic"

        onClicked: tasksModel.requestNewInstance(menu.modelIndex)
    }

    PlasmaComponents.MenuItem {
        id: virtualDesktopsMenuItem

        visible: virtualDesktopInfo.numberOfDesktops > 1
                 && (visualParent && get(atm.IsLauncher) !== true
                     && get(atm.IsStartup) !== true
                     && get(atm.IsVirtualDesktopsChangeable) === true)

        enabled: visible

        text: i18n("Move To &Desktop")

        Connections {
            target: virtualDesktopInfo
            onDesktopIdsChanged: Qt.callLater(virtualDesktopsMenu.refresh)
            onDesktopNamesChanged: Qt.callLater(virtualDesktopsMenu.refresh)
            onNumberOfDesktopsChanged: Qt.callLater(virtualDesktopsMenu.refresh)
        }

        PlasmaComponents.ContextMenu {
            id: virtualDesktopsMenu

            visualParent: virtualDesktopsMenuItem.action

            function refresh() {
                clearMenuItems();

                if (virtualDesktopInfo.numberOfDesktops <= 1) {
                    return;
                }

                var menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("Move &To Current Desktop");
                menuItem.enabled = Qt.binding(function() {
                    var vds = menu.get(atm.VirtualDesktops);
                    return menu.visualParent && (vds !== undefined) && vds.indexOf(virtualDesktopInfo.currentDesktop) == -1;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktops(menu.modelIndex, [virtualDesktopInfo.currentDesktop]);
                });

                menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("&All Desktops");
                menuItem.checkable = true;
                menuItem.checked = Qt.binding(function() {
                    return menu.visualParent && menu.visualParent.m.IsOnAllVirtualDesktops === true;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktops(menu.modelIndex, []);
                });
                backend.setActionGroup(menuItem.action);

                menu.newSeparator(virtualDesktopsMenu);

                for (var i = 0; i < virtualDesktopInfo.desktopNames.length; ++i) {
                    menuItem = menu.newMenuItem(virtualDesktopsMenu);
                    //menuItem.text = i18nc("1 = number of desktop, 2 = desktop name", "%1 Desktop %2", i + 1, virtualDesktopInfo.desktopNames[i]);
                    menuItem.text = (i + 1) + ". " + virtualDesktopInfo.desktopNames[i];
                    menuItem.checkable = true;
                    menuItem.checked = Qt.binding((function(i) {
                        return function() {
                            return (menu.visualParent && menu.get(atm.VirtualDesktops).indexOf(virtualDesktopInfo.desktopIds[i]) > -1);
                        };
                    })(i));
                    menuItem.clicked.connect((function(i) {
                        return function() {
                            return tasksModel.requestVirtualDesktops(menu.modelIndex, [virtualDesktopInfo.desktopIds[i]]);
                        };
                    })(i));
                    backend.setActionGroup(menuItem.action);
                }

                menu.newSeparator(virtualDesktopsMenu);

                menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("&New Desktop");
                menuItem.clicked.connect(function() {
                    tasksModel.requestNewVirtualDesktop(menu.modelIndex);
                });
            }

            Component.onCompleted: refresh()
        }
    }


    PlasmaComponents.MenuItem {
        id: activitiesDesktopsMenuItem

        visible: activityInfo.numberOfRunningActivities > 1
                 && (visualParent && !visualParent.m.IsLauncher
                     && !visualParent.m.IsStartup)
                 && !root.disableAllWindowsFunctionality

        enabled: visible

        text: i18n("Move To &Activity")

        Connections {
            target: activityInfo

            onNumberOfRunningActivitiesChanged: activitiesDesktopsMenu.refresh()
        }

        PlasmaComponents.ContextMenu {
            id: activitiesDesktopsMenu

            visualParent: activitiesDesktopsMenuItem.action

            function refresh() {
                clearMenuItems();

                if (activityInfo.numberOfRunningActivities <= 1) {
                    return;
                }

                var menuItem = menu.newMenuItem(activitiesDesktopsMenu);
                menuItem.text = i18n("Add To Current Activity");
                menuItem.enabled = Qt.binding(function() {
                    return menu.visualParent && menu.visualParent.m.Activities.length > 0 &&
                            menu.visualParent.m.Activities.indexOf(activityInfo.currentActivity) < 0;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestActivities(menu.modelIndex, menu.visualParent.m.Activities.concat(activityInfo.currentActivity));
                });

                menuItem = menu.newMenuItem(activitiesDesktopsMenu);
                menuItem.text = i18n("All Activities");
                menuItem.checkable = true;
                menuItem.checked = Qt.binding(function() {
                    return menu.visualParent && menu.visualParent.m.Activities.length === 0;
                });
                menuItem.clicked.connect(function() {
                    var checked = menuItem.checked;
                    var newActivities = menu.visualParent.m.Activities;
                    var size = newActivities.length;

                    newActivities = undefined; // will cast to an empty QStringList i.e all activities
                    if (size === 0) {
                        newActivities = new Array(activityInfo.currentActivity);
                    }

                    tasksModel.requestActivities(menu.modelIndex, newActivities);
                });

                menu.newSeparator(activitiesDesktopsMenu);

                var runningActivities = activityInfo.runningActivities();
                for (var i = 0; i < runningActivities.length; ++i) {
                    var activityId = runningActivities[i];

                    menuItem = menu.newMenuItem(activitiesDesktopsMenu);
                    menuItem.text = activityInfo.activityName(runningActivities[i]);
                    menuItem.checkable = true;
                    menuItem.checked = Qt.binding( (function(activityId) {
                        return function() {
                            return menu.visualParent && menu.visualParent.m.Activities.indexOf(activityId) >= 0;
                        };
                    })(activityId));
                    menuItem.clicked.connect((function(activityId) {
                        return function () {
                            var checked = menuItem.checked;
                            var newActivities = menu.visualParent.m.Activities;
                            var index = newActivities.indexOf(activityId)

                            if (index < 0) {
                                newActivities = newActivities.concat(activityId);
                            } else {
                                //newActivities = newActivities.splice(index, 1);  //this does not work!!!
                                newActivities.splice(index, 1);
                            }
                            return tasksModel.requestActivities(menu.modelIndex, newActivities);
                        };
                    })(activityId));
                }

                menu.newSeparator(activitiesDesktopsMenu);
            }

            Component.onCompleted: refresh()
        }
    }

    PlasmaComponents.MenuItem {
        id: moreActionsMenuItem

        visible: (visualParent
                  && visualParent.m.IsLauncher !== true
                  && visualParent.m.IsStartup !== true
                  && root.showWindowActions
                  && !root.disableAllWindowsFunctionality)

        enabled: visible

        text: i18n("More Actions")
        icon: "view-more-symbolic"

        PlasmaComponents.ContextMenu {
            visualParent: moreActionsMenuItem.action

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.m.IsMovable === true

                text: i18n("&Move")
                icon: "transform-move"

                onClicked: tasksModel.requestMove(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.m.IsResizable === true

                text: i18n("Re&size")
                icon: "transform-scale"

                onClicked: tasksModel.requestResize(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                visible: (visualParent
                          && visualParent.m.IsLauncher !== true
                          && visualParent.m.IsStartup !== true
                          && root.showWindowActions
                          && !root.disableAllWindowsFunctionality)

                enabled: visualParent && visualParent.m.IsMaximizable === true

                checkable: true
                checked: visualParent && visualParent.m.IsMaximized === true

                text: i18n("Ma&ximize")
                icon: "window-maximize"

                onClicked: tasksModel.requestToggleMaximized(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                visible: (visualParent
                          && visualParent.m.IsLauncher !== true
                          && visualParent.m.IsStartup !== true
                          && root.showWindowActions
                          && !root.disableAllWindowsFunctionality)

                enabled: visualParent && visualParent.m.IsMinimizable === true

                checkable: true
                checked: visualParent && visualParent.m.IsMinimized === true

                text: i18n("Mi&nimize")
                icon: "window-minimize"

                onClicked: tasksModel.requestToggleMinimized(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsKeepAbove === true

                text: i18n("Keep &Above Others")
                icon: "window-keep-above"

                onClicked: tasksModel.requestToggleKeepAbove(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsKeepBelow === true

                text: i18n("Keep &Below Others")
                icon: "window-keep-below"

                onClicked: tasksModel.requestToggleKeepBelow(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.m.IsFullScreenable === true

                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsFullScreen === true

                text: i18n("&Fullscreen")
                icon: "view-fullscreen"

                onClicked: tasksModel.requestToggleFullScreen(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.m.IsShadeable === true

                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsShaded === true

                text: i18n("&Shade")
                icon: "window-shade"

                onClicked: tasksModel.requestToggleShaded(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                separator: true
            }

            PlasmaComponents.MenuItem {
                visible: (plasmoid.configuration.groupingStrategy !== 0) && menu.visualParent.m.IsWindow === true

                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsGroupable === true

                text: i18n("Allow this program to be grouped")

                onClicked: tasksModel.requestToggleGrouping(menu.modelIndex)
            }
        }
    }

    /*    PlasmaComponents.MenuItem {
        separator: true

        visible: (visualParent
                  && visualParent.m.IsLauncher !== true
                  && visualParent.m.IsStartup !== true
                  && root.showWindowActions)
    }*/

    //// NEW Launchers Mechanism
    PlasmaComponents.MenuItem {
        id: launcherToggleAction

        visible: visualParent
                 && get(atm.IsLauncher) !== true
                 && get(atm.IsStartup) !== true
                 && (activityInfo.numberOfRunningActivities < 2)
        //&& plasmoid.immutability !== PlasmaCore.Types.SystemImmutable

        enabled: visualParent && get(atm.LauncherUrlWithoutIcon) !== ""

        checkable: true

        text: i18n("&Pin Launcher")
        icon: "window-pin"

        onClicked: {
            if (tasksModel.launcherPosition(get(atm.LauncherUrlWithoutIcon)) !== -1) {
                appletAbilities.launchers.removeLauncher(get(atm.LauncherUrl));
            } else {
                appletAbilities.launchers.addLauncher(get(atm.LauncherUrl))
            }
        }
    }

    PlasmaComponents.MenuItem {
        id: showLauncherInActivitiesItem

        text: i18n("&Pin Launcher")
        icon: "window-pin"

        visible: visualParent && (!visualParent.isSeparator || (visualParent.isSeparator && root.inEditMode))
        // && get(atm.IsLauncher) !== true
                 && get(atm.IsStartup) !== true
                 && plasmoid.immutability !== PlasmaCore.Types.SystemImmutable
                 && (activityInfo.numberOfRunningActivities >= 2)

        Connections {
            target: activityInfo
            onNumberOfRunningActivitiesChanged: activitiesDesktopsMenu.refresh()
        }

        PlasmaComponents.ContextMenu {
            id: activitiesLaunchersMenu
            visualParent: showLauncherInActivitiesItem.action

            function refresh() {
                clearMenuItems();

                if (menu.visualParent === null) return;

                var createNewItem = function(id, title, url, activities) {
                    var result = menu.newMenuItem(activitiesLaunchersMenu);
                    result.text = title;

                    result.visible = true;
                    result.checkable = true;

                    result.checked = activities.some(function(activity) { return activity === id });

                    result.clicked.connect(
                                function() {
                                    if (result.checked) {
                                        appletAbilities.launchers.addLauncherToActivity(url,id);
                                    } else {
                                        appletAbilities.launchers.removeLauncherFromActivity(url, id);
                                    }
                                }
                                );

                    return result;
                }

                if (menu.visualParent === null) return;

                var url = menu.get(atm.LauncherUrlWithoutIcon);

                var activities = tasksModel.launcherActivities(url);

                var NULL_UUID = "00000000-0000-0000-0000-000000000000";

                createNewItem(NULL_UUID, i18n("On All Activities"), url, activities);

                if (activityInfo.numberOfRunningActivities <= 1) {
                    return;
                }

                createNewItem(activityInfo.currentActivity, i18n("On The Current Activity"), url, activities);

                menu.newSeparator(activitiesLaunchersMenu);

                var runningActivities = activityInfo.runningActivities();

                runningActivities.forEach(function(id) {
                    createNewItem(id, activityInfo.activityName(id), url, activities);
                });
            }

            Component.onCompleted: {
                menu.onVisualParentChanged.connect(refresh);
                refresh();
            }
        }
    }

    PlasmaComponents.MenuItem {
        visible: (visualParent && !visualParent.isSeparator && get(atm.IsLauncher) === true)
                 && plasmoid.immutability !== PlasmaCore.Types.SystemImmutable

        text: i18n("Unpin Launcher")
        icon: "window-unpin"

        onClicked: {
            appletAbilities.launchers.removeLauncher(get(atm.LauncherUrlWithoutIcon));
        }
    }

    //////END OF NEW ARCHITECTURE

    PlasmaComponents.MenuItem {
        id: addInternalSeparatorItem
        enabled: !visualParent.tailItemIsSeparator || !visualParent.headItemIsSeparator
        visible: visualParent.hasShownLauncher
        icon: "add"
        text: !visualParent.tailItemIsSeparator ? i18nc("add separator","Add %1", tailSeparatorText) : i18nc("add separator","Add %1", headSeparatorText)

        onClicked: {
            var pos=visualParent.itemIndex;

            if (!visualParent.tailItemIsSeparator) {
                appletAbilities.launchers.addInternalSeparatorAtPos(pos);
            } else {
                appletAbilities.launchers.addInternalSeparatorAtPos(pos+1);
            }
        }
    }

    PlasmaComponents.MenuItem {
        id: removeFollowingInternalSeparatorItem
        visible: visualParent && visualParent.headItemIsSeparator

        icon: "remove"
        text: i18nc("remove separator", "Remove %1", headSeparatorText)

        onClicked: {
            if (visualParent.headItemIsSeparator) {
                appletAbilities.launchers.removeInternalSeparatorAtPos(visualParent.itemIndex + 1);
            }
        }
    }

    PlasmaComponents.MenuItem {
        id: removeTailInternalSeparatorItem
        visible: visualParent && visualParent.tailItemIsSeparator

        icon: "remove"
        text: i18nc("remove separator", "Remove %1", tailSeparatorText)

        onClicked: {
            if (visualParent.tailItemIsSeparator) {
                appletAbilities.launchers.removeInternalSeparatorAtPos(visualParent.itemIndex - 1);
            }
        }
    }

    PlasmaComponents.MenuItem {
        id: alternativesMenuItem
        visible: (appletAbilities.myView.isReady && appletAbilities.myView.inEditMode)
                 || (!appletAbilities.myView.isReady && plasmoid.userConfiguring /*normal plasmoid in the desktop*/)
        text: plasmoid.action("alternatives").text
        icon: plasmoid.action("alternatives").icon

        onClicked: plasmoid.action("alternatives").trigger();
    }

    PlasmaComponents.MenuItem {
        id: myViewActions
        separator: true
        visible: false
    }

    PlasmaComponents.MenuItem {
        separator: true
        visible: removePlasmoidInMyViewEditMode.visible
    }

    PlasmaComponents.MenuItem {
        id: removePlasmoidInMyViewEditMode
        //! Workaround: this is preferred compared to:
        //!   action:plasmoid.action("remove")
        //! which shows the action always and not dependent of myView.inEditMode flag
        text: plasmoid.action("remove").text
        icon: plasmoid.action("remove").icon
        visible: appletAbilities.myView.isReady && appletAbilities.myView.inEditMode

        onClicked: plasmoid.action("remove").trigger();
    }

    PlasmaComponents.MenuItem {
        section: true
        text: i18n("Window")
        visible: closeWindowItem.visible
    }

    //!move window Close button at the very bottom in order to not alter users workflow
    //!comparing with the design decisions of other taskmanagers
    PlasmaComponents.MenuItem {
        id: closeWindowItem
        visible: (visualParent && visualParent.m.IsLauncher !== true && visualParent.m.IsStartup !== true) && !root.disableAllWindowsFunctionality

        enabled: visualParent && visualParent.m.IsClosable === true

        text: i18n("&Close")
        icon: "window-close"

        onClicked: {
            if (appletAbilities.parabolic.factor.zoom>1) {
                delayWindowRemovalTimer.modelIndex = menu.modelIndex;
                delayWindowRemovalTimer.start();
            } else {
                tasksModel.requestClose(menu.modelIndex);
            }
        }
    }
}
