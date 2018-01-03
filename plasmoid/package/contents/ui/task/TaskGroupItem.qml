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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.1 as Latte

Item{
    id:glowFrame
    width: ( icList.orientation === Qt.Horizontal ) ? wrapper.regulatorWidth : size
    height: ( icList.orientation === Qt.Vertical ) ? wrapper.regulatorHeight : size

    //property int size: Math.ceil( root.iconSize/13 ) //5
    property int size: root.statesLineSize

    //SystemPalette { id: myPalette; colorGroup: SystemPalette.Active }
    property color isActiveColor: theme.buttonFocusColor
    //property color isShownColor: plasmoid.configuration.threeColorsWindows ? root.shownDotColor : isActiveColor
    //property color isShownColor: isActiveColor
    property color minimizedColor: root.threeColorsWindows ? root.minimizedDotColor : isActiveColor
    property color notActiveColor: mainItemContainer.hasMinimized ? minimizedColor : isActiveColor

    /*Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "yellow"
        color: "transparent"
        opacity:0.6
    }*/
    Item{
        anchors.centerIn: parent

        width: flowItem.width
        height: flowItem.height

        Flow{
            id: flowItem
            flow: ( icList.orientation === Qt.Vertical ) ? Flow.TopToBottom : Flow.LeftToRight

            Latte.GlowPoint{
                id:firstPoint
                visible: ( !IsLauncher ) ? true: false

                basicColor: IsActive===true || (mainItemContainer.isGroupParent && mainItemContainer.hasShown)?
                                glowFrame.isActiveColor : glowFrame.notActiveColor

                glow3D: root.glow3D
                animation: Math.max(1.65*3*units.longDuration,root.durationTime*3*units.longDuration)
                location: plasmoid.location
                glowOpacity: root.glowOpacity
                contrastColor: root.appShadowColorSolid
                attentionColor: colorScopePalette.negativeTextColor

                roundCorners: true
                showAttention: mainItemContainer.showAttention
                showGlow: {
                    if (root.showGlow && (root.glowOption === Latte.Dock.GlowAll || showAttention ))
                        return true;
                    else if (root.showGlow && root.glowOption === Latte.Dock.GlowOnlyOnActive && mainItemContainer.hasActive)
                        return true;
                    else
                        return false;
                }
                showBorder: root.showGlow && root.glow3D

               // opacity: (!mainItemContainer.hasActive && root.showPreviews
               //           && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === mainItemContainer)) ? 0.4 : 1

                property int stateWidth: mainItemContainer.isGroupParent ? (wrapper.regulatorWidth - secondPoint.width) : wrapper.regulatorWidth - spacer.width
                property int stateHeight: mainItemContainer.isGroupParent ? wrapper.regulatorHeight - secondPoint.height : wrapper.regulatorHeight - spacer.height

                property int animationTime: root.durationTime* (0.7*units.longDuration)

                property bool isActive: mainItemContainer.hasActive
                                        || (root.showPreviews && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === mainItemContainer))

                property bool vertical: root.vertical

                property real scaleFactor: wrapper.mScale

                function updateInitialSizes(){
                    if(glowFrame){
                        if(vertical)
                            width = glowFrame.size;
                        else
                            height = glowFrame.size;

                        if(vertical && isActive && root.activeIndicatorType === Latte.Dock.LineIndicator)
                            height = stateHeight;
                        else
                            height = glowFrame.size;

                        if(!vertical && isActive && root.activeIndicatorType === Latte.Dock.LineIndicator)
                            width = stateWidth;
                        else
                            width = glowFrame.size;
                    }
                }


                onIsActiveChanged: {
                    // if(mainItemContainer.hasActive || windowsPreviewDlg.visible)
                    if (root.activeIndicatorType === Latte.Dock.LineIndicator)
                        activeAndReverseAnimation.start();
                }

                onScaleFactorChanged: {
                    if(!activeAndReverseAnimation.running && !root.vertical && isActive && root.activeIndicatorType === Latte.Dock.LineIndicator){
                        width = stateWidth;
                    }
                    else if (!activeAndReverseAnimation.running && root.vertical && isActive && root.activeIndicatorType === Latte.Dock.LineIndicator){
                        height = stateHeight;
                    }
                }

                onStateWidthChanged:{
                    if(!activeAndReverseAnimation.running && !vertical && isActive && root.activeIndicatorType === Latte.Dock.LineIndicator)
                        width = stateWidth;
                }

                onStateHeightChanged:{
                    if(!activeAndReverseAnimation.running && vertical && isActive && root.activeIndicatorType === Latte.Dock.LineIndicator)
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
                    to: (mainItemContainer.hasActive && root.activeIndicatorType === Latte.Dock.LineIndicator)
                        || (root.showPreviews && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === mainItemContainer))
                        ? (root.vertical ? firstPoint.stateHeight : firstPoint.stateWidth) : glowFrame.size
                    duration: firstPoint.animationTime
                    easing.type: Easing.InQuad

                    onStopped: firstPoint.updateInitialSizes()
                }
            }

            Item{
                id:spacer
                width: secondPoint.visible ? 0.5*glowFrame.size : 0
                height: secondPoint.visible ? 0.5*glowFrame.size : 0
            }

            Latte.GlowPoint{
                id:secondPoint
                width: visible ? glowFrame.size : 0
                height: width

                glow3D: root.glow3D
                animation: Math.max(1.65*3*units.longDuration,root.durationTime*3*units.longDuration)
                location: plasmoid.location
                glowOpacity: root.glowOpacity
                contrastColor: root.appShadowColorSolid
                showBorder: root.showGlow && root.glow3D

                basicColor: state2Color //mainItemContainer.hasActive ? state2Color : state1Color
                roundCorners: true
                showGlow: root.showGlow  && root.glowOption === Latte.Dock.GlowAll
                visible:  ( mainItemContainer.isGroupParent && ((root.dotsOnActive && root.activeIndicatorType === Latte.Dock.LineIndicator)
                                                                || root.activeIndicatorType === Latte.Dock.DotIndicator
                                                                || !mainItemContainer.hasActive) )? true: false

                //when there is no active window
                property color state1Color: mainItemContainer.hasShown ? glowFrame.isActiveColor : glowFrame.minimizedColor
                //when there is active window
                property color state2Color: mainItemContainer.hasMinimized ? glowFrame.minimizedColor : glowFrame.isActiveColor
            }
        }
    }
}// number of windows indicator

