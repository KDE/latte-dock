/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte 0.2 as Latte

Grid {
    id: appletsContainer

    columns: root.isVertical ? 1 : 0
    columnSpacing: 0
    flow: isHorizontal ? Grid.LeftToRight : Grid.TopToBottom
    rows: root.isHorizontal ? 1 : 0
    rowSpacing: 0

    //Layout.preferredWidth: width
    //Layout.preferredHeight: height

    property int alignment: Latte.Types.BottomEdgeCenterAlign
    property int beginIndex: 0
    property int offset: 0

    readonly property int count: children.length

    //it is used in calculations for fillWidth,fillHeight applets
    property int sizeWithNoFillApplets: 0

    Binding{
        target: appletsContainer
        property:"sizeWithNoFillApplets"
        when: appletsContainer
        value: {
            if (!visibilityManager || !visibilityManager.normalState && !(root.editMode && !root.inConfigureAppletsMode))
                return;

            var space = 0;
            for (var i=0; i<appletsContainer.children.length; ++i){
                if (appletsContainer.children[i] && !appletsContainer.children[i].needsFillSpace && !appletsContainer.children[i].isHidden) {
                    space = root.isHorizontal ? space + appletsContainer.children[i].width : space + appletsContainer.children[i].height;
                }
            }

            return space;
        }
    }

    property int shownApplets: {
        var res = 0;

        for (var i=0; i<children.length; ++i){
            if (children[i] && children[i].applet
                    && (children[i].applet.status === PlasmaCore.Types.HiddenStatus || children[i].isInternalViewSplitter)) {
                //do nothing
            } else if (children[i] && children[i].applet){
                res = res + 1;
            }
        }

        return res;
    }

    //it is used in calculations for fillWidth,fillHeight applets
    property int fillApplets:{
        var no = 0;
        for (var i=0; i<children.length; ++i){
            if (children[i] && children[i].needsFillSpace) {
                //console.log("fill :::: " + children[i].applet.pluginName);
                no++;
            }
        }

        return no;
    }

    property int firstVisibleIndex: -1
    property int lastVisibleIndex: -1

    Binding{
        target: appletsContainer
        property:"firstVisibleIndex"
        when: appletsContainer
        value: {
            if (root.inConfigureAppletsMode) {
                return;
            }

            var ind = -1;
            for (var i=0; i<=children.length-1; ++i){
                if (children[i]
                        && (children[i].index<ind || ind === -1)
                        && children[i].applet
                        && !children[i].isHidden
                        && !children[i].isInternalViewSplitter) {
                    ind = children[i].index;
                }
            }

            return ind;
        }
    }

    Binding{
        target: appletsContainer
        property:"lastVisibleIndex"
        when: appletsContainer
        value: {
            if (root.inConfigureAppletsMode) {
                return;
            }

            var ind = -1;
            for (var i=children.length-1; i>=0; --i){
                if (children[i]
                        && children[i].index>ind
                        && children[i].applet
                        && !children[i].isHidden
                        && !children[i].isInternalViewSplitter) {
                    ind = children[i].index;
                }
            }

            return ind;
        }
    }

    onCountChanged: {
        if (root.editMode) {
            //! this is mainly used when removing/adding internal view splitters
            //! in order to not break the parabolic effect from wrong indexes
            root.updateIndexes();
        }
    }

    onFillAppletsChanged: layoutsContainer.updateSizeForAppletsInFill();
    onShownAppletsChanged: layoutsContainer.updateSizeForAppletsInFill();
    onSizeWithNoFillAppletsChanged: layoutsContainer.updateSizeForAppletsInFill();


    //////////////////////////BEGIN states
    //user set Panel Positions
    // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left Edge
        State {
            name: "leftCenter"
            when: appletsContainer.alignment === Latte.Types.LeftEdgeCenterAlign

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
            when: appletsContainer.alignment === Latte.Types.LeftEdgeTopAlign

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
            when: appletsContainer.alignment === Latte.Types.LeftEdgeBottomAlign

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
            when: appletsContainer.alignment === Latte.Types.RightEdgeCenterAlign

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
            when: appletsContainer.alignment === Latte.Types.RightEdgeTopAlign

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
            when: appletsContainer.alignment === Latte.Types.RightEdgeBottomAlign

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
            when: appletsContainer.alignment === Latte.Types.BottomEdgeCenterAlign

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
            when: appletsContainer.alignment === Latte.Types.BottomEdgeLeftAlign

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
            when: appletsContainer.alignment === Latte.Types.BottomEdgeRightAlign

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
            when: appletsContainer.alignment === Latte.Types.TopEdgeCenterAlign

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
            when: appletsContainer.alignment === Latte.Types.TopEdgeLeftAlign

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
            when: appletsContainer.alignment === Latte.Types.TopEdgeRightAlign

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
