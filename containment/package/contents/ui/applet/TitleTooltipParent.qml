/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
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

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

Item{
    id: visual
    width: root.isVertical ?  thickness : size
    height: root.isVertical ? size : thickness

    property int size: 1// root.iconSize
    property int thickness: (root.zoomFactor * root.realSize) + root.statesLineSize
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
