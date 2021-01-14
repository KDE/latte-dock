/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

Item {
    readonly property bool isReady: root.layoutsManager !== null

    property string layoutName: ""

    function addAbilityClient(client) {
        layoutsManager.syncedLaunchers.addAbilityClient(client);
    }

    function removeAbilityClient(client) {
        layoutsManager.syncedLaunchers.removeAbilityClient(client);
    }

    function addSyncedLauncher(group, launcherUrl) {
        layoutsManager.syncedLaunchers.addLauncher(layoutName,
                                                   group,
                                                   launcherUrl);
    }

    function removeSyncedLauncher(group, launcherUrl) {
        layoutsManager.syncedLaunchers.removeLauncher(layoutName,
                                                      group,
                                                      launcherUrl);
    }

    function addSyncedLauncherToActivity(group, launcherUrl, activityId) {
        layoutsManager.syncedLaunchers.addLauncherToActivity(layoutName,
                                                             group,
                                                             launcherUrl,
                                                             activityId);
    }

    function removeSyncedLauncherFromActivity(group, launcherUrl, activityId) {
        layoutsManager.syncedLaunchers.removeLauncherFromActivity(layoutName,
                                                                  group,
                                                                  launcherUrl,
                                                                  activityId);
    }

    function addDroppedLaunchers(group, urls) {
        layoutsManager.syncedLaunchers.urlsDropped(layoutName,
                                                   group,
                                                   urls);
    }

    function validateSyncedLaunchersOrder(senderId, group, orderedlaunchers) {
        layoutsManager.syncedLaunchers.validateLaunchersOrder(layoutName,
                                                              senderId,
                                                              group,
                                                              orderedlaunchers);
    }
}
