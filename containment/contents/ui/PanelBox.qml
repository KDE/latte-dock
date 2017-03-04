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

    width: root.isHorizontal ? panelWidth : smallSize
    height: root.isVertical ? panelHeight : smallSize

    opacity: root.useThemePanel ? 1 : 0

    property int panelWidth: {
        if (root.drawShadowsExternal) {
            return root.width;
        } else {
            if ((root.panelAlignment === Latte.Dock.Justify) && root.isHorizontal && !root.editMode) {
                return root.maxLength;
            } else {
                return mainLayout.width + spacing;
            }
        }
    }

    property int panelHeight: {
        if (root.drawShadowsExternal) {
            return root.height;
        } else {
            if ((root.panelAlignment === Latte.Dock.Justify) && root.isVertical && !root.editMode) {
                return root.maxLength;
            } else {
                return mainLayout.height + spacing;
            }
        }
    }

    property int spacing: (root.panelAlignment === Latte.Dock.Center
                           || plasmoid.configuration.panelPosition === Latte.Dock.Justify) ?
                              root.panelEdgeSpacing/2 : root.panelEdgeSpacing/4
    property int smallSize: Math.max(3.7*root.statesLineSize, 16)

    Behavior on opacity{
        NumberAnimation { duration: 150 }
    }

    Binding {
        target: root
        property: "realPanelLength"
        value: root.isVertical ? barLine.height : barLine.width
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

    /// the current theme's panel
    PlasmaCore.FrameSvgItem{
        id: shadowsSvgItem

        width: root.isVertical ? panelSize + marginsWidth - (solidBackground.leftIncreaser + solidBackground.rightIncreaser) :
                                 Math.min(parent.width + marginsWidth, root.width - marginsWidth)
        height: root.isVertical ? Math.min(parent.height + marginsHeight, root.height - marginsHeight) :
                                  panelSize + marginsHeight - (solidBackground.topIncreaser + solidBackground.bottomIncreaser)

        imagePath: root.drawShadowsExternal ? "" : "widgets/panel-background"
        prefix: root.drawShadowsExternal ? "" : "shadow"

        opacity: root.useThemePanel ? 1 : 0
        visible: (opacity == 0) ? false : true

        enabledBorders: dock ? dock.enabledBorders : 0

        property int marginsWidth: {
            if (root.drawShadowsExternal) {
                return 0;
            } else {
                if (root.panelAlignment === Latte.Dock.Left)
                    return margins.right;
                else if (root.panelAlignment === Latte.Dock.Right)
                    return margins.left;
                else
                    return margins.left+margins.right;
            }
        }

        property int marginsHeight: {
            if (root.drawShadowsExternal) {
                return 0;
            } else {
                if (root.panelAlignment === Latte.Dock.Top)
                    return margins.bottom;
                else if (root.panelAlignment === Latte.Dock.Bottom)
                    return margins.top;
                else
                    return margins.top + margins.bottom;
            }
        }

        property int panelSize: automaticPanelSize
        property int automaticPanelSize: {
            if (root.drawShadowsExternal) {
                var iconS = 1.2*plasmoid.configuration.iconSize + 1;
                root.realPanelThickness = iconS;
                return iconS;
            } else {
                var icons = root.statesLineSize + root.iconSize + root.iconMargin + 1;
                var panels = root.themePanelSize + root.panelMargin;

                root.realPanelThickness = icons;
                if (icons > panels) {
                    return panels;
                } else {
                    return icons;
                }
            }
        }

        property int shadowsSize: {
            if (shadowsSvgItem && root.useThemePanel){
                if (root.isVertical){
                    if (plasmoid.location === PlasmaCore.Types.LeftEdge)
                        return shadowsSvgItem.margins.right;
                    else if (plasmoid.location === PlasmaCore.Types.RightEdge)
                        return shadowsSvgItem.margins.left;
                } else {
                    if (plasmoid.location === PlasmaCore.Types.BottomEdge)
                        return shadowsSvgItem.margins.top;
                    else if (plasmoid.location === PlasmaCore.Types.TopEdge)
                        return shadowsSvgItem.margins.bottom;
                }
            } else {
                return 0;
            }
        }

        Behavior on opacity{
            NumberAnimation { duration: 200 }
        }

        Binding {
            target: root
            property: "panelShadow"
            when: shadowsSvgItem
            value: shadowsSvgItem.shadowsSize
        }

        Binding {
            target: root
            property: "realPanelSize"
            when: shadowsSvgItem
            value: shadowsSvgItem.panelSize
        }


        PlasmaCore.FrameSvgItem{
            id: solidBackground
            anchors.leftMargin: shadowsSvgItem.margins.left - leftIncreaser
            anchors.rightMargin: shadowsSvgItem.margins.right - rightIncreaser
            anchors.topMargin: shadowsSvgItem.margins.top - topIncreaser
            anchors.bottomMargin: shadowsSvgItem.margins.bottom - bottomIncreaser
            anchors.fill:parent

            imagePath: root.solidPanel ? "opaque/dialogs/background" : "widgets/panel-background"

            //! the increases used when the user forces a solid background and the background
            //! must be increased in order to look ok in the corners
            property int rightIncreaser: {
                if (!(root.solidPanel && root.isVertical && plasmoid.location === PlasmaCore.Types.LeftEdge))
                    return 0;
                else
                    return hiddenPanelBackground.margins.right;
            }

            property int leftIncreaser: {
                if (!(root.solidPanel && root.isVertical && plasmoid.location === PlasmaCore.Types.RightEdge))
                    return 0;
                else
                    return hiddenPanelBackground.margins.left;
            }

            property int topIncreaser: {
                if (!(root.solidPanel && root.isVertical && plasmoid.location === PlasmaCore.Types.BottomEdge))
                    return 0;
                else
                    return hiddenPanelBackground.margins.top;
            }

            property int bottomIncreaser: {
                if (!(root.solidPanel && root.isVertical && plasmoid.location === PlasmaCore.Types.TopEdge))
                    return 0;
                else
                    return hiddenPanelBackground.margins.bottom;
            }


            Binding {
                target: root
                property: "panelMargin"
                value: {
                    if (root.useThemePanel){
                        if (root.isVertical){
                            if (plasmoid.location === PlasmaCore.Types.LeftEdge)
                                return solidBackground.margins.right;
                            else if (plasmoid.location === PlasmaCore.Types.RightEdge)
                                return solidBackground.margins.left;
                        } else {
                            if (plasmoid.location === PlasmaCore.Types.BottomEdge)
                                return solidBackground.margins.top;
                            else if (plasmoid.location === PlasmaCore.Types.TopEdge)
                                return solidBackground.margins.bottom;
                        }
                    } else {
                        return 0;
                    }
                }
            }

            onRepaintNeeded: {
                if (root.drawShadowsExternal)
                    adjustPrefix();
            }

            enabledBorders: dock ? dock.enabledBorders : 0

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
                if (hasElementPrefix(pre)) {
                    prefix = pre;
                } else {
                    prefix = "";
                }
            }
        }

        PlasmaCore.FrameSvgItem{
            id: hiddenPanelBackground
            imagePath: "widgets/panel-background"
            visible: false
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
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
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
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
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
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
                target: shadowsSvgItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        }
    ]
    //END states
}


