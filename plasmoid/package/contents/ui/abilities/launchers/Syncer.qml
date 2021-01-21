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

Item {
    id:_syncer
    property bool isBlocked: false
    readonly property bool isActive: bridge !== null && group !== LatteCore.Types.UniqueLaunchers
    readonly property int clientId: plasmoid.id

    //! Connections
    Component.onCompleted: {
        if (isActive) {
            bridge.launchers.addAbilityClient(_syncer);
        }
    }

    Component.onDestruction: {
        if (bridge) {
            bridge.launchers.removeAbilityClient(_syncer);
        }
    }

    onIsActiveChanged: {
        if (isActive) {
            bridge.launchers.addAbilityClient(_syncer);
        } else if (bridge) {
            bridge.launchers.removeAbilityClient(_syncer);
        }
    }

    Connections {
        target: isActive ? bridge.launchers : null
        onIsReadyChanged: {
            if (bridge.launchers.isReady && _syncer.isActive) {
                bridge.launchers.addAbilityClient(_syncer);
            }
        }
    }

    //! All following actions are triggerred from Main SyncedLaunchers handler
    function addSyncedLauncher(group, launcher) {
        if (group === _launchers.group) {
            tasksModel.requestAddLauncher(launcher);
            _launchers.launcherChanged(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function removeSyncedLauncher(group, launcher) {
        if (group === _launchers.group) {
            _launchers.launcherInRemoving(launcher)
            tasksModel.requestRemoveLauncher(launcher);
            _launchers.launcherChanged(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function addSyncedLauncherToActivity(group, launcher, activity) {
        if (group === _launchers.group) {
            if (activity !== activityInfo.currentActivity && _launchers.isOnAllActivities(launcher)) {
                _launchers.launcherInRemoving(launcher);
            }

            tasksModel.requestAddLauncherToActivity(launcher, activity);
            _launchers.launcherChanged(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function removeSyncedLauncherFromActivity(group, launcher, activity) {
        if (group === _launchers.group) {
            if (activity === activityInfo.currentActivity) {
                _launchers.launcherInRemoving(launcher)
            }

            tasksModel.requestRemoveLauncherFromActivity(launcher, activity);
            _launchers.launcherChanged(launcher);
            tasksModel.syncLaunchers();
        }
    }

    function dropSyncedUrls(group, urls) {
        if (group === _launchers.group) {
            urls.forEach(function (item) {
                _launchers.addDroppedLauncher(item);
            });
        }
    }

    function validateSyncedLaunchersOrder(group, orderedLaunchers) {
        if (group === _launchers.group && !isBlocked) {
            validator.stop();
            validator.launchers = orderedLaunchers;
            validator.start();
        }
    }

}
