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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.2 as Latte

import "../code/ColorizerTools.js" as ColorizerTools

Item{
    id: indicatorRoot

    readonly property real factor: 0.08
    readonly property int size: factor * parentItem.currentIconSize
    readonly property int extraMaskThickness: parentItem.reversedEnabled && parentItem.glowEnabled ? 1.7 * (factor * parentItem.maxIconSize) : 0
    readonly property Item parentItem: parent.manager

    property real textColorBrightness: ColorizerTools.colorBrightness(theme.textColor)

    property color isActiveColor: theme.buttonFocusColor
    property color minimizedColor: {
        if (parentItem.multiColorEnabled) {
            return (textColorBrightness > 127.5 ? Qt.darker(theme.textColor, 1.7) : Qt.lighter(theme.textColor, 7));
        }

        return isActiveColor;
    }
    property color notActiveColor: parentItem.isMinimized ? minimizedColor : isActiveColor

    /*Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "yellow"
        color: "transparent"
        opacity:0.6
    }*/

    Item{
        id: mainIndicatorElement

        width: flowItem.width
        height: flowItem.height

        Flow{
            id: flowItem
            flow: plasmoid.formFactor === PlasmaCore.Types.Vertical ? Flow.TopToBottom : Flow.LeftToRight

            Latte.GlowPoint{
                id:firstPoint
                opacity: {
                    if (parentItem.isTask) {
                        return parentItem.isLauncher || (parentItem.inRemoving && !activeAndReverseAnimation.running) ? 0 : 1
                    }

                    if (parentItem.isApplet) {
                        return (parentItem.isActive || activeAndReverseAnimation.running) ? 1 : 0
                    }
                }

                basicColor: parentItem.isActive || (parentItem.isGroup && parentItem.hasShown) ? indicatorRoot.isActiveColor : indicatorRoot.notActiveColor

                size: indicatorRoot.size
                glow3D: parentItem.glow3D
                animation: Math.max(1.65*3*units.longDuration,parentItem.durationTime*3*units.longDuration)
                location: plasmoid.location
                glowOpacity: parentItem.glowOpacity
                contrastColor: parentItem.shadowColor
                attentionColor: theme.negativeTextColor

                roundCorners: true
                showAttention: parentItem.inAttention
                showGlow: {
                    if (parentItem.glowEnabled && (parentItem.glowOption === Latte.Types.GlowAll || showAttention ))
                        return true;
                    else if (parentItem.glowEnabled && parentItem.glowOption === Latte.Types.GlowOnlyOnActive && parentItem.hasActive)
                        return true;
                    else
                        return false;
                }
                showBorder: parentItem.glowEnabled && parentItem.glow3D

                property int stateWidth: parentItem.isGroup ? indicatorRoot.width - secondPoint.width : indicatorRoot.width - spacer.width
                property int stateHeight: parentItem.isGroup ? indicatorRoot.height - secondPoint.height : indicatorRoot.width - spacer.height

                property int animationTime: parentItem.durationTime* (0.7*units.longDuration)

                property bool isActive: parentItem.hasActive || parentItem.isActive

                property bool vertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

                property real scaleFactor: parentItem.scaleFactor

                function updateInitialSizes(){
                    if(indicatorRoot){
                        if(vertical)
                            width = indicatorRoot.size;
                        else
                            height = indicatorRoot.size;

                        if(vertical && isActive && parentItem.activeIndicatorType === Latte.Types.LineIndicator)
                            height = stateHeight;
                        else
                            height = indicatorRoot.size;

                        if(!vertical && isActive && parentItem.activeIndicatorType === Latte.Types.LineIndicator)
                            width = stateWidth;
                        else
                            width = indicatorRoot.size;
                    }
                }


                onIsActiveChanged: {
                    if (parentItem.activeIndicatorType === Latte.Types.LineIndicator)
                        activeAndReverseAnimation.start();
                }

                onScaleFactorChanged: {
                    if(!activeAndReverseAnimation.running && !vertical && isActive && parentItem.activeIndicatorType === Latte.Types.LineIndicator){
                        width = stateWidth;
                    }
                    else if (!activeAndReverseAnimation.running && vertical && isActive && parentItem.activeIndicatorType === Latte.Types.LineIndicator){
                        height = stateHeight;
                    }
                }

                onStateWidthChanged:{
                    if(!activeAndReverseAnimation.running && !vertical && isActive && parentItem.activeIndicatorType === Latte.Types.LineIndicator)
                        width = stateWidth;
                }

                onStateHeightChanged:{
                    if(!activeAndReverseAnimation.running && vertical && isActive && parentItem.activeIndicatorType === Latte.Types.LineIndicator)
                        height = stateHeight;
                }

                onVerticalChanged: updateInitialSizes();

                Component.onCompleted: {
                    updateInitialSizes();

                    parentItem.onCurrentIconSizeChanged.connect(updateInitialSizes);
                }

                Component.onDestruction: {
                    parentItem.onCurrentIconSizeChanged.disconnect(updateInitialSizes);
                }

                NumberAnimation{
                    id: activeAndReverseAnimation
                    target: firstPoint
                    property: plasmoid.formFactor === PlasmaCore.Types.Vertical ? "height" : "width"
                    to: parentItem.hasActive && parentItem.activeIndicatorType === Latte.Types.LineIndicator
                        ? (plasmoid.formFactor === PlasmaCore.Types.Vertical ? firstPoint.stateHeight : firstPoint.stateWidth) : indicatorRoot.size
                    duration: firstPoint.animationTime
                    easing.type: Easing.InQuad

                    onStopped: firstPoint.updateInitialSizes()
                }
            }

            Item{
                id:spacer
                width: secondPoint.visible ? 0.5*indicatorRoot.size : 0
                height: secondPoint.visible ? 0.5*indicatorRoot.size : 0
            }

            Latte.GlowPoint{
                id:secondPoint
                width: visible ? indicatorRoot.size : 0
                height: width

                size: indicatorRoot.size
                glow3D: parentItem.glow3D
                animation: Math.max(1.65*3*units.longDuration,parentItem.durationTime*3*units.longDuration)
                location: plasmoid.location
                glowOpacity: parentItem.glowOpacity
                contrastColor: parentItem.shadowColor
                showBorder: parentItem.glowEnabled && parentItem.glow3D

                basicColor: state2Color
                roundCorners: true
                showGlow: parentItem.glowEnabled  && parentItem.glowOption === Latte.Types.GlowAll
                visible:  ( parentItem.isGroup && ((parentItem.dotsOnActive && parentItem.activeIndicatorType === Latte.Types.LineIndicator)
                                                                || parentItem.activeIndicatorType === Latte.Types.DotIndicator
                                                                || !parentItem.hasActive) )? true: false

                //when there is no active window
                property color state1Color: parentItem.hasShown ? indicatorRoot.isActiveColor : indicatorRoot.minimizedColor
                //when there is active window
                property color state2Color: parentItem.hasMinimized ? indicatorRoot.minimizedColor : indicatorRoot.isActiveColor
            }
        }

        states: [
            State {
                name: "left"
                when: ((plasmoid.location === PlasmaCore.Types.LeftEdge && !parentItem.reversedEnabled) ||
                       (plasmoid.location === PlasmaCore.Types.RightEdge && parentItem.reversedEnabled))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                        top:undefined; bottom:undefined; left:parent.left; right:undefined;}
                }
            },
            State {
                name: "bottom"
                when: ((plasmoid.location === PlasmaCore.Types.BottomEdge && !parentItem.reversedEnabled) ||
                       (plasmoid.location === PlasmaCore.Types.TopEdge && parentItem.reversedEnabled))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                        top:undefined; bottom:parent.bottom; left:undefined; right:undefined;}
                }
            },
            State {
                name: "top"
                when: ((plasmoid.location === PlasmaCore.Types.TopEdge && !parentItem.reversedEnabled) ||
                       (plasmoid.location === PlasmaCore.Types.BottomEdge && parentItem.reversedEnabled))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                        top:parent.top; bottom:undefined; left:undefined; right:undefined;}
                }
            },
            State {
                name: "right"
                when: ((plasmoid.location === PlasmaCore.Types.RightEdge && !parentItem.reversedEnabled) ||
                       (plasmoid.location === PlasmaCore.Types.LeftEdge && parentItem.reversedEnabled))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                        top:undefined; bottom:undefined; left:undefined; right:parent.right;}
                }
            }
        ]
    }
}// number of windows indicator

