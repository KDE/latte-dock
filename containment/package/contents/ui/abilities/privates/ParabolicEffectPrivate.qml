/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

import org.kde.latte.abilities.host 0.1 as AbilityHost

AbilityHost.ParabolicEffect {
    id: parabolic

    property Item animations: null
    property Item debug: null
    property Item layouts: null
    property QtObject view: null

    readonly property bool horizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal

    property bool restoreZoomIsBlockedFromApplet: false
    property int lastParabolicItemIndex: -1

    Connections {
        target: parabolic
        onRestoreZoomIsBlockedChanged: {
            if (!parabolic.restoreZoomIsBlocked) {
                parabolic.startRestoreZoomTimer();
            } else {
                parabolic.stopRestoreZoomTimer();
            }

        }

        onCurrentParabolicItemChanged: {
            if (!currentParabolicItem) {
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

    //! do not update during dragging/moving applets inConfigureAppletsMode
    readonly property bool isBindingUpdateEnabled: !(root.dragOverlay && root.dragOverlay.pressed)

    Binding{
        target: parabolic
        property: "restoreZoomIsBlockedFromApplet"
        when: isBindingUpdateEnabled
        value: {
            var grid;

            for (var l=0; l<=2; ++l) {
                if (l===0) {
                    grid = layouts.startLayout;
                } else if (l===1) {
                    grid = layouts.mainLayout;
                } else if (l===2) {
                    grid = layouts.endLayout;
                }

                for (var i=0; i<grid.children.length; ++i){
                    var appletItem = grid.children[i];
                    if (appletItem
                            && appletItem.communicator
                            && appletItem.communicator.parabolicEffectIsSupported
                            && appletItem.communicator.bridge.parabolic.client.local.restoreZoomIsBlocked) {
                        return true;
                    }
                }
            }

            return false;
        }
    }

    function startRestoreZoomTimer(){
        if (restoreZoomIsBlocked) {
            return;
        }

        restoreZoomTimer.start();
    }

    function stopRestoreZoomTimer(){
        if (restoreZoomTimer.running) {
            restoreZoomTimer.stop();
        }
    }

    function setDirectRenderingEnabled(value) {
        _privates.directRenderingEnabled = value;
    }

    function setCurrentParabolicItem(item) {
        view.parabolic.currentItem = item;
    }

    function setCurrentParabolicItemIndex(index) {
        if (!directRenderingEnabled
                && lastParabolicItemIndex > -1
                && index > -1
                && Math.abs(lastParabolicItemIndex-index) >=2 ) {
            //! rapid movement
            setDirectRenderingEnabled(true);
        }

        lastParabolicItemIndex = index;
    }

    function applyParabolicEffect(index, currentMousePosition, center) {
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
        interval: 50

        onTriggered: {
            if (parabolic.restoreZoomIsBlocked || currentParabolicItem) {
                return;
            }

            parabolic.lastParabolicItemIndex = -1;
            parabolic.setDirectRenderingEnabled(false);
            parabolic.sglClearZoom();

            if (debug.timersEnabled) {
                console.log("containment timer: RestoreZoomTimer called...");
            }
        }
    }
}
