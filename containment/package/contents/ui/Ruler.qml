/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7

import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.1 as Latte

Item{
    id: rulerItem

    opacity: behaveAsPlasmaPanel ? 0.7 : 1

    x: {
        if (root.isHorizontal) {
            return xL;
        } else {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge){
                return editModeVisual.x + editModeVisual.width - theme.defaultFont.pixelSize - rMargin ;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge){
                return editModeVisual.x + rMargin ;
            }
        }
    }

    y: {
        if (root.isVertical) {
            return yL;
        } else {
            if (plasmoid.location === PlasmaCore.Types.BottomEdge){
                return editModeVisual.y + rMargin;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge){
                return editModeVisual.y + editModeVisual.height - theme.defaultFont.pixelSize - rMargin;
            }
        }

    }

    property int length: root.isHorizontal ? (root.behaveAsPlasmaPanel ? root.width - root.maxIconSize/4 : root.maxLength):
                                             (root.behaveAsPlasmaPanel ? root.height - root.maxIconSize/4 : root.maxLength)

    property int rMargin: 3
    property int xL: 0
    property int yL: 0

    property color foregroundColor: "#d7e3ff"

    Connections{
        target: plasmoid
        onLocationChanged: initializeEditPosition();
    }

    Connections{
        target: root
        onMaxIconSizeChanged: initializeEditPosition();
        onPanelAlignmentChanged: initializeEditPosition();
        onOffsetChanged: initializeEditPosition();
        onMaxLengthChanged: initializeEditPosition();
        onEditModeChanged: {
            if (editMode) {
                initializeEditPosition();
            }
        }
    }

    Connections{
        target: editModeVisual

        onRootThicknessChanged: {
            rulerItem.initializeEditPosition();
        }

        onThicknessChanged: {
            rulerItem.initializeEditPosition();
        }
    }

    Behavior on width {
        NumberAnimation {
            duration: 250
            easing.type: Easing.OutCubic
        }
    }

    Behavior on height {
        NumberAnimation {
            duration: 250
            easing.type: Easing.OutCubic
        }
    }

    Behavior on x {
        enabled: root.isHorizontal
        NumberAnimation {
            duration: 250
            easing.type: Easing.OutCubic
        }
    }

    Behavior on y {
        enabled: root.isVertical
        NumberAnimation {
            duration: 250
            easing.type: Easing.OutCubic
        }
    }

   /* Behavior on opacity {
        NumberAnimation {
            duration: 250
            easing.type: Easing.OutCubic
        }
    }*/

    Grid{
        id: rulerGrid
        width: root.isHorizontal ? parent.width : undefined
        height: root.isVertical ? parent.height : undefined

        rows: root.isHorizontal ? 1 : 0
        columns: root.isVertical ? 1 : 0
        rowSpacing: 2
        columnSpacing: 2

        flow: root.isHorizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight

        property int freeSpace: {
            if (root.isHorizontal) {
                return rulerItem.width - (startLine.width + startArrow.width + labelItem.width + endArrow.width + endArrow.width);
            } else {
                return rulerItem.height - (startLine.height + startArrow.height + labelItem.height + endArrow.height + endArrow.height);
            }
        }

        layer.enabled: true
        layer.effect: DropShadow{
            radius: 6
            fast: true
            samples: 2 * radius
            color: "#040404"
        }

        Rectangle{
            id: startLine
            width: root.isHorizontal ? 2 : theme.defaultFont.pixelSize
            height: root.isVertical ? 2 : theme.defaultFont.pixelSize

            color: foregroundColor
        }

        Item{
            id: startArrow
            width: root.isHorizontal ? 0.6 * theme.defaultFont.pixelSize : theme.defaultFont.pixelSize
            height: root.isVertical ? 0.6 * theme.defaultFont.pixelSize : theme.defaultFont.pixelSize

            clip:true

            Rectangle{
                anchors.verticalCenter: root.isHorizontal ? parent.verticalCenter : parent.bottom
                anchors.horizontalCenter: root.isHorizontal ? parent.right : parent.horizontalCenter
                width: 0.75*theme.defaultFont.pixelSize
                height: width
                rotation: 45

                color: foregroundColor
            }
        }

        Item{
            id: startSpacer
            width: root.isHorizontal ? rulerGrid.freeSpace / 2 : theme.defaultFont.pixelSize
            height: root.isVertical ? rulerGrid.freeSpace / 2 : theme.defaultFont.pixelSize

            Rectangle{
                height: root.isHorizontal ? 2 : parent.height
                width: root.isVertical ? 2 : parent.width

                anchors.centerIn: parent

                color: foregroundColor
            }
        }

        Item {
            id: labelItem
            width: root.isHorizontal ? labelMetricsRec.width : labelMetricsRec.height / 2
            height: root.isVertical ? labelMetricsRec.width : labelMetricsRec.height / 2

            PlasmaComponents.Label{
                id: maxLengthLbl

                anchors.centerIn: parent

                text: i18n("Maximum Length")
                color: foregroundColor

                transformOrigin: Item.Center

                rotation: {
                    if (root.isHorizontal) {
                        return 0;
                    } else if (plasmoid.location === PlasmaCore.Types.LeftEdge){
                        return 90;
                    } else if (plasmoid.location === PlasmaCore.Types.RightEdge){
                        return -90;
                    }
                }

                Rectangle {
                    id: labelMetricsRec
                    anchors.fill: parent
                    visible: false
                }
            }
        }

        Item{
            id: endSpacer
            width: startSpacer.width
            height: startSpacer.height

            Rectangle{
                height: root.isHorizontal ? 2 : parent.height
                width: root.isVertical ? 2 : parent.width

                anchors.centerIn: parent

                color: foregroundColor
            }
        }

        Item{
            id: endArrow
            width: root.isHorizontal ? 0.6 * theme.defaultFont.pixelSize : theme.defaultFont.pixelSize
            height: root.isVertical ? 0.6 * theme.defaultFont.pixelSize : theme.defaultFont.pixelSize
            clip:true

            Rectangle{
                anchors.verticalCenter: root.isHorizontal ? parent.verticalCenter : parent.top
                anchors.horizontalCenter: root.isHorizontal ? parent.left : parent.horizontalCenter
                width: 0.75*theme.defaultFont.pixelSize
                height: width
                rotation: 45

                color: foregroundColor
            }
        }

        Rectangle{
            id: endLine
            width: root.isHorizontal ? 2 : theme.defaultFont.pixelSize
            height: root.isVertical ? 2 : theme.defaultFont.pixelSize

            color: foregroundColor
        }
    }


    function initializeEditPosition() {
        if (root.editMode) {
            /* if (plasmoid.location === PlasmaCore.Types.LeftEdge){
                x = 0;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                y =editModeVisual.thickness - 1.5 *theme.defaultFont.pixelSize;
            } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                y = editModeVisual.rootThickness - editModeVisual.thickness + 0.5 * theme.defaultFont.pixelSize;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                x = editModeVisual.rootThickness - editModeVisual.thickness + 0.5 * theme.defaultFont.pixelSize;
            }*/

            if (root.isHorizontal) {
                if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                    xL = root.width/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Left) {
                    xL = root.offset;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    xL = root.width/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Right) {
                    xL = root.width - rulerItem.length - root.offset;
                }
            } else if (root.isVertical) {
                if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                    yL = root.height/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Top) {
                    yL = root.offset;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    yL = root.height/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Bottom) {
                    yL = root.height - rulerItem.length - root.offset;
                }
            }
        }
    }
}
