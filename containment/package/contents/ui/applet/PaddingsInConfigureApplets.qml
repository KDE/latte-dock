/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: paddingsVisual

    readonly property int thickness: {
        if ((appletItem.canFillThickness && !appletItem.isMarginsAreaSeparator && !communicator.indexerIsSupported)
                || (appletItem.canFillScreenEdge && !communicator.indexerIsSupported) ) {
            return 1;
        } else if (appletItem.inMarginsArea || appletItem.isMarginsAreaSeparator) {
            return appletItem.metrics.marginsArea.marginThickness;
        }

        return appletItem.metrics.margin.thickness;
    }
    readonly property int length: root.isHorizontal ? wrapper.width : wrapper.height

    property color color: "blue"

    Rectangle {
        id: tailPadding
        anchors.topMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? appletItem.metrics.margin.screenEdge : 0
        anchors.leftMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? appletItem.metrics.margin.screenEdge : 0
        anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? appletItem.metrics.margin.screenEdge : 0
        anchors.rightMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? appletItem.metrics.margin.screenEdge : 0

        width: root.isHorizontal ? length : thickness
        height: root.isHorizontal ? thickness : length

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

        width: root.isHorizontal ? length : thickness
        height: root.isHorizontal ? thickness : length
        color: parent.color

        states:[
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
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
                width: Math.max(0.6 * paddingsVisual.length, 4)
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
