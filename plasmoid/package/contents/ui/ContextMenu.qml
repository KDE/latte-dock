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

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.activities 0.1 as Activities
import org.kde.taskmanager 0.1 as TaskManager

import org.kde.latte 0.1 as Latte

import "../code/activitiesTools.js" as ActivitiesTools

PlasmaComponents.ContextMenu {
    id: menu

    property bool changingLayout: false

    property QtObject mpris2Source
    property QtObject backend

    property var modelIndex
    readonly property var atm: TaskManager.AbstractTasksModel

    placement: {
        if (plasmoid.location == PlasmaCore.Types.LeftEdge) {
            return PlasmaCore.Types.RightPosedTopAlignedPopup;
        } else if (plasmoid.location == PlasmaCore.Types.TopEdge) {
            return PlasmaCore.Types.BottomPosedLeftAlignedPopup;
        } else {
            return PlasmaCore.Types.TopPosedLeftAlignedPopup;
        }
    }

    minimumWidth: visualParent ? visualParent.width : 1

    property bool isOnAllActivitiesLauncher: true
    property bool showAllPlaces: false

    property int activitiesCount: 0


    onStatusChanged: {
        if (visualParent && get(atm.LauncherUrlWithoutIcon) != null && status == PlasmaComponents.DialogStatus.Open) {
            launcherToggleAction.checked = (tasksModel.launcherPosition(get(atm.LauncherUrlWithoutIcon)) != -1);
            activitiesDesktopsMenu.refresh();
        } else if (status == PlasmaComponents.DialogStatus.Closed) {
            root.startCheckRestoreZoomTimer(100);
            menu.destroy();
            backend.ungrabMouse(visualParent);
        }
    }

    function get(modelProp) {
        return tasksModel.data(modelIndex, modelProp)
    }

    function show() {
        //trying to use the dragging mechanism in order to not hide the dock
        root.disableRestoreZoom = true;
        root.signalActionsBlockHiding(1);
        //root.signalDraggingState(true);
        loadDynamicLaunchActions(visualParent.m.LauncherUrlWithoutIcon);
        // backend.ungrabMouse(visualParent);
        openRelative();
        windowsPreviewDlg.contextMenu = true;
        windowsPreviewDlg.hide();

        icList.directRender = false;

        if (root.latteDock){
            root.latteDock.hideTooltipLabel();
            root.latteDock.globalDirectRender = false;
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
        var lists = [];

        //From Plasma 5.10 and frameworks 5.34, places are also supported
        if (Latte.WindowSystem.frameworksVersion >= 336384
                && (typeof backend.placesActions === "function")) {
            lists = [
                        backend.jumpListActions(launcherUrl, menu),
                        backend.placesActions(launcherUrl, showAllPlaces, menu),
                        backend.recentDocumentActions(launcherUrl, menu)
                    ]
        } else {
            lists = [
                        backend.jumpListActions(launcherUrl, menu),
                        backend.recentDocumentActions(launcherUrl, menu)
                    ];
        }

        lists.forEach(function (list) {
            for (var i = 0; i < list.length; ++i) {
                var item = newMenuItem(menu);
                item.action = list[i];
                menu.addMenuItem(item, virtualDesktopsMenuItem);
            }

            if (list.length > 0) {
                menu.addMenuItem(newSeparator(menu), virtualDesktopsMenuItem);
            }
        });

        // Add Media Player control actions
        var sourceName = mpris2Source.sourceNameForLauncherUrl(launcherUrl, get(atm.AppPid));

        if (sourceName && !(get(atm.LegacyWinIdList) != undefined && get(atm.LegacyWinIdList).length > 1)) {
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
        if (Latte.WindowSystem.frameworksVersion >= 336384) {
            // Cannot have "Connections" as child of PlasmaCoponents.ContextMenu.
            backend.showAllPlaces.connect(function() {
                visualParent.showContextMenu({showAllPlaces: true});
            });
        }
        //  updateOnAllActivitiesLauncher();
    }


    Component.onDestruction: {
        if (!changingLayout) {
            windowsPreviewDlg.contextMenu = false;
            root.contextMenu = null;
            backend.ungrabMouse(visualParent);
            root.signalActionsBlockHiding(-1);
            //root.signalDraggingState(false);
            root.disableRestoreZoom = false;
            root.startCheckRestoreZoomTimer(100);
        }
    }

    /// Sub Items

    PlasmaComponents.MenuItem {
        id: virtualDesktopsMenuItem

        visible: virtualDesktopInfo.numberOfDesktops > 1
                 && (visualParent && visualParent.m.IsLauncher !== true
                     && visualParent.m.IsStartup !== true
                     && visualParent.m.IsVirtualDesktopChangeable === true)

        enabled: visible

        text: i18n("Move To Desktop")

        Connections {
            target: virtualDesktopInfo

            onNumberOfDesktopsChanged: virtualDesktopsMenu.refresh()
            onDesktopNamesChanged: virtualDesktopsMenu.refresh()
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
                menuItem.text = i18n("Move To Current Desktop");
                menuItem.enabled = Qt.binding(function() {
                    return menu.visualParent && menu.visualParent.m.VirtualDesktop != virtualDesktopInfo.currentDesktop;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktop(menu.modelIndex, 0);
                });

                menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("All Desktops");
                menuItem.checkable = true;
                menuItem.checked = Qt.binding(function() {
                    return menu.visualParent && menu.visualParent.m.IsOnAllVirtualDesktops === true;
                });
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktop(menu.modelIndex, 0);
                });
                backend.setActionGroup(menuItem.action);

                menu.newSeparator(virtualDesktopsMenu);

                for (var i = 0; i < virtualDesktopInfo.desktopNames.length; ++i) {
                    menuItem = menu.newMenuItem(virtualDesktopsMenu);
                    //menuItem.text = i18nc("1 = number of desktop, 2 = desktop name", "%1 Desktop %2", i + 1, virtualDesktopInfo.desktopNames[i]);
                    menuItem.text = (i + 1) + ". " + virtualDesktopInfo.desktopNames[i];
                    menuItem.checkable = true;
                    menuItem.checked = Qt.binding((function(i) {
                        return function() { return menu.visualParent && menu.visualParent.m.VirtualDesktop == (i + 1) };
                    })(i));
                    menuItem.clicked.connect((function(i) {
                        return function() { return tasksModel.requestVirtualDesktop(menu.modelIndex, i + 1); };
                    })(i));
                    backend.setActionGroup(menuItem.action);
                }

                menu.newSeparator(virtualDesktopsMenu);

                menuItem = menu.newMenuItem(virtualDesktopsMenu);
                menuItem.text = i18n("New Desktop");
                menuItem.clicked.connect(function() {
                    tasksModel.requestVirtualDesktop(menu.modelIndex, virtualDesktopInfo.numberOfDesktops + 1)
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
        visible: (visualParent
                  && visualParent.m.IsLauncher !== true
                  && visualParent.m.IsStartup !== true
                  && root.showWindowActions
                  && !root.disableAllWindowsFunctionality)

        enabled: visualParent && visualParent.m.IsMinimizable === true

        checkable: true
        checked: visualParent && visualParent.m.IsMinimized === true

        text: i18n("Minimize")

        onClicked: tasksModel.requestToggleMinimized(menu.modelIndex)
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

        text: i18n("Maximize")

        onClicked: tasksModel.requestToggleMaximized(menu.modelIndex)
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

        PlasmaComponents.ContextMenu {
            visualParent: moreActionsMenuItem.action

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.m.IsMovable === true

                text: i18n("Move")
                icon: "transform-move"

                onClicked: tasksModel.requestMove(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.m.IsResizable === true

                text: i18n("Resize")

                onClicked: tasksModel.requestResize(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsKeepAbove === true

                text: i18n("Keep Above Others")
                icon: "go-up"

                onClicked: tasksModel.requestToggleKeepAbove(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsKeepBelow === true

                text: i18n("Keep Below Others")
                icon: "go-down"

                onClicked: tasksModel.requestToggleKeepBelow(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.m.IsFullScreenable === true

                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsFullScreen === true

                text: i18n("Fullscreen")
                icon: "view-fullscreen"

                onClicked: tasksModel.requestToggleFullScreen(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                enabled: menu.visualParent && menu.visualParent.m.IsShadeable === true

                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsShaded === true

                text: i18n("Shade")

                onClicked: tasksModel.requestToggleShaded(menu.modelIndex)
            }

            PlasmaComponents.MenuItem {
                separator: true
            }

            PlasmaComponents.MenuItem {
                visible: (plasmoid.configuration.groupingStrategy != 0) && menu.visualParent.m.IsWindow === true

                checkable: true
                checked: menu.visualParent && menu.visualParent.m.IsGroupable === true

                text: i18n("Allow this program to be grouped")

                onClicked: tasksModel.requestToggleGrouping(menu.modelIndex)
            }
        }
    }

    PlasmaComponents.MenuItem {
        id: startNewInstanceItem
        visible: (visualParent && visualParent.m.IsLauncher !== true && visualParent.m.IsStartup !== true)

        enabled: visualParent && visualParent.m.LauncherUrlWithoutIcon != null

        text: i18n("Start New Instance")
        icon: "system-run"

        onClicked: tasksModel.requestNewInstance(menu.modelIndex)
    }

    PlasmaComponents.MenuItem {
        separator: true

        visible: (visualParent
                  && visualParent.m.IsLauncher !== true
                  && visualParent.m.IsStartup !== true
                  && root.showWindowActions)
    }

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

        text: i18nc("Toggle action for showing a launcher button while the application is not running", "&Pin")

        onClicked: {
            if (tasksModel.launcherPosition(get(atm.LauncherUrlWithoutIcon)) != -1) {
                var launcher = get(atm.LauncherUrl);

                if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                    latteDock.universalLayoutManager.launchersSignals.removeLauncher(root.managedLayoutName,
                                                                                     latteDock.launchersGroup, launcher);
                } else {
                    root.launcherForRemoval = launcher;
                    tasksModel.requestRemoveLauncher(launcher);
                    root.launchersUpdatedFor(launcher);
                }

            } else {
                var launcher = get(atm.LauncherUrl);
                if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                    latteDock.universalLayoutManager.launchersSignals.addLauncher(root.managedLayoutName,
                                                                                  latteDock.launchersGroup, launcher);
                } else {
                    tasksModel.requestAddLauncher(launcher);
                    root.launchersUpdatedFor(launcher);
                }
            }
        }
    }

    PlasmaComponents.MenuItem {
        id: showLauncherInActivitiesItem

        text: i18n("&Pin")

        visible: visualParent && (!visualParent.isSeparator || (visualParent.isSeparator && root.editMode))
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
                                        if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                                            latteDock.universalLayoutManager.launchersSignals.addLauncherToActivity(root.managedLayoutName,
                                                                                                                    latteDock.launchersGroup, url, id);
                                        } else {
                                            if (id !== tasksModel.activity && (activities[0] === "00000000-0000-0000-0000-000000000000")) {
                                                root.launcherForRemoval = url;
                                            }

                                            tasksModel.requestAddLauncherToActivity(url, id);
                                            root.launchersUpdatedFor(url);
                                        }
                                    } else {
                                        if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                                            latteDock.universalLayoutManager.launchersSignals.removeLauncherFromActivity(root.managedLayoutName,
                                                                                                                         latteDock.launchersGroup, url, id);
                                        } else {
                                            if (id === tasksModel.activity) {
                                                root.launcherForRemoval = url;
                                            }
                                            tasksModel.requestRemoveLauncherFromActivity(url, id);
                                            root.launchersUpdatedFor(url);
                                        }
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

        text: i18nc("Remove launcher button for application shown while it is not running", "Unpin")

        onClicked: {
            var launcher = get(atm.LauncherUrlWithoutIcon);

            if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                latteDock.universalLayoutManager.launchersSignals.removeLauncher(root.managedLayoutName,
                                                                                 latteDock.launchersGroup, launcher);
            } else {
                root.launcherForRemoval = launcher
                tasksModel.requestRemoveLauncher(launcher);
                root.launchersUpdatedFor(launcher);
            }
        }
    }

    //////END OF NEW ARCHITECTURE

    PlasmaComponents.MenuItem {
        separator: true
    }

    PlasmaComponents.MenuItem {
        id: addInternalSeparatorItem
        visible: root.editMode

        icon: "add"
        text: i18n("Add Separator")

        onClicked: {
            var pos=visualParent.itemIndex;

            root.addInternalSeparatorAtPos(pos);
        }
    }

    PlasmaComponents.MenuItem {
        id: removeInternalSeparatorItem
        visible: root.editMode && visualParent.isSeparator

        icon: "remove"
        text: i18n("Remove Separator")
        enabled: parabolicManager.hasInternalSeparator

        onClicked: {
            //root.removeLastSeparator();
            var launcher = get(atm.LauncherUrlWithoutIcon);

            if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                latteDock.universalLayoutManager.launchersSignals.removeLauncher(root.managedLayoutName,
                                                                                 latteDock.launchersGroup, launcher);
            } else {
                root.launcherForRemoval = launcher;
                tasksModel.requestRemoveLauncher(launcher);
                root.launchersUpdatedFor(launcher);
            }
        }
    }

    PlasmaComponents.MenuItem {
        separator: true
        visible: root.editMode
    }

    PlasmaComponents.MenuItem {
        id: layoutsMenuItem

        action: latteDock ?  latteDock.containmentActions()[1] : plasmoid.action("configure")
        enabled: visible
        visible: latteDock && latteDock.universalLayoutManager.menuLayouts.length>1
    }

    PlasmaComponents.MenuItem {
        id: alternativesMenuItem
        visible: root.editMode && !visualParent.isSeparator
        text: plasmoid.action("alternatives").text
        icon: plasmoid.action("alternatives").icon

        onClicked: plasmoid.action("alternatives").trigger();
    }

    PlasmaComponents.MenuItem {
        id: addWidgets

        action: latteDock ? latteDock.containmentActions()[2] : plasmoid.action("configure");
        visible:  latteDock
    }


    PlasmaComponents.MenuItem {
        id: configureItem

        action: latteDock ? latteDock.containmentActions()[3] : plasmoid.action("configure")
    }

    //! BEGIN: Plasmoid actions when it isnt inside a Latte dock
    PlasmaComponents.MenuItem {
        id: removePlasmoid
        visible: !latteDock && !plasmoid.immutable

        text: plasmoid.action("remove").text
        icon: plasmoid.action("remove").icon

        onClicked: plasmoid.action("remove").trigger();
    }

    PlasmaComponents.MenuItem {
        id: configurePlasmoid
        visible: !latteDock && !plasmoid.immutable

        text: plasmoid.action("configure").text
        icon: plasmoid.action("configure").icon

        onClicked: plasmoid.action("configure").trigger();
    }
    //! END: Plasmoid actions when it isnt inside a Latte dock

    PlasmaComponents.MenuItem {
        separator: true
        visible: closeWindowItem.visible
    }

    //!move window Close button at the very bottom in order to not alter users workflow
    //!comparing with the design decisions of other taskmanagers
    PlasmaComponents.MenuItem {
        id: closeWindowItem
        visible: (visualParent && visualParent.m.IsLauncher !== true && visualParent.m.IsStartup !== true)

        enabled: visualParent && visualParent.m.IsClosable === true

        text: i18n("Close")
        icon: "window-close"

        onClicked: {
            if (root.zoomFactor>1) {
                delayWindowRemovalTimer.modelIndex = menu.modelIndex;
                delayWindowRemovalTimer.start();
            } else {
                tasksModel.requestClose(menu.modelIndex);
            }
        }
    }
}
