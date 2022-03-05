/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore

Item {
    id:_syncer
    property bool isBlocked: false
    readonly property bool isActive: bridge !== null && bridge.launchers.host !==null
    readonly property int clientId: plasmoid.id
    // used to identify launchers that need to be synced event though their launchers group type
    // does not support it e.g. taskmanagers synced in different screens
    readonly property string syncedGroupId: _launchers.groupId

    //! Connections
    Component.onCompleted: {
        if (isActive) {
            bridge.launchers.host.addAbilityClient(_syncer);
        }
    }

    Component.onDestruction: {
        if (bridge) {
            bridge.launchers.host.removeAbilityClient(_syncer);
        }
    }

    onIsActiveChanged: {
        if (isActive) {
            bridge.launchers.host.addAbilityClient(_syncer);
        } else if (bridge) {
            bridge.launchers.host.removeAbilityClient(_syncer);
        }
    }

    Connections {
        target: isActive ? bridge.launchers.host : null
        onIsReadyChanged: {
            if (bridge.launchers.host.isReady && _syncer.isActive) {
                bridge.launchers.host.addAbilityClient(_syncer);
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
