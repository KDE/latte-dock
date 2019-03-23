/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

LatteComponents.IndicatorItem{
    id: root

    readonly property real factor: 0.08
    readonly property int size: factor * indicator.currentIconSize
    readonly property int extraMaskThickness: reversedEnabled && glowEnabled ? 1.7 * (factor * indicator.maxIconSize) : 0

    property real textColorBrightness: colorBrightness(theme.textColor)

    property color isActiveColor: theme.buttonFocusColor
    property color minimizedColor: {
        if (minimizedTaskColoredDifferently) {
            return (textColorBrightness > 127.5 ? Qt.darker(theme.textColor, 1.7) : Qt.lighter(theme.textColor, 7));
        }

        return isActiveColor;
    }
    property color notActiveColor: indicator.isMinimized ? minimizedColor : isActiveColor

    //! Common Options
    readonly property bool reversedEnabled: indicator.shared.reversed

    //! Configuration Options
    readonly property bool extraDotOnActive: indicator.configuration.extraDotOnActive
    readonly property bool minimizedTaskColoredDifferently: indicator.configuration.minimizedTaskColoredDifferently
    readonly property int activeStyle: indicator.configuration.activeStyle
    //!glow options
    readonly property bool glowEnabled: indicator.configuration.glowEnabled
    readonly property bool glow3D: indicator.configuration.glow3D
    readonly property int glowApplyTo: indicator.configuration.glowApplyTo
    readonly property real glowOpacity: indicator.configuration.glowOpacity

    /*Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "yellow"
        color: "transparent"
        opacity:0.6
    }*/

    function colorBrightness(color) {
        return colorBrightnessFromRGB(color.r * 255, color.g * 255, color.b * 255);
    }

    // formula for brightness according to:
    // https://www.w3.org/TR/AERT/#color-contrast
    function colorBrightnessFromRGB(r, g, b) {
        return (r * 299 + g * 587 + b * 114) / 1000
    }

    Item{
        id: mainIndicatorElement

        width: flowItem.width
        height: flowItem.height

        Flow{
            id: flowItem
            flow: plasmoid.formFactor === PlasmaCore.Types.Vertical ? Flow.TopToBottom : Flow.LeftToRight

            LatteComponents.GlowPoint{
                id:firstPoint
                opacity: {
                    if (indicator.isTask) {
                        return indicator.isLauncher || (indicator.inRemoving && !activeAndReverseAnimation.running) ? 0 : 1
                    }

                    if (indicator.isApplet) {
                        return (indicator.isActive || activeAndReverseAnimation.running) ? 1 : 0
                    }
                }

                basicColor: indicator.isActive || (indicator.isGroup && indicator.hasShown) ? root.isActiveColor : root.notActiveColor

                size: root.size
                glow3D: glow3D
                animation: Math.max(1.65*3*units.longDuration,indicator.durationTime*3*units.longDuration)
                location: plasmoid.location
                glowOpacity: root.glowOpacity
                contrastColor: indicator.shadowColor
                attentionColor: theme.negativeTextColor

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
                showBorder: glowEnabled && glow3D

                property int stateWidth: indicator.isGroup ? root.width - secondPoint.width : root.width - spacer.width
                property int stateHeight: indicator.isGroup ? root.height - secondPoint.height : root.width - spacer.height

                property int animationTime: indicator.durationTime* (0.7*units.longDuration)

                property bool isActive: indicator.hasActive || indicator.isActive

                property bool vertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

                property real scaleFactor: indicator.scaleFactor

                function updateInitialSizes(){
                    if(root){
                        if(vertical)
                            width = root.size;
                        else
                            height = root.size;

                        if(vertical && isActive && activeStyle === 0 /*Line*/)
                            height = stateHeight;
                        else
                            height = root.size;

                        if(!vertical && isActive && activeStyle === 0 /*Line*/)
                            width = stateWidth;
                        else
                            width = root.size;
                    }
                }


                onIsActiveChanged: {
                    if (activeStyle === 0 /*Line*/)
                        activeAndReverseAnimation.start();
                }

                onScaleFactorChanged: {
                    if(!activeAndReverseAnimation.running && !vertical && isActive && activeStyle === 0 /*Line*/){
                        width = stateWidth;
                    }
                    else if (!activeAndReverseAnimation.running && vertical && isActive && activeStyle === 0 /*Line*/){
                        height = stateHeight;
                    }
                }

                onStateWidthChanged:{
                    if(!activeAndReverseAnimation.running && !vertical && isActive && activeStyle === 0 /*Line*/)
                        width = stateWidth;
                }

                onStateHeightChanged:{
                    if(!activeAndReverseAnimation.running && vertical && isActive && activeStyle === 0 /*Line*/)
                        height = stateHeight;
                }

                onVerticalChanged: updateInitialSizes();

                Component.onCompleted: {
                    updateInitialSizes();

                    if (indicator) {
                        indicator.onCurrentIconSizeChanged.connect(updateInitialSizes);
                    }
                }

                Component.onDestruction: {
                    if (indicator) {
                        indicator.onCurrentIconSizeChanged.disconnect(updateInitialSizes);
                    }
                }

                NumberAnimation{
                    id: activeAndReverseAnimation
                    target: firstPoint
                    property: plasmoid.formFactor === PlasmaCore.Types.Vertical ? "height" : "width"
                    to: indicator.hasActive && activeStyle === 0 /*Line*/
                        ? (plasmoid.formFactor === PlasmaCore.Types.Vertical ? firstPoint.stateHeight : firstPoint.stateWidth) : root.size
                    duration: firstPoint.animationTime
                    easing.type: Easing.InQuad

                    onStopped: firstPoint.updateInitialSizes()
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
                animation: Math.max(1.65*3*units.longDuration,indicator.durationTime*3*units.longDuration)
                location: plasmoid.location
                glowOpacity: root.glowOpacity
                contrastColor: indicator.shadowColor
                showBorder: glowEnabled && glow3D

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
                    target: mainIndicatorElement
                    anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                        top:undefined; bottom:undefined; left:parent.left; right:undefined;}
                }
            },
            State {
                name: "bottom"
                when: (plasmoid.location === PlasmaCore.Types.Floating ||
                       (plasmoid.location === PlasmaCore.Types.BottomEdge && !reversedEnabled) ||
                       (plasmoid.location === PlasmaCore.Types.TopEdge && reversedEnabled))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                        top:undefined; bottom:parent.bottom; left:undefined; right:undefined;}
                }
            },
            State {
                name: "top"
                when: ((plasmoid.location === PlasmaCore.Types.TopEdge && !reversedEnabled) ||
                       (plasmoid.location === PlasmaCore.Types.BottomEdge && reversedEnabled))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                        top:parent.top; bottom:undefined; left:undefined; right:undefined;}
                }
            },
            State {
                name: "right"
                when: ((plasmoid.location === PlasmaCore.Types.RightEdge && !reversedEnabled) ||
                       (plasmoid.location === PlasmaCore.Types.LeftEdge && reversedEnabled))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                        top:undefined; bottom:undefined; left:undefined; right:parent.right;}
                }
            }
        ]
    }
}// number of windows indicator

