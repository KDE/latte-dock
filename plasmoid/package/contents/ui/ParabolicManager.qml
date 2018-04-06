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

import org.kde.latte 0.1 as Latte

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

    //tasks that change state (launcher,startup,window) and
    //at the next state must look the same
    //(id, mScale)
    property variant frozenTasks: []

    //the internal separators in the form
    //(launcherUrl, index)
    property variant separators: []

    //new launchers in order to be moved in correct place
    //(launcher, pos)
    property variant launchersToBeMoved: []

    Connections{
        target: root
        onTasksCountChanged:parManager.updateTasksEdgesIndexes();
        onDragSourceChanged: {
            if (!root.dragSource && parManager.hasInternalSeparator) {
                //! Send the internal separators to other docks
                var tasks = icList.contentItem.children;
                var size = icList.contentItem.children.length;
                var tempSeparatorsArray = [];
                var tempSeparatorsIndexes = [];

                for(var i=0; i<size; ++i){
                    if (i>=icList.contentItem.children.length) {
                        break;
                    }

                    if (tasks[i] && tasks[i].isSeparator) {
                        tempSeparatorsArray.push(tasks[i].launcherUrl);
                        tempSeparatorsIndexes.push(tasks[i].itemIndex);
                    }
                }

                if (tempSeparatorsArray.length > 0) {
                    if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                        //reorder to lowest
                        for(var i=0; i<tempSeparatorsIndexes.length; ++i) {
                            var lowIndex = i;
                            for(var j=i; j<tempSeparatorsIndexes.length; ++j) {
                                if (tempSeparatorsIndexes[j]<tempSeparatorsIndexes[lowIndex]) {
                                    lowIndex = j;
                                }
                            }

                            if (lowIndex !== i) {
                                //moving
                                var tempInd = tempSeparatorsIndexes[lowIndex];
                                tempSeparatorsIndexes.splice(lowIndex, 1);
                                tempSeparatorsIndexes.splice(i, 0, tempInd);

                                var tempName = tempSeparatorsArray[lowIndex];
                                tempSeparatorsArray.splice(lowIndex, 1);
                                tempSeparatorsArray.splice(i, 0, tempName);
                            }
                        }

                        latteDock.universalLayoutManager.launchersSignals.internalSeparators(root.managedLayoutName,
                                                                                             plasmoid.id,
                                                                                             latteDock.launchersGroup,
                                                                                             tempSeparatorsArray,
                                                                                             tempSeparatorsIndexes);
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        updateTasksEdgesIndexes();
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

        // console.debug(leftScale + "  " + rightScale + " " + index);

        //first applets accessed
        var gPAppletId = -1;
        var lPAppletId = -1;

        //secondary applets accessed to restore zoom
        var gAppletId = -1;
        var lAppletId = -1;

        var gStep = 1;
        var lStep = 1;


        //console.log("--- task style 2...");
        var aGId1 = availableHigherIndex(index+1);
        var aLId1 = availableLowerIndex(index-1);

        gPAppletId = updateIdSendScale(aGId1, rightScale, 0);
        lPAppletId = updateIdSendScale(aLId1, leftScale, 0);

        // console.log("index:"+index + " lattePos:"+latteDock.latteAppletPos);
        // console.log("gApp:"+gPAppletId+" lApp:"+lPAppletId+ " aG1:"+aGId1+" aLId1:"+aLId1);

        gStep = aGId1 - index;
        lStep = index - aLId1;

        if (latteDock) {
            if (gPAppletId > -1)
                gStep = Math.abs(gPAppletId - latteDock.latteAppletPos + (root.tasksCount-1-index));

            if (lPAppletId > -1)
                lStep = Math.abs(lPAppletId - latteDock.latteAppletPos - index);
        }

        //console.log("gs:"+gStep+" ls:"+lStep);

        gAppletId = updateIdSendScale(index+gStep+1, 1, 0);
        lAppletId = updateIdSendScale(index-lStep-1, 1, 0);

        //console.log(" cgApp:"+gAppletId+" clApp:"+lAppletId);

        clearTasksGreaterThan(aGId1+1);
        clearTasksLowerThan(aLId1-1);

        if (latteDock){
            if (gAppletId > -1)
                latteDock.parabolicManager.clearAppletsGreaterThan(gAppletId);
            else
                latteDock.parabolicManager.clearAppletsGreaterThan(latteDock.latteAppletPos);

            if (lAppletId > -1)
                latteDock.parabolicManager.clearAppletsLowerThan(lAppletId);
            else
                latteDock.parabolicManager.clearAppletsLowerThan(latteDock.latteAppletPos-1);
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
            //console.log("add separator:"+launcher+" at:"+taskIndex);
            separators.push({launcherUrl: launcher, index: taskIndex});
            updated = true;
        } else if (currentPos>-1 && taskIndex === -1) {
            //remove that separator
            //console.log("remove separator:"+launcher);
            separators.splice(currentPos,1);
            updated = true;
        } else if (currentPos>-1 && taskIndex>-1 && separators[currentPos].index !== taskIndex) {
            //update that separator
            //console.log("update separator:"+launcher+" from:"+separators[currentPos].index+" -> "+taskIndex);

            separators[currentPos].index = taskIndex;
            updated = true;
        }

        //if (separators.length > 0)
        //    console.log(separators[0].launcherUrl+ " _________ " + separators[0].index);

        if (updated) {
            //console.log("message sent...");
            hasInternalSeparator = separators.length > 0;

            updateTasksEdgesIndexes();

            root.separatorsUpdated();
        }
    }

    function separatorArrayPos(launcher) {
        var res = -1;
        var sLength = separators.length;

        for (var i=0; i<sLength; ++i) {
            //!safety checker
            if (i>=separators.length)
                return false;

            if (separators[i].launcherUrl === launcher)
                return i;
        }

        return res;
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
        var sLength = separators.length;

        for (var i=0; i<sLength; ++i) {
            //!safety checker
            if (i>=separators.length)
                return false;

            if (separators[i].index === taskIndex)
                return true;
        }

        return false;
    }

    function taskIsForcedHidden(taskIndex) {
        var task = icList.childAtIndex(taskIndex);

        //!tasks that become hidden there is a chance to have index===-1 and to not be
        //!able to be tracked down
        return ((!task && (taskIndex>=0 && taskIndex<root.tasksCount)) || task.isForcedHidden);
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

        return pseudoIndex + root.tasksNumbersBase;
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
            var internseparators = 0;
            for(var i=firstRealTaskIndex; i<lastRealTaskIndex; ++i) {
                if (taskIsSeparator(i)) {
                    internseparators = internseparators + 1;
                }
            }

            return space + 1 - internseparators;
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

    //! launchersToBeMoved, new launchers to have been added and must be repositioned
    function addLauncherToBeMoved(launcherUrl, toPos) {
        if (!isLauncherToBeMoved(launcherUrl)) {
            launchersToBeMoved.push({launcher: launcherUrl, pos: Math.max(0,toPos)});
            //console.log("-add launcher-");
            //printLaunchersToBeMoved()
        }
    }

    function printLaunchersToBeMoved(){
        for (var j=0; j<launchersToBeMoved.length; ++j){
            console.log(launchersToBeMoved[j].launcher+ " - "+launchersToBeMoved[j].pos);
        }
    }

    function moveLauncherToCorrectPos(launcherUrl, from) {
        if (isLauncherToBeMoved(launcherUrl)) {
            launchersToBeMovedTimer.from = from;
            launchersToBeMovedTimer.to = posOfLauncherToBeMoved(launcherUrl);
            launchersToBeMovedTimer.launcherUrl = launcherUrl

            removeLauncherToBeMoved(launcherUrl);
            launchersToBeMovedTimer.start();
        }
    }

    function removeLauncherToBeMoved(launcherUrl) {
        if (isLauncherToBeMoved(launcherUrl)) {
            var sLength = launchersToBeMoved.length;
            var index = -1;

            for (var i=0; i<sLength; ++i) {
                //!safety checker
                if (i>=launchersToBeMoved.length)
                    return -1;

                if (launchersToBeMoved[i].launcher === launcherUrl) {
                    index = i;
                    break;
                }
            }

            if (index > -1) {
                // console.log("removing launcher to be moved:: "+launcherUrl);
                launchersToBeMoved.splice(index, 1);
            }
        }
    }

    function posOfLauncherToBeMoved(launcherUrl) {
        var sLength = launchersToBeMoved.length;

        for (var i=0; i<sLength; ++i) {
            //!safety checker
            if (i>=launchersToBeMoved.length)
                return -1;

            if (launchersToBeMoved[i].launcher === launcherUrl)
                return launchersToBeMoved[i].pos;
        }

        return -1;
    }

    function isLauncherToBeMoved(launcher) {
        return (posOfLauncherToBeMoved(launcher) >= 0);
    }

    //!Trying to avoid a binding loop in TaskDelegate for modelLauncherUrl and
    //!proper updating in separators indexes
    Timer {
        id: launchersToBeMovedTimer
        interval: 50
        property int from: -1
        property int to: -1

        property string launcherUrl: ""

        onTriggered: {
            //console.log("to be moved: "+launcherUrl + " - " + from +" -> "+to)

            tasksModel.move(from, to);
            if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                latteDock.universalLayoutManager.launchersSignals.moveTask(root.managedLayoutName,
                                                                           plasmoid.id, latteDock.launchersGroup, from, to);
            }

            if (isSeparator(launcherUrl)) {
                setSeparator(launcherUrl,to);
            }

            tasksModel.syncLaunchers();
        }
    }
}
