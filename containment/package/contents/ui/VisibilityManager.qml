/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.1
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte

Item{
    id: manager

    anchors.fill: parent

    property QtObject window

    property bool debugMagager: Qt.application.arguments.indexOf("--mask") >= 0

    property bool blockUpdateMask: false
    property bool inForceHiding: false //is used when the docks are forced in hiding e.g. when changing layouts
    property bool normalState : false  // this is being set from updateMaskArea
    property bool previousNormalState : false // this is only for debugging purposes
    property bool panelIsBiggerFromIconSize: root.useThemePanel && (root.themePanelThickness >= (root.iconSize + root.thickMargin))

    property int animationSpeed: Latte.WindowSystem.compositingActive ?
                                     (editModeVisual.inEditMode ? editModeVisual.speed * 0.8 : root.appliedDurationTime * 1.4 * units.longDuration) : 0

    property bool inSlidingIn: false //necessary because of its init structure
    property alias inSlidingOut: slidingAnimationAutoHiddenOut.running
    property bool inTempHiding: false
    property int length: root.isVertical ?  Screen.height : Screen.width   //screenGeometry.height : screenGeometry.width

    property int slidingOutToPos: ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) ?
                                      -thicknessNormal : thicknessNormal;

    property int thicknessAutoHidden: Latte.WindowSystem.compositingActive ?  2 : 1
    property int thicknessMid:  (1 + (0.65 * (root.maxZoomFactor-1)))*(root.iconSize+root.thickMargins+extraThickMask) //needed in some animations
    property int thicknessNormal: Math.max(root.iconSize + root.thickMargins + extraThickMask + 1,
                                           root.realPanelSize + root.panelShadow)
    property int thicknessZoom: ((root.iconSize+root.thickMargins+extraThickMask) * root.maxZoomFactor) + 2
    //it is used to keep thickness solid e.g. when iconSize changes from auto functions
    property int thicknessMidOriginal: Math.max(thicknessNormalOriginal,extraThickMask + (1 + (0.65 * (root.maxZoomFactor-1)))*(root.maxIconSize+root.maxThickMargin)) //needed in some animations
    property int thicknessNormalOriginal: root.maxIconSize + (root.maxThickMargin * 2) //this way we always have the same thickness published at all states
    /*property int thicknessNormalOriginal: !root.behaveAsPlasmaPanel || root.editMode ?
                                               thicknessNormalOriginalValue : root.realPanelSize + root.panelShadow*/

    property int thicknessNormalOriginalValue: root.maxIconSize + (root.maxThickMargin * 2) + extraThickMask + 1
    property int thicknessZoomOriginal: Math.max( ((root.maxIconSize+(root.maxThickMargin * 2)) * root.maxZoomFactor) + extraThickMask + 2,
                                                 root.realPanelSize + root.panelShadow,
                                                 (Latte.WindowSystem.compositingActive ? thicknessEditMode + root.editShadow : thicknessEditMode))

    //! is used from Panel in edit mode in order to provide correct masking
    property int thicknessEditMode: thicknessNormalOriginalValue + editModeVisual.settingsThickness

    //! is used to increase the mask thickness
    readonly property int marginBetweenContentsAndRuler: root.editMode ? 10 : 0
    property int extraThickMask: marginBetweenContentsAndRuler + Math.max(indicatorsExtraThickMask, shadowsExtraThickMask)
    //! this is set from indicators when they need extra thickness mask size
    readonly property int indicatorsExtraThickMask: indicators.info.extraMaskThickness
    property int shadowsExtraThickMask: {
        if (Latte.WindowSystem.isPlatformWayland) {
            return 0;
        }

        //! 45% of max shadow size in px.
        var shadowMaxNeededMargin = 0.45 * root.appShadowSizeOriginal;
        var shadowOpacity = (plasmoid.configuration.shadowOpacity) / 100;
        //! +40% of shadow opacity in percentage
        shadowOpacity = shadowOpacity + shadowOpacity*0.4;

        //! This way we are trying to calculate how many pixels are needed in order for the shadow
        //! to be drawn correctly without being cut of from View::mask() under X11
        shadowMaxNeededMargin = (shadowMaxNeededMargin * shadowOpacity);

        //! give some more space when items shadows are enabled and extremely big
        if (root.enableShadows && root.maxThickMargin < shadowMaxNeededMargin) {
                return shadowMaxNeededMargin - root.maxThickMargin;
        }

        return 0;
    }

    Binding{
        target: latteView
        property:"maxThickness"
        //! prevents updating window geometry during closing window in wayland and such fixes a crash
        when: latteView && !inTempHiding && !inForceHiding
        value: thicknessZoomOriginal
    }

    property bool validIconSize: (root.iconSize===root.maxIconSize || root.iconSize === root.automaticIconSizeBasedSize)
    property bool inPublishingState: validIconSize && !inSlidingIn && !inSlidingOut && !inTempHiding && !inForceHiding

    Binding{
        target: latteView
        property:"normalThickness"
        when: latteView && inPublishingState
        value: thicknessNormalOriginal
    }

    Binding{
        target: latteView
        property:"editThickness"
        when: latteView
        value: thicknessEditMode
    }

    Binding{
        target: latteView
        property: "type"
        when: latteView
        value: root.viewType
    }

    Binding{
        target: latteView
        property: "behaveAsPlasmaPanel"
        when: latteView
        value: root.editMode ? false : root.behaveAsPlasmaPanel
    }

    Binding{
        target: latteView
        property: "fontPixelSize"
        when: theme
        value: theme.defaultFont.pixelSize
    }

    Binding{
        target: latteView
        property:"inEditMode"
        when: latteView
        value: root.editMode
    }

    Binding{
        target: latteView
        property:"latteTasksArePresent"
        when: latteView
        value: latteApplet !== null
    }

    Binding{
        target: latteView
        property: "maxLength"
        when: latteView
        value: root.inConfigureAppletsMode ? 1 : plasmoid.configuration.maxLength/100
    }

    Binding{
        target: latteView
        property: "offset"
        when: latteView
        value: plasmoid.configuration.offset
    }

    Binding{
        target: latteView
        property: "alignment"
        when: latteView
        value: root.panelAlignment
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "backgroundOpacity"
        when: latteView && latteView.effects
        value: root.currentPanelTransparency
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "drawEffects"
        when: latteView && latteView.effects
        value: Latte.WindowSystem.compositingActive &&
               (((root.blurEnabled && root.useThemePanel)
                 || (root.blurEnabled && root.forceSolidPanel && Latte.WindowSystem.compositingActive))
                && (!root.inStartup || inForceHiding || inTempHiding))
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "drawShadows"
        when: latteView && latteView.effects
        value: root.drawShadowsExternal && (!root.inStartup || inForceHiding || inTempHiding)
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property:"editShadow"
        when: latteView && latteView.effects
        value: root.editShadow
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property:"innerShadow"
        when: latteView && latteView.effects
        value: {
            if (editModeVisual.editAnimationEnded && !root.behaveAsPlasmaPanel) {
                return root.editShadow;
            } else {
                return root.panelShadow;
            }
        }
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "settingsMaskSubtracted"
        when: latteView && latteView.effects
        value: {
            if (Latte.WindowSystem.compositingActive
                    && root.editMode
                    && editModeVisual.editAnimationEnded
                    && (root.animationsNeedBothAxis === 0 || root.zoomFactor===1) ) {
                return true;
            } else {
                return false;
            }
        }
    }

    Binding{
        target: latteView && latteView.windowsTracker ? latteView.windowsTracker : null
        property: "enabled"
        when: latteView && latteView.windowsTracker && latteView.visibility
        value: (latteView && latteView.visibility
                && !(latteView.visibility.mode === Latte.Types.AlwaysVisible /* Visibility */
                     || latteView.visibility.mode === Latte.Types.WindowsGoBelow
                     || latteView.visibility.mode === Latte.Types.AutoHide))
               || root.appletsNeedWindowsTracking > 0                        /*Applets Neew Windows Tracking */
               || root.dragActiveWindowEnabled                               /*Dragging Active Window(Empty Areas)*/
               || ((root.backgroundOnlyOnMaximized                           /*Dynamic Background */
                    || plasmoid.configuration.solidBackgroundForMaximized
                    || root.disablePanelShadowMaximized
                    || root.windowColors !== Latte.Types.NoneWindowColors))
    }

    Connections{
        target:root
        onPanelShadowChanged: updateMaskArea();
        onPanelThickMarginHighChanged: updateMaskArea();
    }

    Connections{
        target: layoutsManager
        onCurrentLayoutIsSwitching: {
            if (Latte.WindowSystem.compositingActive && latteView && latteView.layout && latteView.layout.name === layoutName) {
                manager.inTempHiding = true;
                manager.inForceHiding = true;
                root.clearZoom();
                manager.slotMustBeHide();
            }
        }
    }

    Connections{
        target: themeExtended ? themeExtended : null
        onRoundnessChanged: latteView.effects.forceMaskRedraw();
        onThemeChanged: latteView.effects.forceMaskRedraw();
    }

    onNormalStateChanged: {
        if (normalState) {
            root.updateAutomaticIconSize();
            root.updateSizeForAppletsInFill();
        }
    }

    onThicknessZoomOriginalChanged: {
        updateMaskArea();
    }

    function slotContainsMouseChanged() {
        if(latteView.visibility.containsMouse) {
            updateMaskArea();

            if (slidingAnimationAutoHiddenOut.running && !inTempHiding && !inForceHiding) {
                slotMustBeShown();
            }
        }
    }

    function slotMustBeShown() {
        //  console.log("show...");
        if (!slidingAnimationAutoHiddenIn.running && !inTempHiding && !inForceHiding){
            slidingAnimationAutoHiddenIn.init();
        }
    }

    function slotMustBeHide() {
        //! prevent sliding-in on startup if the dodge modes have sent a hide signal
        if (inStartupTimer.running && root.inStartup) {
            root.inStartup = false;
        }

        // console.log("hide....");
        if((!slidingAnimationAutoHiddenOut.running && !latteView.visibility.blockHiding
            && !latteView.visibility.containsMouse) || inForceHiding) {
            slidingAnimationAutoHiddenOut.init();
        }
    }

    //! functions used for sliding out/in during location/screen changes
    function slotHideDockDuringLocationChange() {
        inTempHiding = true;
        blockUpdateMask = true;
        slotMustBeHide();
    }

    function slotShowDockAfterLocationChange() {
        slidingAnimationAutoHiddenIn.init();
    }

    function sendHideDockDuringLocationChangeFinished(){
        blockUpdateMask = false;
        latteView.positioner.hideDockDuringLocationChangeFinished();
    }

    function sendSlidingOutAnimationEnded() {
        latteView.visibility.hide();
        latteView.visibility.isHidden = true;

        if (visibilityManager.debugMagager) {
            console.log("hiding animation ended...");
        }

        sendHideDockDuringLocationChangeFinished();
    }

    ///test maskArea
    function updateMaskArea() {
        if (!latteView || blockUpdateMask) {
            return;
        }

        var localX = 0;
        var localY = 0;

        normalState = ((root.animationsNeedBothAxis === 0) && (root.animationsNeedLength === 0))
                || (latteView.visibility.isHidden && !latteView.visibility.containsMouse && root.animationsNeedThickness == 0);


        // debug maskArea criteria
        if (debugMagager) {
            console.log(root.animationsNeedBothAxis + ", " + root.animationsNeedLength + ", " +
                        root.animationsNeedThickness + ", " + latteView.visibility.isHidden);

            if (previousNormalState !== normalState) {
                console.log("normal state changed to:" + normalState);
                previousNormalState = normalState;
            }
        }

        var tempLength = root.isHorizontal ? width : height;
        var tempThickness = root.isHorizontal ? height : width;

        var space = 0;

        if (Latte.WindowSystem.compositingActive) {
            if (root.useThemePanel){
                space = root.totalPanelEdgeSpacing + root.panelMarginLength + 1;
            } else {
                space = root.totalPanelEdgeSpacing + 1;
            }
        } else {
            space = root.totalPanelEdgeSpacing + root.panelMarginLength;
        }

        var noCompositingEdit = !Latte.WindowSystem.compositingActive && root.editMode;

        if (Latte.WindowSystem.compositingActive || noCompositingEdit) {
            if (normalState) {
                //console.log("entered normal state...");
                //count panel length


                //used when !compositing and in editMode
                if (noCompositingEdit) {
                    tempLength = root.isHorizontal ? root.width : root.height;
                } else {
                    if(root.isHorizontal) {
                        tempLength = plasmoid.configuration.panelPosition === Latte.Types.Justify ?
                                    layoutsContainer.width + space : layoutsContainer.mainLayout.width + space;
                    } else {
                        tempLength = plasmoid.configuration.panelPosition === Latte.Types.Justify ?
                                    layoutsContainer.height + space : layoutsContainer.mainLayout.height + space;
                    }
                }

                tempThickness = thicknessNormal;

                if (root.animationsNeedThickness > 0) {
                    tempThickness = Latte.WindowSystem.compositingActive ? thicknessZoom : thicknessNormal;
                }

                if (latteView.visibility.isHidden && !slidingAnimationAutoHiddenOut.running ) {
                    tempThickness = thicknessAutoHidden;
                }

                //configure x,y based on plasmoid position and root.panelAlignment(Alignment)
                if ((plasmoid.location === PlasmaCore.Types.BottomEdge) || (plasmoid.location === PlasmaCore.Types.TopEdge)) {
                    if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                        localY = latteView.visibility.isHidden && latteView.visibility.supportsKWinEdges ?
                                    latteView.height + tempThickness : latteView.height - tempThickness;
                    } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                        localY = latteView.visibility.isHidden && latteView.visibility.supportsKWinEdges ?
                                    -tempThickness : 0;
                    }

                    if (noCompositingEdit) {
                        localX = 0;
                    } else if (plasmoid.configuration.panelPosition === Latte.Types.Justify) {
                        localX = (latteView.width/2) - tempLength/2 + root.offset;
                    } else if (root.panelAlignment === Latte.Types.Left) {
                        localX = root.offset;
                    } else if (root.panelAlignment === Latte.Types.Center) {
                        localX = (latteView.width/2) - tempLength/2 + root.offset;
                    } else if (root.panelAlignment === Latte.Types.Right) {
                        localX = latteView.width - layoutsContainer.mainLayout.width - space - root.offset;
                    }
                } else if ((plasmoid.location === PlasmaCore.Types.LeftEdge) || (plasmoid.location === PlasmaCore.Types.RightEdge)){
                    if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                        localX = latteView.visibility.isHidden && latteView.visibility.supportsKWinEdges ?
                                    -tempThickness : 0;
                    } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                        localX = latteView.visibility.isHidden && latteView.visibility.supportsKWinEdges ?
                                    latteView.width + tempThickness : latteView.width - tempThickness;
                    }

                    if (noCompositingEdit) {
                        localY = 0;
                    } else if (plasmoid.configuration.panelPosition === Latte.Types.Justify) {
                        localY = (latteView.height/2) - tempLength/2 + root.offset;
                    } else if (root.panelAlignment === Latte.Types.Top) {
                        localY = root.offset;
                    } else if (root.panelAlignment === Latte.Types.Center) {
                        localY = (latteView.height/2) - tempLength/2 + root.offset;
                    } else if (root.panelAlignment === Latte.Types.Bottom) {
                        localY = latteView.height - layoutsContainer.mainLayout.height - space - root.offset;
                    }
                }
            } else {
                if(root.isHorizontal)
                    tempLength = Screen.width; //screenGeometry.width;
                else
                    tempLength = Screen.height; //screenGeometry.height;

                //grow only on length and not thickness
                if(root.animationsNeedLength>0 && root.animationsNeedBothAxis === 0) {

                    //this is used to fix a bug with shadow showing when the animation of edit mode
                    //is triggered
                    tempThickness = editModeVisual.editAnimationEnded ? thicknessEditMode + root.editShadow : thicknessEditMode

                    if (latteView.visibility.isHidden && !slidingAnimationAutoHiddenOut.running ) {
                        tempThickness = thicknessAutoHidden;
                    } else if (root.animationsNeedThickness > 0) {
                        tempThickness = thicknessZoomOriginal;
                    }
                } else{
                    //use all thickness space
                    if (latteView.visibility.isHidden && !slidingAnimationAutoHiddenOut.running ) {
                        tempThickness = Latte.WindowSystem.compositingActive ? thicknessAutoHidden : thicknessNormalOriginal;
                    } else {
                        tempThickness = thicknessZoomOriginal;
                    }
                }

                //configure the x,y position based on thickness
                if(plasmoid.location === PlasmaCore.Types.RightEdge)
                    localX = Math.max(0,latteView.width - tempThickness);
                else if(plasmoid.location === PlasmaCore.Types.BottomEdge)
                    localY = Math.max(0,latteView.height - tempThickness);
            }
        } // end of compositing calculations

        var maskArea = latteView.effects.mask;

        if (Latte.WindowSystem.compositingActive) {
            var maskLength = maskArea.width; //in Horizontal
            if (root.isVertical) {
                maskLength = maskArea.height;
            }

            var maskThickness = maskArea.height; //in Horizontal
            if (root.isVertical) {
                maskThickness = maskArea.width;
            }
        } else if (!noCompositingEdit){
            //! no compositing case
            if (!latteView.visibility.isHidden || !latteView.visibility.supportsKWinEdges) {
                localX = latteView.effects.rect.x;
                localY = latteView.effects.rect.y;
            } else {
                if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    localX = latteView.effects.rect.x;
                    localY = latteView.effects.rect.y+latteView.effects.rect.height+thicknessAutoHidden;
                } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                    localX = latteView.effects.rect.x;
                    localY = latteView.effects.rect.y - thicknessAutoHidden;
                } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    localX = latteView.effects.rect.x - thicknessAutoHidden;
                    localY = latteView.effects.rect.y;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    localX = latteView.effects.rect.x + latteView.effects.rect.width + 1;
                    localY = latteView.effects.rect.y;
                }
            }

            if (root.isHorizontal) {
                tempThickness = latteView.effects.rect.height;
                tempLength = latteView.effects.rect.width;
            } else {
                tempThickness = latteView.effects.rect.width;
                tempLength = latteView.effects.rect.height;
            }
        }

        //  console.log("Not updating mask...");
        if( maskArea.x !== localX || maskArea.y !== localY
                || maskLength !== tempLength || maskThickness !== tempThickness) {

            // console.log("Updating mask...");
            var newMaskArea = Qt.rect(-1,-1,0,0);
            newMaskArea.x = localX;
            newMaskArea.y = localY;

            if (isHorizontal) {
                newMaskArea.width = tempLength;
                newMaskArea.height = tempThickness;
            } else {
                newMaskArea.width = tempThickness;
                newMaskArea.height = tempLength;
            }

            if (!Latte.WindowSystem.compositingActive) {
                latteView.effects.mask = newMaskArea;
            } else {
                if (latteView.behaveAsPlasmaPanel && !root.editMode) {
                    latteView.effects.mask = Qt.rect(0,0,root.width,root.height);
                } else {
                    latteView.effects.mask = newMaskArea;
                }
            }
        }

        var validIconSize = (root.iconSize===root.maxIconSize || root.iconSize === root.automaticIconSizeBasedSize);

        //console.log("reached updating geometry ::: "+dock.maskArea);
        if(inPublishingState && (normalState || root.editMode)) {
            var tempGeometry = Qt.rect(latteView.effects.mask.x, latteView.effects.mask.y, latteView.effects.mask.width, latteView.effects.mask.height);

            //the shadows size must be removed from the maskArea
            //before updating the localDockGeometry
            if ((!latteView.behaveAsPlasmaPanel || root.editMode)
                    && Latte.WindowSystem.compositingActive) {
                var fixedThickness = root.editMode ? root.iconSize + root.thickMargins : root.realPanelThickness;

                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    tempGeometry.width = fixedThickness;
                } else {
                    tempGeometry.height = fixedThickness;
                }

                if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    tempGeometry.y = latteView.height - fixedThickness;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    tempGeometry.x = latteView.width - fixedThickness;
                }

                //set the boundaries for latteView local geometry
                //qBound = qMax(min, qMin(value, max)).
                tempGeometry.x = Math.max(0, Math.min(tempGeometry.x, latteView.width));
                tempGeometry.y = Math.max(0, Math.min(tempGeometry.y, latteView.height));
                tempGeometry.width = Math.min(tempGeometry.width, latteView.width);
                tempGeometry.height = Math.min(tempGeometry.height, latteView.height);
            }

            //console.log("update geometry ::: "+tempGeometry);
            if (!Latte.WindowSystem.compositingActive) {
                latteView.localGeometry = latteView.effects.rect;
            } else {
                latteView.localGeometry = tempGeometry;
            }
        }

    }

    Loader{
        anchors.fill: parent
        active: root.debugMode

        sourceComponent: Item{
            anchors.fill:parent

            Rectangle{
                id: windowBackground
                anchors.fill: parent
                border.color: "red"
                border.width: 1
                color: "transparent"
            }

            Rectangle{
                x: latteView ? latteView.effects.mask.x : -1
                y: latteView ? latteView.effects.mask.y : -1
                height: latteView ? latteView.effects.mask.height : 0
                width: latteView ? latteView.effects.mask.width : 0

                border.color: "green"
                border.width: 1
                color: "transparent"
            }
        }
    }

    /***Hiding/Showing Animations*****/

    //////////////// Animations - Slide In - Out
    SequentialAnimation{
        id: slidingAnimationAutoHiddenOut

        ScriptAction{
            script: {
                root.isHalfShown = true;
            }
        }

        PropertyAnimation {
            target: layoutsContainer
            property: root.isVertical ? "x" : "y"
            to: {
                if (Latte.WindowSystem.compositingActive) {
                    return slidingOutToPos;
                } else {
                    if ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) {
                        return slidingOutToPos + 1;
                    } else {
                        return slidingOutToPos - 1;
                    }
                }
            }
            duration: manager.animationSpeed
            easing.type: Easing.InQuad
        }

        ScriptAction{
            script: {
                latteView.visibility.isHidden = true;
            }
        }

        onStarted: {
            if (manager.debugMagager) {
                console.log("hiding animation started...");
            }
        }

        onStopped: {
            //! Trying to move the ending part of the signals at the end of editing animation
            if (!manager.inTempHiding) {
                manager.updateMaskArea();
            } else {
                if (!editModeVisual.inEditMode) {
                    manager.sendSlidingOutAnimationEnded();
                }
            }

            latteView.visibility.slideOutFinished();
        }

        function init() {
            if (!latteView.visibility.blockHiding) {
                start();
            }
        }
    }

    SequentialAnimation{
        id: slidingAnimationAutoHiddenIn

        PauseAnimation{
            duration: manager.inTempHiding && animationsEnabled ? 500 : 0
        }

        PropertyAnimation {
            target: layoutsContainer
            property: root.isVertical ? "x" : "y"
            to: 0
            duration: manager.animationSpeed
            easing.type: Easing.OutQuad
        }

        ScriptAction{
            script: {
                root.isHalfShown = false;
                root.inStartup = false;
            }
        }

        onStarted: {
            latteView.visibility.show();

            if (manager.debugMagager) {
                console.log("showing animation started...");
            }
        }

        onStopped: {
            inSlidingIn = false;

            if (manager.inTempHiding) {
                manager.inTempHiding = false;
                updateAutomaticIconSize();
            }

            manager.inTempHiding = false;
            updateAutomaticIconSize();

            if (manager.debugMagager) {
                console.log("showing animation ended...");
            }

            latteView.visibility.slideInFinished();
        }

        function init() {
            // if (!latteView.visibility.blockHiding)
            inSlidingIn = true;

            if (slidingAnimationAutoHiddenOut.running) {
                slidingAnimationAutoHiddenOut.stop();
            }

            latteView.visibility.isHidden = false;
            updateMaskArea();

            start();
        }
    }
}
