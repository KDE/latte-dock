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

    signal sglUpdateLowerItemScale(int delegateIndex, real newScale, real step);
    signal sglUpdateHigherItemScale(int delegateIndex, real newScale, real step);

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

        sglUpdateHigherItemScale(index+1 , rightScale, 0);
        sglUpdateLowerItemScale(index-1, leftScale, 0);

        return {leftScale:leftScale, rightScale:rightScale};
    }

    function clearAppletsGreaterThan(index) {
    }

    function clearAppletsLowerThan(index) {
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
            var lastTaskIndex = root.latteApplet.tasksBaseIndex + root.latteApplet.parabolicManager.countRealTasks;
            return (index>root.latteApplet.tasksBaseIndex && index<=lastTaskIndex);
        }

        return false;
    }

}
