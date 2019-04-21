/*
 * Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7
import QtQuick.Controls 1.4

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Flickable{
    id: flickableContainer
    clip: false
    flickableDirection: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? Flickable.HorizontalFlick : Flickable.VerticalFlick
    interactive: false

    property int offset: 0

    readonly property bool contentsExceed:  !root.vertical ? Math.floor(contentWidth) > width : Math.floor(contentHeight) > height
    readonly property int contentsExtraSpace: {
        if (contentsExceed) {
            if (!root.vertical) {
                return contentWidth - width;
            } else {
                return contentHeight - height;
            }
        }

        return 0;
    }

    readonly property int scrollFirstPos: 0
    readonly property int scrollLastPos: contentsExtraSpace
    readonly property int scrollStep: root.iconSize * 1.5

    readonly property int currentPos: !root.vertical ? contentX : contentY

    readonly property int alignment: {
        if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            if (centered) return Latte.Types.LeftEdgeCenterAlign;
            if (root.panelAlignment === Latte.Types.Top) return Latte.Types.LeftEdgeTopAlign;
            if (root.panelAlignment === Latte.Types.Bottom) return Latte.Types.LeftEdgeBottomAlign;
        }

        if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            if (centered) return Latte.Types.RightEdgeCenterAlign;
            if (root.panelAlignment === Latte.Types.Top) return Latte.Types.RightEdgeTopAlign;
            if (root.panelAlignment === Latte.Types.Bottom) return Latte.Types.RightEdgeBottomAlign;
        }

        if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
            if (centered) return Latte.Types.BottomEdgeCenterAlign;

            if ((root.panelAlignment === Latte.Types.Left && !reversed)
                    || (root.panelAlignment === Latte.Types.Right && reversed)) {
                return Latte.Types.BottomEdgeLeftAlign;
            }

            if ((root.panelAlignment === Latte.Types.Right && !reversed)
                    || (root.panelAlignment === Latte.Types.Left && reversed)) {
                return Latte.Types.BottomEdgeRightAlign;
            }
        }

        if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            if (centered) return Latte.Types.TopEdgeCenterAlign;

            if ((root.panelAlignment === Latte.Types.Left && !reversed)
                    || (root.panelAlignment === Latte.Types.Right && reversed)) {
                return Latte.Types.TopEdgeLeftAlign;
            }

            if ((root.panelAlignment === Latte.Types.Right && !reversed)
                    || (root.panelAlignment === Latte.Types.Left && reversed)) {
                return Latte.Types.TopEdgeRightAlign;
            }
        }

        return Latte.Types.BottomEdgeCenterAlign;
    }


    function increasePos() {
        if (!root.vertical) {
            contentX = Math.min(scrollLastPos, contentX + scrollStep);
        } else {
            contentY = Math.min(scrollLastPos, contentY + scrollStep);
        }

    }

    function decreasePos() {
        if (!root.vertical) {
            contentX = Math.max(scrollFirstPos, contentX - scrollStep);
        } else {
            contentY = Math.max(scrollFirstPos, contentY - scrollStep);
        }
    }

    onContentsExtraSpaceChanged: {
        if (!root.vertical) {
            if (contentX < scrollFirstPos) {
                contentX = scrollFirstPos;
            } else if (contentX > scrollLastPos) {
                contentX = scrollLastPos;
            }
        } else {
            if (contentY < scrollFirstPos) {
                contentY = scrollFirstPos;
            } else if (contentY > scrollLastPos) {
                contentY = scrollLastPos;
            }
        }
    }

    Behavior on contentX {
        NumberAnimation {
            duration: root.durationTime*units.longDuration
            easing.type: Easing.OutQuad
        }
    }

    Behavior on contentY {
        NumberAnimation {
            duration: root.durationTime*units.longDuration
            easing.type: Easing.OutQuad
        }
    }

    //////////////////////////BEGIN states
    //user set Panel Positions
    // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left Edge
        State {
            name: "leftCenter"
            when: flickableContainer.alignment === Latte.Types.LeftEdgeCenterAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: flickableContainer.offset;
            }
        },
        State {
            name: "leftTop"
            when: flickableContainer.alignment === Latte.Types.LeftEdgeTopAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:flickableContainer.offset;    anchors.bottomMargin:flickableContainer.lastMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "leftBottom"
            when: flickableContainer.alignment === Latte.Types.LeftEdgeBottomAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:flickableContainer.lastMargin;    anchors.bottomMargin:flickableContainer.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Right Edge
        State {
            name: "rightCenter"
            when: flickableContainer.alignment === Latte.Types.RightEdgeCenterAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: flickableContainer.offset;
            }
        },
        State {
            name: "rightTop"
            when: flickableContainer.alignment === Latte.Types.RightEdgeTopAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:flickableContainer.offset;    anchors.bottomMargin:flickableContainer.lastMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "rightBottom"
            when: flickableContainer.alignment === Latte.Types.RightEdgeBottomAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:flickableContainer.lastMargin;    anchors.bottomMargin:flickableContainer.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Bottom Edge
        State {
            name: "bottomCenter"
            when: flickableContainer.alignment === Latte.Types.BottomEdgeCenterAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: flickableContainer.offset; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottomLeft"
            when: flickableContainer.alignment === Latte.Types.BottomEdgeLeftAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: flickableContainer.offset;    anchors.rightMargin:flickableContainer.lastMargin;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottomRight"
            when: flickableContainer.alignment === Latte.Types.BottomEdgeRightAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: flickableContainer.lastMargin;    anchors.rightMargin:flickableContainer.offset;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Top Edge
        State {
            name: "topCenter"
            when: flickableContainer.alignment === Latte.Types.TopEdgeCenterAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: flickableContainer.offset; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topLeft"
            when: flickableContainer.alignment === Latte.Types.TopEdgeLeftAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: flickableContainer.offset;    anchors.rightMargin:flickableContainer.lastMargin;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topRight"
            when: flickableContainer.alignment === Latte.Types.TopEdgeRightAlign

            AnchorChanges {
                target: flickableContainer
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: flickableContainer;
                anchors.leftMargin: flickableContainer.lastMargin;    anchors.rightMargin:flickableContainer.offset;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        }
    ]
}
