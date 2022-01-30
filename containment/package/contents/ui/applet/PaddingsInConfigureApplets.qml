/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: paddingsVisual

    readonly property int headThickness: {
        if ((appletItem.canFillThickness && !appletItem.isMarginsAreaSeparator && !communicator.indexerIsSupported)
                || (appletItem.canFillScreenEdge && !communicator.indexerIsSupported) ) {
            return 1;
        } else if (appletItem.isMarginsAreaSeparator) {
            return appletItem.metrics.marginsArea.headThickness + maxMarginAreaSeparatorGap;
        } else if (appletItem.inMarginsArea) {
            return appletItem.metrics.marginsArea.headThickness;
        }

        return appletItem.metrics.margin.headThickness;
    }

    readonly property int tailThickness: {
        if ((appletItem.canFillThickness && !appletItem.isMarginsAreaSeparator && !communicator.indexerIsSupported)
                || (appletItem.canFillScreenEdge && !communicator.indexerIsSupported) ) {
            return 1;
        } else if (appletItem.isMarginsAreaSeparator) {
            return appletItem.metrics.marginsArea.tailThickness + maxMarginAreaSeparatorGap;
        } else if (appletItem.inMarginsArea) {
            return appletItem.metrics.marginsArea.tailThickness;
        }

        return appletItem.metrics.margin.tailThickness;
    }

    readonly property int length: root.isHorizontal ? wrapper.width : wrapper.height

    readonly property int parentThickness: root.isHorizontal ? wrapper.height : wrapper.width
    readonly property int maxMarginAreaSeparatorCenteredRectLength: Math.max(0.6 * paddingsVisual.length, 4)
    readonly property int marginAreaSeparatorFreeThickness: ((parentThickness - appletItem.metrics.marginsArea.headThickness - appletItem.metrics.marginsArea.tailThickness - maxMarginAreaSeparatorCenteredRectLength - 10) / 2)
    readonly property int maxMarginAreaSeparatorGap: Math.max(3, 0.5 * marginAreaSeparatorFreeThickness)

    property color color: "blue"

    Rectangle {
        id: tailPadding
        anchors.topMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? appletItem.metrics.margin.screenEdge : 0
        anchors.leftMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? appletItem.metrics.margin.screenEdge : 0
        anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? appletItem.metrics.margin.screenEdge : 0
        anchors.rightMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? appletItem.metrics.margin.screenEdge : 0

        width: root.isHorizontal ? length : tailThickness
        height: root.isHorizontal ? tailThickness : length

        color: parent.color

        states:[
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: tailPadding
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
            },
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: tailPadding
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: tailPadding
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: tailPadding
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            }
        ]
    }

    Rectangle {
        id: headPadding
        anchors.topMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? normalMargin : 0
        anchors.leftMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? normalMargin : 0
        anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? normalMargin : 0
        anchors.rightMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? normalMargin : 0

        width: root.isHorizontal ? length : headThickness
        height: root.isHorizontal ? headThickness : length
        color: parent.color

        readonly property int normalMargin: appletItem.metrics.mask.thickness.normalForItems - headThickness

        states:[
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
            },
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            }
        ]
    }

    Loader {
        id: marginsAreaSeparatorVisual
        active: appletItem.isMarginsAreaSeparator && root.inConfigureAppletsMode
        sourceComponent: Item {
            width: plasmoid.formFactor === PlasmaCore.Types.Vertical ? appletItem.metrics.totals.thickness : paddingsVisual.length
            height: plasmoid.formFactor === PlasmaCore.Types.Vertical ? paddingsVisual.length : appletItem.metrics.totals.thickness

            Rectangle {
                anchors.centerIn: parent
                width: paddingsVisual.maxMarginAreaSeparatorCenteredRectLength
                height: width
                radius: 2
                color: paddingsVisual.color
            }
        }

        states:[
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: marginsAreaSeparatorVisual
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: marginsAreaSeparatorVisual
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: marginsAreaSeparatorVisual
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: marginsAreaSeparatorVisual
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
            }
        ]
    }
}
