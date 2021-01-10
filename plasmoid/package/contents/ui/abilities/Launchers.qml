/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.core 0.2 as LatteCore

import "launchers" as LaunchersPart

Item {
    id: _launchers
    signal launcherChanged(string launcherUrl);
    signal launcherRemoved(string launcherUrl);
    signal launcherInRemoving(string launcherUrl);

    property int group: LatteCore.Types.UniqueLaunchers
    property Item bridge: null
    property Item layout: null

    property QtObject tasksModel: null

    readonly property LaunchersPart.Actions actions: LaunchersPart.Actions{}
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
        return _launchers.tasksModel.launcherPosition(url) != -1;
    }

    function addLauncher(launcherUrl) {
        if (latteView && !inUniqueGroup()) {
            latteView.layoutsManager.launchersSignals.addLauncher(root.viewLayoutName,
                                                                  launchers.group,
                                                                  launcherUrl);
        } else {
            _launchers.tasksModel.requestAddLauncher(launcherUrl);
            _launchers.launcherChanged(launcherUrl);
        }
    }

    function removeLauncher(launcherUrl) {
        if (latteView && !inUniqueGroup()) {
            latteView.layoutsManager.launchersSignals.removeLauncher(root.viewLayoutName,
                                                                     launchers.group,
                                                                     launcherUrl);
        } else {
            _launchers.launcherInRemoving(launcherUrl);
            _launchers.tasksModel.requestRemoveLauncher(launcherUrl);
            _launchers.launcherRemoved(launcherUrl);
        }
    }

    function addLauncherToActivity(launcherUrl, activityId) {
        if (latteView && !inUniqueGroup()) {
            latteView.layoutsManager.launchersSignals.addLauncherToActivity(root.viewLayoutName,
                                                                            launchers.group,
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
        if (latteView && !inUniqueGroup()) {
            latteView.layoutsManager.launchersSignals.removeLauncherFromActivity(root.viewLayoutName,
                                                                                 launchers.group,
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

    function inCurrentActivity(launcherUrl) {
        var activities = _launchers.tasksModel.launcherActivities(launcherUrl);

        if (activities.length === 0 || activities.indexOf(_NULLACTIVITYID_) !== -1 || activities.indexOf(activityInfo.currentActivity) !== -1) {
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

        if (viewLayout) {
            if (latteView && latteView.layoutsManager
                    && latteView.viewLayout && latteView.universalSettings
                    && !_launchers.inUniqueGroup()) {

                if (_launchers.inLayoutGroup()) {
                    launchersList = latteView.viewLayout.launchers;
                } else if (_launchers.inGlobalGroup()) {
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


    function validateOrder(orderedLaunchers) {
        validator.stop();
        validator.launchers = orderedLaunchers;
        validator.start();
    }

    //! Connections

    onGroupChanged:{
        if(latteView) {
            _launchers.tasksModel.updateLaunchersList();
        }
    }

    Connections {
        target: root
        onLatteViewChanged: {
            if (root.latteView) {
                if (!_launchers.inUniqueGroup()) {
                    _launchers.tasksModel.updateLaunchersList();
                }
            }
        }
    }

    Connections {
        target: _launchers.tasksModel
        onLauncherListChanged: {
            if (viewLayout) {
                if (latteView && latteView.layoutsManager
                        && latteView.viewLayout && latteView.universalSettings
                        && !_launchers.inUniqueGroup()) {

                    if (_launchers.inLayoutGroup()) {
                        latteView.viewLayout.launchers = _launchers.tasksModel.launcherList;
                    } else if (_launchers.inGlobalGroup()) {
                        latteView.universalSettings.launchers = _launchers.tasksModel.launcherList;
                    }

                    if (inDraggingPhase) {
                        if (latteView && !_launchers.inUniqueGroup()) {
                            latteView.layoutsManager.launchersSignals.validateLaunchersOrder(root.viewLayoutName,
                                                                                             plasmoid.id,
                                                                                             _launchers.group,
                                                                                             _launchers.currentShownLauncherList());
                        }
                    }
                } else {
                    plasmoid.configuration.launchers59 = _launchers.tasksModel.launcherList;
                }
            } else if (!latteView) {
                // This way we make sure that a delayed viewLayout initialization does not store irrelevant launchers from different
                // group to UNIQUE launchers group
                plasmoid.configuration.launchers59 = _launchers.tasksModel.launcherList;
            }
        }
    }
}
