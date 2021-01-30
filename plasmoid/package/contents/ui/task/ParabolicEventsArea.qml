/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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
    readonly property real center: abilityItem.parabolicItem.center

    MouseArea {
        id: parabolicMouseArea
        anchors.fill: parent
        enabled: visible
        hoverEnabled: true
        visible: abilityItem.abilities.parabolic.currentParabolicItem !== _parabolicArea

        onEntered: {
            abilityItem.abilities.parabolic.setCurrentParabolicItem(_parabolicArea);

            if (isParabolicEnabled) {
                var vIndex = abilityItem.abilities.shortcuts.shortcutIndex(abilityItem.itemIndex);
                abilityItem.abilities.parabolic.setCurrentParabolicItemIndex(vIndex);
            }

            // mouseX/Y can not be trusted at this point
            //_parabolicArea.parabolicEntered(mouseX, mouseY);
        }
    }

    Connections {
        target: abilityItem
        onIsParabolicEventBlockedChanged: {
            if (!abilityItem.isParabolicEventBlocked && _parabolicArea.containsMouse) {
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

        if (isParabolicEnabled) {
            //! mouseX/Y can NOW be trusted because ParabolicEnterd event is triggered from View::Parabolic class
            var current = abilityItem.isHorizontal ? mouseX : mouseY;
            lastParabolicPos = current;
            calculateParabolicScales(current);
        }

        if (isThinTooltipEnabled) {
            abilityItem.abilities.thinTooltip.show(abilityItem.tooltipVisualParent, abilityItem.thinTooltipText);
        }
    }

    onParabolicMove: {
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        if (isParabolicEnabled) {
            var mousePos = abilityItem.isHorizontal ? mouseX : mouseY;

            if (mousePos<0 || abilityItem.isParabolicEventBlocked) {
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
        if (abilityItem.abilities.parabolic.factor.zoom===1 || abilityItem.abilities.parabolic.restoreZoomIsBlocked || isParabolicEventBlocked) {
            return;
        }

        //use the new parabolic ability in order to handle all parabolic effect messages
        var scales = abilityItem.abilities.parabolic.applyParabolicEffect(index, currentMousePosition, center);

        //Left hiddenSpacer for first task
        if((index === abilityItem.abilities.indexer.firstVisibleItemIndex) && abilityItem.abilities.containment.isFirstAppletInContainment) {
            hiddenSpacerLeft.nScale = scales.leftScale - 1;
        }

        //Right hiddenSpacer for last task
        if((index === abilityItem.abilities.indexer.lastVisibleItemIndex) && abilityItem.abilities.containment.isLastAppletInContainment) {
            hiddenSpacerRight.nScale =  scales.rightScale - 1;
        }

        if (!parabolicItem.isUpdatingOnlySpacers) {
            abilityItem.parabolicItem.zoom = abilityItem.abilities.parabolic.factor.zoom;
        } else {
            var subSpacerScale = (taskItem.abilities.parabolic.factor.zoom-1)/2;

            hiddenSpacerLeft.nScale = subSpacerScale;
            hiddenSpacerRight.nScale = subSpacerScale;
        }
    } //zoom

    function updateScale(nIndex, nScale, step){
        if (!_parabolicArea.containsMouse && (index === nIndex) && !isParabolicEventBlocked){
            /*if (taskItem.inAttentionAnimation) {
                var subSpacerScale = (nScale-1)/2;

                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            } else if (!inBlockingAnimation || taskItem.inMimicParabolicAnimation) {

            }*/

            if (parabolicItem.isUpdatingOnlySpacers) {
                var subSpacerScale = (nScale-1)/2;

                hiddenSpacerLeft.nScale = subSpacerScale;
                hiddenSpacerRight.nScale = subSpacerScale;
            } else {
                var newScale = 1;

                if(nScale >= 0) {
                    newScale = nScale + step;
                } else {
                    newScale = abilityItem.parabolicItem.zoom + step;
                }

                /*if (inMimicParabolicAnimation && mimicParabolicScale === -1) {
                   mimicParabolicScale = newScale;
               }*/

                abilityItem.parabolicItem.zoom = newScale;
            }
        }
    }

    function sltUpdateLowerItemScale(delegateIndex, newScale, step) {
        if (delegateIndex === index) {
            if (!abilityItem.isSeparator && !abilityItem.isHidden) {
                //! when accepted
                updateScale(delegateIndex, newScale, step);

                if (newScale > 1) { // clear lower items
                    abilityItem.abilities.parabolic.sglUpdateLowerItemScale(delegateIndex-1, 1, 0);
                }
            } else {
                abilityItem.abilities.parabolic.sglUpdateLowerItemScale(delegateIndex-1, newScale, step);
            }
        } else if ((newScale === 1) && (index < delegateIndex)) {
            updateScale(index, 1, 0);
        }
    }

    function sltUpdateHigherItemScale(delegateIndex, newScale, step) {
        if (delegateIndex === index) {
            if (!abilityItem.isSeparator && !abilityItem.isHidden) {
                //! when accepted
                updateScale(delegateIndex, newScale, step);

                if (newScale > 1) { // clear lower items
                    abilityItem.abilities.parabolic.sglUpdateHigherItemScale(delegateIndex+1, 1, 0); // clear higher items
                }
            } else {
                abilityItem.abilities.parabolic.sglUpdateHigherItemScale(delegateIndex+1, newScale, step);
            }
        } else if ((newScale === 1) && (index > delegateIndex)) {
            updateScale(index, 1, 0);
        }
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
