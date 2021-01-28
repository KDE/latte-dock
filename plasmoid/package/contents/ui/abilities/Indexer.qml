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

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.abilities.client 0.1 as AbilityClient

AbilityClient.Indexer {
    id: _indexer
    property Item layout: null

    property bool updateIsBlocked: false

    readonly property bool tailAppletIsSeparator: isActive ? bridge.indexer.tailAppletIsSeparator : false
    readonly property bool headAppletIsSeparator: isActive ? bridge.indexer.headAppletIsSeparator : false

    readonly property bool isReady: !updateIsBlocked
    readonly property int maxIndex: 99999

    readonly property alias visibleItemsCount: _privates.visibleItemsCount
    readonly property alias firstVisibleItemIndex: _privates.firstVisibleItemIndex
    readonly property alias lastVisibleItemIndex: _privates.lastVisibleItemIndex

    QtObject {
        id: _privates
        property int firstVisibleItemIndex: -1
        property int lastVisibleItemIndex: -1
        property int visibleItemsCount: 0
    }

    Binding {
        target: _privates
        property: "firstVisibleItemIndex"
        when: isReady
        value: {
            var ind = maxIndex;
            for(var i=0; i<layout.children.length; ++i) {
                var item = layout.children[i];
                if (item && item.itemIndex>=0
                        && hidden.indexOf(item.itemIndex)<0
                        && separators.indexOf(item.itemIndex)<0
                        && item.itemIndex < ind) {
                    ind = item.itemIndex;
                }
            }

            return ind === maxIndex ? -1 : ind;
        }
    }

    Binding {
        target: _privates
        property: "lastVisibleItemIndex"
        when: isReady
        value: {
            var ind = -1;

            for(var i=0; i<layout.children.length; ++i) {
                var item = layout.children[i];
                if (item && item.itemIndex>=0
                        && hidden.indexOf(item.itemIndex)<0
                        && separators.indexOf(item.itemIndex)<0
                        && item.itemIndex > ind) {

                    //console.log("org.kde.latte SETTING UP ::: " + item.itemIndex + " / " + layout.children.length);
                    ind = item.itemIndex;
                }
            }

            return ind;
        }
    }

    Binding {
        target: _privates
        property: "visibleItemsCount"
        value: {
            var count = 0;
            for(var i=0; i<layout.children.length; ++i) {
                var item = layout.children[i];
                if (item && item.itemIndex>=0
                        && hidden.indexOf(item.itemIndex)<0
                        && separators.indexOf(item.itemIndex)<0) {
                    count = count + 1;
                }
            }

            return count;
        }
    }

    Binding {
        target: _indexer
        property: "separators"
        when: isReady
        value: {
            var seps = [];

            for (var i=0; i<layout.children.length; ++i){
                var item = layout.children[i];
                if (item && item.isSeparator && item.itemIndex>=0) {
                    seps.push(item.itemIndex);
                }
            }

            return seps;
        }
    }

    readonly property bool firstTailItemIsSeparator: {
        if (visibleItemsCount === layout.children.length) {
            return false;
        }

        for(var i=0; i<firstVisibleItemIndex; ++i) {
            if (separators.indexOf(i)>=0) {
                return true;
            }
        }

        return false;
    }

    readonly property bool lastHeadItemIsSeparator: {
        if (visibleItemsCount === layout.children.length) {
            return false;
        }

        var len = layout.children.length;

        for(var i=len-1; i>lastVisibleItemIndex; --i) {
            if (separators.indexOf(i)>=0) {
                return true;
            }
        }

        return false;
    }

    hidden: {
        var hdns = [];

        for (var i=0; i<layout.children.length; ++i){
            var item = layout.children[i];
            if (item && item.isHidden && item.itemIndex>=0) {
                hdns.push(item.itemIndex);
            }
        }

        return hdns;
    }

    function visibleIndex(taskIndex) {
        if (taskIndex<firstVisibleItemIndex
                || taskIndex>lastVisibleItemIndex
                || hidden.indexOf(taskIndex) >= 0
                || separators.indexOf(taskIndex) >= 0) {
            return -1;
        }

        var vindex = -1;
        if (latteBridge) {
            vindex = latteBridge.indexer.host.visibleIndex(latteBridge.indexer.appletIndex);
        }

        for (var i=0; i<layout.children.length; ++i){
            var item = layout.children[i];
            if (hidden.indexOf(item.itemIndex)>=0 || separators.indexOf(item.itemIndex)>=0) {
                continue;
            }

            if (item.itemIndex<taskIndex) {
                vindex = vindex + 1;
            }
        }

        return vindex;
    }

}
