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
// ParabolicManager is responsible for triggering all the messages to applets
// that are neighbour to the hovered applet. This will help a lot to catch cases
// such as separator applets, hidden applets and proper clearing zoom.

Item {
    id: parManager

    property var hidden: []
    property var separators: []

    //!this is used in order to update the index when the signal is for the internal latte plasmoid
    function updateIdSendScale(appIndex, index, zScale, zStep){
        // console.log(appIndex + " _ "+index+ " _ "+zScale + " _ "+ zStep);
        if(root.latteApplet && ((appIndex<root.latteAppletPos && index>=root.latteAppletPos)
                                || (appIndex>root.latteAppletPos && index<=root.latteAppletPos)) ){
            // console.log("ch 1...");
            var appStep = Math.abs(root.latteAppletPos-appIndex);
            var signalStep = Math.abs(index - appIndex);

            var taskIndex = -1;
            if(appIndex<root.latteAppletPos){
                taskIndex = signalStep-appStep;
            } else if (appIndex>root.latteAppletPos){
                taskIndex = root.tasksCount-1 - (signalStep-appStep);
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

        if(!root.latteApplet || Math.abs(root.latteAppletPos-index)>1 || !root.hasInternalSeparator
                || (root.hasInternalSeparator
                    && ((root.latteApplet.internalSeparatorPos>0 && root.latteApplet.internalSeparatorPos<root.tasksCount-1)
                        || (root.latteApplet.internalSeparatorPos===0 && index>root.latteAppletPos)
                        || (root.latteApplet.internalSeparatorPos===root.tasksCount-1 && index<root.latteAppletPos)))
                ){
            gAppletIndex = availableHigherId(index+1);
            lAppletIndex = availableLowerId(index-1);

            updateIdSendScale(index, lAppletIndex, leftScale, 0);
            updateIdSendScale(index, gAppletIndex, rightScale, 0);

            tLIndex = availableLowerId(lAppletIndex-1);
            tHIndex = availableHigherId(gAppletIndex+1);

            gTaskIndex = updateIdSendScale(index, tHIndex, 1 ,0);
            lTaskIndex = updateIdSendScale(index, tLIndex, 1, 0);
        } else{
            if(root.latteApplet.internalSeparatorPos === 0){
                gAppletIndex = index + 2 ;
                lAppletIndex= index - 1;
                updateIdSendScale(index, gAppletIndex, rightScale, 0);
                updateIdSendScale(index, lAppletIndex, leftScale, 0);

                gTaskIndex = updateIdSendScale(index, index+3, 1, 0);
                lTaskIndex = updateIdSendScale(index, index-2, 1, 0);

            } else if(root.hasInternalSeparator && root.latteApplet.internalSeparatorPos === root.tasksCount-1) {
                gAppletIndex = index + 1;
                lAppletIndex= index - 2;
                updateIdSendScale(index, lAppletIndex, leftScale, 0);
                updateIdSendScale(index, gAppletIndex, rightScale, 0);

                gTaskIndex = updateIdSendScale(index, index+2, 1, 0);
                lTaskIndex = updateIdSendScale(index, index-3, 1, 0);
            }
        }

        if (latteApplet) {
            if (gTaskIndex === -1 && lTaskIndex === -1){
                latteApplet.parabolicManager.clearTasksGreaterThan(0);
            } else {
                if (gTaskIndex > -1)
                    latteApplet.parabolicManager.clearTasksGreaterThan(gTaskIndex-1);
                if (lTaskIndex > -1)
                    latteApplet.parabolicManager.clearTasksLowerThan(lTaskIndex+1);
            }
        }

        clearAppletsGreaterThan(gAppletIndex, 1, 0);
        clearAppletsLowerThan(lAppletIndex, 1, 0);

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
        if (previousId === nextId)
            return;

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

        // console.log("separators : "+separators);
    }

    // update the registered hidden applets
    // -1, no = add hidden
    // no, -1 = remove hidden
    // no, no = update hidden position
    function setHidden(previousId, nextId) {
        if (previousId === nextId)
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

        while (separators.indexOf(next) !== -1 || hidden.indexOf(next) !== -1)
            next = next - 1;

        return next;
    }

    function availableHigherId(from) {
        var next = from;

        while (separators.indexOf(next) !== -1 || hidden.indexOf(next) !== -1)
            next = next + 1;

        return next;
    }
}
