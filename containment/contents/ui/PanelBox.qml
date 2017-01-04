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

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

import org.kde.latte 0.1 as Latte

Item{
    id:barLine

    opacity: root.useThemePanel ? 1 : 0
  //  parent: root
    z:0

    property int panelWidth: secondLayout.count > 0 && root.isHorizontal && !root.editMode ?
                                 layoutsContainer.width + 2*spacing : mainLayout.width + spacing
    property int panelHeight: secondLayout.count > 0 && root.isVertical && !root.editMode ?
                                  layoutsContainer.height + 2*spacing : mainLayout.height + spacing

    width: root.isHorizontal ? panelWidth : smallSize
    height: root.isVertical ? panelHeight : smallSize

    property int spacing: (root.panelAlignment === Latte.Dock.Center
                           || plasmoid.configuration.panelPosition === Latte.Dock.Justify) ?
                              root.panelEdgeSpacing : root.panelEdgeSpacing/2
    property int smallSize: Math.max(3.7*root.statesLineSize, 16)

    Behavior on opacity{
        NumberAnimation { duration: 150 }
    }

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


    /// item which is used as anchors for the plasma's theme
    Item{
        id:belower

        width: (plasmoid.location === PlasmaCore.Types.LeftEdge) ? shadowsSvgItem.margins.left : shadowsSvgItem.margins.right
        height: (plasmoid.location === PlasmaCore.Types.BottomEdge)? shadowsSvgItem.margins.bottom : shadowsSvgItem.margins.top
    }


    /// the current theme's panel
    PlasmaCore.FrameSvgItem{
        id: shadowsSvgItem

        width: root.isVertical ? panelSize + margins.left + margins.right: parent.width + margins.left + margins.right
        height: root.isVertical ? parent.height + margins.left + margins.right : panelSize + margins.top + margins.bottom

        imagePath: "translucent/widgets/panel-background"
        prefix:"shadow"

        opacity: root.useThemePanel ? 1 : 0
        visible: (opacity == 0) ? false : true

        property int panelSize: ((plasmoid.location === PlasmaCore.Types.BottomEdge) ||
                                 (plasmoid.location === PlasmaCore.Types.TopEdge)) ?
                                    root.themePanelSize + belower.height:
                                    root.themePanelSize + belower.width

        Behavior on opacity{
            NumberAnimation { duration: 200 }
        }

        Binding {
            target: root
            property: "realPanelSize"
            when: shadowsSvgItem
            value: {
                var space = ((plasmoid.location === PlasmaCore.Types.BottomEdge) ||
                             (plasmoid.location === PlasmaCore.Types.TopEdge)) ?
                                belower.height : belower.width

                return shadowsSvgItem.panelSize + space;
            }

        }


        PlasmaCore.FrameSvgItem{
            anchors.margins: belower.width-1
            anchors.fill:parent
            // imagePath: root.transparentPanel ? "translucent/widgets/panel-background" :
            //                                   "widgets/panel-background"
            imagePath: "widgets/panel-background"
        }
    }

    //BEGIN states
    //user set Panel Positions
    //0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left
        State {
            name: "leftCenter"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Center)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.left; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:belower.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        ///Left
        State {
            name: "leftTop"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Top)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:parent.top; left:undefined; right:parent.left; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:belower.top; bottom:undefined; left:belower.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        ///Left
        State {
            name: "leftBottom"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Bottom)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:parent.bottom; bottom:undefined; left:undefined; right:parent.left; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:belower.bottom; left:belower.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "leftDouble"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Justify)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.left; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:belower.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        ///Right
        State {
            name: "rightCenter"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Center)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:undefined; left:parent.right; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:undefined; right:belower.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        State {
            name: "rightTop"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Top)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:parent.top; left:parent.right; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:belower.top; bottom:undefined; left:undefined; right:belower.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "rightBottom"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Bottom)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined }
            }
            AnchorChanges {
                target: belower
                anchors{ top:parent.bottom; bottom:undefined; left:parent.right; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:belower.bottom; left:undefined; right:belower.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "rightDouble"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Justify)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:undefined; left:parent.right; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:undefined; right:belower.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        ///Bottom
        State {
            name: "bottomCenter"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Center)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:parent.bottom; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:belower.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomLeft"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Left)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:parent.bottom; bottom:undefined; left:undefined; right:parent.left; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:belower.bottom; left:belower.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomRight"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Right)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:parent.bottom; bottom:undefined; left:parent.right; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:belower.bottom; left:undefined; right:belower.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomDouble"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Justify)

            AnchorChanges {
                target: barLine
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:parent.bottom; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:belower.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        ///Top
        State {
            name: "topCenter"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Center)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:parent.top; left:undefined; right:undefined;  horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:belower.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "topLeft"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Left)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:parent.top; left:undefined; right:parent.left;  horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:belower.top; bottom:undefined; left:belower.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "topRight"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Right)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:parent.top; left:parent.right; right:undefined;  horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:belower.top; bottom:undefined; left:undefined; right:belower.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "topDouble"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Justify)

            AnchorChanges {
                target: barLine
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: belower
                anchors{ top:undefined; bottom:parent.top; left:undefined; right:undefined;  horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsSvgItem
                anchors{ top:belower.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        }
    ]
    //END states
}

