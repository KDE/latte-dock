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

import org.kde.latte.abilities.applets 0.1 as AppletAbility

AppletAbility.ParabolicEffect {
    id: parabolic
    local.factor.zoom: ( 1 + (plasmoid.configuration.zoomLevel / 20) )
    local.factor.maxZoom: Math.max(local.factor.zoom, 1.6)

    readonly property bool horizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal

    Component.onCompleted: {
        parabolic.sglUpdateLowerItemScale.connect(sltTrackLowerItemScale);
        parabolic.sglUpdateHigherItemScale.connect(sltTrackHigherItemScale);
    }

    Component.onDestruction: {
        parabolic.sglUpdateLowerItemScale.disconnect(sltTrackLowerItemScale);
        parabolic.sglUpdateHigherItemScale.disconnect(sltTrackHigherItemScale);
    }

    Connections {
        target: parabolic
        onSglClearZoom: parabolic.local._privates.lastIndex = -1;
        onRestoreZoomIsBlockedChanged: {
            if (!parabolic.restoreZoomIsBlocked) {
                parabolic.startRestoreZoomTimer();
            } else {
                parabolic.stopRestoreZoomTimer();
            }
        }
    }

    function startRestoreZoomTimer(){        
        if (restoreZoomIsBlocked) {
            return;
        }

        if (bridge) {
            bridge.parabolic.host.startRestoreZoomTimer();
        } else {
            restoreZoomTimer.start();
        }
    }

    function stopRestoreZoomTimer(){
        if (bridge) {
            bridge.parabolic.host.stopRestoreZoomTimer();
        } else {
            restoreZoomTimer.stop();
        }
    }

    function setDirectRenderingEnabled(value) {
        if (tasksExtendedManager.waitingLaunchersLength() > 0) {
            return;
        }

        if (bridge) {
            bridge.parabolic.host.setDirectRenderingEnabled(value);
        } else {
            local._privates.directRenderingEnabled = value;
        }
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
        if (parabolic.local._privates.lastIndex === -1) {
            setDirectRenderingEnabled(false);
        }

        //! last item requested calculations
        parabolic.local._privates.lastIndex = index;

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

    function invkClearZoom() {
        if (parabolic.restoreZoomIsBlocked) {
            return
        }

        if (bridge) {
            bridge.parabolic.host.sglClearZoom();
        } else {
            parabolic.sglClearZoom();
        }
    }

    //! TIMERS

    //! Timer to check if the mouse is outside the applet in order to restore items scales to 1.0
    //! IMPORTANT ::: This timer should be used only when the Latte plasmoid is not inside a Latte dock
    Timer{
        id: restoreZoomTimer
        interval: 90

        onTriggered: {
            if(parabolic.bridge) {
                console.log("Plasmoid, restoreZoomTimer was called, even though it shouldn't...");
            }

            parabolic.invkClearZoom();
        }
    }
}
