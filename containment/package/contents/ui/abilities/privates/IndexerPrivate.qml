/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Indexer {
    id: indxr
    property Item layouts: null
    property bool updateIsBlocked: false

    property var clients: []
    property var clientsBridges: []

    property var marginsAreaSeparators: []

    property int clientsTrackingWindowsCount: 0

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

    Binding{
        target: indxr
        property: "marginsAreaSeparators"
        when: !updateIsBlocked
        value: {
            var seps = [];
            var grid;

            for (var l=0; l<=2; ++l) {
                if (l===0) {
                    grid = layouts.startLayout;
                } else if (l===1) {
                    grid = layouts.mainLayout;
                } else if (l===2) {
                    grid = layouts.endLayout;
                }

                for (var i=0; i<grid.children.length; ++i){
                    var appletItem = grid.children[i];
                    if (appletItem
                            && appletItem.isMarginsAreaSeparator
                            && appletItem.index>=0) {
                        seps.push(appletItem.index);
                    }
                }
            }

            return seps;
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

    Binding{
        target: indxr
        property: "clientsTrackingWindowsCount"
        when: !(root.appletIsDragged || updateIsBlocked)
        value: {
            var cnts = 0;
            var grid;

            for (var l=0; l<=2; ++l) {
                if (l===0) {
                    grid = layouts.startLayout;
                } else if (l===1) {
                    grid = layouts.mainLayout;
                } else if (l===2) {
                    grid = layouts.endLayout;
                }

                for (var i=0; i<grid.children.length; ++i){
                    var appletItem = grid.children[i];
                    if (appletItem
                            && appletItem.communicator
                            && appletItem.communicator.requires.windowsTrackingEnabled) {
                        cnts = cnts + 1;
                    }
                }
            }

            return cnts;
        }
    }

    function visibleItemsBeforeCount(layout, actualIndex) {
        var visibleItems = 0;

        for (var i=0; i<layout.children.length; ++i){
            var appletItem = layout.children[i];
            if (appletItem && appletItem.index<actualIndex) {
                if ((separators.indexOf(appletItem.index) >= 0)
                        || (marginsAreaSeparators.indexOf(appletItem.index)>=0)
                        || (hidden.indexOf(appletItem.index) >= 0)) {
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

        return visibleItems + 1;
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
