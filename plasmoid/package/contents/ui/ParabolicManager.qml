/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

// holds all the logic around parabolic effect signals into one place.
// ParabolicManager is responsible for triggering all the messages to tasks
// that are neighbour to the hovered task. This will help a lot to catch cases
// such as separators and proper clearing zoom.

Item {
    id: parManager

    property bool hasInternalSeparator: false

    property int firstRealTaskIndex: -1
    property int lastRealTaskIndex: -1
    property int countRealTasks: -1

    signal sglUpdateLowerItemScale(int delegateIndex, real newScale, real step);
    signal sglUpdateHigherItemScale(int delegateIndex, real newScale, real step);

    property int lastIndex: -1

    Connections{
        target: root
        onTasksCountChanged: parManager.updateTasksEdgesIndexes();
        onHiddenTasksUpdated: parManager.updateTasksEdgesIndexes();

        onClearZoomSignal: parManager.lastIndex = -1;
    }

    Component.onCompleted: {
        updateHasInternalSeparator();
        updateTasksEdgesIndexes();
        root.separatorsUpdated.connect(updateHasInternalSeparator);

        parManager.sglUpdateLowerItemScale.connect(sltTrackLowerItemScale);
        parManager.sglUpdateHigherItemScale.connect(sltTrackHigherItemScale);
    }

    Component.onDestruction: {
        root.separatorsUpdated.disconnect(updateHasInternalSeparator);

        parManager.sglUpdateLowerItemScale.disconnect(sltTrackLowerItemScale);
        parManager.sglUpdateHigherItemScale.disconnect(sltTrackHigherItemScale);
    }

    function updateTasksEdgesIndexes() {
        var newFirstTask = firstRealTask();
        var newLastTask = lastRealTask();

        if (newFirstTask !== firstRealTaskIndex || newLastTask !== lastRealTaskIndex ){
            firstRealTaskIndex = newFirstTask;
            lastRealTaskIndex = newLastTask;

            root.separatorsUpdated();
        }

        countRealTasks = realTasks();
    }

    function updateHasInternalSeparator() {
        var count = icList.contentItem.children.length;
        for (var i=0; i<count; ++i) {
            var task = icList.childAtIndex(i);

            if (task && task.isSeparator){
                hasInternalSeparator = true;
                return;
            }
        }

        hasInternalSeparator = false;
    }

    function hostRequestUpdateLowerItemScale(newScale, step){
        //! function called from host
        sglUpdateLowerItemScale(root.tasksCount-1, newScale, step);
    }

    function hostRequestUpdateHigherItemScale(newScale, step){
        //! function called from host
        sglUpdateHigherItemScale(0, newScale, step);
    }

    function sltTrackLowerItemScale(delegateIndex, newScale, step){
        //! send update signal to host
        if (latteBridge) {
            if (delegateIndex === -1) {
                latteBridge.parabolic.clientRequestUpdateLowerItemScale(newScale, step);
            } else if (newScale === 1 && delegateIndex>=0) {
                latteBridge.parabolic.clientRequestUpdateLowerItemScale(1, 0);
            }
        }
    }

    function sltTrackHigherItemScale(delegateIndex, newScale, step) {
        //! send update signal to host
        if (latteBridge) {
            if (delegateIndex >= root.tasksCount) {
                latteBridge.parabolic.clientRequestUpdateHigherItemScale(newScale, step);
            } else if (newScale === 1 && delegateIndex<root.tasksCount) {
                latteBridge.parabolic.clientRequestUpdateHigherItemScale(1, 0);
            }
        }
    }

    function applyParabolicEffect(index, currentMousePosition, center) {
        if (lastIndex === -1) {
            root.setGlobalDirectRender(false);
        }

        lastIndex = index;

        var rDistance = Math.abs(currentMousePosition  - center);

        //check if the mouse goes right or down according to the center
        var positiveDirection =  ((currentMousePosition  - center) >= 0 );

        if (Qt.application.layoutDirection === Qt.RightToLeft && !root.vertical) {
            positiveDirection = !positiveDirection;
        }

        var minimumZoom = 1;

        //finding the zoom center e.g. for zoom:1.7, calculates 0.35
        var zoomCenter = ((root.zoomFactor + minimumZoom)/2) - 1;

        //computes the in the scale e.g. 0...0.35 according to the mouse distance
        //0.35 on the edge and 0 in the center
        var firstComputation = (rDistance / center) * (zoomCenter-minimumZoom+1);

        //calculates the scaling for the neighbour tasks
        var bigNeighbourZoom = Math.min(1 + zoomCenter + firstComputation, root.zoomFactor);
        var smallNeighbourZoom = Math.max(1 + zoomCenter - firstComputation, minimumZoom);

        //bigNeighbourZoom = Number(bigNeighbourZoom.toFixed(4));
        //smallNeighbourZoom = Number(smallNeighbourZoom.toFixed(4));

        var leftScale;
        var rightScale;

        if(positiveDirection === true){
            rightScale = bigNeighbourZoom;
            leftScale = smallNeighbourZoom;
        } else {
            rightScale = smallNeighbourZoom;
            leftScale = bigNeighbourZoom;
        }

        sglUpdateHigherItemScale(index+1 , rightScale, 0);
        sglUpdateLowerItemScale(index-1, leftScale, 0);

        // console.debug(leftScale + "  " + rightScale + " " + index);
        return {leftScale:leftScale, rightScale:rightScale};
    }

    function clearTasksGreaterThan(index) {
    }

    function clearTasksLowerThan(index) {
    }

    function neighbourIsHovered(index) {
        if (parManager.lastIndex<0)
            return;

        if (Math.abs(index - parManager.lastIndex)<=1)
            return true;

        return false;
    }

    function availableLowerIndex(from) {
        var next = from;

        while (next>=0
               && (taskIsSeparator(next) || (root.showWindowsOnlyFromLaunchers && taskIsForcedHidden(next))) )
            next = next - 1;

        return next;
    }

    function availableHigherIndex(from) {
        var next = from;

        while (next<=root.tasksCount-1
               && (taskIsSeparator(next) || (root.showWindowsOnlyFromLaunchers && taskIsForcedHidden(next))) ) {
            next = next + 1;
        }

        return next;
    }

    function isSeparator(launcher){
        return (launcher.indexOf("latte-separator")!==-1 && launcher.indexOf(".desktop")!==1);
    }

    function taskIsSeparator(taskIndex){
        var task = icList.childAtIndex(taskIndex);

        return (task && !task.isForcedHidden && task.isSeparator);
    }

    function taskIsForcedHidden(taskIndex) {
        var task = icList.childAtIndex(taskIndex);

        //!tasks that become hidden there is a chance to have index===-1 and to not be
        //!able to be tracked down
        return ((!task && (taskIndex>=0 && taskIndex<tasksModel.count)) || task.isForcedHidden);
    }

    function separatorExists(separator){
        return (tasksModel.launcherPosition(separator)>=0);
    }

    //! the real index task after we take into account the separators before it
    //! for example the first task if there is a separator before it is 1, it isnt 0
    function realTaskIndex(pseudoIndex) {
        if (hasInternalSeparator) {
            var steps = pseudoIndex + 1;
            var pos = 0;

            while (steps > 0) {
                if (!taskIsSeparator(pos)) {
                    steps = steps - 1;
                }

                if (steps > 0)
                    pos = pos + 1;
            }

            return pos;
        } else {
            return pseudoIndex;
        }
    }

    //! the pseudo index task after we take into account the separators before it
    //! for example the third task if there is a separator before it is 1, it isnt 2
    function pseudoTaskIndex(realIndex) {
        var pseudoIndex = realIndex;

        if (hasInternalSeparator) {
            for (var i=0; i<realIndex; i++){
                if (taskIsSeparator(i)) {
                    pseudoIndex = pseudoIndex-1;
                }
            }
        }

        return pseudoIndex + root.tasksBaseIndex;
    }

    //! first available task index found after consequent internal separators or hidden tasks in the start
    function firstRealTask() {
        if (hasInternalSeparator) {
            var i=0;

            while (i<=root.tasksCount-1) {
                if (!taskIsSeparator(i) && !taskIsForcedHidden(i)) {
                    return i;
                }

                i = i + 1;
            }
        }

        return root.tasksCount > 0 ? 0 : -1;
    }

    //! last available task index found after consequent internal separators in the end
    function lastRealTask() {
        if (hasInternalSeparator || root.showWindowsOnlyFromLaunchers) {
            var i = tasksModel.count - 1;

            while (i>=0) {
                if (!taskIsSeparator(i) && !taskIsForcedHidden(i) ) {
                    return i;
                }

                i = i - 1;
            }
        }

        return root.tasksCount > 0 ? root.tasksCount-1 : -1;
    }

    //! the real number of tasks if we remove the internal separators and hidden windows in the end
    function realTasks() {
        var space = lastRealTaskIndex - firstRealTaskIndex;

        if (space >= 0) {
            var ignored = 0;
            for(var i=firstRealTaskIndex; i<lastRealTaskIndex; ++i) {
                if (taskIsSeparator(i) || taskIsForcedHidden(i)) {
                    ignored = ignored + 1;
                }
            }

            return space + 1 - ignored;
        }

        return 0;
    }

    function freeAvailableSeparatorName() {
        var available = false;
        var no = 1;

        var separatorName = "";

        while(!available && no
              <20) {
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

    function lastPresentSeparatorName() {
        var max = -1;
        var arrayPos = -1;

        var sLength = separators.length;

        for (var i=0; i<sLength; ++i) {
            //!safety checker
            if (i>=separators.length)
                return false;

            if (separators[i].index > max) {
                max = separators[i].index;
                arrayPos = i;
            }
        }

        if (arrayPos>-1)
            return separators[arrayPos].launcherUrl;
        else
            return "";
    }
}
