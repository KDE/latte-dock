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

    property real center: (root.isHorizontal ? appletItem.width : appletItem.height) / 2
    property var lastMousePoint: { "x": 0, "y": 0 }

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
        lastMousePoint.x = mouseX;
        lastMousePoint.y = mouseY;

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
                appletItem.layouts.currentSpot = mouseX;
                calculateParabolicScales(mouseX);
            } else{
                appletItem.layouts.currentSpot = mouseY;
                calculateParabolicScales(mouseY);
            }
        }
    }

    onParabolicMove: {
        lastMousePoint.x = mouseX;
        lastMousePoint.y = mouseY;

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
                        appletItem.layouts.currentSpot = mouseX;
                        calculateParabolicScales(mouseX);
                    }
                }
                else{
                    var step = Math.abs(appletItem.layouts.currentSpot-mouseY);
                    if (step >= appletItem.animations.hoverPixelSensitivity){
                        appletItem.layouts.currentSpot = mouseY;
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

    Connections{
        target: appletItem.myView

        //! During dock sliding-in because the parabolic effect isnt trigerred
        //! immediately but we wait first the dock to go to its final normal
        //! place we might miss the activation of the parabolic effect.
        //! By catching that signal we are trying to solve this.
        onIsShownFullyChanged: {
            if (appletItem.myView.isShownFully && _parabolicArea.containsMouse) {
                _parabolicArea.parabolicMove(_parabolicArea.lastMousePoint.x, _parabolicArea.lastMousePoint.y);
            }
        }
    }

    function calculateParabolicScales(currentMousePosition){
        if (parabolic.factor.zoom===1 || parabolic.restoreZoomIsBlocked) {
            return;
        }

        //use the new parabolic effect manager in order to handle all parabolic effect messages
        var scales = parabolic.applyParabolicEffect(index, currentMousePosition, center);

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


    function updateScale(nIndex, nScale, step){
        if(appletItem && !appletItem.containsMouse && (appletItem.index === nIndex)){
            if ( (parabolicEffectIsSupported && !appletItem.originalAppletBehavior && !appletItem.communicator.indexerIsSupported)
                    && (applet && applet.status !== PlasmaCore.Types.HiddenStatus)){
                if(nScale >= 0) {
                    wrapper.zoomScale = nScale + step;
                } else {
                    wrapper.zoomScale = wrapper.zoomScale + step;
                }
            }
        }
    }

    function sltUpdateLowerItemScale(delegateIndex, newScale, step) {
        if (delegateIndex === appletItem.index) {
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateLowerItemScale(newScale, step);
                return;
            }

            if (!appletItem.isSeparator && !appletItem.isMarginsAreaSeparator && !appletItem.isHidden) {
                //! when accepted
                updateScale(delegateIndex, newScale, step);

                if (newScale > 1) { // clear lower items
                    parabolic.sglUpdateLowerItemScale(delegateIndex-1, 1, 0);
                }
            } else {
                parabolic.sglUpdateLowerItemScale(delegateIndex-1, newScale, step);
            }
        } else if ((newScale === 1) && (appletItem.index < delegateIndex)) {
            //! apply zoom clearing
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateLowerItemScale(1, step);
            } else {
                updateScale(appletItem.index, 1, 0);
            }
        }
    }

    function sltUpdateHigherItemScale(delegateIndex, newScale, step) {
        if (delegateIndex === appletItem.index) {
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateHigherItemScale(newScale, step);
                return;
            }

            if (!appletItem.isSeparator && !appletItem.isMarginsAreaSeparator && !appletItem.isHidden) {
                //! when accepted
                updateScale(delegateIndex, newScale, step);

                if (newScale > 1) { // clear higher items
                    parabolic.sglUpdateHigherItemScale(delegateIndex+1, 1, 0);
                }
            } else {
                parabolic.sglUpdateHigherItemScale(delegateIndex+1, newScale, step);
            }
        } else if ((newScale === 1) && (appletItem.index > delegateIndex)) {
            //! apply zoom clearing
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateHigherItemScale(1, step);
            } else {
                updateScale(appletItem.index, 1, 0);
            }
        }
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
