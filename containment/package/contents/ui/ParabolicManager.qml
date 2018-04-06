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
import org.kde.plasma.core 2.0 as PlasmaCore

// holds all the logic around parabolic effect signals into one place.
// ParabolicManager is responsible for triggering all the messages to applets
// that are neighbour to the hovered applet. This will help a lot to catch cases
// such as separator applets, hidden applets and proper clearing zoom.

Item {
    id: parManager

    property var hidden: []
    property var separators: []

    //!this is used in order to update the index when the signal is for the internal latte plasmoid
    //!this is used in order to update the index when the signal is for the internal latte plasmoid
    function updateIdSendScale(appIndex, index, zScale, zStep){
        // console.log(appIndex + " _ "+index+ " _ "+zScale + " _ "+ zStep);
        if(root.latteApplet
                && root.latteApplet.parabolicManager.firstRealTaskIndex !== -1
                && ((appIndex<root.latteAppletPos && index>=root.latteAppletPos)
                    || (appIndex>root.latteAppletPos && index<=root.latteAppletPos)) ){
            var appStep = Math.abs(root.latteAppletPos-appIndex);
            var signalStep = Math.abs(index - appIndex);

            var taskIndex = -1;
            var internSepStep = 0;
            if(appIndex<root.latteAppletPos){
                if (root.latteApplet.parabolicManager.taskIsSeparator(0)
                        || (!root.showWindowsWithNoLaunchers && root.latteApplet.parabolicManager.taskIsForcedHidden(0)) )
                    internSepStep = root.latteApplet.parabolicManager.availableHigherIndex(0);

                taskIndex = signalStep-appStep+internSepStep;
                if (root.latteApplet.parabolicManager.taskIsSeparator(taskIndex))
                    taskIndex = root.latteApplet.parabolicManager.availableHigherIndex(taskIndex + 1);

                //console.log("normal:" + taskIndex + " step:"+internSepStep + " zoom:"+zScale);
            } else if (appIndex>root.latteAppletPos){
                if (root.latteApplet.parabolicManager.taskIsSeparator(root.tasksCount-1)
                        || (!root.showWindowsWithNoLaunchers && root.latteApplet.parabolicManager.taskIsForcedHidden(root.tasksCount-1)) )
                    internSepStep = Math.abs(root.tasksCount-1 - root.latteApplet.parabolicManager.availableLowerIndex(root.tasksCount-1));

                taskIndex = root.tasksCount-1 - (signalStep-appStep) - internSepStep;
                if (root.latteApplet.parabolicManager.taskIsSeparator(taskIndex))
                    taskIndex = root.latteApplet.parabolicManager.availableLowerIndex(taskIndex - 1);

                //console.log("reverse:" + taskIndex + " step:"+internSepStep + " zoom:"+zScale);
            }

            root.latteApplet.updateScale(taskIndex, zScale,zStep);

            return taskIndex;
        } else {
            // console.log("ch 2...");
            root.updateScale(index, zScale, zStep);
            return -1;
        }
    }


    function applyParabolicEffect(index, currentMousePosition, center) {
        var rDistance = Math.abs(currentMousePosition  - center);

        //check if the mouse goes right or down according to the center
        var positiveDirection =  ((currentMousePosition  - center) >= 0 );

        if (Qt.application.layoutDirection === Qt.RightToLeft && !root.isVertical) {
            positiveDirection = !positiveDirection;
        }

        //finding the zoom center e.g. for zoom:1.7, calculates 0.35
        var zoomCenter = (root.zoomFactor - 1) / 2

        //computes the in the scale e.g. 0...0.35 according to the mouse distance
        //0.35 on the edge and 0 in the center
        var firstComputation = (rDistance / center) * zoomCenter;

        //calculates the scaling for the neighbour tasks
        var bigNeighbourZoom = Math.min(1 + zoomCenter + firstComputation, root.zoomFactor);
        var smallNeighbourZoom = Math.max(1 + zoomCenter - firstComputation, 1);

        //bigNeighbourZoom = Number(bigNeighbourZoom.toFixed(4));
        //smallNeighbourZoom = Number(smallNeighbourZoom.toFixed(4));

        var leftScale;
        var rightScale;

        if(positiveDirection === true){
            rightScale = bigNeighbourZoom;
            leftScale = smallNeighbourZoom;
        }
        else {
            rightScale = smallNeighbourZoom;
            leftScale = bigNeighbourZoom;
        }

        var gAppletIndex = -1;
        var lAppletIndex = -1;

        var gTaskIndex = -1;
        var lTaskIndex = -1;

        var tLIndex = -1;
        var tHIndex = -1;

        var gAppN = availableHigherId(index+1);
        var lAppN = availableLowerId(index-1);

        var latteNeighbour = root.latteApplet && ((gAppN === root.latteAppletPos) || (lAppN === root.latteAppletPos ));

        if(!root.latteApplet || !latteNeighbour || !root.hasInternalSeparator
                || (root.latteApplet && root.latteApplet.parabolicManager.firstRealTaskIndex===-1)
                || (root.latteApplet && root.hasInternalSeparator
                    && ((!root.latteApplet.parabolicManager.taskIsSeparator(0) && !root.latteApplet.parabolicManager.taskIsSeparator(root.tasksCount-1))
                        || (root.latteApplet.parabolicManager.taskIsSeparator(0) && index>root.latteAppletPos)
                        || (root.latteApplet.parabolicManager.taskIsSeparator(root.tasksCount-1) && index<root.latteAppletPos)))
                ){
            //console.log("style 1...");
            gAppletIndex = gAppN;
            lAppletIndex = lAppN;

            updateIdSendScale(index, gAppletIndex, rightScale, 0);
            updateIdSendScale(index, lAppletIndex, leftScale, 0);

            tLIndex = (lAppletIndex !== root.latteAppletPos) ? availableLowerId(lAppletIndex-1) : lAppletIndex-1;
            tHIndex = (gAppletIndex !== root.latteAppletPos) ? availableHigherId(gAppletIndex+1) : gAppletIndex+1;

            gTaskIndex = updateIdSendScale(index, tHIndex, 1 ,0);
            lTaskIndex = updateIdSendScale(index, tLIndex, 1, 0);
        } else{
            if(gAppN === root.latteAppletPos && root.latteApplet.parabolicManager.taskIsSeparator(0)){
                //console.log("style 2...");
                gAppletIndex = availableHigherId(index+1);
                lAppletIndex= availableLowerId(index-1);
                updateIdSendScale(index, gAppletIndex, rightScale, 0);
                updateIdSendScale(index, lAppletIndex, leftScale, 0);

                tLIndex = availableLowerId(lAppletIndex-1);
                gTaskIndex = updateIdSendScale(index, gAppletIndex+1, 1, 0);
                lTaskIndex = updateIdSendScale(index, tLIndex, 1, 0);
            } else if(lAppN === root.latteAppletPos && root.latteApplet.parabolicManager.taskIsSeparator(root.tasksCount-1))  {
                //console.log("style 3...");
                gAppletIndex = gAppN;
                lAppletIndex= lAppN;
                updateIdSendScale(index, lAppletIndex, leftScale, 0);
                updateIdSendScale(index, gAppletIndex, rightScale, 0);

                tHIndex = availableHigherId(gAppletIndex+1);
                gTaskIndex = updateIdSendScale(index, tHIndex, 1, 0);
                lTaskIndex = updateIdSendScale(index, lAppletIndex-1, 1, 0);
            }
        }

        //!when there isnt a single task
        if (latteApplet && (root.latteApplet.parabolicManager.firstRealTaskIndex !== root.latteApplet.parabolicManager.lastRealTaskIndex)) {
            if (gTaskIndex === -1 && lTaskIndex === -1){
                latteApplet.parabolicManager.clearTasksGreaterThan(0);
            } else {
                if (gTaskIndex > -1)
                    latteApplet.parabolicManager.clearTasksGreaterThan(gTaskIndex);
                if (lTaskIndex > -1)
                    latteApplet.parabolicManager.clearTasksLowerThan(lTaskIndex);
            }
        }

        clearAppletsGreaterThan(gAppletIndex+1, 1, 0);
        clearAppletsLowerThan(lAppletIndex-1, 1, 0);

        return {leftScale:leftScale, rightScale:rightScale};
    }

    function clearAppletsGreaterThan(index) {
        var startLastIndex = layoutsContainer.startLayout.beginIndex+layoutsContainer.startLayout.count-1;
        if (index<startLastIndex) {
            for (var i=index+1; i<=startLastIndex; ++i)
                root.updateScale(i, 1, 0);
        }

        var mainLastIndex = layoutsContainer.mainLayout.beginIndex+layoutsContainer.mainLayout.count-1;
        if (index<mainLastIndex){
            var mainClearStart = index>layoutsContainer.mainLayout.beginIndex ? index+1 : layoutsContainer.mainLayout.beginIndex;
            for (var j=mainClearStart; j<=mainLastIndex; ++j)
                root.updateScale(j, 1, 0);
        }

        var endLastIndex = layoutsContainer.endLayout.beginIndex+layoutsContainer.endLayout.count-1;
        if (index<endLastIndex){
            var endClearStart = index>layoutsContainer.endLayout.beginIndex ? index+1 : layoutsContainer.endLayout.beginIndex;
            for (var k=endClearStart; k<=endLastIndex; ++k)
                root.updateScale(k, 1, 0);
        }
    }

    function clearAppletsLowerThan(index) {
        var startBeginIndex = layoutsContainer.startLayout.beginIndex;
        var startLastIndex = layoutsContainer.startLayout.beginIndex+layoutsContainer.startLayout.count-1;
        if (index>startBeginIndex) {
            var startClearStart = index<=startLastIndex ? index-1 : startLastIndex;
            for (var i=startClearStart; i>=startBeginIndex; --i)
                root.updateScale(i, 1, 0);
        }

        var mainBeginIndex = layoutsContainer.mainLayout.beginIndex;
        var mainLastIndex = layoutsContainer.mainLayout.beginIndex+layoutsContainer.mainLayout.count-1;
        if (index>mainBeginIndex) {
            var mainClearStart = index<=mainLastIndex ? index-1 : mainLastIndex;
            for (var j=mainClearStart; j>=mainBeginIndex; --j)
                root.updateScale(j, 1, 0);
        }

        var endBeginIndex = layoutsContainer.endLayout.beginIndex;
        var endLastIndex = layoutsContainer.endLayout.beginIndex+layoutsContainer.endLayout.count-1;
        if (index>endBeginIndex) {
            var endClearStart = index<=endLastIndex ? index-1 : endLastIndex;
            for (var k=endClearStart; k>=endBeginIndex; --k)
                root.updateScale(k, 1, 0);
        }
    }

    // update the registered separators
    // -1, no = add separator
    // no, -1 = remove separator
    // no, no = update separator position
    function setSeparator(previousId, nextId) {
        if (previousId === nextId && separators.indexOf(nextId)>-1)
            return;

        //if (plasmoid.location === PlasmaCore.Types.BottomEdge)
        //    console.log("message: "+previousId + " - " + nextId);

        var update=false;
        //should update
        if (previousId>-1 && nextId>-1)
            update=true;

        //remove
        if ((previousId>-1 && nextId===-1) || update) {
            var ind = separators.indexOf(previousId);
            if (ind>-1)
                separators.splice(ind,1);
        }

        //add
        if ((previousId===-1 && nextId>-1) || update) {
            separators.push(nextId);
        }

        //if (plasmoid.location === PlasmaCore.Types.BottomEdge)
        //    console.log("separators : "+separators);

        root.separatorsUpdated();
    }

    // update the registered hidden applets
    // -1, no = add hidden
    // no, -1 = remove hidden
    // no, no = update hidden position
    function setHidden(previousId, nextId) {
        if (previousId === nextId && hidden.indexOf(nextId)>-1)
            return;

        var update=false;
        //should update
        if (previousId>-1 && nextId>-1)
            update=true;

        //remove
        if ((previousId>-1 && nextId===-1) || update) {
            var ind = hidden.indexOf(previousId);
            if (ind>-1)
                hidden.splice(ind,1);
        }

        //add
        if ((previousId===-1 && nextId>-1) || update) {
            hidden.push(nextId);
        }

        // console.log("hidden : "+hidden);
    }

    function availableLowerId(from) {
        var next = from;

        while (separators.indexOf(next) !== -1
               || hidden.indexOf(next) !== -1
               || (root.latteApplet && root.latteAppletPos===next && root.latteApplet.parabolicManager.firstRealTaskIndex === -1 ))
            next = next - 1;

        return next;
    }

    function availableHigherId(from) {
        var next = from;

        while (separators.indexOf(next) !== -1
               || hidden.indexOf(next) !== -1
               || (root.latteApplet && root.latteAppletPos===next && root.latteApplet.parabolicManager.firstRealTaskIndex === -1 ))
            next = next + 1;

        return next;
    }

    function isSeparator(index){
        return (separators.indexOf(index) !== -1)
    }

    function isHidden(index) {
        return (hidden.indexOf(index) !== -1)
    }

    //! the pseudo index applet after we take into account the separators before it, hidden applets,
    //! spacers etc. for example the third applet if there is a separator before it is 1, it isnt 2
    function pseudoAppletIndex(realIndex) {
        var counter = 0;
        var originalAppletFound = false;

        for (var i=0; i<layoutsContainer.startLayout.children.length; ++i){
            var applet = layoutsContainer.startLayout.children[i];

            if (applet) {
                if (applet.index < realIndex) {
                    if (applet.isLattePlasmoid) {
                        counter = counter + root.latteApplet.parabolicManager.countRealTasks;
                    } else if (applet.canShowAppletNumberBadge) {
                        counter = counter + 1;
                    }
                } else if (applet.index === realIndex) {
                    originalAppletFound = true;
                    break;
                }
            }
        }

        if (!originalAppletFound) {
            for (var j=0; j<layoutsContainer.mainLayout.children.length; ++j){
                var applet = layoutsContainer.mainLayout.children[j];

                if (applet) {
                    if (applet.index < realIndex) {
                        if (applet.isLattePlasmoid) {
                            counter = counter + root.latteApplet.parabolicManager.countRealTasks;
                        } else if (applet.canShowAppletNumberBadge) {
                            counter = counter + 1;
                        }
                    } else if (applet.index === realIndex) {
                        originalAppletFound = true;
                        break;
                    }
                }
            }
        }

        if (!originalAppletFound) {
            for (var k=0; j<layoutsContainer.endLayout.children.length; ++k){
                var applet = layoutsContainer.endLayout.children[k];

                if (applet) {
                    if (applet.index < realIndex) {
                        if (applet.isLattePlasmoid) {
                            counter = counter + root.latteApplet.parabolicManager.countRealTasks;
                        } else if (applet.canShowAppletNumberBadge) {
                            counter = counter + 1;
                        }
                    } else if (applet.index === realIndex) {
                        originalAppletFound = true;
                        break;
                    }
                }
            }
        }

        return counter + 1;
    }

    function pseudoIndexBelongsToLatteApplet(index) {
        if (root.latteApplet) {
            var lastTaskIndex = root.latteApplet.tasksNumbersBase + root.latteApplet.parabolicManager.countRealTasks;
            return (index>root.latteApplet.tasksNumbersBase && index<=lastTaskIndex);
        }

        return false;
    }

}
