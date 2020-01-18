/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.taskmanager 0.1 as TaskManager

import org.kde.latte 0.2 as Latte

Loader {
    id: tasksLoader
    active: root.scrollAction === Latte.Types.ScrollTasks || root.scrollAction === Latte.Types.ScrollToggleMinimized
    sourceComponent: Item {
        TaskManager.TasksModel {
            id: tasksModel
            virtualDesktop: virtualDesktopInfo.currentDesktop
            screenGeometry: latteView.screenGeometry
            activity: activityInfo.currentActivity

            filterByVirtualDesktop: true
            filterByScreen: true
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
