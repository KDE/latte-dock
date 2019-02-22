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

Item{
    id: indicatorRoot

    property int size: 0.075*root.iconSize

    property color isActiveColor: theme.buttonFocusColor
    property color minimizedColor: root.threeColorsWindows ? root.minimizedDotColor : isActiveColor
    property color notActiveColor: taskItem.hasMinimized ? minimizedColor : isActiveColor

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
                visible: ( !IsLauncher ) ? true: false

                basicColor: IsActive===true || (taskItem.isGroupParent && taskItem.hasShown) ? indicatorRoot.isActiveColor : indicatorRoot.notActiveColor

                glow3D: root.glow3D
                animation: Math.max(1.65*3*units.longDuration,root.durationTime*3*units.longDuration)
                location: plasmoid.location
                glowOpacity: root.glowOpacity
                contrastColor: root.appShadowColorSolid
                attentionColor: colorScopePalette.negativeTextColor

                roundCorners: true
                showAttention: taskItem.showAttention
                showGlow: {
                    if (root.showGlow && (root.glowOption === Latte.Types.GlowAll || showAttention ))
                        return true;
                    else if (root.showGlow && root.glowOption === Latte.Types.GlowOnlyOnActive && taskItem.hasActive)
                        return true;
                    else
                        return false;
                }
                showBorder: root.showGlow && root.glow3D

               // opacity: (!taskItem.hasActive && root.showPreviews
               //           && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === taskItem)) ? 0.4 : 1

                property int stateWidth: taskItem.isGroupParent ? indicatorRoot.width - secondPoint.width : indicatorRoot.width - spacer.width
                property int stateHeight: taskItem.isGroupParent ? indicatorRoot.height - secondPoint.height : indicatorRoot.width - spacer.height

                property int animationTime: root.durationTime* (0.7*units.longDuration)

                property bool isActive: taskItem.hasActive
                                        || (root.showPreviews && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === taskItem))

                property bool vertical: root.vertical

                property real scaleFactor: wrapper.mScale

                function updateInitialSizes(){
                    if(indicatorRoot){
                        if(vertical)
                            width = indicatorRoot.size;
                        else
                            height = indicatorRoot.size;

                        if(vertical && isActive && root.activeIndicatorType === Latte.Types.LineIndicator)
                            height = stateHeight;
                        else
                            height = indicatorRoot.size;

                        if(!vertical && isActive && root.activeIndicatorType === Latte.Types.LineIndicator)
                            width = stateWidth;
                        else
                            width = indicatorRoot.size;
                    }
                }


                onIsActiveChanged: {
                    // if(taskItem.hasActive || windowsPreviewDlg.visible)
                    if (root.activeIndicatorType === Latte.Types.LineIndicator)
                        activeAndReverseAnimation.start();
                }

                onScaleFactorChanged: {
                    if(!activeAndReverseAnimation.running && !root.vertical && isActive && root.activeIndicatorType === Latte.Types.LineIndicator){
                        width = stateWidth;
                    }
                    else if (!activeAndReverseAnimation.running && root.vertical && isActive && root.activeIndicatorType === Latte.Types.LineIndicator){
                        height = stateHeight;
                    }
                }

                onStateWidthChanged:{
                    if(!activeAndReverseAnimation.running && !vertical && isActive && root.activeIndicatorType === Latte.Types.LineIndicator)
                        width = stateWidth;
                }

                onStateHeightChanged:{
                    if(!activeAndReverseAnimation.running && vertical && isActive && root.activeIndicatorType === Latte.Types.LineIndicator)
                        height = stateHeight;
                }

                onVerticalChanged: updateInitialSizes();

                Component.onCompleted: {
                    updateInitialSizes();

                    root.onIconSizeChanged.connect(updateInitialSizes);
                }

                Component.onDestruction: {
                    root.onIconSizeChanged.disconnect(updateInitialSizes);
                }

                NumberAnimation{
                    id: activeAndReverseAnimation
                    target: firstPoint
                    property: root.vertical ? "height" : "width"
                    to: (taskItem.hasActive && root.activeIndicatorType === Latte.Types.LineIndicator)
                        || (root.showPreviews && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === taskItem))
                        ? (root.vertical ? firstPoint.stateHeight : firstPoint.stateWidth) : indicatorRoot.size
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

                glow3D: root.glow3D
                animation: Math.max(1.65*3*units.longDuration,root.durationTime*3*units.longDuration)
                location: plasmoid.location
                glowOpacity: root.glowOpacity
                contrastColor: root.appShadowColorSolid
                showBorder: root.showGlow && root.glow3D

                basicColor: state2Color //taskItem.hasActive ? state2Color : state1Color
                roundCorners: true
                showGlow: root.showGlow  && root.glowOption === Latte.Types.GlowAll
                visible:  ( taskItem.isGroupParent && ((root.dotsOnActive && root.activeIndicatorType === Latte.Types.LineIndicator)
                                                                || root.activeIndicatorType === Latte.Types.DotIndicator
                                                                || !taskItem.hasActive) )? true: false

                //when there is no active window
                property color state1Color: taskItem.hasShown ? indicatorRoot.isActiveColor : indicatorRoot.minimizedColor
                //when there is active window
                property color state2Color: taskItem.hasMinimized ? indicatorRoot.minimizedColor : indicatorRoot.isActiveColor
            }
        }

        states: [
            State {
                name: "left"
                when: ((plasmoid.location === PlasmaCore.Types.LeftEdge && !root.reverseLinesPosition) ||
                       (plasmoid.location === PlasmaCore.Types.RightEdge && root.reverseLinesPosition))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                        top:undefined; bottom:undefined; left:parent.left; right:undefined;}
                }
            },
            State {
                name: "bottom"
                when: ((plasmoid.location === PlasmaCore.Types.BottomEdge && !root.reverseLinesPosition) ||
                       (plasmoid.location === PlasmaCore.Types.TopEdge && root.reverseLinesPosition))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                        top:undefined; bottom:parent.bottom; left:undefined; right:undefined;}
                }
            },
            State {
                name: "top"
                when: ((plasmoid.location === PlasmaCore.Types.TopEdge && !root.reverseLinesPosition) ||
                       (plasmoid.location === PlasmaCore.Types.BottomEdge && root.reverseLinesPosition))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                        top:parent.top; bottom:undefined; left:undefined; right:undefined;}
                }
            },
            State {
                name: "right"
                when: ((plasmoid.location === PlasmaCore.Types.RightEdge && !root.reverseLinesPosition) ||
                       (plasmoid.location === PlasmaCore.Types.LeftEdge && root.reverseLinesPosition))

                AnchorChanges {
                    target: mainIndicatorElement
                    anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                        top:undefined; bottom:undefined; left:undefined; right:parent.right;}
                }
            }
        ]
    }
}// number of windows indicator

