/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
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

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.latte 0.1 as Latte

import "../code/HeuristicTools.js" as HeuristicTools

Item{
    id: layoutsContainer

    readonly property bool isHidden: root.inStartup || (dock && dock.visibility && dock.visibility.isHidden)
    readonly property bool useMaxLength: (plasmoid.configuration.panelPosition === Latte.Dock.Justify)
                                         && ((!root.editMode && !root.behaveAsPlasmaPanel )
                                             || (behaveAsPlasmaPanel && root.editMode))

    property int allCount: root.latteApplet ? _mainLayout.count-1+latteApplet.tasksCount : _mainLayout.count
    property int currentSpot: -1000
    property int hoveredIndex: -1

    readonly property int fillApplets: _startLayout.fillApplets + _mainLayout.fillApplets + _endLayout.fillApplets

    property Item startLayout : _startLayout
    property Item mainLayout: _mainLayout
    property Item endLayout: _endLayout

    Binding {
        target: layoutsContainer
        property: "x"
        value: {
            if ( dock && root.isHorizontal && useMaxLength ){
                return ((dock.width/2) - (root.maxLength/2)); // + root.offset)
            } else {
                if ((visibilityManager.inSlidingIn || visibilityManager.inSlidingOut) && root.isVertical){
                    return;
                }

                if (layoutsContainer.isHidden && root.isVertical) {
                    if (Latte.WindowSystem.compositingActive) {
                        return visibilityManager.slidingOutToPos;
                    } else {
                        if ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) {
                            return visibilityManager.slidingOutToPos + 1;
                        } else {
                            return visibilityManager.slidingOutToPos - 1;
                        }
                    }
                } else {
                    return 0;
                }
            }
        }
    }

    Binding{
        target: layoutsContainer
        property: "y"
        value: {
            if ( dock && root.isVertical && useMaxLength ) {
                return ((dock.height/2) - (root.maxLength/2));// + root.offset);
            } else {
                if ((visibilityManager.inSlidingIn || visibilityManager.inSlidingOut) && root.isHorizontal){
                    return;
                }

                if (layoutsContainer.isHidden && root.isHorizontal) {
                    if (Latte.WindowSystem.compositingActive) {
                        return visibilityManager.slidingOutToPos;
                    } else {
                        if ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) {
                            return visibilityManager.slidingOutToPos + 1;
                        } else {
                            return visibilityManager.slidingOutToPos - 1;
                        }
                    }
                } else {
                    return 0;
                }
            }
        }
    }

    width:  root.isHorizontal && useMaxLength ? root.maxLength : parent.width
    height: root.isVertical && useMaxLength ? root.maxLength : parent.height
    z:10

    property bool animationSent: false
    property bool shouldCheckHalfs: (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && (_mainLayout.children>1)

    property int contentsWidth: _startLayout.width + _mainLayout.width + _endLayout.width
    property int contentsHeight: _startLayout.height + _mainLayout.height + _endLayout.height

    onContentsWidthChanged: {
        if (root.isHorizontal){
            var firstHalfExited = false;
            var secondHalfExited = false;

            if (shouldCheckHalfs){
                firstHalfExited = ( (_startLayout.width + _mainLayout.width/2) >= root.maxLength/2 );
                secondHalfExited = ( (_endLayout.width + _mainLayout.width/2) >= root.maxLength/2 );
            }

            if (dock && ((contentsWidth >= root.maxLength) || firstHalfExited || secondHalfExited)) {
                updateAutomaticIconSize();
            }

            if (!animationSent) {
                animationSent = true;
                slotAnimationsNeedLength(1);
            }

            HeuristicTools.updateSizeForAppletsInFill();

            delayUpdateMaskArea.start();
        }
    }

    onContentsHeightChanged: {
        if (root.isVertical){
            var firstHalfExited = false;
            var secondHalfExited = false;

            if (shouldCheckHalfs){
                firstHalfExited = ( (_startLayout.height + _mainLayout.height/2) >= root.maxLength/2 );
                secondHalfExited = ( (_endLayout.height + _mainLayout.height/2) >= root.maxLength/2 );
            }

            if (dock && ((contentsHeight >= root.maxLength) || firstHalfExited || secondHalfExited)) {
                updateAutomaticIconSize();
            }

            if (!animationSent) {
                animationSent = true;
                slotAnimationsNeedLength(1);
            }

            HeuristicTools.updateSizeForAppletsInFill();

            delayUpdateMaskArea.start();
        }
    }

    onXChanged: root.updateEffectsArea();
    onYChanged: root.updateEffectsArea();

    Grid{
        id:_startLayout

        columns: root.isVertical ? 1 : 0
        columnSpacing: 0
        flow: isHorizontal ? Grid.LeftToRight : Grid.TopToBottom
        rows: root.isHorizontal ? 1 : 0
        rowSpacing: 0

        Layout.preferredWidth: width
        Layout.preferredHeight: height

        property int beginIndex: 0
        property int count: children.length

        //it is used in calculations for fillWidth,fillHeight applets
        property int sizeWithNoFillApplets: 0

        Binding{
            target: _startLayout
            property:"sizeWithNoFillApplets"
            when: _startLayout
            value: {
                if (!visibilityManager || !visibilityManager.normalState)
                    return;

                var space = 0;
                for (var i=0; i<_startLayout.children.length; ++i){
                    if (_startLayout.children[i] && !_startLayout.children[i].needsFillSpace) {
                        space = root.isHorizontal ? space + _startLayout.children[i].width : space + _startLayout.children[i].height;
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

        onFillAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();
        onShownAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();
        onSizeWithNoFillAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();

        states:[
            State {
                name: "bottom"
                when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                AnchorChanges {
                    target: _startLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _startLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom;
                    anchors.leftMargin: root.totalPanelEdgeSpacing/2;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                }
            },
            State {
                name: "left"
                when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                AnchorChanges {
                    target: _startLayout
                    anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _startLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:root.totalPanelEdgeSpacing/2;    anchors.bottomMargin:0;
                }
            },
            State {
                name: "right"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                AnchorChanges {
                    target: _startLayout
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _startLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:root.totalPanelEdgeSpacing/2;    anchors.bottomMargin:0;
                }
            },
            State {
                name: "top"
                when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                AnchorChanges {
                    target: _startLayout
                    anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _startLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop;
                    anchors.leftMargin: root.totalPanelEdgeSpacing/2;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                }
            }
        ]
    }

    // This is the main Layout, in contrary with the others
    Grid{
        id: _mainLayout

        columns: root.isVertical ? 1 : 0
        columnSpacing: 0
        flow: isHorizontal ? Grid.LeftToRight : Grid.TopToBottom
        rows: root.isHorizontal ? 1 : 0
        rowSpacing: 0

        Layout.preferredWidth: width
        Layout.preferredHeight: height

        property int beginIndex: 100
        property int count: children.length
        //it is used in calculations for fillWidth,fillHeight applets
        property int sizeWithNoFillApplets: 0

        Binding{
            target: _mainLayout
            property:"sizeWithNoFillApplets"
            when: _mainLayout
            value: {
                //! in EditMode we must update that size because the user may add new applets
                //! and the contents may go out of bounds
                if (!visibilityManager || (!visibilityManager.normalState && !root.editMode))
                    return;

                var space = 0;
                for (var i=0; i<_mainLayout.children.length; ++i){
                    if (_mainLayout.children[i] && !_mainLayout.children[i].needsFillSpace) {
                        space = root.isHorizontal ? space + _mainLayout.children[i].width : space + _mainLayout.children[i].height;
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

        onFillAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();
        onShownAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();
        onSizeWithNoFillAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();

        transitions: Transition {
            enabled: editModeVisual.plasmaEditMode
            AnchorAnimation {
                duration: 0.8 * root.animationTime
                easing.type: Easing.OutCubic
            }
        }

        //////////////////////////BEGIN states
        //user set Panel Positions
        // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
        states: [
            ///Left Edge
            State {
                name: "leftCenter"
                when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&((root.panelAlignment === Latte.Dock.Center)||(root.panelAlignment === Latte.Dock.Justify))

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: root.offset;
                }
            },
            State {
                name: "leftTop"
                when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Top)

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:root.offsetFixed;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State {
                name: "leftBottom"
                when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Bottom)

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:root.offsetFixed;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            ///Right Edge
            State {
                name: "rightCenter"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&((root.panelAlignment === Latte.Dock.Center)||(root.panelAlignment === Latte.Dock.Justify))

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: root.offset;
                }
            },
            State {
                name: "rightTop"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Top)

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:root.offsetFixed;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State {
                name: "rightBottom"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Bottom)

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:root.offsetFixed;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            ///Bottom Edge
            State {
                name: "bottomCenter"
                when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&((root.panelAlignment === Latte.Dock.Center)||(root.panelAlignment === Latte.Dock.Justify))

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: root.offset; anchors.verticalCenterOffset: 0;
                }
            },
            State {
                name: "bottomLeft"
                when: (plasmoid.location === PlasmaCore.Types.BottomEdge)
                      &&(((root.panelAlignment === Latte.Dock.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                         || ((root.panelAlignment === Latte.Dock.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                    anchors.leftMargin: root.offsetFixed;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State {
                name: "bottomRight"
                when: (plasmoid.location === PlasmaCore.Types.BottomEdge)
                      &&(((root.panelAlignment === Latte.Dock.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                         ||((root.panelAlignment === Latte.Dock.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                    anchors.leftMargin: 0;    anchors.rightMargin:root.offsetFixed;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            ///Top Edge
            State {
                name: "topCenter"
                when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&((root.panelAlignment === Latte.Dock.Center)||(root.panelAlignment === Latte.Dock.Justify))

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: root.offset; anchors.verticalCenterOffset: 0;
                }
            },
            State {
                name: "topLeft"
                when: (plasmoid.location === PlasmaCore.Types.TopEdge)
                      &&(((root.panelAlignment === Latte.Dock.Left)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                         || ((root.panelAlignment === Latte.Dock.Right)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                    anchors.leftMargin: root.offsetFixed;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State {
                name: "topRight"
                when: (plasmoid.location === PlasmaCore.Types.TopEdge)
                      &&(((root.panelAlignment === Latte.Dock.Right)&&(Qt.application.layoutDirection !== Qt.RightToLeft))
                         ||((root.panelAlignment === Latte.Dock.Left)&&(Qt.application.layoutDirection === Qt.RightToLeft)))

                AnchorChanges {
                    target: _mainLayout
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                    anchors.leftMargin: 0;    anchors.rightMargin:root.offsetFixed;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            }
        ]
        ////////////////END states

    }

    Grid{
        id:_endLayout

        columns: root.isVertical ? 1 : 0
        columnSpacing: 0
        flow: isHorizontal ? Grid.LeftToRight : Grid.TopToBottom
        rows: root.isHorizontal ? 1 : 0
        rowSpacing: 0

        Layout.preferredWidth: width
        Layout.preferredHeight: height

        property int beginIndex: 200
        property int count: children.length

        //it is used in calculations for fillWidth,fillHeight applets
        property int sizeWithNoFillApplets: 0

        Binding{
            target: _endLayout
            property:"sizeWithNoFillApplets"
            when: _endLayout
            value: {
                if (!visibilityManager || !visibilityManager.normalState)
                    return;

                var space = 0;
                for (var i=0; i<_endLayout.children.length; ++i){
                    if (_endLayout.children[i] && !_endLayout.children[i].needsFillSpace) {
                        space = root.isHorizontal ? space + _endLayout.children[i].width : space + _endLayout.children[i].height;
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

        onFillAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();
        onShownAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();
        onSizeWithNoFillAppletsChanged: HeuristicTools.updateSizeForAppletsInFill();

        states:[
            State {
                name: "bottom"
                when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                AnchorChanges {
                    target: _endLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _endLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                    anchors.leftMargin: 0;    anchors.rightMargin:root.totalPanelEdgeSpacing/2;     anchors.topMargin:0;    anchors.bottomMargin:0;
                }
            },
            State {
                name: "left"
                when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                AnchorChanges {
                    target: _endLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _endLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:root.totalPanelEdgeSpacing/2;
                }
            },
            State {
                name: "right"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                AnchorChanges {
                    target: _endLayout
                    anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _endLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                    anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:root.totalPanelEdgeSpacing/2;
                }
            },
            State {
                name: "top"
                when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                AnchorChanges {
                    target: _endLayout
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: _endLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                    anchors.leftMargin: 0;    anchors.rightMargin:root.totalPanelEdgeSpacing/2;     anchors.topMargin:0;    anchors.bottomMargin:0;
                }
            }
        ]
    }


    function updateSizeForAppletsInFill() {
        HeuristicTools.updateSizeForAppletsInFill();
    }
}
