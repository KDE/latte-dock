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

// holds all the logic around parabolic effect signals into one place.
// ParabolicManager is responsible for triggering all the messages to tasks
// that are neighbour to the hovered task. This will help a lot to catch cases
// such as separators and proper clearing zoom.

Item {
    id: parManager

    property bool hasInternalSeparator: false
    property int internalSeparatorPos: -1

    property int firstRealTaskIndex: -1
    property int lastRealTaskIndex: -1

    //tasks that change state (launcher,startup,window) and
    //at the next state must look the same
    //(id, mScale)
    property variant frozenTasks: []

    //the internal separators in the form
    //(launcherUrl, index)
    property variant separators: []

    //!this is used in order to update the index when the signal is for applets
    //!outside the latte plasmoid
    function updateIdSendScale(index, zScale, zStep){
        if ((index>=0 && index<=root.tasksCount-1) || (!root.latteDock)){
            root.updateScale(index, zScale, zStep);
            return -1;
        } else{
            var appletId = latteDock.latteAppletPos;
            if (index<0)
                appletId = latteDock.parabolicManager.availableLowerId(latteDock.latteAppletPos + index);
            else if (index>root.tasksCount-1){
                var step=index-root.tasksCount+1;
                appletId = latteDock.parabolicManager.availableHigherId(latteDock.latteAppletPos + step);
            }

            latteDock.updateScale(appletId, zScale, zStep);
            return appletId;
        }
    }

    function applyParabolicEffect(index, currentMousePosition, center) {
        var rDistance = Math.abs(currentMousePosition  - center);

        //check if the mouse goes right or down according to the center
        var positiveDirection =  ((currentMousePosition  - center) >= 0 );

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

        // console.debug(leftScale + "  " + rightScale + " " + index);

        //first applets accessed
        var gPAppletId = -1;
        var lPAppletId = -1;

        //secondary applets accessed to restore zoom
        var gAppletId = -1;
        var lAppletId = -1;

        var gStep = 1;
        var lStep = 1;



        var nextG1 = availableHigherIndex(index+1);
        var nextL1 = availableLowerIndex(index-1);

        gPAppletId = updateIdSendScale(nextG1, rightScale, 0);
        lPAppletId = updateIdSendScale(nextL1, leftScale, 0);


        if (latteDock) {
            if (gPAppletId > -1)
                gStep = Math.abs(gPAppletId - latteDock.latteAppletPos);
            else if (lPAppletId > -1)
                lStep = Math.abs(lPAppletId - latteDock.latteAppletPos);
        }

        if (gPAppletId === -1) {
            nextG1 = availableHigherIndex(nextG1+1);
        }

        if (lPAppletId === -1) {
            nextL1 = availableLowerIndex(nextL1-1);
        }

        gAppletId = updateIdSendScale(nextG1+gStep, 1, 0);
        lAppletId = updateIdSendScale(nextL1-lStep, 1, 0);

        clearTasksGreaterThan(nextG1+1);
        clearTasksLowerThan(nextL1-1);

        if (latteDock){
            if (gAppletId > -1)
                latteDock.parabolicManager.clearAppletsGreaterThan(gAppletId);
            else
                latteDock.parabolicManager.clearAppletsGreaterThan(latteDock.latteAppletPos);

            if (lAppletId > -1)
                latteDock.parabolicManager.clearAppletsLowerThan(lAppletId);
            else
                latteDock.parabolicManager.clearAppletsLowerThan(latteDock.latteAppletPos);
        }

        return {leftScale:leftScale, rightScale:rightScale};
    }

    function clearTasksGreaterThan(index) {
        if (index<root.tasksCount-1){
            for(var i=index+1; i<root.tasksCount; ++i)
                root.updateScale(i, 1, 0);
        }
    }

    function clearTasksLowerThan(index) {
        if (index>0 && root.tasksCount>2) {
            for(var i=0; i<index; ++i)
                root.updateScale(i, 1, 0);
        }
    }

    function neighbourIsHovered(index) {
        if (icList.hoveredIndex<0)
            return;

        if (Math.abs(index - icList.hoveredIndex)<=1)
            return true;

        var hovered = icList.hoveredIndex;

        if (root.hasInternalSeparator && hovered>-1
                && (hovered === index
                    || hovered === availableLowerIndex(index-1)
                    || hovered === availableHigherIndex(index+1) )) {
                return true;
        }

        return false;
    }

    //! Frozen Tasks functions

    function getFrozenTask(identifier) {
        for(var i=0; i<frozenTasks.length; ++i) {
            if (frozenTasks[i].id === identifier) {
                return frozenTasks[i];
            }
        }
    }

    function removeFrozenTask(identifier) {
        var taskIndex = -1;
        for(var i=0; i<frozenTasks.length; ++i) {
            if (frozenTasks[i].id === identifier) {
                taskIndex = i;
            }
        }

        if (taskIndex > -1) {
            frozenTasks.splice(taskIndex, 1);
        }
    }

    function setFrozenTask(identifier, scale) {
        var frozenTaskExists = false;
        console.log("SET FROZEN :: "+identifier+" - "+scale);
        var frozenTask = getFrozenTask(identifier);

        if (frozenTask) {
            frozenTask.mScale = scale;
        } else {
            frozenTasks.push({id: identifier, mScale: scale});
        }
    }

    //! SEPARATORS functions

    // update the registered separators
    // launcherUrl, no = add/update separator
    // launcherUrl, -1 = remove separator

    function setSeparator(launcher, taskIndex) {
        var currentPos = separatorArrayPos(launcher);
        var updated = false;

        if (currentPos === -1 && taskIndex >=0){
            //add that separator
            separators.push({launcherUrl: launcher, index: taskIndex});
            updated = true;
        } else if (currentPos>-1 && taskIndex === -1) {
            //remove that separator
            separators.splice(currentPos,1);
            updated = true;
        } else if (currentPos>-1 && taskIndex>-1 && separators[currentPos].index !== taskIndex) {
            //update that separator
            separators[currentPos].index = taskIndex;
            updated = true;
        }

        //if (separators.length > 0)
        //    console.log(separators[0].launcherUrl+ " _________ " + separators[0].index);

        if (updated) {
            //console.log("message sent...");
            hasInternalSeparator = separators.length > 0;
            internalSeparatorPos = hasInternalSeparator ? separators[0].index : -1;
            firstRealTaskIndex = firstRealTask();
            lastRealTaskIndex = lastRealTask();

            root.separatorsUpdated();
        }
    }

    function separatorArrayPos(launcher) {
        var res = -1;

        for (var i=0; i<separators.length; ++i) {
            if (separators[i].launcherUrl === launcher)
                return i;
        }

        return res;
    }

    function availableLowerIndex(from) {
        var next = from;

        while (next>=0 && taskIsSeparator(next))
            next = next - 1;

        return next;
    }

    function availableHigherIndex(from) {
        var next = from;

        while (next<=root.tasksCount-1 && taskIsSeparator(next))
            next = next + 1;

        return next;
    }

    function isSeparator(launcher){
        return (launcher.indexOf("latte-separator")!==-1 && launcher.indexOf(".desktop")!==1);
    }

    function taskIsSeparator(taskIndex){
        for (var i=0; i<separators.length; ++i) {
            if (separators[i].index === taskIndex)
                return true;
        }

        return false;
    }

    function separatorExists(separator){
        if (!isSeparator(separator))
            return false;

        for (var i=0; i<separators.length; ++i) {
            if (separators[i].launcherUrl.indexOf(separator)>=0)
                return true;
        }

        return false;
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

    //! first available task index found after consequent internal separators in the start
    function firstRealTask() {
        if (hasInternalSeparator) {
            var i=0;

            while (i<=root.tasksCount-1) {
                if (!taskIsSeparator(i)) {
                    return i;
                }

                i = i + 1;
            }
        }

        return -1;
    }

    //! last available task index found after consequent internal separators in the end
    function lastRealTask() {
        if (hasInternalSeparator) {
            var i=root.tasksCount - 1;

            while (i>=0) {
                if (!taskIsSeparator(i)) {
                    return i;
                }

                i = i - 1;
            }
        }

        return -1;
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

        for (var i=0; i<separators.length; ++i) {
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
