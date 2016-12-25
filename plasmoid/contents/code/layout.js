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

function horizontalMargins() {
    return taskFrame.margins.left + taskFrame.margins.right;
}

function verticalMargins() {
    return taskFrame.margins.top + taskFrame.margins.bottom;
}

function adjustMargin(height, margin) {
    var available = height - verticalMargins();

    if (available < units.iconSizes.small) {
        return Math.floor((margin * (units.iconSizes.small / available)) / 3);
    }

    return margin;
}

function launcherLayoutTasks() {
    return Math.round(tasksModel.launcherCount / Math.floor(preferredMinWidth() / launcherWidth()));
}

function launcherLayoutWidthDiff() {
    return (launcherLayoutTasks() * taskWidth()) - (tasksModel.launcherCount * launcherWidth());
}

function logicalTaskCount() {
    var count = (tasksModel.count - tasksModel.launcherCount) + launcherLayoutTasks();

    return Math.max(tasksModel.count ? 1 : 0, count);
}

function maxStripes() {
    var length = tasks.vertical ? taskList.width : taskList.height;
    var minimum = tasks.vertical ? preferredMinWidth() : preferredMinHeight();

    return Math.min(plasmoid.configuration.maxStripes, Math.max(1, Math.floor(length / minimum)));
}

function tasksPerStripe() {
    if (plasmoid.configuration.forceStripes) {
        return Math.ceil(logicalTaskCount() / maxStripes());
    } else {
        var length = tasks.vertical ? taskList.height : taskList.width;
        var minimum = tasks.vertical ? preferredMinHeight() : preferredMinWidth();

        return Math.floor(length / minimum);
    }
}

function calculateStripes() {
    var stripes = plasmoid.configuration.forceStripes ? plasmoid.configuration.maxStripes : Math.min(plasmoid.configuration.maxStripes, Math.ceil(logicalTaskCount() / tasksPerStripe()));

    return Math.min(stripes, maxStripes());
}

function full() {
    return (maxStripes() == calculateStripes());
}

function optimumCapacity(width, height) {
    var length = tasks.vertical ? height : width;
    var maximum = tasks.vertical ? preferredMaxHeight() : preferredMaxWidth();

    return (Math.ceil(length / maximum) * maxStripes());
}

function layoutWidth() {
    if (plasmoid.configuration.forceStripes && !tasks.vertical) {
        return Math.min(tasks.width, Math.max(preferredMaxWidth(), tasksPerStripe() * preferredMaxWidth()));
    } else {
        return tasks.width;
    }
}

function layoutHeight() {
    if (plasmoid.configuration.forceStripes && tasks.vertical) {
        return Math.min(tasks.height, Math.max(preferredMaxHeight(), tasksPerStripe() * preferredMaxHeight()));
    } else {
        return tasks.height;
    }
}

function preferredMinWidth() {
    var width = launcherWidth();

    if (!tasks.vertical && !tasks.iconsOnly) {
        width += (units.smallSpacing * 2) + (theme.mSize(theme.defaultFont).width * 12);
    }

    return width;
}

function preferredMaxWidth() {
    if (tasks.iconsOnly) {
        if (tasks.vertical) {
            return tasks.width + verticalMargins();
        } else {
            return tasks.height + horizontalMargins();
        }
    }

    if (plasmoid.configuration.groupingStrategy != 0 && !plasmoid.configuration.groupPopups) {
        return preferredMinWidth();
    }

    return Math.floor(preferredMinWidth() * 1.6);
}

function preferredMinHeight() {
    // TODO FIXME UPSTREAM: Port to proper font metrics for descenders once we have access to them.
    return theme.mSize(theme.defaultFont).height + 4;
}

function preferredMaxHeight() {
    return verticalMargins() + Math.min(units.iconSizes.small * 3, theme.mSize(theme.defaultFont).height * 3);
}

function taskWidth() {
    if (tasks.vertical) {
        return Math.floor(taskList.width / calculateStripes());
    } else {
        if (full() && Math.max(1, logicalTaskCount()) > tasksPerStripe()) {
            return Math.floor(taskList.width / Math.ceil(logicalTaskCount() / maxStripes()));
        } else {
            return Math.min(preferredMaxWidth(), Math.floor(taskList.width / Math.min(logicalTaskCount(), tasksPerStripe())));
        }
    }
}

function taskHeight() {
    if (tasks.vertical) {
        if (full() && Math.max(1, logicalTaskCount()) > tasksPerStripe()) {
            return Math.floor(taskList.height / Math.ceil(logicalTaskCount() / maxStripes()));
        } else {
            return Math.min(preferredMaxHeight(), Math.floor(taskList.height / Math.min(logicalTaskCount(), tasksPerStripe())));
        }
    } else {
        return Math.floor(taskList.height / calculateStripes());
    }
}

function launcherWidth() {
    var baseWidth = tasks.vertical ? preferredMinHeight() : Math.min(tasks.height, units.iconSizes.small * 3);

    return (baseWidth + horizontalMargins())
        - (adjustMargin(baseWidth, taskFrame.margins.top) + adjustMargin(baseWidth, taskFrame.margins.bottom));
}

function layout(container) {
    var item;
    var stripes = calculateStripes();
    var taskCount = tasksModel.count - tasksModel.launcherCount;
    var width = taskWidth();
    var adjustedWidth = width;
    var height = taskHeight();

    if (!tasks.vertical && stripes == 1 && taskCount)
    {
        var shrink = ((tasksModel.count - tasksModel.launcherCount) * preferredMaxWidth())
            + (tasksModel.launcherCount * launcherWidth()) > taskList.width;
        width = Math.min(shrink ? width + Math.floor(launcherLayoutWidthDiff() / taskCount) : width,
            preferredMaxWidth());
    }

    for (var i = 0; i < container.count; ++i) {
        item = container.itemAt(i);

        if (!item) {
            continue;
        }

        adjustedWidth = width;

        if (!tasks.vertical && !tasks.iconsOnly && (plasmoid.configuration.separateLaunchers || stripes == 1)) {
            if (item.m.IsLauncher === true) {
                adjustedWidth = launcherWidth();
            } else if (stripes > 1 && i == tasksModel.launcherCount) {
                adjustedWidth += launcherLayoutWidthDiff();
            }
        }

        item.width = adjustedWidth;
        item.height = height;
        item.visible = true;
    }
}
