/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

Item {
    id: appletsContainer

    property Item grid: null

    // items in grid
    readonly property int count: grid.children.length

    property bool onlyInternalSplitters: false

    //it is used in calculations for fillWidth,fillHeight applets
    property int shownApplets: 0
    property int fillApplets: 0

    //it is used in calculations for fillWidth,fillHeight applets
    property int sizeWithNoFillApplets: 0

    readonly property int maxIndex: 99999
    property int firstVisibleIndex: -1
    property int lastVisibleIndex: -1

    //! do not update during dragging/moving applets inConfigureAppletsMode
    readonly property bool updateIsBlocked: (root.dragOverlay && root.dragOverlay.pressed) || appletsInParentChange

    Binding{
        target: appletsContainer
        property:"sizeWithNoFillApplets"
        when: appletsContainer && grid && !updateIsBlocked && inNormalFillCalculationsState
        value: {
            var space = 0;
            for (var i=0; i<grid.children.length; ++i){
                if (grid.children[i]
                        && !grid.children[i].isAutoFillApplet
                        && !grid.children[i].isHidden) {
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
                } else if (grid.children[i] && grid.children[i].applet){/*internal splitters are ignored this way*/
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
                if (grid.children[i] && grid.children[i].isAutoFillApplet) {
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
                        && grid.children[i].isPlaceHolder
                        || (grid.children[i].isInternalViewSplitter
                            && !grid.children[i].isHidden)) {
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

    onFillAppletsChanged: layouter.updateSizeForAppletsInFill();
    onShownAppletsChanged: layouter.updateSizeForAppletsInFill();
    onSizeWithNoFillAppletsChanged: layouter.updateSizeForAppletsInFill();
}
