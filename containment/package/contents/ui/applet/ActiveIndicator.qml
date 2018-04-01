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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.1 as Latte

Item{
    id:glowFrame
    width: !root.isVertical ? parent.width : size
    height: root.isVertical ? parent.height : size

    property int size: mimicPlasmaPanel ? 4 : root.statesLineSize

    property bool mimicPlasmaPanel: !root.latteApplet && plasmoid.configuration.panelSize===100 ?
                                        !plasmoid.configuration.reverseLinesPosition : false

    /*Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "yellow"
        color: "transparent"
        opacity:0.6
    }*/

    Latte.GlowPoint{
        id:activePoint
        anchors.centerIn: parent

        basicColor: theme.buttonFocusColor
        roundCorners: true
        showGlow: root.showGlow

        glow3D: root.glow3D
        animation: Math.max(1.65*3*units.longDuration,root.durationTime*3*units.longDuration)
        location: plasmoid.location
        glowOpacity: root.glowOpacity
        contrastColor: root.appShadowColorSolid

        opacity:{
            if ( (!vertical && width <= glowFrame.size && !isActive)
                    || (vertical && height <= glowFrame.size && !isActive))
                return 0;

            return 1;
        }

        property bool isActive: (container.isExpanded && !container.isSystray
                                 && applet.pluginName !== root.plasmoidName
                                 && applet.pluginName !== "org.kde.activeWindowControl"
                                 && applet.pluginName !== "org.kde.plasma.appmenu")

        property bool vertical: root.isVertical

        property int animationTime: root.durationTime * (1.2*units.longDuration)
        property int stateWidth: root.activeIndicatorType === Latte.Dock.DotIndicator ? glowFrame.size : glowFrame.width
        property int stateHeight: root.activeIndicatorType === Latte.Dock.DotIndicator ? glowFrame.size : glowFrame.height

        property real scaleFactor: wrapper.zoomScale

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
            if ( root.activeIndicatorType === Latte.Dock.LineIndicator ) {
                if(!activeAndReverseAnimation.running && !root.vertical && isActive){
                    width = stateWidth;
                }
                else if (!activeAndReverseAnimation.running && root.vertical && isActive){
                    height = stateHeight;
                }
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
            target: activePoint
            property: root.isVertical ? "height" : "width"
            to: !activePoint.isActive ? (root.isVertical ? activePoint.stateHeight : activePoint.stateWidth) : glowFrame.size
            duration: activePoint.animationTime
            easing.type: Easing.InQuad

            onStopped: activePoint.updateInitialSizes()
        }
    }

    states: [
        State {
            name: "leftLatte"
            when: ((plasmoid.location === PlasmaCore.Types.LeftEdge && !root.reverseLinesPosition) ||
                   (plasmoid.location === PlasmaCore.Types.RightEdge && root.reverseLinesPosition)) && !mimicPlasmaPanel

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                    top:undefined; bottom:undefined; left:undefined; right:parent.left;}
            }
        },
        State {
            name: "bottomLatte"
            when: ((plasmoid.location === PlasmaCore.Types.BottomEdge && !root.reverseLinesPosition) ||
                   (plasmoid.location === PlasmaCore.Types.TopEdge && root.reverseLinesPosition)) && !mimicPlasmaPanel

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:parent.bottom; bottom:undefined; left:undefined; right:undefined;}
            }
        },
        State {
            name: "topLatte"
            when: ((plasmoid.location === PlasmaCore.Types.TopEdge && !root.reverseLinesPosition) ||
                   (plasmoid.location === PlasmaCore.Types.BottomEdge && root.reverseLinesPosition)) && !mimicPlasmaPanel

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:undefined; bottom:parent.top; left:undefined; right:undefined;}
            }
        },
        State {
            name: "rightLatte"
            when: ((plasmoid.location === PlasmaCore.Types.RightEdge && !root.reverseLinesPosition) ||
                   (plasmoid.location === PlasmaCore.Types.LeftEdge && root.reverseLinesPosition)) && !mimicPlasmaPanel

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                    top:undefined; bottom:undefined; left:parent.right; right:undefined;}
            }
        },
        State {
            name: "topPlasma"
            when: plasmoid.location === PlasmaCore.Types.TopEdge && mimicPlasmaPanel

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:undefined; bottom:parent.bottom; left:undefined; right:undefined;}
            }
        },
        State {
            name: "bottomPlasma"
            when: plasmoid.location === PlasmaCore.Types.BottomEdge && mimicPlasmaPanel

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:parent.top; bottom:undefined; left:undefined; right:undefined;}
            }
        },
        State {
            name: "leftPlasma"
            when: plasmoid.location === PlasmaCore.Types.LeftEdge && mimicPlasmaPanel

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:undefined; bottom:undefined; left:undefined; right:parent.right;}
            }
        },
        State {
            name: "rightPlasma"
            when: plasmoid.location === PlasmaCore.Types.LeftEdge && mimicPlasmaPanel

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:undefined; bottom:undefined; left:parent.left; right:undefined;}
            }
        }
    ]
}


