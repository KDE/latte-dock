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
import org.kde.plasma.core 2.0 as PlasmaCore


Item {

    readonly property int thickness: wrapper.marginsThickness / 2
    readonly property int length: root.isHorizontal ? wrapper.width : wrapper.height

    Rectangle {
        id: headPadding
        width: root.isHorizontal ? length : wrapper.headThicknessMargin
        height: root.isHorizontal ? wrapper.headThicknessMargin : length
        color: colorizerManager.highlightColor

        states:[
            State{
                name: "horizontal"
                when: root.isHorizontal

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "vertical"
                when: root.isVertical

                AnchorChanges{
                    target: headPadding
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            }
        ]
    }

    Rectangle {
        id: tailPadding
        width: root.isHorizontal ? length : wrapper.tailThicknessMargin
        height: root.isHorizontal ? wrapper.tailThicknessMargin : length
        color: colorizerManager.highlightColor

        states:[
            State{
                name: "horizontal"
                when: root.isHorizontal

                AnchorChanges{
                    target: tailPadding
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }                
            },
            State{
                name: "vertical"
                when: root.isVertical

                AnchorChanges{
                    target: tailPadding
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
            }
        ]
    }
}
