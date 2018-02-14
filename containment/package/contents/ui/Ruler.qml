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

    width: root.isHorizontal ? root.maxLength : thickness
    height: root.isVertical ? root.maxLength : thickness

    opacity: root.editMode ? 1 : 0

    property int rulerAnimationTime: 0.8 * root.animationTime

    readonly property bool containsMouse: rulerMouseArea.containsMouse
    readonly property int thickness: theme.defaultFont.pixelSize + rMargin

    readonly property string tooltip: i18n("You can use mouse wheel to change the maximum length")

    x: {
        if (root.isHorizontal) {
            return xL;
        } else {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge){
                return editModeVisual.x + editModeVisual.width - theme.defaultFont.pixelSize;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge){
                return editModeVisual.x;
            }
        }
    }

    y: {
        if (root.isVertical) {
            return yL;
        } else {
            if (plasmoid.location === PlasmaCore.Types.BottomEdge){
                return editModeVisual.y;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge){
                return editModeVisual.y + editModeVisual.height - theme.defaultFont.pixelSize;
            }
        }

    }

    property int length: root.maxLength

    property int rMargin: 3
    property int xL: {
        if (root.isHorizontal) {
            if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                return root.width/2 - rulerItem.length/2 + root.offset;
            } else if (root.panelAlignment === Latte.Dock.Left) {
                return root.offset;
            } else if (root.panelAlignment === Latte.Dock.Center) {
                return root.width/2 - rulerItem.length/2 + root.offset;
            } else if (root.panelAlignment === Latte.Dock.Right) {
                return root.width - rulerItem.length - root.offset;
            }
        } else {
            return ;
        }
    }

    property int yL: {
        if (root.isVertical) {
            if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                return root.height/2 - rulerItem.length/2 + root.offset;
            } else if (root.panelAlignment === Latte.Dock.Top) {
                return root.offset;
            } else if (root.panelAlignment === Latte.Dock.Center) {
                return root.height/2 - rulerItem.length/2 + root.offset;
            } else if (root.panelAlignment === Latte.Dock.Bottom) {
                return root.height - rulerItem.length - root.offset;
            }
        } else {
            return;
        }

    }


    property int foregroundTextShadow: {
        if ((editModeVisual.layoutColor === "gold")
                || (editModeVisual.layoutColor === "pink")
                || (editModeVisual.layoutColor === "wheat"))  {
            return 1;
        } else {
            return 6;
        }
    }

    property color foregroundColor:{
        if (editModeVisual.layoutColor === "blue") {
            return "#D7E3FF";
        } else if (editModeVisual.layoutColor === "brown") {
            return "#F0D7BA";
        } else if (editModeVisual.layoutColor === "darkgrey") {
            return "#ECECEC";
        } else if (editModeVisual.layoutColor === "gold") {
            return "#78573B";
        } else if (editModeVisual.layoutColor === "green") {
            return "#D0E7CD";
        } else if (editModeVisual.layoutColor === "lightskyblue") {
            return "#E3F8FF";
        } else if (editModeVisual.layoutColor === "orange") {
            return "#FFEADE";
        } else if (editModeVisual.layoutColor === "pink") {
            return "#743C46";
        } else if (editModeVisual.layoutColor === "purple") {
            return "#ECD9FF";
        } else if (editModeVisual.layoutColor === "red") {
            return "#FFCACA";
        } else if (editModeVisual.layoutColor === "wheat") {
            return "#705548";
        } else {
            return "#D7E3FF";
        }
    }

    Behavior on width {
        NumberAnimation {
            id: horizontalAnimation
            duration: rulerAnimationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on height {
        NumberAnimation {
            id: verticalAnimation
            duration: rulerAnimationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on x {
        enabled: root.isHorizontal && !offsetAnimation.running
        NumberAnimation {
            duration: rulerAnimationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on y {
        enabled: root.isVertical && !offsetAnimation.running
        NumberAnimation {
            duration: rulerAnimationTime
            easing.type: Easing.OutCubic
        }
    }

    Grid{
        id: rulerGrid
        width: root.isHorizontal ? parent.length : undefined
        height: root.isVertical ? parent.length : undefined

        rows: root.isHorizontal ? 1 : 0
        columns: root.isVertical ? 1 : 0
        spacing: 2

        flow: root.isHorizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight

        x: {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                return -rMargin;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                return rMargin;
            } else {
                return 0;
            }
        }

        y: {
            if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                return rMargin;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                return -rMargin;
            } else {
                return 0;
            }
        }

        property int freeSpace: {
            if (root.isHorizontal) {
                return rulerItem.width - rulerGrid.spacing - 1 //((rulerGrid.children.length-2) * rulerGrid.spacing)
                        - (startLine.width + startArrow.width + labelItem.width + endArrow.width + endArrow.width);
            } else {
                return rulerItem.height - rulerGrid.spacing - 1 //((rulerGrid.children.length-2) * rulerGrid.spacing)
                        - (startLine.height + startArrow.height + labelItem.height + endArrow.height + endArrow.height);
            }
        }

        layer.enabled: true
        layer.effect: DropShadow{
            radius: foregroundTextShadow
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
    } // end of grid
}
