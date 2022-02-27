/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

Grid {
    id: appletsContainer

    columns: root.isVertical ? 1 : 0
    columnSpacing: 0
    flow: isHorizontal ? Grid.LeftToRight : Grid.TopToBottom
    rows: root.isHorizontal ? 1 : 0
    rowSpacing: 0

    opacity: {
        if (root.inConfigureAppletsMode && root.myView.alignment===LatteCore.Types.Justify && layoutsContainer.mainLayout.isCoveredFromSideLayouts){
            if (dragOverlay && (!dragOverlay.currentHoveredLayout || dragOverlay.currentHoveredLayout === appletsContainer)) {
                return 1;
            } else {
                return 0.3;
            }
        }

        return 1;
    }

    readonly property real length : root.isHorizontal ? width - ignoredLength : height - ignoredLength
    property real ignoredLength: 0

    property int alignment: LatteCore.Types.BottomEdgeCenterAlign
    property int beginIndex: 0
    property int offset: 0

    //////////////////////////BEGIN states
    //user set Panel Positions
    // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left Edge
        State {
            name: "leftCenter"
            when: appletsContainer.alignment === LatteCore.Types.LeftEdgeCenterAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: appletsContainer.offset;
            }
        },
        State {
            name: "leftTop"
            when: appletsContainer.alignment === LatteCore.Types.LeftEdgeTopAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:appletsContainer.offset;    anchors.bottomMargin:appletsContainer.lastMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "leftBottom"
            when: appletsContainer.alignment === LatteCore.Types.LeftEdgeBottomAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:appletsContainer.lastMargin;    anchors.bottomMargin:appletsContainer.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Right Edge
        State {
            name: "rightCenter"
            when: appletsContainer.alignment === LatteCore.Types.RightEdgeCenterAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: appletsContainer.offset;
            }
        },
        State {
            name: "rightTop"
            when: appletsContainer.alignment === LatteCore.Types.RightEdgeTopAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:appletsContainer.offset;    anchors.bottomMargin:appletsContainer.lastMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "rightBottom"
            when: appletsContainer.alignment === LatteCore.Types.RightEdgeBottomAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:appletsContainer.lastMargin;    anchors.bottomMargin:appletsContainer.offset;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Bottom Edge
        State {
            name: "bottomCenter"
            when: appletsContainer.alignment === LatteCore.Types.BottomEdgeCenterAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: appletsContainer.offset; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottomLeft"
            when: appletsContainer.alignment === LatteCore.Types.BottomEdgeLeftAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                anchors.leftMargin: appletsContainer.offset;    anchors.rightMargin:appletsContainer.lastMargin;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "bottomRight"
            when: appletsContainer.alignment === LatteCore.Types.BottomEdgeRightAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                anchors.leftMargin: appletsContainer.lastMargin;    anchors.rightMargin:appletsContainer.offset;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        ///Top Edge
        State {
            name: "topCenter"
            when: appletsContainer.alignment === LatteCore.Types.TopEdgeCenterAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: appletsContainer.offset; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topLeft"
            when: appletsContainer.alignment === LatteCore.Types.TopEdgeLeftAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                anchors.leftMargin: appletsContainer.offset;    anchors.rightMargin:appletsContainer.lastMargin;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "topRight"
            when: appletsContainer.alignment === LatteCore.Types.TopEdgeRightAlign

            AnchorChanges {
                target: appletsContainer
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: appletsContainer; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                anchors.leftMargin: appletsContainer.lastMargin;    anchors.rightMargin:appletsContainer.offset;     anchors.topMargin:0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        }
    ]
    ////////////////END states
}
