/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item{
    id: visual
    width: root.isVertical ?  thickness : size
    height: root.isVertical ? size : thickness

    property int size: 1
    property int thickness: Math.min(Math.max(minimumThickness, preferredThickness), maximumThickness)
    property int minimumThickness: 0
    readonly property int preferredThickness: (parabolic.factor.zoom * metrics.totals.thickness) + metrics.margin.screenEdge
    property int maximumThickness: 9999

    property Item metrics: null
    property Item parabolic: null
    //border.width: 1
    //border.color: "green"
    //color: "transparent"

    states:[
        State{
            name: "bottom"
            when: plasmoid.location === PlasmaCore.Types.BottomEdge

            AnchorChanges{
                target: visual;
                anchors.horizontalCenter: parent.horizontalCenter;
                anchors.verticalCenter: undefined;
                anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
            }
        },
        State{
            name: "top"
            when: plasmoid.location === PlasmaCore.Types.TopEdge

            AnchorChanges{
                target:visual;
                anchors.horizontalCenter: parent.horizontalCenter;
                anchors.verticalCenter: undefined;
                anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
            }
        },
        State{
            name: "left"
            when: plasmoid.location === PlasmaCore.Types.LeftEdge

            AnchorChanges{
                target: visual;
                anchors.horizontalCenter: undefined;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
            }
        },
        State{
            name: "right"
            when: plasmoid.location === PlasmaCore.Types.RightEdge

            AnchorChanges{
                target: visual;
                anchors.horizontalCenter: undefined;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
            }
        }
    ]
}
