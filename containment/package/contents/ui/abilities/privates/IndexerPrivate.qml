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
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.abilities.definitions 0.1 as AbilityDefinition

AbilityDefinition.Indexer {
    id: indxr
    property Item layouts: null
    property bool updateIsBlocked: false

    property var clients: []
    property var clientsBridges: []

    Binding{
        target: indxr
        property: "separators"
        when: !updateIsBlocked
        value: {
            var seps = [];

            var sLayout = layouts.startLayout;
            for (var i=0; i<sLayout.children.length; ++i){
                var appletItem = sLayout.children[i];
                if (appletItem && appletItem.isSeparator && appletItem.index>=0) {
                    seps.push(appletItem.index);
                }
            }

            var mLayout = layouts.mainLayout;
            for (var i=0; i<mLayout.children.length; ++i){
                var appletItem = mLayout.children[i];
                if (appletItem && appletItem.isSeparator && appletItem.index>=0) {
                    seps.push(appletItem.index);
                }
            }

            var eLayout = layouts.endLayout;
            for (var i=0; i<eLayout.children.length; ++i){
                var appletItem = eLayout.children[i];
                if (appletItem && appletItem.isSeparator && appletItem.index>=0) {
                    seps.push(appletItem.index);
                }
            }

            return seps;
        }
    }

    Binding {
        target: indxr
        property: "hidden"
        when: !updateIsBlocked
        value: {
            var hdn = [];

            var sLayout = layouts.startLayout;
            for (var i=0; i<sLayout.children.length; ++i){
                var appletItem = sLayout.children[i];
                if (appletItem && appletItem.isHidden && appletItem.index>=0) {
                    hdn.push(appletItem.index);
                }
            }

            var mLayout = layouts.mainLayout;
            for (var i=0; i<mLayout.children.length; ++i){
                var appletItem = mLayout.children[i];
                if (appletItem && appletItem.isHidden && appletItem.index>=0) {
                    hdn.push(appletItem.index);
                }
            }

            var eLayout = layouts.endLayout;
            for (var i=0; i<eLayout.children.length; ++i){
                var appletItem = eLayout.children[i];
                if (appletItem && appletItem.isHidden && appletItem.index>=0) {
                    hdn.push(appletItem.index);
                }
            }

            return hdn;
        }
    }

    Binding {
        target: indxr
        property: "clients"
        when: !updateIsBlocked
        value: {
            var clns = [];

            var sLayout = layouts.startLayout;
            for (var i=0; i<sLayout.children.length; ++i){
                var appletItem = sLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.indexerIsSupported) {
                    clns.push(appletItem.index);
                }
            }

            var mLayout = layouts.mainLayout;
            for (var i=0; i<mLayout.children.length; ++i){
                var appletItem = mLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.indexerIsSupported) {
                    clns.push(appletItem.index);
                }
            }

            var eLayout = layouts.endLayout;
            for (var i=0; i<eLayout.children.length; ++i){
                var appletItem = eLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.indexerIsSupported) {
                    clns.push(appletItem.index);
                }
            }

            return clns;
        }
    }

    Binding {
        target: indxr
        property: "clientsBridges"
        when: !updateIsBlocked
        value: {
            var bdgs = [];

            var sLayout = layouts.startLayout;
            for (var i=0; i<sLayout.children.length; ++i){
                var appletItem = sLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.indexerIsSupported
                        && appletItem.communicator.bridge
                        && appletItem.communicator.bridge.indexer) {
                    bdgs.push(appletItem.communicator.bridge.indexer);
                }
            }

            var mLayout = layouts.mainLayout;
            for (var i=0; i<mLayout.children.length; ++i){
                var appletItem = mLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.indexerIsSupported
                        && appletItem.communicator.bridge
                        && appletItem.communicator.bridge.indexer) {
                    bdgs.push(appletItem.communicator.bridge.indexer);
                }
            }

            var eLayout = layouts.endLayout;
            for (var i=0; i<eLayout.children.length; ++i){
                var appletItem = eLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.indexerIsSupported
                        && appletItem.communicator.bridge
                        && appletItem.communicator.bridge.indexer) {
                    bdgs.push(appletItem.communicator.bridge.indexer);
                }
            }

            return bdgs;
        }
    }

    function visibleItemsBeforeCount(layout, actualIndex) {
        var visibleItems = 0;

        for (var i=0; i<layout.children.length; ++i){
            var appletItem = layout.children[i];
            if (appletItem && appletItem.index<actualIndex) {
                if ((separators.indexOf(appletItem.index) >= 0) || (hidden.indexOf(appletItem.index) >= 0)) {
                    //! ignore hidden and separators applets
                    continue;
                } else if (!appletItem.communicator || !appletItem.communicator.indexerIsSupported) {
                    //! single item applet
                    visibleItems = visibleItems + 1;
                } else if (appletItem.communicator && appletItem.communicator.indexerIsSupported) {
                    //! multi items applet
                    visibleItems = visibleItems + appletItem.communicator.bridge.indexer.client.visibleItemsCount;
                }
            }
        }

        return visibleItems;
    }

    function visibleIndex(actualIndex) {
        if ((separators.indexOf(actualIndex) >= 0) || (hidden.indexOf(actualIndex) >= 0)) {
            return -1;
        }

        var visibleItems = visibleItemsBeforeCount(layouts.startLayout, actualIndex) +
                visibleItemsBeforeCount(layouts.mainLayout, actualIndex) +
                visibleItemsBeforeCount(layouts.endLayout, actualIndex);

        return visibleItems;
    }

    function visibleIndexBelongsAtApplet(applet, itemVisibleIndex) {
        if (itemVisibleIndex<0 || !applet) {
            return false;
        }

        var vIndexBase = visibleIndex(applet.index);

        if (vIndexBase === itemVisibleIndex) {
            return true;
        } else if (applet.communicator
                   && applet.communicator.indexerIsSupported
                   && applet.communicator.bridge
                   && applet.communicator.bridge.indexer) {
            if (itemVisibleIndex >= vIndexBase
                    && itemVisibleIndex< (vIndexBase + applet.communicator.bridge.indexer.client.visibleItemsCount)) {
                return true;
            }
        }

        return false;
    }
}
