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

import org.kde.latte 0.1 as Latte

Item{
    id: manager

    anchors.fill: parent

    property QtObject window

    property bool debugMagager: Qt.application.arguments.indexOf("--mask") >= 0

    property bool blockUpdateMask: false
    property bool inForceHiding: false //is used when the docks are forced in hiding e.g. when changing layouts
    property bool normalState : false  // this is being set from updateMaskArea
    property bool previousNormalState : false // this is only for debugging purposes
    property bool panelIsBiggerFromIconSize: root.useThemePanel && (root.themePanelSize >= root.iconSize)

    property int animationSpeed: Latte.WindowSystem.compositingActive ? root.durationTime * 1.2 * units.longDuration : 0
    property bool inSlidingIn: false //necessary because of its init structure
    property alias inSlidingOut: slidingAnimationAutoHiddenOut.running
    property bool inTempHiding: false
    property int length: root.isVertical ?  Screen.height : Screen.width   //screenGeometry.height : screenGeometry.width

    property int slidingOutToPos: ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) ?
                                      -thicknessNormal : thicknessNormal;

    property int statesLineSizeOriginal: root.latteApplet ? Math.ceil( root.maxIconSize/13 ) : 0
    property int thickReverseAndGlowExtraSize: (root.reverseLinesPosition && root.showGlow && !behaveAsPlasmaPanel) ? 2*statesLineSize : 0;
    property int thickReverseAndGlowExtraSizeOriginal: Math.ceil(2*root.maxIconSize/13 )

    property int thicknessAutoHidden: Latte.WindowSystem.compositingActive ?  2 : 1
    property int thicknessMid: root.statesLineSize + (1 + (0.65 * (root.zoomFactor-1)))*(root.iconSize+root.thickMargin + thickReverseAndGlowExtraSize) //needed in some animations
    property int thicknessNormal: Math.max(root.statesLineSize + root.iconSize + root.thickMargin + thickReverseAndGlowExtraSize +1,
                                           root.realPanelSize + root.panelShadow)
    property int thicknessZoom: root.statesLineSize + ((root.iconSize+root.thickMargin + thickReverseAndGlowExtraSize) * root.zoomFactor) + 2
    //it is used to keep thickness solid e.g. when iconSize changes from auto functions
    property int thicknessMidOriginal: Math.max(thicknessNormalOriginal, statesLineSizeOriginal + thickReverseAndGlowExtraSizeOriginal + (1 + (0.65 * (root.zoomFactor-1)))*(root.maxIconSize+root.thickMarginOriginal)) //needed in some animations
    property int thicknessNormalOriginal: !root.behaveAsPlasmaPanel || root.editMode ?
                                              thicknessNormalOriginalValue : root.realPanelSize + root.panelShadow

    property int thicknessNormalOriginalValue: statesLineSizeOriginal + thickReverseAndGlowExtraSizeOriginal +
                                               root.maxIconSize + root.thickMarginOriginal + 1
    property int thicknessZoomOriginal: Math.max(statesLineSizeOriginal + thickReverseAndGlowExtraSizeOriginal
                                                 + ((root.maxIconSize+root.thickMarginOriginal) * root.zoomFactor) + 2,
                                                 root.realPanelSize + root.panelShadow,
                                                 thicknessEditMode)

    property int thicknessEditMode: thicknessNormalOriginalValue + theme.defaultFont.pixelSize + root.editShadow

    Binding{
        target: dock
        property:"maxThickness"
        //! prevents updating window geometry during closing window in wayland and such fixes a crash
        when: dock && !inTempHiding && !inForceHiding
        value: thicknessZoomOriginal
    }

    Binding{
        target: dock
        property:"normalThickness"
        when: dock
        value: thicknessNormalOriginal
    }

    Binding{
        target: dock
        property: "behaveAsPlasmaPanel"
        when: dock
        value: root.editMode ? false : root.behaveAsPlasmaPanel
    }

    Binding{
        target: dock
        property: "colorizerSupport"
        when: dock
        value: root.forceColorizer && plasmoid.configuration.solidBackgroundForMaximized
    }

    Binding{
        target: dock
        property: "dockTransparency"
        when: dock
        value: root.panelTransparency
    }

    Binding{
        target: dock
        property: "drawShadows"
        when: dock
        value: root.drawShadowsExternal && (!root.inStartup || inForceHiding || inTempHiding)
    }

    Binding{
        target: dock
        property: "drawEffects"
        when: dock
        value: ((root.blurEnabled && root.useThemePanel && !root.solidPanel)
                || (root.blurEnabled && root.forceSolidPanel && dock.visibility.existsWindowMaximized && Latte.WindowSystem.compositingActive))
               && (!root.inStartup || inForceHiding || inTempHiding)
    }

    Binding{
        target: dock
        property: "fontPixelSize"
        when: theme
        value: theme.defaultFont.pixelSize
    }

    Binding{
        target: dock
        property:"inEditMode"
        when: dock
        value: root.editMode
    }

    Binding{
        target: dock
        property:"shadow"
        when: dock
        value: root.panelShadow
    }

    Binding{
        target: dock
        property: "maxLength"
        when: dock
        value: root.editMode ? 1 : plasmoid.configuration.maxLength/100
    }


    Binding{
        target: dock
        property: "offset"
        when: dock
        value: plasmoid.configuration.offset
    }

    Binding{
        target: dock
        property: "alignment"
        when: dock
        value: root.panelAlignment
    }

    Binding{
        target: dock && dock.visibility ? dock.visibility : null
        property: "enabledDynamicBackground"
        when: dock && dock.visibility
        value: (root.backgroundOnlyOnMaximized
                || plasmoid.configuration.solidBackgroundForMaximized
                || root.disablePanelShadowMaximized)
               && Latte.WindowSystem.compositingActive
    }

    Connections{
        target:root
        onPanelShadowChanged: updateMaskArea();
        onPanelMarginChanged: updateMaskArea();
    }

    Connections{
        target: universalLayoutManager
        onCurrentLayoutIsSwitching: {
            if (Latte.WindowSystem.compositingActive && root.dockManagedLayout && root.dockManagedLayout.name === layoutName) {
                manager.inTempHiding = true;
                manager.inForceHiding = true;
                root.clearZoom();
                manager.slotMustBeHide();
            }
        }
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
        if(dock.visibility.containsMouse) {
            updateMaskArea();
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
        if((!slidingAnimationAutoHiddenOut.running && !dock.visibility.blockHiding
            && !dock.visibility.containsMouse) || inForceHiding) {
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
        dock.hideDockDuringLocationChangeFinished();
    }

    ///test maskArea
    function updateMaskArea() {
        if (!dock || blockUpdateMask) {
            return;
        }

        var localX = 0;
        var localY = 0;

        normalState = ((root.animationsNeedBothAxis === 0) && (root.animationsNeedLength === 0))
                || (dock.visibility.isHidden && !dock.visibility.containsMouse && root.animationsNeedThickness == 0);

        // debug maskArea criteria
        if (debugMagager) {
            console.log(root.animationsNeedBothAxis + ", " + root.animationsNeedLength + ", " +
                        root.animationsNeedThickness + ", " + dock.visibility.isHidden);

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

        if (Latte.WindowSystem.compositingActive) {
            if (normalState) {
                //console.log("entered normal state...");
                //count panel length

                var noCompositingEdit = !Latte.WindowSystem.compositingActive && root.editMode;
                //used when !compositing and in editMode
                if (noCompositingEdit) {
                    tempLength = root.isHorizontal ? root.width : root.height;
                } else {
                    if(root.isHorizontal) {
                        tempLength = plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                    layoutsContainer.width + space : layoutsContainer.mainLayout.width + space;
                    } else {
                        tempLength = plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                    layoutsContainer.height + space : layoutsContainer.mainLayout.height + space;
                    }
                }

                tempThickness = thicknessNormal;

                if (root.animationsNeedThickness > 0) {
                    tempThickness = Latte.WindowSystem.compositingActive ? thicknessZoom : thicknessNormal;
                }

                if (dock.visibility.isHidden && !slidingAnimationAutoHiddenOut.running ) {
                    tempThickness = thicknessAutoHidden;
                }

                //configure x,y based on plasmoid position and root.panelAlignment(Alignment)
                if ((plasmoid.location === PlasmaCore.Types.BottomEdge) || (plasmoid.location === PlasmaCore.Types.TopEdge)) {
                    if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                        localY = dock.visibility.isHidden && dock.visibility.supportsKWinEdges ?
                                    dock.height + tempThickness : dock.height - tempThickness;
                    } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                        localY = dock.visibility.isHidden && dock.visibility.supportsKWinEdges ?
                                    -tempThickness : 0;
                    }

                    if (noCompositingEdit) {
                        localX = 0;
                    } else if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                        localX = (dock.width/2) - tempLength/2 + root.offset;
                    } else if (root.panelAlignment === Latte.Dock.Left) {
                        localX = root.offset;
                    } else if (root.panelAlignment === Latte.Dock.Center) {
                        localX = (dock.width/2) - tempLength/2 + root.offset;
                    } else if (root.panelAlignment === Latte.Dock.Right) {
                        localX = dock.width - layoutsContainer.mainLayout.width - space - root.offset;
                    }
                } else if ((plasmoid.location === PlasmaCore.Types.LeftEdge) || (plasmoid.location === PlasmaCore.Types.RightEdge)){
                    if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                        localX = dock.visibility.isHidden && dock.visibility.supportsKWinEdges ?
                                    -tempThickness : 0;
                    } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                        localX = dock.visibility.isHidden && dock.visibility.supportsKWinEdges ?
                                    dock.width + tempThickness : dock.width - tempThickness;
                    }

                    if (noCompositingEdit) {
                        localY = 0;
                    } else if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                        localY = (dock.height/2) - tempLength/2 + root.offset;
                    } else if (root.panelAlignment === Latte.Dock.Top) {
                        localY = root.offset;
                    } else if (root.panelAlignment === Latte.Dock.Center) {
                        localY = (dock.height/2) - tempLength/2 + root.offset;
                    } else if (root.panelAlignment === Latte.Dock.Bottom) {
                        localY = dock.height - layoutsContainer.mainLayout.height - space - root.offset;
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
                    tempThickness = editModeVisual.editAnimationEnded ? thicknessNormalOriginal + theme.defaultFont.pixelSize + root.editShadow :
                                                                        thicknessNormalOriginal + theme.defaultFont.pixelSize

                    if (dock.visibility.isHidden && !slidingAnimationAutoHiddenOut.running ) {
                        tempThickness = thicknessAutoHidden;
                    } else if (root.animationsNeedThickness > 0) {
                        tempThickness = thicknessZoomOriginal;
                    }
                } else{
                    //use all thickness space
                    if (dock.visibility.isHidden && !slidingAnimationAutoHiddenOut.running ) {
                        tempThickness = Latte.WindowSystem.compositingActive ? thicknessAutoHidden : thicknessNormalOriginal;
                    } else {
                        tempThickness = thicknessZoomOriginal;
                    }
                }

                //configure the x,y position based on thickness
                if(plasmoid.location === PlasmaCore.Types.RightEdge)
                    localX = Math.max(0,dock.width - tempThickness);
                else if(plasmoid.location === PlasmaCore.Types.BottomEdge)
                    localY = Math.max(0,dock.height - tempThickness);
            }
        } // end of compositing calculations

        var maskArea = dock.maskArea;

        if (Latte.WindowSystem.compositingActive) {
            var maskLength = maskArea.width; //in Horizontal
            if (root.isVertical) {
                maskLength = maskArea.height;
            }

            var maskThickness = maskArea.height; //in Horizontal
            if (root.isVertical) {
                maskThickness = maskArea.width;
            }
        } else {
            //! no compositing case
            if (!dock.visibility.isHidden || !dock.visibility.supportsKWinEdges) {
                localX = dock.effectsArea.x;
                localY = dock.effectsArea.y;
            } else {
                if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    localX = dock.effectsArea.x;
                    localY = dock.effectsArea.y+dock.effectsArea.height+thicknessAutoHidden;
                } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                    localX = dock.effectsArea.x;
                    localY = dock.effectsArea.y - thicknessAutoHidden;
                } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    localX = dock.effectsArea.x - thicknessAutoHidden;
                    localY = dock.effectsArea.y;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    localX = dock.effectsArea.x + dock.effectsArea.width + 1;
                    localY = dock.effectsArea.y;
                }
            }

            if (root.isHorizontal) {
                tempThickness = dock.effectsArea.height;
                tempLength = dock.effectsArea.width;
            } else {
                tempThickness = dock.effectsArea.width;
                tempLength = dock.effectsArea.height;
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
                dock.maskArea = newMaskArea;//dock.effectsArea;
            } else {
                if (dock.behaveAsPlasmaPanel && !root.editMode) {
                    dock.maskArea = Qt.rect(0,0,root.width,root.height);
                } else {
                    dock.maskArea = newMaskArea;
                }
            }
        }

        //console.log("reached updating geometry ::: "+dock.maskArea);
        if(normalState){

            var tempGeometry = Qt.rect(dock.maskArea.x, dock.maskArea.y, dock.maskArea.width, dock.maskArea.height);

            //the shadows size must be removed from the maskArea
            //before updating the localDockGeometry
            if ((!dock.behaveAsPlasmaPanel || root.editMode)
                    && Latte.WindowSystem.compositingActive) {
                var fixedThickness = root.realPanelThickness;

                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    tempGeometry.width = fixedThickness;
                } else {
                    tempGeometry.height = fixedThickness;
                }

                if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    tempGeometry.y = dock.height - fixedThickness;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    tempGeometry.x = dock.width - fixedThickness;
                }

                //set the boundaries for dock local geometry
                //qBound = qMax(min, qMin(value, max)).
                tempGeometry.x = Math.max(0, Math.min(tempGeometry.x, dock.width));
                tempGeometry.y = Math.max(0, Math.min(tempGeometry.y, dock.height));
                tempGeometry.width = Math.min(tempGeometry.width, dock.width);
                tempGeometry.height = Math.min(tempGeometry.height, dock.height);
            }

            //console.log("update geometry ::: "+tempGeometry);
            if (!Latte.WindowSystem.compositingActive) {
                dock.localGeometry = dock.effectsArea;
            } else {
                dock.localGeometry = tempGeometry;
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
                x: dock ? dock.maskArea.x : -1
                y: dock ? dock.maskArea.y : -1
                height: dock ? dock.maskArea.height : 0
                width: dock ? dock.maskArea.width : 0

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
                dock.visibility.isHidden = true;
            }
        }

        onStarted: {
            if (manager.debugMagager) {
                console.log("hiding animation started...");
            }
        }

        onStopped: {
            dock.visibility.isHidden = true;

            if (manager.debugMagager) {
                console.log("hiding animation ended...");
            }

            if (!manager.inTempHiding) {
                updateMaskArea();
            } else if (plasmoid.configuration.durationTime === 0) {
                sendHideDockDuringLocationChangeFinished();
            }
        }

        function init() {
            if (!dock.visibility.blockHiding)
                start();
        }
    }

    SequentialAnimation{
        id: slidingAnimationAutoHiddenIn

        PauseAnimation{
            duration: manager.inTempHiding && root.durationTime>0 ? 500 : 0
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
        }

        function init() {
            // if (!dock.visibility.blockHiding)
            inSlidingIn = true;

            if (slidingAnimationAutoHiddenOut.running) {
                slidingAnimationAutoHiddenOut.stop();
            }

            dock.visibility.isHidden = false;
            updateMaskArea();

            start();
        }
    }
}
