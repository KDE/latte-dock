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
    property bool internalSeparatorHidden: (internalSeparatorPos === 0) || (internalSeparatorPos === root.tasksCount - 1)
    property int internalSeparatorPos: -1

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

        if(!hasInternalSeparator || Math.abs(index-internalSeparatorPos)>=2){
            //console.log("--- task style 1...");
            gPAppletId = updateIdSendScale(index+1, rightScale, 0);
            lPAppletId = updateIdSendScale(index-1, leftScale, 0);

            //console.log("index:"+index + " lattePos:"+latteDock.latteAppletPos);
            //console.log("gApp:"+gPAppletId+" lApp:"+lPAppletId);

            if (latteDock) {
                if (gPAppletId > -1)
                    gStep = Math.abs(gPAppletId - latteDock.latteAppletPos);
                else if (lPAppletId > -1)
                    lStep = Math.abs(lPAppletId - latteDock.latteAppletPos);
            }
            //console.log("gs:"+gStep+" ls:"+lStep);

            gAppletId = updateIdSendScale(index+gStep+1, 1, 0);
            lAppletId = updateIdSendScale(index-lStep-1, 1, 0);

            //console.log(" cgApp:"+gAppletId+" clApp:"+lAppletId);

            clearTasksGreaterThan(index+1);
            clearTasksLowerThan(index-1);
        } else if(root.internalSeparatorPos>=0) {
            if(internalSeparatorPos === index+1){
                //console.log("--- task style 2...");
                gPAppletId = updateIdSendScale(index+2, rightScale, 0);
                lPAppletId = updateIdSendScale(index-1, leftScale, 0);

                //console.log("index:"+index + " lattePos:"+latteDock.latteAppletPos);
                //console.log("gApp:"+gPAppletId+" lApp:"+lPAppletId);

                if (latteDock) {
                    gStep = 2;
                    if (gPAppletId > -1)
                        gStep = Math.abs(gPAppletId - latteDock.latteAppletPos);
                    else if (lPAppletId > -1)
                        lStep = Math.abs(lPAppletId - latteDock.latteAppletPos);
                }

                //console.log("gs:"+gStep+" ls:"+lStep);

                gAppletId = updateIdSendScale(index+gStep+2, 1, 0);
                lAppletId = updateIdSendScale(index-lStep-1, 1, 0);

                //console.log(" cgApp:"+gAppletId+" clApp:"+lAppletId);

                clearTasksGreaterThan(index+2);
                clearTasksLowerThan(index-1);
            } else if(internalSeparatorPos === index-1) {
                //console.log("--- task style 3...");
                gPAppletId = updateIdSendScale(index+1, rightScale, 0);
                lPAppletId = updateIdSendScale(index-2, leftScale, 0);

                //console.log("index:"+index + " lattePos:"+latteDock.latteAppletPos);
                //console.log("gApp:"+gPAppletId+" lApp:"+lPAppletId);

                if (latteDock) {
                    gStep = 1;
                    lStep = 2;
                    if (gPAppletId > -1)
                        gStep = Math.abs(gPAppletId - latteDock.latteAppletPos);
                    else if (lPAppletId > -1)
                        lStep = Math.abs(lPAppletId - latteDock.latteAppletPos);
                }

                //console.log("gs:"+gStep+" ls:"+lStep);

                gAppletId = updateIdSendScale(index+gStep+1, 1, 0);
                lAppletId = updateIdSendScale(index-lStep-2, 1, 0);

                //console.log(" cgApp:"+gAppletId+" clApp:"+lAppletId);

                clearTasksGreaterThan(index+1);
                clearTasksLowerThan(index-2);
            }
        }

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

        if (root.hasInternalSeparator && Math.abs(index - icList.hoveredIndex) === 2) {
            if ( (index === root.hoveredIndex - 2 && root.internalSeparatorPos === root.hoveredIndex - 1)
                    || (index === root.hoveredIndex + 2 && root.internalSeparatorPos === root.hoveredIndex + 1))
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

        if (updated) {
            hasInternalSeparator = separators.length > 0;
            internalSeparatorPos = hasInternalSeparator ? separators[0].index : -1;
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

        while (separators.indexOf(next) !== -1 || hidden.indexOf(next) !== -1)
            next = next - 1;

        return next;
    }

    function availableHigherIndex(from) {
        var next = from;

        while (separators.indexOf(next) !== -1 || hidden.indexOf(next) !== -1)
            next = next + 1;

        return next;
    }

    function isSeparator(launcher){
        return (launcher.indexOf("latte-separator")!==-1 && launcher.indexOf(".desktop")!==1);
    }
}
