/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item{
    id:glowFrame
    width: ( icList.orientation === Qt.Horizontal ) ? wrapper.regulatorWidth : size
    height: ( icList.orientation === Qt.Vertical ) ? wrapper.regulatorHeight : size

    //property int size: Math.ceil( panel.iconSize/13 ) //5
    property int size: panel.statesLineSize

    //SystemPalette { id: myPalette; colorGroup: SystemPalette.Active }
    property color isActiveColor: theme.buttonFocusColor
    //property color isShownColor: plasmoid.configuration.threeColorsWindows ? panel.shownDotColor : isActiveColor
    property color isShownColor: isActiveColor
    property color minimizedColor: plasmoid.configuration.threeColorsWindows ? panel.minimizedDotColor : isActiveColor
    property color notActiveColor: mainItemContainer.hasMinimized ? minimizedColor : isShownColor

   /* Rectangle{
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

            GlowPoint{
                id:firstPoint
                visible: ( !IsLauncher ) ? true: false

                basicColor: (mainItemContainer.hasActive) ?
                                glowFrame.isActiveColor : glowFrame.notActiveColor

                roundCorners: true
                showAttention: model.IsDemandingAttention ? true : false

                property int stateWidth: mainItemContainer.isGroupParent ? (wrapper.regulatorWidth - secondPoint.width) : wrapper.regulatorWidth - spacer.width
                property int stateHeight: mainItemContainer.isGroupParent ? wrapper.regulatorHeight - secondPoint.height : wrapper.regulatorHeight - spacer.height

                property int animationTime: plasmoid.configuration.durationTime* (0.7*units.longDuration)

                property bool isActive: mainItemContainer.hasActive
                                        || (panel.showPreviews && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === mainItemContainer))

                property bool vertical: panel.vertical

                property real scaleFactor: wrapper.scale

                function updateInitialSizes(){
                    if(glowFrame){
                        if(vertical)
                            width = glowFrame.size;
                        else
                            height = glowFrame.size;

                        if(vertical && isActive)
                            height = stateHeight;
                        else
                            height = glowFrame.size;

                        if(!vertical && isActive)
                            width = stateWidth;
                        else
                            width = glowFrame.size;
                    }
                }


                onIsActiveChanged: {
                   // if(mainItemContainer.hasActive || windowsPreviewDlg.visible)
                        activeAndReverseAnimation.start();
                }

                onScaleFactorChanged: {
                    if(!activeAndReverseAnimation.running && !panel.vertical && isActive){
                        width = stateWidth;
                    }
                    else if (!activeAndReverseAnimation.running && panel.vertical && isActive){
                        height = stateHeight;
                    }
                }

                onStateWidthChanged:{
                    if(!activeAndReverseAnimation.running && !vertical && isActive)
                        width = stateWidth;
                }

                onStateHeightChanged:{
                    if(!activeAndReverseAnimation.running && vertical && isActive)
                        height = stateHeight;
                }

                onVerticalChanged: updateInitialSizes();

                Component.onCompleted: {
                    updateInitialSizes();

                    panel.onIconSizeChanged.connect(updateInitialSizes);
                }

                NumberAnimation{
                    id: activeAndReverseAnimation
                    target: firstPoint
                    property: panel.vertical ? "height" : "width"
                    to: mainItemContainer.hasActive
                        || (panel.showPreviews && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === mainItemContainer))
                        ? (panel.vertical ? firstPoint.stateHeight : firstPoint.stateWidth) : glowFrame.size
                    duration: firstPoint.animationTime
                    easing.type: Easing.InQuad

                    onStopped: firstPoint.updateInitialSizes()
                }
            }

            Item{
                id:spacer
                width: mainItemContainer.isGroupParent ? 0.5*glowFrame.size : 0
                height: mainItemContainer.isGroupParent ? 0.5*glowFrame.size : 0
            }

            GlowPoint{
                id:secondPoint
                width: visible ? glowFrame.size : 0
                height: width

                basicColor: ((mainItemContainer.hasActive)&&(!(mainItemContainer.hasMinimized))) ? state2Color : state1Color
                roundCorners: true
                visible:  ( mainItemContainer.isGroupParent && plasmoid.configuration.dotsOnActive )
                          || (mainItemContainer.isGroupParent && !mainItemContainer.hasActive)? true: false

                //when there is no active window
                property color state1Color: mainItemContainer.hasShown ? glowFrame.isShownColor : glowFrame.minimizedColor
                //when there is active window
                property color state2Color: mainItemContainer.hasMinimized ? glowFrame.minimizedColor : glowFrame.isShownColor
            }
        }
    }
}// number of windows indicator

