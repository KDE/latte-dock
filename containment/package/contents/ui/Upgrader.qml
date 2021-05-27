/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item{
    Component.onCompleted:  {
        v010_upgrade();
    }

    function v010_upgrade() {
        root.upgrader_v010_alignment();

        if (!plasmoid.configuration.shadowsUpgraded) {
            if (plasmoid.configuration.shadows > 0) {
                plasmoid.configuration.appletShadowsEnabled = true;
            } else {
                plasmoid.configuration.appletShadowsEnabled = false;
            }

            plasmoid.configuration.shadowsUpgraded = true;
        }

        if (!plasmoid.configuration.tasksUpgraded) {
            v010_tasksMigrateTimer.start();
        }

    }

    Item {
        id: v010_tasksUpgrader
        Repeater {
            id: v010_tasksRepeater
            model: latteView && !plasmoid.configuration.tasksUpgraded ? latteView.extendedInterface.latteTasksModel : null
            Item {
                id: tasksApplet
                Component.onCompleted: {
                    if (index === 0) {
                        console.log(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! !!!!!!!!!!!!!!!!");
                        console.log(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! !!!!!!!!!!!!!!!!");
                        console.log(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! !!!!!!!!!!!!!!!!");
                        console.log(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! !!!!!!!!!!!!!!!!");
                        console.log(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! !!!!!!!!!!!!!!!!");
                        console.log(" Migrating Tasks Applet for v0.10...");
                        tasks.configuration.launchersGroup = plasmoid.configuration.launchersGroup;
                        tasks.configuration.showWindowActions = plasmoid.configuration.showWindowActions;
                        tasks.configuration.showWindowsOnlyFromLaunchers = plasmoid.configuration.showWindowsOnlyFromLaunchers;
                        tasks.configuration.groupTasksByDefault = plasmoid.configuration.groupTasksByDefault;
                        tasks.configuration.showOnlyCurrentScreen = plasmoid.configuration.showOnlyCurrentScreen;
                        tasks.configuration.showOnlyCurrentDesktop = plasmoid.configuration.showOnlyCurrentDesktop;
                        tasks.configuration.showOnlyCurrentActivity = plasmoid.configuration.showOnlyCurrentActivity;
                        tasks.configuration.showInfoBadge = plasmoid.configuration.showInfoBadge;
                        tasks.configuration.showProgressBadge = plasmoid.configuration.showProgressBadge;
                        tasks.configuration.showAudioBadge = plasmoid.configuration.showAudioBadge;
                        tasks.configuration.audioBadgeActionsEnabled = plasmoid.configuration.audioBadgeActionsEnabled;
                        tasks.configuration.infoBadgeProminentColorEnabled = plasmoid.configuration.infoBadgeProminentColorEnabled;
                        tasks.configuration.animationLauncherBouncing = plasmoid.configuration.animationLauncherBouncing;
                        tasks.configuration.animationWindowInAttention = plasmoid.configuration.animationWindowInAttention;
                        tasks.configuration.animationNewWindowSliding = plasmoid.configuration.animationNewWindowSliding;
                        tasks.configuration.animationWindowAddedInGroup = plasmoid.configuration.animationWindowAddedInGroup;
                        tasks.configuration.animationWindowRemovedFromGroup = plasmoid.configuration.animationWindowRemovedFromGroup;
                        tasks.configuration.scrollTasksEnabled = plasmoid.configuration.scrollTasksEnabled;
                        tasks.configuration.autoScrollTasksEnabled = plasmoid.configuration.autoScrollTasksEnabled;
                        tasks.configuration.manualScrollTasksType = plasmoid.configuration.manualScrollTasksType;
                        tasks.configuration.leftClickAction = plasmoid.configuration.leftClickAction;
                        tasks.configuration.middleClickAction = plasmoid.configuration.middleClickAction;
                        tasks.configuration.hoverAction = plasmoid.configuration.hoverAction;
                        tasks.configuration.taskScrollAction = plasmoid.configuration.taskScrollAction;
                        tasks.configuration.modifierClickAction = plasmoid.configuration.modifierClickAction;
                        tasks.configuration.modifier = plasmoid.configuration.modifier;
                        tasks.configuration.modifierClick = plasmoid.configuration.modifierClick;
                        tasks.configuration.isDroppedLauncherAddedOnlyInCurrentTasks = plasmoid.configuration.addLaunchersInTaskManager;
                        console.log("Migrating Tasks Applet for v0.10 succeeded ...");

                        plasmoid.configuration.tasksUpgraded = true;
                    }
                }
            }
        }
    }


    //! v0.10 Timer to check that first-upgrade process ended
    //! when View does not have any Tasks plasmoid
    Timer {
        id: v010_tasksMigrateTimer
        interval: 10000
        onTriggered: {
            plasmoid.configuration.tasksUpgraded = true;
            console.log("Tasks Migration ended....");
        }
    }
}
