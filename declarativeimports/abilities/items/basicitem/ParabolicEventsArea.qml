/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: _parabolicArea
    signal parabolicEntered(real mouseX, real mouseY);
    signal parabolicMove(real mouseX, real mouseY);
    signal parabolicExited();

    property int lastMouseX: 0
    property int lastMouseY: 0
    property int lastParabolicPos: 0

    readonly property bool containsMouse: (abilityItem.abilities.parabolic.currentParabolicItem === _parabolicArea) || parabolicMouseArea.containsMouse

    readonly property bool isParabolicEnabled: parabolicEventsAreaLoader.isParabolicEnabled
    readonly property bool isThinTooltipEnabled: parabolicEventsAreaLoader.isThinTooltipEnabled
    readonly property real length: abilityItem.isHorizontal ? abilityItem.width : abilityItem.height

    MouseArea {
        id: parabolicMouseArea
        anchors.fill: parent
        enabled: visible
        hoverEnabled: true
        visible: abilityItem.abilities.parabolic.currentParabolicItem !== _parabolicArea

        onEntered: {
            abilityItem.abilities.parabolic.setCurrentParabolicItem(_parabolicArea);

            if (isThinTooltipEnabled) {
                abilityItem.abilities.thinTooltip.show(abilityItem.tooltipVisualParent, abilityItem.thinTooltipText);
            }

            if (isParabolicEnabled) {
                var vIndex = abilityItem.abilities.shortcuts.shortcutIndex(abilityItem.itemIndex);
                abilityItem.abilities.parabolic.setCurrentParabolicItemIndex(vIndex);
            }

            // mouseX/Y can not be trusted at this point
            //_parabolicArea.parabolicEntered(mouseX, mouseY);
        }
    }

    Connections {
        target: parabolicItem
        onIsParabolicEventBlockedChanged: {
            if (!parabolicItem.isParabolicEventBlocked && _parabolicArea.containsMouse) {
                _parabolicArea.parabolicEntered(lastMouseX, lastMouseY);
            }
        }
    }

    Connections{
        target: abilityItem.abilities.myView

        //! During dock sliding-in because the parabolic effect isnt trigerred
        //! immediately but we wait first the dock to go to its final normal
        //! place we might miss the activation of the parabolic effect.
        //! By catching that signal we are trying to solve this.
        onIsShownFullyChanged: {
            if (abilityItem.abilities.myView.isShownFully && _parabolicArea.containsMouse) {
                _parabolicArea.parabolicMove(lastMouseX, lastMouseY);
            }
        }
    }

    onParabolicEntered: {
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        restoreAnimation.stop();

        if (isThinTooltipEnabled) {
            abilityItem.abilities.thinTooltip.show(abilityItem.tooltipVisualParent, abilityItem.thinTooltipText);
        }

        if (isParabolicEnabled) {
            //! mouseX/Y can NOW be trusted because ParabolicEnterd event is triggered from View::Parabolic class
            var current = abilityItem.isHorizontal ? mouseX : mouseY;
            lastParabolicPos = current;
            calculateParabolicScales(current);
        }
    }

    onParabolicMove: {
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        if (isParabolicEnabled) {
            var mousePos = abilityItem.isHorizontal ? mouseX : mouseY;

            if (mousePos<0 || parabolicItem.isParabolicEventBlocked) {
                return;
            }

            if (abilityItem.abilities.myView.isReady && !abilityItem.abilities.myView.isShownFully) {
                return;
            }

            if( ((abilityItem.parabolicItem.zoom === 1 || abilityItem.parabolicItem.zoom === abilityItem.abilities.parabolic.factor.zoom)
                 && !abilityItem.abilities.parabolic.directRenderingEnabled)
                    || abilityItem.abilities.parabolic.directRenderingEnabled) {

                var step = Math.abs(lastParabolicPos-mousePos);
                if (step >= abilityItem.abilities.animations.hoverPixelSensitivity){
                    lastParabolicPos = mousePos;
                    calculateParabolicScales(mousePos);
                }
            }
        }
    }

    onParabolicExited: {
        lastParabolicPos = 0;

        if (isThinTooltipEnabled) {
            abilityItem.abilities.thinTooltip.hide(abilityItem.tooltipVisualParent);
        }
    }

    function calculateParabolicScales( currentMousePosition ){
        if (abilityItem.abilities.parabolic.factor.zoom===1
                || abilityItem.abilities.parabolic.restoreZoomIsBlocked
                || parabolicItem.isParabolicEventBlocked) {
            return;
        }

        if (abilityItem.parabolicItem.zoom === 1 && (abilityItem.isFirstItemInContainer || abilityItem.isLastItemInContainer)) {
            //! first hover of first or last items in container
            //! this way we make sure that neighbour items will increase their zoom faster
            var substep = length/4;
            var center = length/2;
            currentMousePosition = Math.min(Math.max(currentMousePosition, center-substep), center+substep);
        }

        //use the new parabolic ability in order to handle all parabolic effect messages
        var scales = abilityItem.abilities.parabolic.applyParabolicEffect(index, currentMousePosition, length);

        if (!parabolicItem.isUpdatingOnlySpacers) {
            abilityItem.parabolicItem.zoom = abilityItem.abilities.parabolic.factor.zoom;
        } else {
            var subSpacerScale = (abilityItem.abilities.parabolic.factor.zoom-1)/2;

            hiddenSpacerLeft.nScale = subSpacerScale;
            hiddenSpacerRight.nScale = subSpacerScale;
        }
    } //zoom

    function updateScale(nIndex, nScale){
        if (index === nIndex /*&& !_parabolicArea.containsMouse*/ /*&& !parabolicItem.isParabolicEventBlocked*/){ //!disabled them in order to provide smoother parabolic effect during dock showing and first hovering
            if (parabolicItem.isUpdatingOnlySpacers) {
                var subSpacerScale = (nScale-1)/2;

                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            } else {
                abilityItem.parabolicItem.zoom = Math.max(1, nScale);
            }
        }
    }

    function sltUpdateItemScale(delegateIndex, newScales, islower) {
        var ishigher = !islower;
        var clearrequestedfromlastacceptedsignal = (newScales.length===1) && (newScales[0]===1);
        var sideindex = islower ? index-1 : index+1;

        if (delegateIndex === index) {
            if (newScales.length <= 0) {
                return
            }

            var nextscales = newScales.slice();                       //first copy scales in order to not touch referenced/same array to other slots

            if (!abilityItem.isSeparator && !abilityItem.isHidden) {  //accept signal and apply the first scale in the stack
                updateScale(delegateIndex, nextscales[0]);            //apply scale
                nextscales.splice(0,1);                               //remove accepted and previously applied scale

                if ((nextscales.length===1) && (nextscales[0]===1)) { //send clearrequestedfromlastacceptedsignal to inform neighbours in that direction to release zoom
                    if (islower) {
                        abilityItem.abilities.parabolic.sglUpdateLowerItemScale(sideindex, nextscales);
                    } else {
                        abilityItem.abilities.parabolic.sglUpdateHigherItemScale(sideindex, nextscales);
                    }
                    return;
                }
            }

            if (!clearrequestedfromlastacceptedsignal) {              //send remaining scales in the stack as long as this is not the clearrequestedfromlastacceptedsignal, in order to not send twice
                if (islower) {
                    abilityItem.abilities.parabolic.sglUpdateLowerItemScale(sideindex, nextscales);
                } else {
                    abilityItem.abilities.parabolic.sglUpdateHigherItemScale(sideindex, nextscales);
                }
            }
        } else if ((islower && clearrequestedfromlastacceptedsignal && (index < delegateIndex))           //accept requestedfromlastacceptedsignal in lower direction if that is the case
                   || (ishigher && clearrequestedfromlastacceptedsignal && (index > delegateIndex))) {    //accept requestedfromlastacceptedsignal in higher direction if that is the case
            updateScale(index, 1);
        }
    }

    function sltUpdateLowerItemScale(delegateIndex, newScales) {
        var islower = true;
        sltUpdateItemScale(delegateIndex, newScales, islower);
    }

    function sltUpdateHigherItemScale(delegateIndex, newScales) {
        var ishigher = false;
        sltUpdateItemScale(delegateIndex, newScales, ishigher);
    }

    Component.onCompleted: {
        abilityItem.abilities.parabolic.sglUpdateLowerItemScale.connect(sltUpdateLowerItemScale);
        abilityItem.abilities.parabolic.sglUpdateHigherItemScale.connect(sltUpdateHigherItemScale);
    }

    Component.onDestruction: {
        abilityItem.abilities.parabolic.sglUpdateLowerItemScale.disconnect(sltUpdateLowerItemScale);
        abilityItem.abilities.parabolic.sglUpdateHigherItemScale.disconnect(sltUpdateHigherItemScale);
    }
}
