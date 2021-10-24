/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

LatteComponents.IndicatorItem{
    id: root
    extraMaskThickness: reversedEnabled && glowEnabled ? 1.7 * (factor * indicator.maxIconSize) : 0

    enabledForApplets: indicator && indicator.configuration ? indicator.configuration.enabledForApplets : true
    lengthPadding: indicator && indicator.configuration ? indicator.configuration.lengthPadding : 0.08
    backgroundCornerMargin: indicator && indicator.configuration ? indicator.configuration.backgroundCornerMargin : 1.00

    readonly property real factor: indicator.configuration.size
    readonly property int size: factor * indicator.currentIconSize
    readonly property int thickLocalMargin: indicator.configuration.thickMargin * indicator.currentIconSize

    readonly property int screenEdgeMargin: plasmoid.location === PlasmaCore.Types.Floating || reversedEnabled ? 0 : indicator.screenEdgeMargin

    readonly property int thicknessMargin: screenEdgeMargin + thickLocalMargin + (glowEnabled ? 1 : 0)

    property real textColorBrightness: colorBrightness(indicator.palette.textColor)

    property color isActiveColor: indicator.palette.buttonFocusColor
    property color minimizedColor: {
        if (minimizedTaskColoredDifferently) {
            return (textColorBrightness > 127.5 ? Qt.darker(indicator.palette.textColor, 1.7) : Qt.lighter(indicator.palette.textColor, 7));
        }

        return isActiveColor;
    }

    property color notActiveColor: indicator.isMinimized ? minimizedColor : isActiveColor

    //! Common Options
    readonly property bool reversedEnabled: indicator.configuration.reversed

    //! Configuration Options
    readonly property bool extraDotOnActive: indicator.configuration.extraDotOnActive
    readonly property bool minimizedTaskColoredDifferently: indicator.configuration.minimizedTaskColoredDifferently
    readonly property int activeStyle: indicator.configuration.activeStyle
    //!glow options
    readonly property bool glowEnabled: indicator.configuration.glowEnabled
    readonly property bool glow3D: indicator.configuration.glow3D
    readonly property int glowApplyTo: indicator.configuration.glowApplyTo
    readonly property real glowOpacity: indicator.configuration.glowOpacity
    readonly property int glowMargins: glowEnabled ? 12 : 0

    /*Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "blue"
        color: "transparent"
    }*/

    function colorBrightness(color) {
        return colorBrightnessFromRGB(color.r * 255, color.g * 255, color.b * 255);
    }

    // formula for brightness according to:
    // https://www.w3.org/TR/AERT/#color-contrast
    function colorBrightnessFromRGB(r, g, b) {
        return (r * 299 + g * 587 + b * 114) / 1000
    }

    Grid{
        id: grid
        columns: plasmoid.formFactor === PlasmaCore.Types.Vertical ? 1 : 0
        rows: plasmoid.formFactor !== PlasmaCore.Types.Vertical ? 1 : 0
        columnSpacing: 0
        rowSpacing: 0

        LatteComponents.GlowPoint{
            id:firstPoint
            width: stateWidth
            height: stateHeight
            opacity: {
                if (indicator.isEmptySpace) {
                    return 0;
                }

                if (indicator.isTask) {
                    return indicator.isLauncher || (indicator.inRemoving && !isAnimating) ? 0 : 1
                }

                if (indicator.isApplet) {
                    return (indicator.isActive || isAnimating) ? 1 : 0
                }
            }

            basicColor: indicator.isActive || (indicator.isGroup && indicator.hasShown) ? root.isActiveColor : root.notActiveColor

            size: root.size
            glow3D: glow3D
            animation: Math.max(1.65*3*LatteCore.Environment.longDuration,indicator.durationTime*3*LatteCore.Environment.longDuration)
            location: plasmoid.location
            glowOpacity: root.glowOpacity
            contrastColor: indicator.shadowColor
            attentionColor: indicator.palette.negativeTextColor

            roundCorners: true
            showAttention: indicator.inAttention
            showGlow: {
                if (glowEnabled && (glowApplyTo === 2 /*All*/ || showAttention ))
                    return true;
                else if (glowEnabled && glowApplyTo === 1 /*OnActive*/ && indicator.hasActive)
                    return true;
                else
                    return false;
            }
            showBorder: glow3D

            property int stateWidth: {
                if (!vertical && isActive && activeStyle === 0 /*Line*/) {
                    return (indicator.isGroup ? root.width - secondPoint.width : root.width - spacer.width) - glowMargins;
                }

                return root.size;
            }

            property int stateHeight: {
                if (vertical && isActive && activeStyle === 0 /*Line*/) {
                    return (indicator.isGroup ? root.height - secondPoint.height : root.height - spacer.height) - glowMargins;
                }

                return root.size;
            }

            property int animationTime: indicator.durationTime* (0.75*LatteCore.Environment.longDuration)

            property bool isActive: indicator.hasActive || indicator.isActive

            property bool vertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

            property real scaleFactor: indicator.scaleFactor

            readonly property bool isAnimating: inGrowAnimation || inShrinkAnimation
            property bool inGrowAnimation: false
            property bool inShrinkAnimation: false

            property bool isBindingBlocked: isAnimating

            readonly property bool isActiveStateForAnimation: indicator.isActive && root.activeStyle === 0 /*Line*/

            onIsActiveStateForAnimationChanged: {
                if (root.activeStyle === 0 /*Line*/) {
                    if (isActiveStateForAnimation) {
                        inGrowAnimation = true;
                        inShrinkAnimation = false;
                    } else {
                        inGrowAnimation = false;
                        inShrinkAnimation = true;
                    }
                } else {
                    inGrowAnimation = false;
                    inShrinkAnimation = false;
                }
            }

            onWidthChanged: {
                if (!vertical) {
                    if (inGrowAnimation && width >= stateWidth) {
                        inGrowAnimation = false;
                    } else if (inShrinkAnimation && width <= stateWidth) {
                        inShrinkAnimation = false;
                    }
                }
            }

            onHeightChanged: {
                if (vertical) {
                    if (inGrowAnimation && height >= stateHeight) {
                        inGrowAnimation = false;
                    } else if (inShrinkAnimation && height <= stateHeight) {
                        inShrinkAnimation = false;
                    }
                }
            }

            Behavior on width {
                enabled: (!firstPoint.vertical && (firstPoint.isAnimating || firstPoint.opacity === 0/*first showing requirement*/))
                NumberAnimation {
                    duration: firstPoint.animationTime
                    easing.type: Easing.Linear
                }
            }

            Behavior on height {
                enabled: (firstPoint.vertical && (firstPoint.isAnimating || firstPoint.opacity === 0/*first showing requirement*/))
                NumberAnimation {
                    duration: firstPoint.animationTime
                    easing.type: Easing.Linear
                }
            }
        }

        Item{
            id:spacer
            width: secondPoint.visible ? 0.5*root.size : 0
            height: secondPoint.visible ? 0.5*root.size : 0
        }

        LatteComponents.GlowPoint{
            id:secondPoint
            width: visible ? root.size : 0
            height: width

            size: root.size
            glow3D: glow3D
            animation: Math.max(1.65*3*LatteCore.Environment.longDuration,indicator.durationTime*3*LatteCore.Environment.longDuration)
            location: plasmoid.location
            glowOpacity: root.glowOpacity
            contrastColor: indicator.shadowColor
            showBorder: glow3D

            basicColor: state2Color
            roundCorners: true
            showGlow: glowEnabled  && glowApplyTo === 2 /*All*/
            visible:  ( indicator.isGroup && ((extraDotOnActive && activeStyle === 0) /*Line*/
                                              || activeStyle === 1 /*Dot*/
                                              || !indicator.hasActive) ) ? true: false

            //when there is no active window
            property color state1Color: indicator.hasShown ? root.isActiveColor : root.minimizedColor
            //when there is active window
            property color state2Color: indicator.hasMinimized ? root.minimizedColor : root.isActiveColor
        }
    }

    states: [
        State {
            name: "left"
            when: ((plasmoid.location === PlasmaCore.Types.LeftEdge && !reversedEnabled) ||
                   (plasmoid.location === PlasmaCore.Types.RightEdge && reversedEnabled))

            AnchorChanges {
                target: grid
                anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                    top:undefined; bottom:undefined; left:parent.left; right:undefined;}
            }
            PropertyChanges{
                target: grid
                anchors.leftMargin: root.thicknessMargin;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottom"
            when: (plasmoid.location === PlasmaCore.Types.Floating ||
                   (plasmoid.location === PlasmaCore.Types.BottomEdge && !reversedEnabled) ||
                   (plasmoid.location === PlasmaCore.Types.TopEdge && reversedEnabled))

            AnchorChanges {
                target: grid
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:undefined; bottom:parent.bottom; left:undefined; right:undefined;}
            }
            PropertyChanges{
                target: grid
                anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: root.thicknessMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "top"
            when: ((plasmoid.location === PlasmaCore.Types.TopEdge && !reversedEnabled) ||
                   (plasmoid.location === PlasmaCore.Types.BottomEdge && reversedEnabled))

            AnchorChanges {
                target: grid
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:parent.top; bottom:undefined; left:undefined; right:undefined;}
            }
            PropertyChanges{
                target: grid
                anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin: root.thicknessMargin;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "right"
            when: ((plasmoid.location === PlasmaCore.Types.RightEdge && !reversedEnabled) ||
                   (plasmoid.location === PlasmaCore.Types.LeftEdge && reversedEnabled))

            AnchorChanges {
                target: grid
                anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                    top:undefined; bottom:undefined; left:undefined; right:parent.right;}
            }
            PropertyChanges{
                target: grid
                anchors.leftMargin: 0;    anchors.rightMargin: root.thicknessMargin;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        }
    ]
}// number of windows indicator

