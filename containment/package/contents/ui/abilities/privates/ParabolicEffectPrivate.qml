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
    property QtObject settings: null

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
        target: parabolic.layouts
        onContextMenuIsShownChanged: {
            if (!parabolic.layouts.contextMenuIsShown && !restoreZoomTimer.running) {
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
