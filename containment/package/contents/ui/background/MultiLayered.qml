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

import org.kde.latte.core 0.2 as LatteCore

import "../colorizer" as Colorizer

BackgroundProperties{
    id:barLine

    //! Layer 0: Multi-Layer container in order to provide a consistent final element that acts
    //! as a single entity/background
    width: root.isHorizontal ? totals.visualLength : 16
    height: root.isVertical ? totals.visualLength : 16

    opacity: root.useThemePanel ? 1 : 0
    currentOpacity: overlayedBackground.opacity>0 ? overlayedBackground.opacity : solidBackground.opacity

    isShown: (solidBackground.opacity > 0) || (overlayedBackground.opacity > 0)

    hasAllBorders: solidBackground.enabledBorders === PlasmaCore.FrameSvg.AllBorders
    hasLeftBorder: hasAllBorders || ((solidBackground.enabledBorders & PlasmaCore.FrameSvg.LeftBorder) > 0)
    hasRightBorder: hasAllBorders || ((solidBackground.enabledBorders & PlasmaCore.FrameSvg.RightBorder) > 0)
    hasTopBorder: hasAllBorders || ((solidBackground.enabledBorders & PlasmaCore.FrameSvg.TopBorder) > 0)
    hasBottomBorder: hasAllBorders || ((solidBackground.enabledBorders & PlasmaCore.FrameSvg.BottomBorder) > 0)

    shadows.left: hasLeftBorder ? shadowsSvgItem.margins.left : 0
    shadows.right: hasRightBorder ? shadowsSvgItem.margins.right : 0
    shadows.top: hasTopBorder ? shadowsSvgItem.margins.top : 0
    shadows.bottom: hasBottomBorder ? shadowsSvgItem.margins.bottom : 0

    //! it can accept negative values in DockMode
    screenEdgeMargin: root.screenEdgeMarginEnabled ? metrics.margin.screenEdge - shadows.tailThickness : 0

    paddings.top: {
        if (root.isHorizontal && hasTopBorder) {
            var minimum = root.screenEdgeMarginEnabled && themeExtended ? themeExtended.topEdgeRoundness : 0;
            return Math.max(minimum, solidBackground.margins.top);
        }

        return 0;
    }
    paddings.bottom: {
        if (root.isHorizontal && hasBottomBorder) {
            var minimum = root.screenEdgeMarginEnabled && themeExtended ? themeExtended.bottomEdgeRoundness : 0;
            return Math.max(minimum, solidBackground.margins.bottom);
        }

        return 0;
    }

    paddings.left: {
        if (root.isVertical && hasLeftBorder) {
            var minimum = root.screenEdgeMarginEnabled && themeExtended ? themeExtended.leftEdgeRoundness : 0;
            return Math.max(minimum, solidBackground.margins.left);
        }

        return 0;
    }

    paddings.right: {
        if (root.isVertical && hasRightBorder) {
            var minimum = root.screenEdgeMarginEnabled && themeExtended ? themeExtended.rightEdgeRoundness : 0;
            return Math.max(minimum, solidBackground.margins.right);
        }

        return 0;
    }

    length: {
        if (root.behaveAsPlasmaPanel && LatteCore.WindowSystem.compositingActive && !root.editMode) {
            return root.isVertical ? root.height : root.width;
        }

        if (root.panelAlignment === LatteCore.Types.Justify) {
            return root.maxLength;
        }

        if (root.isVertical) {
            return Math.max(root.minLength, layoutsContainerItem.mainLayout.height);
        } else {
            return Math.max(root.minLength, layoutsContainerItem.mainLayout.width);
        }
    }

    thickness: {
        if (root.behaveAsPlasmaPanel) {
            return metrics.totals.thickness;
        } else {
            return Math.min(metrics.totals.thickness, background.totals.visualThickness);
        }
    }

    offset: {
        if (behaveAsPlasmaPanel || !LatteCore.WindowSystem.compositingActive) {
            return 0;
        }

        if (root.isHorizontal) {
            if (root.panelAlignment === LatteCore.Types.Left) {
                return root.offset - shadows.left;
            } else if (root.panelAlignment === LatteCore.Types.Right) {
                return root.offset - shadows.right;
            }
        }

        if (root.isVertical) {
            if (root.panelAlignment === LatteCore.Types.Top) {
                return root.offset - shadows.top;
            } else if (root.panelAlignment === LatteCore.Types.Bottom) {
                return root.offset - shadows.bottom;
            }
        }

        return root.offset;// (root.panelAlignment === LatteCore.Types.Center ? root.offset : 0);
    }

    totals.visualThickness: {
        var itemMargins = root.shrinkThickMargins ? 0 : metrics.totals.thicknessEdges;
        var maximumItem = metrics.iconSize + itemMargins;

        if (totals.minThickness < maximumItem) {
            maximumItem = maximumItem - totals.minThickness;
        }

        var percentage = LatteCore.WindowSystem.compositingActive ? plasmoid.configuration.panelSize/100 : 1;
        return Math.max(totals.minThickness, totals.minThickness + (percentage*maximumItem));
    }

    totals.visualLength: Math.max(background.length + totals.shadowsLength, totals.paddingsLength + totals.shadowsLength)

    property int animationTime: 6*animations.speedFactor.current*animations.duration.small


    Behavior on opacity{
        enabled: LatteCore.WindowSystem.compositingActive
        NumberAnimation {
            duration: barLine.animationTime
        }
    }

    Behavior on opacity{
        enabled: !LatteCore.WindowSystem.compositingActive
        NumberAnimation {
            duration: 0
        }
    }

    onXChanged: solidBackground.updateEffectsArea();
    onYChanged: solidBackground.updateEffectsArea();

    //! Layer 1: Shadows that are drawn around the background but always inside the View window (these are internal drawn shadows).
    //!          When the container has chosen external shadows (these are shadows that are drawn out of the View window from the compositor)
    //!          in such case the internal drawn shadows are NOT drawn at all.
    PlasmaCore.FrameSvgItem{
        id: shadowsSvgItem
        width: root.isVertical ?  background.thickness + totals.shadowsThickness : totals.visualLength
        height: root.isVertical ? totals.visualLength : background.thickness + totals.shadowsThickness

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
                                           || !LatteCore.WindowSystem.compositingActive
                                           || !root.panelShadowsActive
                                           || !themeHasShadow

        Behavior on opacity {
            enabled: LatteCore.WindowSystem.compositingActive
            NumberAnimation { duration: barLine.animationTime }
        }


        Behavior on opacity{
            enabled: !LatteCore.WindowSystem.compositingActive
            NumberAnimation { duration: 0 }
        }

        Behavior on opacity{
            enabled: LatteCore.WindowSystem.compositingActive
            NumberAnimation { duration: barLine.animationTime }
        }

        Behavior on opacity{
            enabled: !LatteCore.WindowSystem.compositingActive
            NumberAnimation { duration: 0 }
        }


        //! Layer 2: Draw fake blurness under background when the user is INEDITMODE state for visual feedback
        Loader {
            anchors.fill: solidBackground
            active: editModeVisual.inEditMode && root.userShowPanelBackground && plasmoid.configuration.blurEnabled
            sourceComponent: Item {
                Image{
                    id: backTiler
                    anchors.fill: parent
                    visible: false

                    fillMode: Image.Tile
                    source: hasBackground ? latteView.layout.background : "../../icons/"+latteView.layout.background+"print.jpg"

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

        //! Layer 3: Provide visual solidness. Plasma themes by design may provide a panel-background svg that is not
        //!          solid. That means that user can not gain full solidness in such cases. This layer is responsible
        //!          to solve the previous mentioned plasma theme limitation.
        Colorizer.CustomBackground {
            id: backgroundLowestRectangle
            anchors.fill: solidBackground
            opacity: normalizedOpacity
            backgroundColor: colorizerManager.backgroundColor
            roundness: overlayedBackground.roundness
            visible: LatteCore.WindowSystem.compositingActive && solidBackground.exceedsThemeOpacityLimits

            readonly property real normalizedOpacity: visible ?  Math.min(1, (appliedOpacity - solidBackground.themeMaxOpacity)/(1-solidBackground.themeMaxOpacity)) : 0
            readonly property real appliedOpacity: visible ? solidBackground.appliedOpacity : 0

            Behavior on opacity{
                enabled: LatteCore.WindowSystem.compositingActive
                NumberAnimation { duration: barLine.animationTime }
            }

            Behavior on opacity{
                enabled: !LatteCore.WindowSystem.compositingActive
                NumberAnimation { duration: 0 }
            }
        }

        //! Layer 4: Original Plasma Theme "panel-background" svg. It is used for calculations and also to draw
        //!          the original background when to special settings and options exist from the user. It is also
        //!          doing one very important job which is to calculate the Effects Rectangle which is used from
        //!          the compositor to provide blurriness and from Mask calculations to provide the View Local Geometry
        PlasmaCore.FrameSvgItem{
            id: solidBackground
            anchors.leftMargin: LatteCore.WindowSystem.compositingActive ? shadowsSvgItem.margins.left : 0
            anchors.rightMargin: LatteCore.WindowSystem.compositingActive ? shadowsSvgItem.margins.right : 0
            anchors.topMargin: LatteCore.WindowSystem.compositingActive ? shadowsSvgItem.margins.top : 0
            anchors.bottomMargin: LatteCore.WindowSystem.compositingActive ? shadowsSvgItem.margins.bottom : 0
            anchors.fill:parent

            opacity: normalizedOpacity

            readonly property bool exceedsThemeOpacityLimits: appliedOpacity > themeMaxOpacity
            readonly property bool forceSolidness: root.forceSolidPanel || !LatteCore.WindowSystem.compositingActive

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

            property int paddingsWidth: margins.left+margins.right
            property int paddingsHeight: margins.top + margins.bottom

            onWidthChanged: updateEffectsArea();
            onHeightChanged: updateEffectsArea();
            onImagePathChanged: solidBackground.adjustPrefix();


            Component.onCompleted: {
                root.updateEffectsArea.connect(updateEffectsArea);
                adjustPrefix();
            }

            Component.onDestruction: {
                root.updateEffectsArea.disconnect(updateEffectsArea);
            }

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

            function updateEffectsArea() {
                if (!updateEffectsAreaTimer.running) {
                    invUpdateEffectsArea();
                    updateEffectsAreaTimer.start();
                }
            }

            function invUpdateEffectsArea(){
                if (!latteView)
                    return;

                if (!LatteCore.WindowSystem.compositingActive) {
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

            Timer {
                id: updateEffectsAreaTimer
                interval: 16 //! 60Hz or 60calls/sec
                onTriggered: solidBackground.invUpdateEffectsArea();
            }

            onRepaintNeeded: {
                if (root.behaveAsPlasmaPanel)
                    adjustPrefix();
            }

            enabledBorders: latteView && latteView.effects ? latteView.effects.enabledBorders : PlasmaCore.FrameSvg.NoBorder

            Behavior on opacity{
                enabled: LatteCore.WindowSystem.compositingActive && !solidBackground.paintInstantly
                NumberAnimation { duration: barLine.animationTime }
            }

            Behavior on opacity{
                enabled: !LatteCore.WindowSystem.compositingActive
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

        //! Layer 5: Plasma theme design does not provide a way to colorize the background. This layer
        //!          solves this by providing a custom background layer that respects the Colorizer palette
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

            readonly property bool forceSolidness: root.forceSolidPanel || !LatteCore.WindowSystem.compositingActive

            Behavior on opacity{
                enabled: LatteCore.WindowSystem.compositingActive
                NumberAnimation { duration: barLine.animationTime }
            }

            Behavior on opacity{
                enabled: !LatteCore.WindowSystem.compositingActive
                NumberAnimation { duration: 0 }
            }

            Behavior on backgroundColor{
                enabled: LatteCore.WindowSystem.compositingActive
                ColorAnimation { duration: barLine.animationTime }
            }

            Behavior on backgroundColor{
                enabled: !LatteCore.WindowSystem.compositingActive
                ColorAnimation { duration: 0 }
            }
        }

        //! Layer 6: Plasma theme design does not provide a way to draw background outline on demand. This layer
        //!          solves this by providing a custom background layer that only draws an outline on top of all
        //!          previous layers
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

    //BEGIN states
    //user set Panel Positions
    //0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left
        State {
            name: "leftCenter"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === LatteCore.Types.Center)

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
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: background.offset;
            }
        },
        State {
            name: "leftJustify"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === LatteCore.Types.Justify)

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
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Left
        State {
            name: "leftTop"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === LatteCore.Types.Top)

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
                anchors.leftMargin: barLine.screenEdgeMargin;    anchors.rightMargin:0;     anchors.topMargin:background.offset;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Left
        State {
            name: "leftBottom"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === LatteCore.Types.Bottom)

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
                anchors.leftMargin: barLine.screenEdgeMargin;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:background.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Right
        State {
            name: "rightCenter"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === LatteCore.Types.Center)

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
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: background.offset;
            }
        },
        State {
            name: "rightJustify"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === LatteCore.Types.Justify)

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
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "rightTop"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === LatteCore.Types.Top)

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
                anchors.leftMargin: 0;    anchors.rightMargin: barLine.screenEdgeMargin;     anchors.topMargin:background.offset;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "rightBottom"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === LatteCore.Types.Bottom)

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
                anchors.leftMargin: 0;    anchors.rightMargin: barLine.screenEdgeMargin;     anchors.topMargin:0;    anchors.bottomMargin:background.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Bottom
        State {
            name: "bottomCenter"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === LatteCore.Types.Center)

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
                anchors.horizontalCenterOffset: background.offset; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottomJustify"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === LatteCore.Types.Justify)

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
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottomLeft"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)
                  &&(((root.panelAlignment === LatteCore.Types.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     || ((root.panelAlignment === LatteCore.Types.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                anchors.leftMargin: background.offset;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin: barLine.screenEdgeMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }

        },
        State {
            name: "bottomRight"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)
                  &&(((root.panelAlignment === LatteCore.Types.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     ||((root.panelAlignment === LatteCore.Types.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                anchors.leftMargin: 0;    anchors.rightMargin:background.offset;     anchors.topMargin:0;    anchors.bottomMargin: barLine.screenEdgeMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Top
        State {
            name: "topCenter"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === LatteCore.Types.Center)

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
                anchors.horizontalCenterOffset: background.offset; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topJustify"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === LatteCore.Types.Justify)

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
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topLeft"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)
                  &&(((root.panelAlignment === LatteCore.Types.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     || ((root.panelAlignment === LatteCore.Types.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                anchors.leftMargin: background.offset;    anchors.rightMargin:0;     anchors.topMargin: barLine.screenEdgeMargin;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topRight"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)
                  &&(((root.panelAlignment === LatteCore.Types.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     ||((root.panelAlignment === LatteCore.Types.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                anchors.leftMargin: 0;    anchors.rightMargin:background.offset;     anchors.topMargin: barLine.screenEdgeMargin;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        }
    ]
    //END states
}


