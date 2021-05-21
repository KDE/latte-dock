/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item{
    id: visual
    width: abilityItem.isVertical ?  thickness : size
    height: abilityItem.isVertical ? size : thickness

    property int size: 1
    property int thickness: 64

    states:[
        State{
            name: "bottom"
            when: abilityItem.location === PlasmaCore.Types.BottomEdge

            AnchorChanges{
                target: visual;
                anchors.horizontalCenter: parent.horizontalCenter;
                anchors.verticalCenter: undefined;
                anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
            }
        },
        State{
            name: "top"
            when: abilityItem.location === PlasmaCore.Types.TopEdge

            AnchorChanges{
                target:visual;
                anchors.horizontalCenter: parent.horizontalCenter;
                anchors.verticalCenter: undefined;
                anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
            }
        },
        State{
            name: "left"
            when: abilityItem.location === PlasmaCore.Types.LeftEdge

            AnchorChanges{
                target: visual;
                anchors.horizontalCenter: undefined;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
            }
        },
        State{
            name: "right"
            when: abilityItem.location === PlasmaCore.Types.RightEdge

            AnchorChanges{
                target: visual;
                anchors.horizontalCenter: undefined;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
            }
        }
    ]
}
