/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

    readonly property alias panelBackgroundSvg: solidBackground

    //! Layer 0: Multi-Layer container in order to provide a consistent final element that acts
    //! as a single entity/background
    width: root.isHorizontal ? totals.visualLength : 16
    height: root.isVertical ? totals.visualLength : 16

    opacity: root.useThemePanel ? 1 : 0
    currentOpacity: overlayedBackground.backgroundOpacity>0 ? overlayedBackground.backgroundOpacity : solidBackground.opacity

    isShown: (solidBackground.opacity > 0) || (overlayedBackground.backgroundOpacity > 0)

    hasAllBorders: solidBackground.enabledBorders === PlasmaCore.FrameSvg.AllBorders
    hasLeftBorder: hasAllBorders || ((solidBackground.enabledBorders & PlasmaCore.FrameSvg.LeftBorder) > 0)
    hasRightBorder: hasAllBorders || ((solidBackground.enabledBorders & PlasmaCore.FrameSvg.RightBorder) > 0)
    hasTopBorder: hasAllBorders || ((solidBackground.enabledBorders & PlasmaCore.FrameSvg.TopBorder) > 0)
    hasBottomBorder: hasAllBorders || ((solidBackground.enabledBorders & PlasmaCore.FrameSvg.BottomBorder) > 0)

    shadows.left: hasLeftBorder && root.behaveAsDockWithMask ? (customShadowIsEnabled ? customShadow : shadowsSvgItem.margins.left) : 0
    shadows.right: hasRightBorder && root.behaveAsDockWithMask ? (customShadowIsEnabled ? customShadow : shadowsSvgItem.margins.right) : 0
    shadows.top: hasTopBorder && root.behaveAsDockWithMask ? (customShadowIsEnabled ? customShadow : shadowsSvgItem.margins.top) : 0
    shadows.bottom: hasBottomBorder && root.behaveAsDockWithMask ? (customShadowIsEnabled ? customShadow : shadowsSvgItem.margins.bottom) : 0

    shadows.fixedLeft: (customDefShadowIsEnabled || customUserShadowIsEnabled) ? customShadow : shadowsSvgItem.fixedMargins.left
    shadows.fixedRight: (customDefShadowIsEnabled || customUserShadowIsEnabled) ? customShadow : shadowsSvgItem.fixedMargins.right
    shadows.fixedTop: (customDefShadowIsEnabled || customUserShadowIsEnabled) ? customShadow : shadowsSvgItem.fixedMargins.top
    shadows.fixedBottom: (customDefShadowIsEnabled || customUserShadowIsEnabled) ? customShadow : shadowsSvgItem.fixedMargins.bottom

    //! it can accept negative values in DockMode
    screenEdgeMargin: root.screenEdgeMarginEnabled ? metrics.margin.screenEdge - shadows.tailThickness : -shadows.tailThickness

    paddings.top: {
        if (hasTopBorder) {
            var customAppliedRadius = customRadiusIsEnabled ? customRadius : 0;
            var themePadding = themeExtendedBackground ? themeExtendedBackground.paddingTop : 0;
            var solidBackgroundPadding = solidBackground.margins.top;

            if (root.isVertical) {
                var expected = customRadiusIsEnabled ? customAppliedRadius : Math.max(themePadding, solidBackgroundPadding);
                expected = Math.max(0, expected - metrics.margin.length); //! remove from roundness padding the applied margins
                return expected * indicators.info.backgroundCornerMargin;
            } else {
                return Math.max(themePadding, solidBackgroundPadding);
            }
        }

        return 0;
    }
    paddings.bottom: {
        if (hasBottomBorder) {
            var customAppliedRadius = customRadiusIsEnabled ? customRadius : 0;
            var themePadding = themeExtendedBackground ? themeExtendedBackground.paddingBottom : 0;
            var solidBackgroundPadding = solidBackground.margins.bottom;

            if (root.isVertical) {
                var expected = customRadiusIsEnabled ? customAppliedRadius : Math.max(themePadding, solidBackgroundPadding);
                expected = Math.max(0, expected - metrics.margin.length); //! remove from roundness padding the applied margins
                return expected * indicators.info.backgroundCornerMargin;
            } else {
                return Math.max(themePadding, solidBackgroundPadding);
            }
        }

        return 0;
    }

    paddings.left: {
        if (hasLeftBorder) {
            var customAppliedRadius = customRadiusIsEnabled ? customRadius : 0;
            var themePadding = themeExtendedBackground ? themeExtendedBackground.paddingLeft : 0;
            var solidBackgroundPadding = solidBackground.margins.left;

            if (root.isHorizontal) {
                var expected = customRadiusIsEnabled ? customAppliedRadius : Math.max(themePadding, solidBackgroundPadding);
                expected = Math.max(0, expected - metrics.margin.length); //! remove from roundness padding the applied margins
                return expected * indicators.info.backgroundCornerMargin;
            } else {
                return Math.max(themePadding, solidBackgroundPadding);
            }
        }

        return 0;
    }

    paddings.right: {
        if (hasRightBorder) {
            var customAppliedRadius = customRadiusIsEnabled? customRadius : 0;
            var themePadding = themeExtendedBackground ? themeExtendedBackground.paddingRight : 0;
            var solidBackgroundPadding = solidBackground.margins.right;

            if (root.isHorizontal) {
                var expected = customRadiusIsEnabled ? customAppliedRadius : Math.max(themePadding, solidBackgroundPadding);
                expected = Math.max(0, expected - metrics.margin.length); //! remove from roundness padding the applied margins
                return expected * indicators.info.backgroundCornerMargin;
            } else {
                return Math.max(themePadding, solidBackgroundPadding);
            }
        }

        return 0;
    }

    length: {
        if (root.behaveAsPlasmaPanel && LatteCore.WindowSystem.compositingActive) {
            return root.isVertical ? root.height : root.width;
        }

        if (myView.alignment === LatteCore.Types.Justify) {
            return root.maxLength;
        }

        return Math.max(root.minLength, layoutsContainerItem.mainLayout.length + totals.paddingsLength);
    }

    thickness: {
        if (root.behaveAsPlasmaPanel) {
            return metrics.totals.thickness;
        } else {
            return Math.min(metrics.totals.thickness, background.totals.visualThickness);
        }
    }

    offset: {
        if (behaveAsPlasmaPanel) {
            return 0;
        }

        if (root.isHorizontal) {
            if (myView.alignment === LatteCore.Types.Left) {
                return root.offset - shadows.left;
            } else if (myView.alignment === LatteCore.Types.Right) {
                return root.offset - shadows.right;
            }
        }

        if (root.isVertical) {
            if (myView.alignment === LatteCore.Types.Top) {
                return root.offset - shadows.top;
            } else if (myView.alignment === LatteCore.Types.Bottom) {
                return root.offset - shadows.bottom;
            }
        }

        var parabolicOffseting = myView.alignment === LatteCore.Types.Center ? layoutsContainerItem.mainLayout.parabolicOffsetting : 0;
        return root.offset + parabolicOffseting;
    }

    totals.visualThickness: {
        var itemMargins = 2*metrics.margin.tailThickness;
        var maximumItem = metrics.iconSize + itemMargins;

        if (totals.minThickness < maximumItem) {
            maximumItem = maximumItem - totals.minThickness;
        }

        var percentage = LatteCore.WindowSystem.compositingActive ? plasmoid.configuration.panelSize/100 : 1;
        return Math.max(totals.minThickness, totals.minThickness + (percentage*maximumItem));
    }

    totals.visualMaxThickness: {
        var itemMargins = 2*metrics.margin.maxTailThickness;
        var maximumItem = metrics.maxIconSize + itemMargins;

        if (totals.minThickness < maximumItem) {
            maximumItem = maximumItem - totals.minThickness;
        }

        var percentage = LatteCore.WindowSystem.compositingActive ? plasmoid.configuration.panelSize/100 : 1;
        return Math.max(totals.minThickness, totals.minThickness + (percentage*maximumItem));
    }

    totals.visualLength: {
        if (root.behaveAsPlasmaPanel) {
            return root.isVertical ? root.height : root.width;
        }

        return Math.max(background.length + totals.shadowsLength, totals.paddingsLength + totals.shadowsLength)
    }

    readonly property int tailRoundness: {
        if ((root.isHorizontal && hasLeftBorder) || (!root.isHorizontal && hasTopBorder)) {
            var customAppliedRadius = customRadiusIsEnabled ? customRadius : 0;
            var themePadding = themeExtendedBackground ? (root.isHorizontal ? themeExtendedBackground.paddingLeft : themeExtendedBackground.paddingTop) : 0;
            var solidBackgroundPadding = root.isHorizontal ? solidBackground.margins.left : solidBackground.margins.top;
            var expected = customRadiusIsEnabled ? customAppliedRadius : Math.max(themePadding, solidBackgroundPadding);
            return Math.max(0, expected - metrics.margin.length);
        }

        return 0;
    }

    readonly property int headRoundness: {
        if ((root.isHorizontal && hasRightBorder) || (!root.isHorizontal && hasBottomBorder)) {
            var customAppliedRadius = customRadiusIsEnabled ? customRadius : 0;
            var themePadding = themeExtendedBackground ? (root.isHorizontal ? themeExtendedBackground.paddingRight : themeExtendedBackground.paddingBottom) : 0;
            var solidBackgroundPadding = root.isHorizontal ? solidBackground.margins.right : solidBackground.margins.bottom;
            var expected = customRadiusIsEnabled ? customAppliedRadius : Math.max(themePadding, solidBackgroundPadding);
            return Math.max(0, expected - metrics.margin.length);
        }

        return 0;
    }

    readonly property int tailRoundnessMargin: {
        //! used from contents geometry in order to remove any roundness sectors, e.g. for popups placement
        if (root.isHorizontal) {
            return paddings.left > metrics.margin.length ? metrics.margin.length : 0
        } else {
            return paddings.top > metrics.margin.length ? metrics.margin.length : 0
        }
    }

    readonly property int headRoundnessMargin: {
        //! used from contents geometry in order to remove any roundness sectors, e.g. for popups placement
        if (root.isHorizontal) {
            return paddings.right > metrics.margin.length ? metrics.margin.length : 0
        } else {
            return paddings.bottom > metrics.margin.length ? metrics.margin.length : 0
        }
    }

    property int animationTime: 6*animations.speedFactor.current*animations.duration.small

    //! Opacity related
    readonly property bool isDefaultOpacityEnabled: plasmoid.configuration.panelTransparency===-1

    //! Metrics related
    readonly property bool isGreaterThanItemThickness: root.useThemePanel && (totals.visualThickness >= (metrics.iconSize + metrics.margin.tailThickness))

    //! CustomShadowedRectangle  properties
    readonly property bool customShadowedRectangleIsEnabled: customRadiusIsEnabled || (customDefShadowIsEnabled || customUserShadowIsEnabled)

    readonly property bool customShadowIsSupported: LatteCore.WindowSystem.compositingActive
                                                    && kirigamiLibraryIsFound

    //!current shadow state but do not change other values of normal mode, for example if a Dock hides its screen edge thickness
    //!shouldnt change the fact that customShadowedRectangle is still used
    readonly property bool customShadowIsEnabled: (customDefShadowIsEnabled || customUserShadowIsEnabled) && panelShadowsActive
    readonly property bool customDefShadowIsEnabled: customShadowIsSupported && !customUserShadowIsEnabled && customRadiusIsEnabled
    readonly property bool customUserShadowIsEnabled: customShadowIsSupported && plasmoid.configuration.backgroundShadowSize >= 0

    readonly property bool customRadiusIsEnabled: kirigamiLibraryIsFound && plasmoid.configuration.backgroundRadius >= 0

    readonly property int customRadius: {
        if (customDefShadowIsEnabled && !customRadiusIsEnabled && themeExtendedBackground) {
            return themeExtendedBackground.roundness;
        }

        return plasmoid.formFactor === PlasmaCore.Types.Horizontal ?
                    (plasmoid.configuration.backgroundRadius/100) * solidBackground.height :
                    (plasmoid.configuration.backgroundRadius/100) * solidBackground.width
    }
    readonly property int customShadow: {
        if (customDefShadowIsEnabled && themeExtendedBackground) {
            return themeExtendedBackground.shadowSize;
        }

        return plasmoid.configuration.backgroundShadowSize;
    }

    readonly property color customShadowColor: themeExtendedBackground ? themeExtendedBackground.shadowColor : "black"

    property QtObject themeExtendedBackground: null

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

    Binding {
        target: barLine
        property: "themeExtendedBackground"
        when: themeExtended
        value: {
            switch(plasmoid.location) {
            case PlasmaCore.Types.BottomEdge: return themeExtended.backgroundBottomEdge;
            case PlasmaCore.Types.LeftEdge: return themeExtended.backgroundLeftEdge;
            case PlasmaCore.Types.TopEdge: return themeExtended.backgroundTopEdge;
            case PlasmaCore.Types.RightEdge: return themeExtended.backgroundRightEdge;
            default: return null;
            }
        }
    }

    onXChanged: solidBackground.updateEffectsArea();
    onYChanged: solidBackground.updateEffectsArea();
    onScreenEdgeMarginChanged: solidBackground.updateEffectsArea();

    //! Layer 1: Shadows that are drawn around the background but always inside the View window (these are internal drawn shadows).
    //!          When the container has chosen external shadows (these are shadows that are drawn out of the View window from the compositor)
    //!          in such case the internal drawn shadows are NOT drawn at all.
    PlasmaCore.FrameSvgItem{
        id: shadowsSvgItem
        width: root.isVertical ?  background.thickness + totals.shadowsThickness : totals.visualLength
        height: root.isVertical ? totals.visualLength : background.thickness + totals.shadowsThickness
        enabledBorders: latteView && latteView.effects ? latteView.effects.enabledBorders : PlasmaCore.FrameSvg.NoBorder
        imagePath: "widgets/panel-background"
        prefix: "shadow"
        opacity: hideShadow || !root.useThemePanel || (root.forceTransparentPanel && !root.forcePanelForBusyBackground) ? 0 : 1
        visible: (opacity == 0) ? false : true

        //! set true by default in order to avoid crash on startup because imagePath is set to ""
        readonly property bool themeHasShadow: themeExtended ? themeExtended.hasShadow : true

        readonly property bool hideShadow: root.behaveAsPlasmaPanel
                                           || !LatteCore.WindowSystem.compositingActive
                                           || !root.panelShadowsActive
                                           || !themeHasShadow
                                           || customShadowedRectangleIsEnabled

        Behavior on opacity {
            enabled: LatteCore.WindowSystem.compositingActive
            NumberAnimation { duration: barLine.animationTime }
        }


        Behavior on opacity{
            enabled: !LatteCore.WindowSystem.compositingActive
            NumberAnimation { duration: 0 }
        }
    }

    //! Layer 2: Provide visual solidness. Plasma themes by design may provide a panel-background svg that is not
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

    //! Layer 3: Original Plasma Theme "panel-background" svg. It is used for calculations and also to draw
    //!          the original background when to special settings and options exist from the user. It is also
    //!          doing one very important job which is to calculate the Effects Rectangle which is used from
    //!          the compositor to provide blurriness and from Mask calculations to provide the View Local Geometry
    PlasmaCore.FrameSvgItem{
        id: solidBackground
        anchors.leftMargin: shadows.left
        anchors.rightMargin: shadows.right
        anchors.topMargin: shadows.top
        anchors.bottomMargin: shadows.bottom
        anchors.fill: shadowsSvgItem

        imagePath: "widgets/panel-background"
        opacity: normalizedOpacity

        readonly property bool exceedsThemeOpacityLimits: appliedOpacity > themeMaxOpacity
        readonly property bool forceSolidness: root.forceSolidPanel || !LatteCore.WindowSystem.compositingActive

        //! must be normalized to plasma theme maximum opacity
        readonly property real normalizedOpacity: Math.min(1, appliedOpacity / themeMaxOpacity)

        readonly property real appliedOpacity: overlayedBackground.backgroundOpacity > 0 && !paintInstantly ? 0 : overlayedBackground.midOpacity
        readonly property real themeMaxOpacity: themeExtendedBackground ? themeExtendedBackground.maxOpacity : 1

        //! When switching from overlaied background to regular one this must be done
        //! instantly otherwise the transition is not smooth
        readonly property bool paintInstantly: (root.hasExpandedApplet && root.plasmaBackgroundForPopups && !customRadiusIsEnabled)

        property rect efGeometry: Qt.rect(-1,-1,0,0)

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
               // invUpdateEffectsArea(); // disabled in order to force Timer at all cases
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
                    if (!root.behaveAsPlasmaPanel) {
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
        }

        Timer {
            id: updateEffectsAreaTimer
            interval: 11 //! 90Hz or 90calls/sec
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

    //! Layer 4: Plasma theme design does not provide a way to colorize the background. This layer
    //!          solves this by providing a custom background layer that respects the Colorizer palette
    Colorizer.CustomBackground {
        id: overlayedBackground
        anchors.fill: solidBackground

        readonly property bool busyBackground: root.forcePanelForBusyBackground
                                               && (solidBackground.opacity === 0 || !solidBackground.paintInstantly)
        readonly property bool coloredView: colorizerManager.mustBeShown && colorizerManager.applyTheme !== theme

        backgroundOpacity: {
            if (busyBackground && !forceSolidness) {
                return root.myView.backgroundStoredOpacity;
            }

            if (coloredView || customShadowedRectangleIsEnabled) {
                return midOpacity;
            }

            return 0;
        }

        backgroundColor: colorizerManager.backgroundColor
        shadowColor: customShadowColor
        shadowSize: customShadowIsEnabled ? customShadow : 0

        roundness: {
            if (customRadiusIsEnabled) {
                return customRadius;
            }

            return themeExtendedBackground ? themeExtendedBackground.roundness : 0
        }

        property real midOpacity: {
            if (forceSolidness) {
                return 1;
            } else if (!root.userShowPanelBackground || root.forcePanelForBusyBackground || root.forceTransparentPanel) {
                return 0;
            } else {
                return root.myView.backgroundStoredOpacity;
            }
        }

        readonly property bool forceSolidness: root.forceSolidPanel || !LatteCore.WindowSystem.compositingActive

        Behavior on backgroundOpacity{
            enabled: LatteCore.WindowSystem.compositingActive
            NumberAnimation { duration: barLine.animationTime }
        }

        Behavior on backgroundOpacity{
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

    //! Layer 5: Plasma theme design does not provide a way to draw background outline on demand. This layer
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


    //BEGIN states
    //user set Panel Positions
    //0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left
        State {
            name: "leftCenter"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(myView.alignment === LatteCore.Types.Center)

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
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(myView.alignment === LatteCore.Types.Justify)

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
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(myView.alignment === LatteCore.Types.Top)

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
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(myView.alignment === LatteCore.Types.Bottom)

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
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(myView.alignment === LatteCore.Types.Center)

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
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(myView.alignment === LatteCore.Types.Justify)

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
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(myView.alignment === LatteCore.Types.Top)

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
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(myView.alignment === LatteCore.Types.Bottom)

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
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(myView.alignment === LatteCore.Types.Center)

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
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(myView.alignment === LatteCore.Types.Justify)

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
                  &&(((myView.alignment === LatteCore.Types.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     || ((myView.alignment === LatteCore.Types.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                  &&(((myView.alignment === LatteCore.Types.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     ||((myView.alignment === LatteCore.Types.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(myView.alignment === LatteCore.Types.Center)

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
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(myView.alignment === LatteCore.Types.Justify)

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
                  &&(((myView.alignment === LatteCore.Types.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     || ((myView.alignment === LatteCore.Types.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                  &&(((myView.alignment === LatteCore.Types.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     ||((myView.alignment === LatteCore.Types.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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


