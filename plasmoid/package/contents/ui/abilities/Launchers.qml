/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore

import "launchers" as LaunchersPart

Item {
    id: _launchers
    readonly property bool isActive: bridge !== null

    signal launcherChanged(string launcherUrl);
    signal launcherRemoved(string launcherUrl);

    //! triggered just before action happens. They are used mostly for animation purposes
    signal launcherInAdding(string launcherUrl);
    signal launcherInRemoving(string launcherUrl);
    signal launcherInMoving(string launcherUrl, int pos);

    signal disabledIsStealingDroppedLaunchers();

    property bool isStealingDroppedLaunchers: false
    property bool isShowingAddLaunchersMessage: false

    property bool __isLoadedDuringViewStartup: false

    property string appletIndex: bridge && bridge.indexer ? String(bridge.indexer.appletIndex) : ""
    property int group: LatteCore.Types.UniqueLaunchers
    property string groupId: view && group === LatteCore.Types.UniqueLaunchers ? String(view.groupId) + "#" + appletIndex : ""

    property Item bridge: null
    property Item layout: null
    property Item view: null
    property QtObject tasksModel: null

    readonly property LaunchersPart.Syncer syncer: LaunchersPart.Syncer{}
    readonly property LaunchersPart.Validator validator: LaunchersPart.Validator{}

    readonly property string _NULLACTIVITYID_: "00000000-0000-0000-0000-000000000000"

    function inUniqueGroup() {
        return group === LatteCore.Types.UniqueLaunchers;
    }

    function inLayoutGroup() {
        return group === LatteCore.Types.LayoutLaunchers;
    }

    function inGlobalGroup() {
        return group === LatteCore.Types.GlobalLaunchers;
    }

    function isSeparator(launcher){
        return (launcher.indexOf("latte-separator")!==-1 && launcher.indexOf(".desktop")!==1);
    }

    function separatorExists(separator){
        return (_launchers.tasksModel.launcherPosition(separator)>=0);
    }

    function freeAvailableSeparatorName() {
        var available = false;
        var no = 1;

        var separatorName = "";

        while(!available && no<20) {
            separatorName = "file:///latte-separator"+no+".desktop";
            if (separatorExists(separatorName)) {
                no = no + 1;
            } else {
                available = true;
            }
        }

        if (available) {
            return separatorName;
        } else {
            return "";
        }
    }

    function hasLauncher(url) {
        return _launchers.tasksModel.launcherPosition(url) >= 0;
    }

    function addLauncher(launcherUrl) {
        if (bridge) {
            bridge.launchers.host.addSyncedLauncher(syncer.clientId,
                                                    launchers.group,
                                                    launchers.groupId,
                                                    launcherUrl);
        } else {
            _launchers.tasksModel.requestAddLauncher(launcherUrl);
            _launchers.launcherChanged(launcherUrl);
        }
    }

    function addDroppedLauncher(launcherUrl) {
        //workaround to protect in case the launcher contains the iconData
        var pos = launcherUrl.indexOf("?iconData=");

        if (pos>0) {
            launcherUrl = launcherUrl.substring( 0, launcherUrl.indexOf("?iconData=" ) );
        }

        var path = launcherUrl;
        var filename = path.split("/").pop();
        _launchers.launcherInAdding(filename);

        tasksModel.requestAddLauncher(launcherUrl);
        launchers.launcherChanged(launcherUrl);
        tasksModel.syncLaunchers();
    }

    function addDroppedLaunchers(urls) {
        //! inform synced docks for new dropped launchers
        if (bridge) {
            bridge.launchers.host.addDroppedLaunchers(syncer.clientId,
                                                      launchers.group,
                                                      launchers.groupId,
                                                      urls);
        } else {
            urls.forEach(function (item) {
                addDroppedLauncher(item);
            });
        }
    }

    function addInternalSeparatorAtPos(pos) {
        var separatorName = freeAvailableSeparatorName();

        if (separatorName !== "") {
            _launchers.launcherInMoving(separatorName, Math.max(0,pos));
            addLauncher(separatorName);
        }
    }

    function removeInternalSeparatorAtPos(pos) {
        var item = childAtLayoutIndex(pos);

        if (item.isSeparator) {
            removeLauncher(item.launcherUrl);
        }
    }

    function removeLauncher(launcherUrl) {
        if (bridge) {
            bridge.launchers.host.removeSyncedLauncher(syncer.clientId,
                                                       launchers.group,
                                                       launchers.groupId,
                                                       launcherUrl);
        } else {
            _launchers.launcherInRemoving(launcherUrl);
            _launchers.tasksModel.requestRemoveLauncher(launcherUrl);
            _launchers.launcherRemoved(launcherUrl);
        }
    }

    function addLauncherToActivity(launcherUrl, activityId) {
        if (bridge) {
            bridge.launchers.host.addSyncedLauncherToActivity(syncer.clientId,
                                                              launchers.group,
                                                              launchers.groupId,
                                                              launcherUrl,
                                                              activityId);
        } else {
            if (activityId !== activityInfo.currentActivity && isOnAllActivities(launcherUrl)) {
                _launchers.launcherInRemoving(launcherUrl);
            }

            _launchers.tasksModel.requestAddLauncherToActivity(launcherUrl, activityId);
            _launchers.launcherChanged(launcherUrl);
        }
    }

    function removeLauncherFromActivity(launcherUrl, activityId) {
        if (bridge) {
            bridge.launchers.host.removeSyncedLauncherFromActivity(syncer.clientId,
                                                                   launchers.group,
                                                                   launchers.groupId,
                                                                   launcherUrl,
                                                                   activityId);
        } else {
            if (activityId === activityInfo.currentActivity) {
                _launchers.launcherInRemoving(launcherUrl);
            }
            _launchers.tasksModel.requestRemoveLauncherFromActivity(launcherUrl, activityId);
            _launchers.launcherChanged(launcherUrl);
        }
    }

    function validateSyncedLaunchersOrder() {
        if (bridge) {
            bridge.launchers.host.validateSyncedLaunchersOrder(syncer.clientId,
                                                               launchers.group,
                                                               launchers.groupId,
                                                               currentShownLauncherList());
        } else {
            /*validator.stop();
            validator.launchers = orderedLaunchers;
            validator.start();*/
        }
    }

    function inCurrentActivity(launcherUrl) {
        if (!hasLauncher(launcherUrl)) {
            return false;
        }

        var activities = _launchers.tasksModel.launcherActivities(launcherUrl);

        if (activities.length ===0
                || activities.indexOf(_NULLACTIVITYID_) >= 0
                || activities.indexOf(activityInfo.currentActivity) >= 0) {
            return true;
        }

        return false;
    }

    function isOnAllActivities(launcherUrl) {
        var activities = _launchers.tasksModel.launcherActivities(launcherUrl);
        return (activities.indexOf(_NULLACTIVITYID_) >= 0)
    }

    function childAtLayoutIndex(position) {
        var tasks = layout.children;

        if (position < 0) {
            return;
        }

        for(var i=0; i<tasks.length; ++i){
            var task = tasks[i];

            if (task.lastValidIndex === position
                    || (task.lastValidIndex === -1 && task.itemIndex === position )) {
                return task;
            }
        }

        return undefined;
    }

    function indexOfLayoutLauncher(url) {
        var tasks = layout.children;

        for(var i=0; i<tasks.length; ++i){
            var task = tasks[i];

            if (task && (task.launcherUrl===url)) {
                return task.itemIndex;
            }
        }

        return -1;
    }

    function currentShownLauncherList() {
        var launch = [];

        var tasks = _launchers.layout.children;
        for(var i=0; i<tasks.length; ++i){
            var task = _launchers.childAtLayoutIndex(i);

            if (task!==undefined && task.launcherUrl!=="" && _launchers.inCurrentActivity(task.launcherUrl)) {
                launch.push(task.launcherUrl);
            }
        }

        return launch;
    }


    function currentStoredLauncherList() {
        var launch = [];
        var launchersList = [];

        if (bridge && bridge.launchers.host.isReady) {
            if (!_launchers.inUniqueGroup()) {
                if (_launchers.inLayoutGroup()) {
                    launchersList = bridge.launchers.host.layoutLaunchers;
                } else if (_launchers.inGlobalGroup()) {
                    launchersList = bridge.launchers.host.universalLaunchers;
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

    function importLauncherListInModel() {
        if (bridge && bridge.launchers.host.isReady && !inUniqueGroup()) {
            if (inLayoutGroup()) {
                console.log("Tasks: Applying LAYOUT Launchers List...");
                tasksModel.launcherList = bridge.launchers.host.layoutLaunchers;
            } else if (inGlobalGroup()) {
                console.log("Tasks: Applying GLOBAL Launchers List...");
                tasksModel.launcherList = bridge.launchers.host.universalLaunchers;
            }
        } else {
            console.log("Tasks: Applying UNIQUE Launchers List...");
            tasksModel.launcherList = plasmoid.configuration.launchers59;
        }
    }


    //! Connections
    Component.onCompleted: {
        if (isActive) {
            bridge.launchers.client = _launchers;
        }
    }

    Component.onDestruction: {
        if (isActive) {
            bridge.launchers.client = null;
        }
    }

    onIsActiveChanged: {
        if (isActive) {
            bridge.launchers.client = _launchers;
        }
    }

    onGroupChanged:{
        if(appletAbilities.myView.isReady) {
            _launchers.importLauncherListInModel();
        }
    }

    Connections {
        target: appletAbilities.myView
        onIsReadyChanged: {
            if(appletAbilities.myView.isReady) {
                if (!_launchers.inUniqueGroup()) {
                    _launchers.importLauncherListInModel();
                }
            }
        }
    }

    Connections {
        target: bridge ? bridge.launchers.host : null
        onIsReadyChanged: {
            if (bridge && bridge.launchers.host.isReady && !_launchers.__isLoadedDuringViewStartup) {
                _launchers.__isLoadedDuringViewStartup = true;
                _launchers.importLauncherListInModel();
            }
        }
    }

    Connections {
        target: _launchers.tasksModel
        onLauncherListChanged: {
            if (bridge && bridge.launchers.host.isReady) {
                if (!_launchers.inUniqueGroup()) {
                    if (_launchers.inLayoutGroup()) {
                        bridge.launchers.host.setLayoutLaunchers(_launchers.tasksModel.launcherList);
                    } else if (_launchers.inGlobalGroup()) {
                        bridge.launchers.host.setUniversalLaunchers(_launchers.tasksModel.launcherList);
                    }
                } else {
                    plasmoid.configuration.launchers59 = _launchers.tasksModel.launcherList;
                }

                if (inDraggingPhase) {
                    _launchers.validateSyncedLaunchersOrder();
                }
            } else if (!appletAbilities.myView.isReady) {
                // This way we make sure that a delayed view.layout initialization does not store irrelevant launchers from different
                // group to UNIQUE launchers group
                plasmoid.configuration.launchers59 = _launchers.tasksModel.launcherList;
            }
        }
    }
}
