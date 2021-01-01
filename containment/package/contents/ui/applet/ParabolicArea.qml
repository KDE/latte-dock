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

    readonly property bool containsMouse: (appletItem.parabolic.currentParabolicItem === _parabolicArea) || parabolicMouseArea.containsMouse

    property real center:root.isHorizontal ?
                             (wrapper.width + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2 :
                             (wrapper.height + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2

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

            var vIndex = appletItem.indexer.visibleIndex(index);
            appletItem.parabolic.setCurrentParabolicItemIndex(vIndex);

            //! mouseX/Y can be trusted in that case in comparison to tasks that the relevant ParabolicAreaMouseArea does not
            _parabolicArea.parabolicEntered(mouseX, mouseY);
        }
    }

    onParabolicEntered: {
        if (restoreAnimation.running) {
            restoreAnimation.stop();
        }

        if (!(isSeparator || isSpacer)) {
            root.showTooltipLabel(appletItem, applet.title);
        }

        if (appletItem.originalAppletBehavior || communicator.requires.parabolicEffectLocked || !parabolicEffectIsSupported) {
            return;
        }

        if (root.isHalfShown || (root.latteApplet
                                 && (root.latteApplet.noTasksInAnimation>0 || root.latteApplet.contextMenu))) {
            return;
        }

        if (root.isHorizontal){
            appletItem.layouts.currentSpot = mouseX;
            calculateParabolicScales(mouseX);
        } else{
            appletItem.layouts.currentSpot = mouseY;
            calculateParabolicScales(mouseY);
        }
    }

    onParabolicMove: {
        if (root.isHalfShown || (root.latteApplet
                                 && (root.latteApplet.noTasksInAnimation>0 || root.latteApplet.contextMenu))) {
            return;
        }

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

    onParabolicExited: {
        root.hideTooltipLabel();
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
            if ( ((parabolicEffectIsSupported && !appletItem.originalAppletBehavior) || appletItem.latteApplet)
                    && (applet && applet.status !== PlasmaCore.Types.HiddenStatus)){
                if(!appletItem.latteApplet){
                    if(nScale >= 0) {
                        wrapper.zoomScale = nScale + step;
                    } else {
                        wrapper.zoomScale = wrapper.zoomScale + step;
                    }
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

            if (!appletItem.isSeparator && !appletItem.isHidden) {
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

            if (!appletItem.isSeparator && !appletItem.isHidden) {
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
