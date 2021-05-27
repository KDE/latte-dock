/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.taskmanager 0.1 as TaskManager

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.containment 0.1 as LatteContainment

Loader {
    id: tasksLoader
    active: root.scrollAction === LatteContainment.Types.ScrollTasks || root.scrollAction === LatteContainment.Types.ScrollToggleMinimized
    sourceComponent: Item {
        TaskManager.TasksModel {
            id: tasksModel
            virtualDesktop: virtualDesktopInfo.currentDesktop
            screenGeometry: latteView ? latteView.screenGeometry : Qt.rect(-1, -1, 0, 0)
            activity: activityInfo.currentActivity

            filterByVirtualDesktop: true
            filterByScreen:latteView ?  true : false
            filterByActivity: true

            launchInPlace: true
            separateLaunchers: true
            groupInline: false

            groupMode: TaskManager.TasksModel.GroupApplications
            sortMode: TaskManager.TasksModel.SortManual
        }

        TaskManager.VirtualDesktopInfo {
            id: virtualDesktopInfo
        }

        TaskManager.ActivityInfo {
            id: activityInfo
        }

        Item{
            id: taskList
            Repeater{
                model: tasksModel
                Item{
                    readonly property var m: model

                    function modelIndex() {
                        return tasksModel.makeModelIndex(index);
                    }
                }
            }
        }

        function activateNextPrevTask(next) {
            var taskIndexList = [];
            var activeTaskIndex = tasksModel.activeTask;

            for (var i = 0; i < taskList.children.length - 1; ++i) {
                var task = taskList.children[i];
                var modelIndex = task.modelIndex(i);

                if (task.m.IsLauncher !== true && task.m.IsStartup !== true) {
                    if (task.m.IsGroupParent === true) {
                        for (var j = 0; j < tasksModel.rowCount(modelIndex); ++j) {
                            taskIndexList.push(tasksModel.makeModelIndex(i, j));
                        }
                    } else {
                        taskIndexList.push(modelIndex);
                    }
                }
            }

            if (!taskIndexList.length) {
                return;
            }

            var target = taskIndexList[0];

            for (var i = 0; i < taskIndexList.length; ++i) {
                if (taskIndexList[i] === activeTaskIndex)
                {
                    if (next && i < (taskIndexList.length - 1)) {
                        target = taskIndexList[i + 1];
                    } else if (!next) {
                        if (i) {
                            target = taskIndexList[i - 1];
                        } else {
                            target = taskIndexList[taskIndexList.length - 1];
                        }
                    }

                    break;
                }
            }

            tasksModel.requestActivate(target);
        }
    }
}
