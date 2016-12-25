/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */


function wheelActivateNextPrevTask(wheelDelta, eventDelta) {
    // magic number 120 for common "one click"
    // See: http://qt-project.org/doc/qt-5/qml-qtquick-wheelevent.html#angleDelta-prop
    wheelDelta += eventDelta;
    var increment = 0;
    while (wheelDelta >= 120) {
        wheelDelta -= 120;
        increment++;
    }
    while (wheelDelta <= -120) {
        wheelDelta += 120;
        increment--;
    }
    while (increment != 0) {
        activateNextPrevTask(increment < 0)
        increment += (increment < 0) ? 1 : -1;
    }

    return wheelDelta;
}

function activateNextPrevTask(next) {
    // FIXME TODO: Unnecessarily convoluted and costly; optimize.

    var taskIndexList = [];
    var activeTaskIndex = tasksModel.activeTask;

    for (var i = 0; i < taskList.children.length - 1; ++i) {
        var task = taskList.children[i];
        var modelIndex = task.modelIndex(i);

        if (task !== undefined){
            if (task.IsLauncher !== true && task.IsStartup !== true) {
                if (task.m.IsGroupParent === true) {
                    for (var j = 0; j < tasksModel.rowCount(modelIndex); ++j) {
                        taskIndexList.push(tasksModel.makeModelIndex(i, j));
                    }
                } else {
                    taskIndexList.push(modelIndex);
                }
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

function insertIndexAt(above, x, y) {
    if (above && above.itemIndex) {
        return above.itemIndex;
    } else {
        var distance = panel.vertical ? y : x;
        //var step = panel.vertical ? LayoutManager.taskWidth() : LayoutManager.taskHeight();
        var step = panel.realSize;
        var stripe = Math.ceil(distance / step);

        /* if (stripe === LayoutManager.calculateStripes()) {
            return tasksModel.count - 1;
        } else {
            return stripe * LayoutManager.tasksPerStripe();
        }*/

        return stripe-1;
    }
}


function publishIconGeometries(taskItems) {   
    for (var i = 0; i < taskItems.length - 1; ++i) {
        var task = taskItems[i];

        if (task !== undefined){
            if (task.IsLauncher !== true && task.IsStartup !== true) {
                tasksModel.requestPublishDelegateGeometry(tasksModel.makeModelIndex(task.itemIndex),
                                                          backend.globalRect(task), task);
            }
        }
    }
}

function taskPrefix(prefix) {
    var effectivePrefix;

    switch (plasmoid.location) {
    case PlasmaCore.Types.LeftEdge:
        effectivePrefix = "west-" + prefix;
        break;
    case PlasmaCore.Types.TopEdge:
        effectivePrefix = "north-" + prefix;
        break;
    case PlasmaCore.Types.RightEdge:
        effectivePrefix = "east-" + prefix;
        break;
    default:
        effectivePrefix = "south-" + prefix;
    }
    if (!frame.hasElementPrefix(effectivePrefix)) {
        return prefix;
    }
    return effectivePrefix;

}
