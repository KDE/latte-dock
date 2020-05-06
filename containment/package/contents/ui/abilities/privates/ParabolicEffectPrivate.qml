/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

import org.kde.latte.abilities.containers 0.1 as ContainerAbility

ContainerAbility.ParabolicEffect {
    id: parabolic

    property Item animations: null
    property Item applets: null
    property QtObject view: null

    readonly property bool horizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal

    Connections {
        target: parabolic
        onSglClearZoom: parabolic._privates.lastIndex = -1;
        onRestoreZoomIsBlockedChanged: {
            if (!parabolic.restoreZoomIsBlocked) {
                parabolic.startRestoreZoomTimer();
            } else {
                parabolic.stopRestoreZoomTimer();
            }
        }
    }

    Connections{
        target: parabolic.view && parabolic.view.visibility ? parabolic.view.visibility : root
        ignoreUnknownSignals : true
        onContainsMouseChanged: {
            if (!parabolic.view.visibility.containsMouse && !restoreZoomTimer.running) {
                parabolic.startRestoreZoomTimer()
            }
        }
    }

    Connections {
        target: parabolic.view
        onContextMenuIsShownChanged: {
            if (!parabolic.view.contextMenuIsShown && !restoreZoomTimer.running) {
                parabolic.startRestoreZoomTimer();
            }
        }
    }

    function startRestoreZoomTimer(){
        if (restoreZoomIsBlocked) {
            return;
        }

        restoreZoomTimer.start();
    }

    function stopRestoreZoomTimer(){
        restoreZoomTimer.stop();
    }

    function setDirectRenderingEnabled(value) {
        _privates.directRenderingEnabled = value;
    }

    function applyParabolicEffect(index, currentMousePosition, center) {
        if (parabolic._privates.lastIndex === -1) {
            setDirectRenderingEnabled(false);
        }

        //! last item requested calculations
        parabolic._privates.lastIndex = index;

        var rDistance = Math.abs(currentMousePosition  - center);

        //check if the mouse goes right or down according to the center
        var positiveDirection =  ((currentMousePosition  - center) >= 0 );

        if (Qt.application.layoutDirection === Qt.RightToLeft && horizontal) {
            positiveDirection = !positiveDirection;
        }

        //finding the zoom center e.g. for zoom:1.7, calculates 0.35
        var zoomCenter = (factor.zoom - 1) / 2

        //computes the in the scale e.g. 0...0.35 according to the mouse distance
        //0.35 on the edge and 0 in the center
        var firstComputation = (rDistance / center) * zoomCenter;

        //calculates the scaling for the neighbour tasks
        var bigNeighbourZoom = Math.min(1 + zoomCenter + firstComputation, factor.zoom);
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


    //! TIMERS

    //! Timer to check if the mouse is still outside the latteView in order to restore applets scales to 1.0
    Timer{
        id: restoreZoomTimer
        interval: 90

        onTriggered: {
            if (parabolic.restoreZoomIsBlocked) {
                return
            }

            parabolic.sglClearZoom();

            if (root.debugModeTimers) {
                console.log("containment timer: RestoreZoomTimer called...");
            }
        }
    }
}
