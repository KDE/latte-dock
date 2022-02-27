/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Item {
    id: appletsContainer

    property Item grid: null

    // items in grid
    readonly property int count: grid.children.length

    property bool onlyInternalSplitters: false

    //it is used in calculations for fillWidth,fillHeight applets
    property int shownApplets: 0
    property int fillApplets: 0
    property int fillRealApplets: 0 //with no internal separators counted

    property int lengthWithoutSplitters: 0 //with no internal separators length

    //it is used in calculations for fillWidth,fillHeight applets
    property int sizeWithNoFillApplets: 0


    readonly property int maxIndex: 99999
    property int firstVisibleIndex: -1
    property int lastVisibleIndex: -1

    //! do not update during dragging/moving applets inConfigureAppletsMode
    readonly property bool updateIsBlocked:  appletsInParentChange //|| (root.dragOverlay && root.dragOverlay.pressed)

    Binding{
        target: appletsContainer
        property:"sizeWithNoFillApplets"
        when: appletsContainer && grid && !updateIsBlocked && inNormalFillCalculationsState
        value: {
            var space = 0;
            for (var i=0; i<grid.children.length; ++i){
                if (grid.children[i] && (grid.children[i].isPlaceHolder || (!grid.children[i].isAutoFillApplet && !grid.children[i].isHidden && !grid.children[i].isParabolicEdgeSpacer))) {

                    if (!grid.children[i].isPlaceHolder && grid.children[i].isInternalViewSplitter) {
                        space += root.maxJustifySplitterSize;
                    } else {
                        space = root.isHorizontal ? space + grid.children[i].width : space + grid.children[i].height;
                    }
                }
            }

            return space;
        }
    }

    Binding{
        target: appletsContainer
        property:"lengthWithoutSplitters"
        when: appletsContainer && grid && !updateIsBlocked && inNormalFillCalculationsState
              && !(dragOverlay && dragOverlay.currentApplet && dragOverlay.currentApplet.isInternalViewSplitter /*avoid binding loop when dragging splitters around*/)
        value: {
            var space = 0;
            for (var i=0; i<grid.children.length; ++i){
                if (grid.children[i] && (grid.children[i].isPlaceHolder || (!grid.children[i].isInternalViewSplitter && !grid.children[i].isHidden))) {
                    space = root.isHorizontal ? space + grid.children[i].width : space + grid.children[i].height;
                }
            }

            return space;
        }
    }


    Binding{
        target: appletsContainer
        property:"shownApplets"
        when: appletsContainer && grid && !updateIsBlocked
        value: {
            var res = 0;

            for (var i=0; i<grid.children.length; ++i){
                if (grid.children[i] && grid.children[i].isHidden) {
                    //do nothing
                } else if (grid.children[i] && (grid.children[i].isPlaceHolder || grid.children[i].applet)){/*internal splitters are ignored this way*/
                    res = res + 1;
                }
            }

            return res;
        }
    }


    Binding{
        target: appletsContainer
        property:"fillApplets"
        when: appletsContainer && grid && !updateIsBlocked
        value: {
            var no = 0;
            for (var i=0; i<grid.children.length; ++i){
                if (grid.children[i]
                        && !grid.children[i].isParabolicEdgeSpacer
                        && !grid.children[i].isPlaceHolder
                        && grid.children[i].isAutoFillApplet
                        && !grid.children[i].isHidden) {
                    //console.log("fill :::: " + children[i].applet.pluginName);
                    no++;
                }
            }

            return no;
        }
    }

    Binding{
        target: appletsContainer
        property:"fillRealApplets"
        when: appletsContainer && grid && !updateIsBlocked
        value: {
            var no = 0;
            for (var i=0; i<grid.children.length; ++i){
                if (grid.children[i]
                        && !grid.children[i].isParabolicEdgeSpacer
                        && !grid.children[i].isPlaceHolder
                        && grid.children[i].isRequestingFill
                        && grid.children[i].applet
                        && !grid.children[i].isHidden) {
                    //console.log("fill :::: " + children[i].applet.pluginName);
                    no++;
                }
            }

            return no;
        }
    }

    Binding{
        target: appletsContainer
        property:"firstVisibleIndex"
        when: appletsContainer && grid && !updateIsBlocked
        value: {
            var ind = maxIndex;
            for(var i=0; i<grid.children.length; ++i) {
                var appletItem = grid.children[i];
                if (appletItem && appletItem.index>=0
                        && indexer.hidden.indexOf(appletItem.index)<0
                        && indexer.separators.indexOf(appletItem.index)<0
                        && appletItem.index < ind) {
                    ind = appletItem.index;
                }
            }

            return ind === maxIndex ? -1 : ind;
        }
    }

    Binding{
        target: appletsContainer
        property:"lastVisibleIndex"
        when: appletsContainer && grid && !updateIsBlocked
        value: {
            var ind = -1;
            for(var i=0; i<grid.children.length; ++i) {
                var appletItem = grid.children[i];
                if (appletItem && appletItem.index>=0
                        && indexer.hidden.indexOf(appletItem.index)<0
                        && indexer.separators.indexOf(appletItem.index)<0
                        && appletItem.index > ind) {
                    ind = appletItem.index;
                }
            }

            return ind;
        }
    }

    Binding{
        target: appletsContainer
        property:"onlyInternalSplitters"
        when: appletsContainer && grid && !updateIsBlocked && inNormalFillCalculationsState
        value: {
            var intsSplits = 0;

            for (var i=0; i<grid.children.length; ++i){
                if (grid.children[i]
                        && !grid.children[i].isParabolicEdgeSpacer
                        && !grid.children[i].isPlaceHolder
                        && grid.children[i].isInternalViewSplitter
                        && !grid.children[i].isHidden) {
                    intsSplits = intsSplits + 1;
                }
            }

            return ((intsSplits > 0) && (intsSplits === grid.children.length));
        }
    }

    onCountChanged: {
        if (root.editMode) {
            //! this is mainly used when removing/adding internal view splitters
            //! in order to not break the parabolic effect from wrong indexes
            root.updateIndexes();
        }
    }
}
