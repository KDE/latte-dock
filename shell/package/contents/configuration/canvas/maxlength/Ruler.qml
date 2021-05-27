/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte.core 0.2 as LatteCore

Item{
    id: rulerItem

    width: root.isHorizontal ? userMaxLength : thickness
    height: root.isVertical ? userMaxLength : thickness

    property int rulerAnimationTime: root.animationSpeed
    property int thicknessMargin: 0

    readonly property bool containsMouse: rulerMouseArea.containsMouse
    readonly property int thickness: theme.defaultFont.pixelSize

    readonly property string tooltip: i18nc("maximum length tooltip, %1 is maximum length percentage","You can use mouse wheel to change maximum length of %1%",plasmoid.configuration.maxLength)

    readonly property int userMaxLength: {
        if (root.isHorizontal) {
            return root.width * (plasmoid.configuration.maxLength/100);
        } else {
            return root.height * (plasmoid.configuration.maxLength/100);
        }
    }

    x: {
        if (root.isHorizontal) {
            return xL;
        } else {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge){
                return settingsRoot.width - thickness - thicknessMargin;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge){
                return thicknessMargin;
            }
        }
    }

    y: {
        if (root.isVertical) {
            return yL;
        } else {
            if (plasmoid.location === PlasmaCore.Types.BottomEdge){
                return thicknessMargin;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge){
                return settingsRoot.height - thickness - thicknessMargin;
            }
        }

    }

    property int length: userMaxLength

    property int thickMargin: 3
    property int xL: 0
    property int yL: 0

    Binding{
        target: ruler
        property: "xL"
        value: {
            if (root.isHorizontal) {
                if (plasmoid.configuration.alignment === LatteCore.Types.Justify) {
                    return root.width/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === LatteCore.Types.Left) {
                    return root.offset;
                } else if (root.panelAlignment === LatteCore.Types.Center) {
                    return root.width/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === LatteCore.Types.Right) {
                    return root.width - rulerItem.length - root.offset;
                }
            } else {
                return ;
            }
        }
    }

    Binding{
        target: ruler
        property: "yL"
        value: {
            if (root.isVertical) {
                if (plasmoid.configuration.alignment === LatteCore.Types.Justify) {
                    return root.height/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === LatteCore.Types.Top) {
                    return root.offset;
                } else if (root.panelAlignment === LatteCore.Types.Center) {
                    return root.height/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === LatteCore.Types.Bottom) {
                    return root.height - rulerItem.length - root.offset;
                }
            } else {
                return;
            }
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
                return -thickMargin;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                return thickMargin;
            } else {
                return 0;
            }
        }

        y: {
            if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                return thickMargin;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                return -thickMargin;
            } else {
                return 0;
            }
        }

        Behavior on width {
            NumberAnimation {
                duration: rulerAnimationTime
                easing.type: Easing.OutCubic
            }
        }

        Behavior on height {
            NumberAnimation {
                duration: rulerAnimationTime
                easing.type: Easing.OutCubic
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

        Rectangle{
            id: startLine
            width: root.isHorizontal ? 2 : theme.defaultFont.pixelSize
            height: root.isVertical ? 2 : theme.defaultFont.pixelSize

            color: settingsRoot.textColor
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

                color: settingsRoot.textColor
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

                color: settingsRoot.textColor
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
                color: settingsRoot.textColor

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

                color: settingsRoot.textColor
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

                color: settingsRoot.textColor
            }
        }

        Rectangle{
            id: endLine
            width: root.isHorizontal ? 2 : theme.defaultFont.pixelSize
            height: root.isVertical ? 2 : theme.defaultFont.pixelSize

            color: settingsRoot.textColor
        }
    } // end of grid

    RulerMouseArea {
        id: rulerMouseArea
        anchors.fill: parent
    }

    PlasmaComponents.Button {
        anchors.fill: parent
        opacity: 0
        tooltip: rulerItem.tooltip
    }
}
