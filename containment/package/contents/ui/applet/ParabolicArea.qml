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

    readonly property bool containsMouse: (appletItem.parabolic.currentParabolicItem === _parabolicArea) || parabolicMouseArea.containsMouse

    readonly property bool hasParabolicMessagesEnabled: parabolicAreaLoader.hasParabolicMessagesEnabled
    readonly property bool isParabolicEnabled: parabolicAreaLoader.isParabolicEnabled
    readonly property bool isThinTooltipEnabled: parabolicAreaLoader.isThinTooltipEnabled    

    property real length: root.isHorizontal ? appletItem.width : appletItem.height

    MouseArea {
        id: parabolicMouseArea
        anchors.fill: parent
        enabled: visible
        hoverEnabled: true
        visible: appletItem.parabolicEffectIsSupported
                 && !communicator.indexerIsSupported
                 && appletItem.parabolic.currentParabolicItem !== _parabolicArea

        onEntered: {
            appletItem.parabolic.setCurrentParabolicItem(_parabolicArea);

            if (isParabolicEnabled) {
                var vIndex = appletItem.indexer.visibleIndex(index);
                appletItem.parabolic.setCurrentParabolicItemIndex(vIndex);
            }

            //! mouseX/Y can be trusted in that case in comparison to tasks that the relevant ParabolicAreaMouseArea does not
            _parabolicArea.parabolicEntered(mouseX, mouseY);
        }
    }

    onParabolicEntered: {
        if (isThinTooltipEnabled && !(isSeparator || isSpacer || isMarginsAreaSeparator)) {
            appletItem.thinTooltip.show(appletItem.tooltipVisualParent, applet.title);
        }

        if (restoreAnimation.running) {
            restoreAnimation.stop();
        }

        if (!appletItem.myView.isShownFully
                || appletItem.originalAppletBehavior
                || !appletItem.parabolicEffectIsSupported
                || communicator.requires.parabolicEffectLocked
                || communicator.indexerIsSupported) {
            return;
        }

        if (isParabolicEnabled) {
            if (root.isHorizontal){
                appletItem.layouts.currentSpot = Math.round(mouseX);
                calculateParabolicScales(mouseX);
            } else{
                appletItem.layouts.currentSpot = Math.round(mouseY);
                calculateParabolicScales(mouseY);
            }
        }
    }

    onParabolicMove: {
        if (!appletItem.myView.isShownFully
                || appletItem.originalAppletBehavior
                || !appletItem.parabolicEffectIsSupported
                || communicator.requires.parabolicEffectLocked
                || communicator.indexerIsSupported) {
            return;
        }

        if (isParabolicEnabled) {
            if( ((wrapper.zoomScale === 1 || wrapper.zoomScale === appletItem.parabolic.factor.zoom) && !parabolic.directRenderingEnabled) || parabolic.directRenderingEnabled) {
                if (root.isHorizontal){
                    var step = Math.abs(appletItem.layouts.currentSpot-mouseX);
                    if (step >= appletItem.animations.hoverPixelSensitivity){
                        appletItem.layouts.currentSpot = Math.round(mouseX);
                        calculateParabolicScales(mouseX);
                    }
                }
                else{
                    var step = Math.abs(appletItem.layouts.currentSpot-mouseY);
                    if (step >= appletItem.animations.hoverPixelSensitivity){
                        appletItem.layouts.currentSpot = Math.round(mouseY);
                        calculateParabolicScales(mouseY);
                    }
                }
            }
        }
    }

    onParabolicExited: {
        if (isThinTooltipEnabled) {
            appletItem.thinTooltip.hide(appletItem.tooltipVisualParent);
        }
    }

    function calculateParabolicScales(currentMousePosition){
        if (parabolic.factor.zoom===1 || parabolic.restoreZoomIsBlocked) {
            return;
        }

        if (wrapper.zoomScale === 1 && (appletItem.firstAppletInContainer || appletItem.lastAppletInContainer)) {
            //! first hover of first or last items in container
            //! this way we make sure that neighbour items will increase their zoom faster
            var substep = length/4;
            var center = length/2;
            currentMousePosition = Math.min(Math.max(currentMousePosition, center-substep), center+substep);
        }

        //use the new parabolic effect manager in order to handle all parabolic effect messages
        var scales = parabolic.applyParabolicEffect(index, currentMousePosition, length);

        //Left hiddenSpacer
        if(appletItem.firstAppletInContainer){
            hiddenSpacerLeft.nScale = scales.leftScale - 1;
        }

        //Right hiddenSpacer  ///there is one more item in the currentLayout ????
        if(appletItem.lastAppletInContainer){
            hiddenSpacerRight.nScale =  scales.rightScale - 1;
        }

        wrapper.zoomScale = parabolic.factor.zoom;
    } //scale


    function updateScale(nIndex, nScale){
        if(appletItem && (appletItem.index === nIndex) /*&& !appletItem.containsMouse*/){ /*disable it in order to increase parabolic effect responsiveness*/
            if ( (parabolicEffectIsSupported && !appletItem.originalAppletBehavior && !appletItem.communicator.indexerIsSupported)
                    && (applet && applet.status !== PlasmaCore.Types.HiddenStatus)){
                    wrapper.zoomScale = Math.max(1, nScale);
            }
        }
    }

    function sltUpdateItemScale(delegateIndex, newScales, islower) {
        var ishigher = !islower;
        var clearrequestedfromlastacceptedsignal = (newScales.length===1) && (newScales[0]===1);
        var sideindex = islower ? appletItem.index-1 : appletItem.index+1;

        if (delegateIndex === appletItem.index) {
            if (communicator.parabolicEffectIsSupported) {
                if (islower) {
                    communicator.bridge.parabolic.client.hostRequestUpdateLowerItemScale(newScales);
                } else {
                    communicator.bridge.parabolic.client.hostRequestUpdateHigherItemScale(newScales);
                }
                return;
            }

            if (newScales.length <= 0) {
                return
            }

            var nextscales = newScales.slice();                                                          //first copy scales in order to not touch referenced/same array to other slots

            if (!appletItem.isSeparator && !appletItem.isMarginsAreaSeparator && !appletItem.isHidden) { //accept signal and apply the first scale in the stack
                updateScale(delegateIndex, nextscales[0]);                                               //apply scale
                nextscales.splice(0, 1);                                                                 //remove accepted and previously applied scale

                if ((nextscales.length===1) && (nextscales[0]===1)) {                                    //send clearrequestedfromlastacceptedsignal to inform neighbours in that direction to release zoom
                    if (islower) {
                        parabolic.sglUpdateLowerItemScale(sideindex, nextscales);
                    } else {
                        parabolic.sglUpdateHigherItemScale(sideindex, nextscales);
                    }
                    return;
                }
            }

            if (!clearrequestedfromlastacceptedsignal) {              //send remaining scales in the stack as long as this is not the clearrequestedfromlastacceptedsignal, in order to not send twice
                if (islower) {
                    parabolic.sglUpdateLowerItemScale(appletItem.index-1, nextscales);
                } else {
                    parabolic.sglUpdateHigherItemScale(appletItem.index+1, nextscales);
                }
            }
        } else if (islower && clearrequestedfromlastacceptedsignal && (appletItem.index < delegateIndex)) { //accept requestedfromlastacceptedsignal in lower direction if that is the case
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateLowerItemScale(newScales);
            } else {
                updateScale(appletItem.index, 1);
            }
        } else if (ishigher && clearrequestedfromlastacceptedsignal && (appletItem.index > delegateIndex)) { //accept requestedfromlastacceptedsignal in higher direction if that is the case
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateHigherItemScale(newScales);
            } else {
                updateScale(appletItem.index, 1);
            }
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
        parabolic.sglUpdateLowerItemScale.connect(sltUpdateLowerItemScale);
        parabolic.sglUpdateHigherItemScale.connect(sltUpdateHigherItemScale);
    }

    Component.onDestruction: {
        parabolic.sglUpdateLowerItemScale.disconnect(sltUpdateLowerItemScale);
        parabolic.sglUpdateHigherItemScale.disconnect(sltUpdateHigherItemScale);
    }
}
