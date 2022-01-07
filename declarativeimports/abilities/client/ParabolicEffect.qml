/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.ParabolicEffect {
    id: parabolic
    property Item bridge: null
    property Item indexer: null
    property Item layout: null

    isEnabled: ref.parabolic.isEnabled
    factor: ref.parabolic.factor
    restoreZoomIsBlocked: bridge ? (bridge.parabolic.host.restoreZoomIsBlocked || local.restoreZoomIsBlocked) : local.restoreZoomIsBlocked
    currentParabolicItem: ref.parabolic.currentParabolicItem
    spread: ref.parabolic.spread

    readonly property bool isActive: bridge !== null
    //! private properties can not go to definition because can not be made readonly in there
    //! special care must be taken in order to be redefined in local properties
    readonly property bool directRenderingEnabled: ref.parabolic._privates.directRenderingEnabled
    readonly property bool horizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    readonly property bool isHovered: {
        if (bridge && bridge.parabolic.host.currentParabolicItem) {
            return bridge.parabolic.host.currentParabolicItem.parent.parent.parent === layout;
        }

        return false;
    }

    readonly property AbilityDefinition.ParabolicEffect local: AbilityDefinition.ParabolicEffect {
        id: _localref
        readonly property bool directRenderingEnabled: _localref._privates.directRenderingEnabled
        spread: 3
    }

    Item {
        id: ref
        readonly property Item parabolic: bridge ? bridge.parabolic.host : local
    }

    onIsActiveChanged: {
        if (isActive) {
            bridge.parabolic.client = parabolic;
        }
    }

    Component.onCompleted: {
        if (isActive) {
            bridge.parabolic.client = parabolic;
        }

        parabolic.sglUpdateLowerItemScale.connect(sltTrackLowerItemScale);
        parabolic.sglUpdateHigherItemScale.connect(sltTrackHigherItemScale);
    }

    Component.onDestruction: {
        if (isActive) {
            bridge.parabolic.client = null;
        }

        parabolic.sglUpdateLowerItemScale.disconnect(sltTrackLowerItemScale);
        parabolic.sglUpdateHigherItemScale.disconnect(sltTrackHigherItemScale);
    }

    Connections {
        target: parabolic
        onRestoreZoomIsBlockedChanged: {
            if (!(bridge || bridge.host)) {
                if (!parabolic.restoreZoomIsBlocked) {
                    parabolic.startRestoreZoomTimer();
                } else {
                    parabolic.stopRestoreZoomTimer();
                }
            }
        }

        onCurrentParabolicItemChanged: {
            if (!parabolic.bridge || !parabolic.bridge.host) {
                if (!parabolic.currentParabolicItem) {
                    parabolic.startRestoreZoomTimer();
                } else {
                    parabolic.stopRestoreZoomTimer();
                }
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
        if (bridge) {
            bridge.parabolic.host.setDirectRenderingEnabled(value);
        } else {
            local._privates.directRenderingEnabled = value;
        }
    }

    function setCurrentParabolicItem(item) {
        if (bridge) {
            bridge.parabolic.host.setCurrentParabolicItem(item);
        } else {
            local.currentParabolicItem = item;
        }
    }

    function hostRequestUpdateLowerItemScale(newScales){
        //! function called from host
        sglUpdateLowerItemScale(indexer.itemsCount-1, newScales);
    }

    function hostRequestUpdateHigherItemScale(newScales){
        //! function called from host
        sglUpdateHigherItemScale(0, newScales);
    }

    function sltTrackLowerItemScale(delegateIndex, newScales){
        //! send update signal to host
        if (bridge) {
            var clearrequestedfromlastacceptedsignal = (newScales.length===1) && (newScales[0]===1);
            if (delegateIndex === -1) {
                bridge.parabolic.clientRequestUpdateLowerItemScale(newScales);
            } else if (clearrequestedfromlastacceptedsignal && delegateIndex>=0) {
                bridge.parabolic.clientRequestUpdateLowerItemScale(newScales);
            }
        }
    }

    function sltTrackHigherItemScale(delegateIndex, newScales) {
        //! send update signal to host
        if (bridge) {
            var clearrequestedfromlastacceptedsignal = (newScales.length===1) && (newScales[0]===1);
            if (delegateIndex >= indexer.itemsCount) {
                bridge.parabolic.clientRequestUpdateHigherItemScale(newScales);
            } else if (clearrequestedfromlastacceptedsignal && delegateIndex<indexer.itemsCount) {
                bridge.parabolic.clientRequestUpdateHigherItemScale(newScales);
            }
        }
    }

    function setCurrentParabolicItemIndex(index) {
        if (bridge) {
            bridge.parabolic.host.setCurrentParabolicItemIndex(index);
        }
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
        interval: 50

        onTriggered: {
            if(parabolic.bridge) {
                console.log("Plasmoid, restoreZoomTimer was called, even though it shouldn't...");
            }

            setDirectRenderingEnabled(false);
            parabolic.invkClearZoom();
        }
    }
}
