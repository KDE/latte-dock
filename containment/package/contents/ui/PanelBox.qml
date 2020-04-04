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
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

import "colorizer" as Colorizer

import org.kde.latte 0.2 as Latte

Item{
    id:barLine

    width: root.isHorizontal ? panelWidth : smallSize
    height: root.isVertical ? panelHeight : smallSize

    opacity: root.useThemePanel ? 1 : 0

    readonly property bool isShown: (solidBackground.opacity > 0) || (overlayedBackground.opacity > 0)

    property int animationTime: 6*root.durationTime*root.shortDuration

    property int screenEdgeMargin: root.screenEdgeMarginEnabled ? root.localScreenEdgeMargin - shadowsSvgItem.screenEdgeShadow : 0

    property int lengthMargins: {
        return root.isVertical ? shadowsSvgItem.marginsHeight : shadowsSvgItem.marginsWidth
    }

    property int panelWidth: {
        if (root.behaveAsPlasmaPanel && Latte.WindowSystem.compositingActive && !root.editMode) {
            return root.width;
        } else {
            if ((root.panelAlignment === Latte.Types.Justify) && root.isHorizontal) {
                return root.maxLength;
            } else {
                return Math.max(root.minLength, layoutsContainerItem.mainLayout.width + spacing);
            }
        }
    }

    property int panelHeight: {
        if (root.behaveAsPlasmaPanel && Latte.WindowSystem.compositingActive && !root.editMode) {
            return root.height;
        } else {
            if ((root.panelAlignment === Latte.Types.Justify) && root.isVertical) {
                return root.maxLength;
            } else {
                return Math.max(root.minLength, layoutsContainerItem.mainLayout.height + spacing);
            }
        }
    }

    property int spacing: {
        if (root.panelAlignment === Latte.Types.Justify && root.maxLengthPerCentage === 100) {
            return 0;
        } else if (!Latte.WindowSystem.compositingActive) {
            return root.panelEdgeSpacing/2;
        } else if (root.panelAlignment === Latte.Types.Center || root.panelAlignment === Latte.Types.Justify || root.offset!==0) {
            return root.panelEdgeSpacing/2;
        } else {
            return root.panelEdgeSpacing/4;
        }
    }
    property int smallSize: 16 //Math.max(3.7*root.statesLineSize, 16)

    Behavior on opacity{
        enabled: Latte.WindowSystem.compositingActive
        NumberAnimation {
            duration: barLine.animationTime
        }
    }

    Behavior on opacity{
        enabled: !Latte.WindowSystem.compositingActive
        NumberAnimation {
            duration: 0
        }
    }

    Binding{
        target: root
        property: "currentPanelTransparency"
        value: overlayedBackground.opacity>0 ? overlayedBackground.opacity*100 : solidBackground.opacity*100
    }

    Binding {
        target: root
        property: "realPanelLength"
        value: root.isVertical ? barLine.height : barLine.width
    }

    Binding {
        target: root
        property: "totalPanelEdgeSpacing"
        value: spacing
    }

    onXChanged: solidBackground.updateEffectsArea();
    onYChanged: solidBackground.updateEffectsArea();

    /// plasmoid's default panel
    /*  BorderImage{
            anchors.fill:parent
            source: "../images/panel-west.png"
            border { left:8; right:8; top:8; bottom:8 }

            opacity: (!root.useThemePanel) ? 1 : 0

            visible: (opacity == 0) ? false : true

            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch

            Behavior on opacity{
                NumberAnimation { duration: 200 }
            }
        }*/

    /// the current theme's panel
    PlasmaCore.FrameSvgItem{
        id: shadowsSvgItem

        width: root.isVertical ? panelSize + marginsWidth : Math.min(parent.width + marginsWidth, root.width - marginsWidth)
        height: root.isVertical ? Math.min(parent.height + marginsHeight, root.height - marginsHeight) : panelSize + marginsHeight

        imagePath: hideShadow ? "" : "widgets/panel-background"
        prefix: hideShadow ? "" : "shadow"

        visible: (opacity == 0) ? false : true

        opacity: {
            if ((root.forceTransparentPanel && !root.forcePanelForBusyBackground)
                    || !root.useThemePanel)
                return 0;
            else
                return 1;
        }

        enabledBorders: latteView && latteView.effects ? latteView.effects.enabledBorders : PlasmaCore.FrameSvg.NoBorder

        //! set true by default in order to avoid crash on startup because imagePath is set to ""
        readonly property bool themeHasShadow: themeExtended ? themeExtended.hasShadow : true

        readonly property bool hideShadow: (root.behaveAsPlasmaPanel && !root.editMode)
                                           || !Latte.WindowSystem.compositingActive
                                           || !root.panelShadowsActive
                                           || !themeHasShadow

        Behavior on opacity {
            enabled: Latte.WindowSystem.compositingActive
            NumberAnimation { duration: barLine.animationTime }
        }


        Behavior on opacity{
            enabled: !Latte.WindowSystem.compositingActive
            NumberAnimation { duration: 0 }
        }

        property int screenEdgeShadow: {
            if (imagePath === "" || !root.screenEdgeMarginEnabled) {
                return 0;
            }

            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                return margins.left;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                return margins.right;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                return margins.top;
            } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                return margins.bottom;
            }
        }

        property int marginsWidth: {
            if (imagePath === "") {
                return 0;
            } else {
                if (root.panelAlignment === Latte.Types.Left && root.offset===0)
                    return margins.right;
                else if (root.panelAlignment === Latte.Types.Right && root.offset===0)
                    return margins.left;
                else
                    return margins.left+margins.right;
            }
        }

        property int marginsHeight: {
            if (imagePath === "") {
                return 0;
            } else {
                if (root.panelAlignment === Latte.Types.Top && root.offset===0)
                    return margins.bottom;
                else if (root.panelAlignment === Latte.Types.Bottom && root.offset===0)
                    return margins.top;
                else
                    return margins.top + margins.bottom;
            }
        }

        property int panelSize: automaticPanelSize
        property int automaticPanelSize: {
            if (root.behaveAsPlasmaPanel) {
                return root.iconSize + root.thickMargins;// + 2;
            } else {
                var icons = root.iconSize + root.thickMargins;// + 2;
                var panelt = root.themePanelThickness;// + 2;

                root.realPanelThickness = icons + root.localScreenEdgeMargin;

                return Math.min(icons, panelt);
            }
        }

        property int shadowsSize: 0

        Binding{
            target: shadowsSvgItem
            property: "shadowsSize"
            value:{
                if (shadowsSvgItem && !root.behaveAsPlasmaPanel && root.useThemePanel && root.panelShadowsActive){
                    if (root.isVertical){
                        if (plasmoid.location === PlasmaCore.Types.LeftEdge)
                            return shadowsSvgItem.margins.right;
                        else if (plasmoid.location === PlasmaCore.Types.RightEdge)
                            return shadowsSvgItem.margins.left;
                    } else {
                        if (plasmoid.location === PlasmaCore.Types.BottomEdge)
                            return shadowsSvgItem.margins.top;
                        else if (plasmoid.location === PlasmaCore.Types.TopEdge)
                            return shadowsSvgItem.margins.bottom;
                    }
                } else {
                    return 0;
                }
            }
        }

        Behavior on opacity{
            enabled: Latte.WindowSystem.compositingActive
            NumberAnimation { duration: barLine.animationTime }
        }

        Behavior on opacity{
            enabled: !Latte.WindowSystem.compositingActive
            NumberAnimation { duration: 0 }
        }

        Binding {
            target: root
            property: "panelShadow"
            when: shadowsSvgItem
            value: shadowsSvgItem.shadowsSize
        }

        Binding {
            target: root
            property: "realPanelSize"
            when: shadowsSvgItem
            value: shadowsSvgItem.panelSize
        }

        Binding {
            target: root
            property: "panelMarginLength"
            when: shadowsSvgItem
            value: root.isVertical ? shadowsSvgItem.marginsHeight : shadowsSvgItem.marginsWidth
        }

        //! Show a fake blurness under panel background for editMode visual feedback
        Loader {
            anchors.fill: solidBackground
            active: editModeVisual.inEditMode && root.userShowPanelBackground && plasmoid.configuration.blurEnabled
            sourceComponent: Item {
                Image{
                    id: backTiler
                    anchors.fill: parent
                    visible: false

                    fillMode: Image.Tile
                    source: hasBackground ? latteView.layout.background : "../icons/"+latteView.layout.background+"print.jpg"

                    readonly property bool hasBackground: (latteView && latteView.layout && latteView.layout.background.startsWith("/")) ?
                                                              true : false
                }

                ShaderEffectSource {
                    id: effectSource
                    anchors.fill: parent
                    visible: false

                    sourceItem: backTiler
                    sourceRect: Qt.rect(0,0, width, height)
                }

                FastBlur{
                    id: blur
                    anchors.fill: parent
                    opacity: editModeVisual.appliedOpacity * 1.4

                    source: effectSource
                    radius: 50
                    visible: false
                }

                OpacityMask {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenterOffset: overlayedBackground.painterRectangle.anchors.horizontalCenterOffset
                    anchors.verticalCenterOffset: overlayedBackground.painterRectangle.anchors.verticalCenterOffset

                    width: overlayedBackground.painterRectangle.width
                    height: overlayedBackground.painterRectangle.height

                    source: blur
                    maskSource: overlayedBackground.painterRectangle
                    opacity: blur.opacity
                }
            }
        }

        //! This is used to provide real solidness
        Colorizer.CustomBackground {
            id: backgroundLowestRectangle
            anchors.fill: solidBackground
            opacity: normalizedOpacity
            backgroundColor: colorizerManager.backgroundColor
            roundness: overlayedBackground.roundness
            visible: Latte.WindowSystem.compositingActive && solidBackground.exceedsThemeOpacityLimits

            readonly property real normalizedOpacity: visible ?  Math.min(1, (appliedOpacity - solidBackground.themeMaxOpacity)/(1-solidBackground.themeMaxOpacity)) : 0
            readonly property real appliedOpacity: visible ? solidBackground.appliedOpacity : 0

            Behavior on opacity{
                enabled: Latte.WindowSystem.compositingActive
                NumberAnimation { duration: barLine.animationTime }
            }

            Behavior on opacity{
                enabled: !Latte.WindowSystem.compositingActive
                NumberAnimation { duration: 0 }
            }
        }

        PlasmaCore.FrameSvgItem{
            id: solidBackground
            anchors.leftMargin: Latte.WindowSystem.compositingActive ? shadowsSvgItem.margins.left : 0
            anchors.rightMargin: Latte.WindowSystem.compositingActive ? shadowsSvgItem.margins.right : 0
            anchors.topMargin: Latte.WindowSystem.compositingActive ? shadowsSvgItem.margins.top : 0
            anchors.bottomMargin: Latte.WindowSystem.compositingActive ? shadowsSvgItem.margins.bottom : 0
            anchors.fill:parent

            opacity: normalizedOpacity

            readonly property bool exceedsThemeOpacityLimits: appliedOpacity > themeMaxOpacity
            readonly property bool forceSolidness: root.forceSolidPanel || !Latte.WindowSystem.compositingActive

            //! must be normalized to plasma theme maximum opacity
            readonly property real normalizedOpacity: Math.min(1, appliedOpacity / themeMaxOpacity)

            readonly property real appliedOpacity: overlayedBackground.opacity > 0 && !paintInstantly ? 0 : overlayedBackground.midOpacity
            readonly property real themeMaxOpacity: {
                if (themeExtended) {
                    switch(plasmoid.location) {
                    case PlasmaCore.Types.BottomEdge: return themeExtended.bottomEdgeMaxOpacity;
                    case PlasmaCore.Types.LeftEdge: return themeExtended.leftEdgeMaxOpacity;
                    case PlasmaCore.Types.TopEdge: return themeExtended.topEdgeMaxOpacity;
                    case PlasmaCore.Types.RightEdge: return themeExtended.rightEdgeMaxOpacity;
                    default: return 0;
                    }
                }

                return 1;
            }

            //! When switching from overlaied background to regular one this must be done
            //! instantly otherwise the transition is not smooth
            readonly property bool paintInstantly: (root.hasExpandedApplet && root.plasmaBackgroundForPopups)
                                                   || root.plasmaStyleBusyForTouchingBusyVerticalView

            property rect efGeometry: Qt.rect(-1,-1,0,0)

            imagePath: "widgets/panel-background"

            onWidthChanged: updateEffectsArea();
            onHeightChanged: updateEffectsArea();

            Component.onCompleted: {
                root.updateEffectsArea.connect(updateEffectsArea);
                adjustPrefix();
            }

            Component.onDestruction: {
                root.updateEffectsArea.disconnect(updateEffectsArea);
            }

            onImagePathChanged: solidBackground.adjustPrefix();

            Connections{
                target: root

                onEditModeChanged: {
                    solidBackground.updateEffectsArea();
                }
            }

            //! Fix for FrameSvgItem QML version not updating its margins after a theme change
            //! with this hack we enforce such update. I could use the repaintNeeded signal but
            //! it is called more often than the themeChanged one.
            Connections {
                target: themeExtended
                onThemeChanged: {
                    solidBackground.adjustPrefix();
                    plasmoid.configuration.panelShadows = !plasmoid.configuration.panelShadows;
                    plasmoid.configuration.panelShadows = !plasmoid.configuration.panelShadows;
                    updateEffectsArea();
                }
            }

            Connections {
                target: latteView ? latteView.visibility : null
                onIsHiddenChanged: solidBackground.updateEffectsArea();
            }


            Connections{
                target: plasmoid
                onLocationChanged: solidBackground.adjustPrefix();
            }

            function updateEffectsArea(){
                if (!latteView)
                    return;

                if (!Latte.WindowSystem.compositingActive) {
                    //! NOCOMPOSITING mode is a special case and Effects Area is also used for
                    //! different calculations for View::mask()
                    var rootGeometry = mapToItem(root, 0, 0);
                    efGeometry.x = rootGeometry.x;
                    efGeometry.y = rootGeometry.y;
                    efGeometry.width = width;
                    efGeometry.height = height;
                } else {
                    if (latteView.visibility.isHidden) {
                        //! valid hide mask
                        efGeometry.x = -1;
                        efGeometry.y = -1;
                        efGeometry.width = 1;
                        efGeometry.height = 1;
                    } else {
                        if (!root.behaveAsPlasmaPanel || root.editMode) {
                            var rootGeometry = mapToItem(root, 0, 0);
                            efGeometry.x = rootGeometry.x;
                            efGeometry.y = rootGeometry.y;
                        } else {
                            efGeometry.x = 0;
                            efGeometry.y = 0;
                        }

                        efGeometry.width = width;
                        efGeometry.height = height;
                    }
                }

                latteView.effects.rect = efGeometry;

                //! needed both for NOCOMPOSITING environments AND
                //! View::localGeometry calculations
                visibilityManager.updateMaskArea();
            }

            Binding {
                target: root
                property: "panelThickMarginBase"
                value: {
                    var minimum = root.screenEdgeMarginEnabled && themeExtended ? themeExtended.topEdgeRoundness : 0;

                    if (root.useThemePanel){
                        if (root.isVertical){
                            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                                return Math.max(minimum, solidBackground.margins.left);
                            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                                return Math.max(minimum, solidBackground.margins.right);
                            }
                        } else {
                            if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                                return Math.max(minimum, solidBackground.margins.bottom);
                            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                                return Math.max(minimum, solidBackground.margins.top);
                            }
                        }
                    } else {
                        return 0;
                    }
                }
            }

            Binding {
                target: root
                property: "panelThickMarginHigh"
                value: {
                    var minimum = root.screenEdgeMarginEnabled && themeExtended ? themeExtended.topEdgeRoundness : 0;

                    if (root.useThemePanel){
                        if (root.isVertical){
                            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                                return Math.max(minimum, solidBackground.margins.right);
                            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                                return Math.max(minimum, solidBackground.margins.left);
                            }
                        } else {
                            if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                                return Math.max(minimum, solidBackground.margins.top);
                            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                                return Math.max(minimum, solidBackground.margins.bottom);
                            }
                        }
                    } else {
                        return 0;
                    }
                }
            }

            onRepaintNeeded: {
                if (root.behaveAsPlasmaPanel)
                    adjustPrefix();
            }

            enabledBorders: latteView && latteView.effects ? latteView.effects.enabledBorders : PlasmaCore.FrameSvg.NoBorder

            Behavior on opacity{
                enabled: Latte.WindowSystem.compositingActive && !solidBackground.paintInstantly
                NumberAnimation { duration: barLine.animationTime }
            }

            Behavior on opacity{
                enabled: !Latte.WindowSystem.compositingActive
                NumberAnimation { duration: 0 }
            }

            function adjustPrefix() {
                if (!plasmoid) {
                    return "";
                }
                var pre;
                switch (plasmoid.location) {
                case PlasmaCore.Types.LeftEdge:
                    pre = "west";
                    break;
                case PlasmaCore.Types.TopEdge:
                    pre = "north";
                    break;
                case PlasmaCore.Types.RightEdge:
                    pre = "east";
                    break;
                case PlasmaCore.Types.BottomEdge:
                    pre = "south";
                    break;
                default:
                    prefix = "";
                }

                prefix = [pre, ""];
            }
        }

        Colorizer.CustomBackground {
            id: overlayedBackground
            anchors.fill: solidBackground

            readonly property bool busyBackground: root.forcePanelForBusyBackground
                                                   && (solidBackground.opacity === 0 || !solidBackground.paintInstantly)
            readonly property bool coloredView: colorizerManager.mustBeShown && colorizerManager.applyTheme !== theme

            opacity: {
                if (busyBackground && !forceSolidness) {
                    return plasmoid.configuration.panelTransparency / 100;
                }

                if (coloredView) {
                    return midOpacity;
                }

                return 0;
            }

            backgroundColor: colorizerManager.backgroundColor

            borderWidth: 1
            borderColor: backgroundColor

            roundness: {
                if (themeExtended) {
                    switch(plasmoid.location) {
                    case PlasmaCore.Types.BottomEdge: return themeExtended.bottomEdgeRoundness;
                    case PlasmaCore.Types.LeftEdge: return themeExtended.leftEdgeRoundness;
                    case PlasmaCore.Types.TopEdge: return themeExtended.topEdgeRoundness;
                    case PlasmaCore.Types.RightEdge: return themeExtended.rightEdgeRoundness;
                    default: return 0;
                    }
                }

                return 0;
            }

            property real midOpacity: {
                if (forceSolidness) {
                    return 1;
                } else if (!root.userShowPanelBackground || root.forcePanelForBusyBackground || root.forceTransparentPanel) {
                    return 0;
                } else {
                    return plasmoid.configuration.panelTransparency / 100;
                }
            }

            readonly property bool forceSolidness: root.forceSolidPanel || !Latte.WindowSystem.compositingActive


            Behavior on opacity{
                enabled: Latte.WindowSystem.compositingActive
                NumberAnimation { duration: barLine.animationTime }
            }

            Behavior on opacity{
                enabled: !Latte.WindowSystem.compositingActive
                NumberAnimation { duration: 0 }
            }
                        
            Behavior on backgroundColor{
                enabled: Latte.WindowSystem.compositingActive
                ColorAnimation { duration: barLine.animationTime }
            }

            Behavior on backgroundColor{
                enabled: !Latte.WindowSystem.compositingActive
                ColorAnimation { duration: 0 }
            }
        }

        //! Outline drawing
        Loader{
            anchors.fill: solidBackground
            active: root.panelOutline && !(root.hasExpandedApplet && root.plasmaBackgroundForPopups)
            sourceComponent: Colorizer.CustomBackground{
                backgroundColor: "transparent"
                borderColor: colorizerManager.outlineColor
                borderWidth: themeExtended ? themeExtended.outlineWidth : 1
                roundness: overlayedBackground.roundness
            }
        }

        //! CustomBackground debugger
        /*Colorizer.CustomBackground {
            anchors.fill: solidBackground
            backgroundColor: "transparent"
            borderWidth: 1
            borderColor: "red"
            roundness: overlayedBackground.roundness
        }*/
    }

    transitions: Transition {
        enabled: editModeVisual.plasmaEditMode
        AnchorAnimation {
            duration: 0.8 * root.animationTime
            easing.type: Easing.OutCubic
        }
    }

    //BEGIN states
    //user set Panel Positions
    //0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left
        State {
            name: "leftCenterOrJustify"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Types.Center || root.panelAlignment === Latte.Types.Justify)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: barLine.screenEdgeMargin;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: (root.panelAlignment === Latte.Types.Center ? root.offset : 0);
            }
        },
        ///Left
        State {
            name: "leftTop"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Types.Top)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: barLine.screenEdgeMargin;    anchors.rightMargin:0;     anchors.topMargin:root.offset;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Left
        State {
            name: "leftBottom"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Types.Bottom)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: barLine.screenEdgeMargin;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:root.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Right
        State {
            name: "rightCenterOrJustify"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Types.Center || root.panelAlignment === Latte.Types.Justify)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin: barLine.screenEdgeMargin;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: (root.panelAlignment === Latte.Types.Center ? root.offset : 0);
            }
        },
        State {
            name: "rightTop"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Types.Top)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin: barLine.screenEdgeMargin;     anchors.topMargin:root.offset;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "rightBottom"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Types.Bottom)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined }
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin: barLine.screenEdgeMargin;     anchors.topMargin:0;    anchors.bottomMargin:root.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Bottom
        State {
            name: "bottomCenterOrJustify"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Types.Center || root.panelAlignment === Latte.Types.Justify)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin: barLine.screenEdgeMargin;
                anchors.horizontalCenterOffset: (root.panelAlignment === Latte.Types.Center ? root.offset : 0); anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottomLeft"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)
                  &&(((root.panelAlignment === Latte.Types.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     || ((root.panelAlignment === Latte.Types.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: root.offset;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin: barLine.screenEdgeMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }

        },
        State {
            name: "bottomRight"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)
                  &&(((root.panelAlignment === Latte.Types.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     ||((root.panelAlignment === Latte.Types.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin:root.offset;     anchors.topMargin:0;    anchors.bottomMargin: barLine.screenEdgeMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Top
        State {
            name: "topCenterOrJustify"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Types.Center || root.panelAlignment === Latte.Types.Justify)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin: barLine.screenEdgeMargin;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: (root.panelAlignment === Latte.Types.Center ? root.offset : 0); anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topLeft"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)
                  &&(((root.panelAlignment === Latte.Types.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     || ((root.panelAlignment === Latte.Types.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: root.offset;    anchors.rightMargin:0;     anchors.topMargin: barLine.screenEdgeMargin;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topRight"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)
                  &&(((root.panelAlignment === Latte.Types.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     ||((root.panelAlignment === Latte.Types.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin:root.offset;     anchors.topMargin: barLine.screenEdgeMargin;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        }
    ]
    //END states
}


