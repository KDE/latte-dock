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

    property int animationTime: 6*root.durationTime*units.shortDuration

    property int lengthMargins: {
        return root.isVertical ? shadowsSvgItem.marginsHeight : shadowsSvgItem.marginsWidth
    }

    property int panelWidth: {
        if (root.behaveAsPlasmaPanel && !root.editMode) {
            return root.width;
        } else {
            if ((root.panelAlignment === Latte.Dock.Justify) && root.isHorizontal) {
                return root.maxLength;
            } else {
                return layoutsContainer.mainLayout.width + spacing;
            }
        }
    }

    property int panelHeight: {
        if (root.behaveAsPlasmaPanel && !root.editMode) {
            return root.height;
        } else {
            if ((root.panelAlignment === Latte.Dock.Justify) && root.isVertical) {
                return root.maxLength;
            } else {
                return layoutsContainer.mainLayout.height + spacing;
            }
        }
    }

    property int spacing: {
        if (!Latte.WindowSystem.compositingActive) {
            return 0;
        } else if (root.panelAlignment === Latte.Dock.Justify && plasmoid.configuration.maxLength === 100) {
            return 0;
        } else if (root.panelAlignment === Latte.Dock.Center || root.panelAlignment === Latte.Dock.Justify || root.offset!==0) {
            return root.panelEdgeSpacing/2;
        } else {
            return root.panelEdgeSpacing/4;
        }
    }
    property int smallSize: Math.max(3.7*root.statesLineSize, 16)

    Behavior on opacity{
        enabled: Latte.WindowSystem.compositingActive
        NumberAnimation {
            duration: barLine.animationTime
        }
    }

    Behavior on opacity{
        enabled: !Latte.WindowSystem.compositingActive
        NumberAnimation {
            duration: 0
        }
    }

    Binding {
        target: root
        property: "realPanelLength"
        value: root.isVertical ? barLine.height : barLine.width
    }

    Binding {
        target: root
        property: "totalPanelEdgeSpacing"
        value: spacing
    }

    onXChanged: solidBackground.updateEffectsArea();
    onYChanged: solidBackground.updateEffectsArea();

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

        imagePath: root.behaveAsPlasmaPanel || !Latte.WindowSystem.compositingActive || !root.panelShadowsActive ? "" : "widgets/panel-background"
        prefix: root.behaveAsPlasmaPanel || !Latte.WindowSystem.compositingActive || !root.panelShadowsActive ? "" : "shadow"

        visible: (opacity == 0) ? false : true

        opacity: {
            if (root.forceTransparentPanel || !root.useThemePanel)
                return 0;
            else
                return 1;
        }

        enabledBorders: dock ? dock.enabledBorders : PlasmaCore.FrameSvg.NoBorder

        Behavior on opacity {
            enabled: Latte.WindowSystem.compositingActive
            NumberAnimation { duration: barLine.animationTime }
        }


        Behavior on opacity{
            enabled: !Latte.WindowSystem.compositingActive
            NumberAnimation { duration: 0 }
        }

        property int marginsWidth: {
            if (imagePath === "") {
                return 0;
            } else {
                if (root.panelAlignment === Latte.Dock.Left && root.offset===0)
                    return margins.right;
                else if (root.panelAlignment === Latte.Dock.Right && root.offset===0)
                    return margins.left;
                else
                    return margins.left+margins.right;
            }
        }

        property int marginsHeight: {
            if (imagePath === "") {
                return 0;
            } else {
                if (root.panelAlignment === Latte.Dock.Top && root.offset===0)
                    return margins.bottom;
                else if (root.panelAlignment === Latte.Dock.Bottom && root.offset===0)
                    return margins.top;
                else
                    return margins.top + margins.bottom;
            }
        }

        property int panelSize: automaticPanelSize
        property int automaticPanelSize: {
            if (root.behaveAsPlasmaPanel) {
                return root.statesLineSize + root.iconSize + root.thickMargin;
            } else {
                var icons = root.statesLineSize + root.iconSize + root.thickMargin + 1;
                var panels = root.themePanelSize + root.panelMargin;

                root.realPanelThickness = icons;
                if (icons > panels) {
                    return panels;
                } else {
                    return icons;
                }
            }
        }

        property int shadowsSize: 0

        Binding{
            target: shadowsSvgItem
            property: "shadowsSize"
            value:{
                if (shadowsSvgItem && !root.behaveAsPlasmaPanel && root.useThemePanel && root.panelShadowsActive){
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
        }

        Behavior on opacity{
            enabled: Latte.WindowSystem.compositingActive
            NumberAnimation { duration: barLine.animationTime }
        }

        Behavior on opacity{
            enabled: !Latte.WindowSystem.compositingActive
            NumberAnimation { duration: 0 }
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

        Binding {
            target: root
            property: "panelMarginLength"
            when: shadowsSvgItem
            value: root.isVertical ? shadowsSvgItem.marginsHeight : shadowsSvgItem.marginsWidth
        }

        //! This is used to provide real solidness without any transparency from the plasma theme
        Rectangle{
            id: solidBackgroundRectangle
            anchors.fill: solidBackground
            color: theme.backgroundColor
            opacity: behaveAsPlasmaPanel && solidBackground.forceSolidness && plasmoid.configuration.maxLength === 100 ? 1 : 0

            Behavior on opacity{
                enabled: Latte.WindowSystem.compositingActive
                NumberAnimation { duration: 2 * barLine.animationTime }
            }

            Behavior on opacity{
                enabled: !Latte.WindowSystem.compositingActive
                NumberAnimation { duration: 0 }
            }
        }

        PlasmaCore.FrameSvgItem{
            id: solidBackground
            anchors.leftMargin: Latte.WindowSystem.compositingActive ? shadowsSvgItem.margins.left - leftIncreaser : 0
            anchors.rightMargin: Latte.WindowSystem.compositingActive ? shadowsSvgItem.margins.right - rightIncreaser : 0
            anchors.topMargin: Latte.WindowSystem.compositingActive ? shadowsSvgItem.margins.top - topIncreaser : 0
            anchors.bottomMargin: Latte.WindowSystem.compositingActive ? shadowsSvgItem.margins.bottom - bottomIncreaser : 0
            anchors.fill:parent

            opacity: {
                if (forceSolidness) {
                    return 1;
                } else if (!plasmoid.configuration.useThemePanel && plasmoid.configuration.solidBackgroundForMaximized) {
                    return 0;
                } else {
                    return plasmoid.configuration.panelTransparency / 100;
                }
            }

            readonly property bool forceSolidness: (root.solidPanel && !plasmoid.configuration.solidBackgroundForMaximized) || root.forceSolidPanel
                                                   || !Latte.WindowSystem.compositingActive

            property rect efGeometry: Qt.rect(-1,-1,0,0)

            imagePath: root.solidPanel ? "opaque/dialogs/background" : "widgets/panel-background"

            onWidthChanged: updateEffectsArea();
            onHeightChanged: updateEffectsArea();

            Component.onCompleted: root.updateEffectsArea.connect(updateEffectsArea);
            Component.onDestruction: root.updateEffectsArea.disconnect(updateEffectsArea);

            Connections{
                target: root

                onEditModeChanged: {
                    if (!root.editMode){
                        solidBackground.updateEffectsArea();
                    }
                }
            }

            //! Fix for FrameSvgItem QML version not updating its margins after a theme change
            //! with this hack we enforce such update. I could use the repaintNeeded signal but
            //! it is called more often than the themeChanged one.
            Connections{
                target: dock
                onThemeChanged: {
                    plasmoid.configuration.panelShadows = !plasmoid.configuration.panelShadows;
                    plasmoid.configuration.panelShadows = !plasmoid.configuration.panelShadows;
                    updateEffectsArea();
                }
            }

            function updateEffectsArea(){
                if (!dock || root.editMode)
                    return;

                var rootGeometry = mapToItem(root, 0, 0);

                efGeometry.x = rootGeometry.x;
                efGeometry.y = rootGeometry.y;
                efGeometry.width = width;
                efGeometry.height = height;

                dock.effectsArea = efGeometry;

                if (!Latte.WindowSystem.compositingActive) {
                    visibilityManager.updateMaskArea();
                }
            }

            //! the increases used when the user forces a solid background and the background
            //! must be increased in order to look ok in the corners
            property int rightIncreaser: {
                if (!(root.solidPanel && root.isVertical && plasmoid.location === PlasmaCore.Types.LeftEdge)
                        || !Latte.WindowSystem.compositingActive)
                    return 0;
                else
                    return hiddenPanelBackground.margins.right;
            }

            property int leftIncreaser: {
                if (!(root.solidPanel && root.isVertical && plasmoid.location === PlasmaCore.Types.RightEdge)
                        || !Latte.WindowSystem.compositingActive)
                    return 0;
                else
                    return hiddenPanelBackground.margins.left;
            }

            property int topIncreaser: {
                if (!(root.solidPanel && root.isVertical && plasmoid.location === PlasmaCore.Types.BottomEdge)
                        || !Latte.WindowSystem.compositingActive)
                    return 0;
                else
                    return hiddenPanelBackground.margins.top;
            }

            property int bottomIncreaser: {
                if (!(root.solidPanel && root.isVertical && plasmoid.location === PlasmaCore.Types.TopEdge)
                        || !Latte.WindowSystem.compositingActive)
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
                    }
                }
            }

            onRepaintNeeded: {
                if (root.behaveAsPlasmaPanel)
                    adjustPrefix();
            }

            enabledBorders: dock ? dock.enabledBorders : PlasmaCore.FrameSvg.NoBorder

            Behavior on opacity{
                enabled: Latte.WindowSystem.compositingActive
                NumberAnimation { duration: barLine.animationTime }
            }

            Behavior on opacity{
                enabled: !Latte.WindowSystem.compositingActive
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

    transitions: Transition {
        enabled: editModeVisual.plasmaEditMode
        AnchorAnimation {
            duration: 0.8 * root.animationTime
            easing.type: Easing.OutCubic
        }
    }

    //BEGIN states
    //user set Panel Positions
    //0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left
        State {
            name: "leftCenterOrJustify"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Center || root.panelAlignment === Latte.Dock.Justify)

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
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: (root.panelAlignment === Latte.Dock.Center ? root.offset : 0);
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
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:root.offset;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
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
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:root.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Right
        State {
            name: "rightCenterOrJustify"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Center || root.panelAlignment === Latte.Dock.Justify)

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
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: (root.panelAlignment === Latte.Dock.Center ? root.offset : 0);
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
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:root.offset;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
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
            PropertyChanges{
                target: barLine
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:root.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Bottom
        State {
            name: "bottomCenterOrJustify"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Center || root.panelAlignment === Latte.Dock.Justify)

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
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: (root.panelAlignment === Latte.Dock.Center ? root.offset : 0); anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottomLeft"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)
                  &&(((root.panelAlignment === Latte.Dock.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     || ((root.panelAlignment === Latte.Dock.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                anchors.leftMargin: root.offset;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }

        },
        State {
            name: "bottomRight"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)
                  &&(((root.panelAlignment === Latte.Dock.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     ||((root.panelAlignment === Latte.Dock.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                anchors.leftMargin: 0;    anchors.rightMargin:root.offset;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Top
        State {
            name: "topCenterOrJustify"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Center || root.panelAlignment === Latte.Dock.Justify)

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
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: (root.panelAlignment === Latte.Dock.Center ? root.offset : 0); anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topLeft"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)
                  &&(((root.panelAlignment === Latte.Dock.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     || ((root.panelAlignment === Latte.Dock.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                anchors.leftMargin: root.offset;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topRight"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)
                  &&(((root.panelAlignment === Latte.Dock.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                     ||((root.panelAlignment === Latte.Dock.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

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
                anchors.leftMargin: 0;    anchors.rightMargin:root.offset;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        }
    ]
    //END states
}


