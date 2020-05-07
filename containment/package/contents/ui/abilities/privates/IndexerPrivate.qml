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

    separators: {
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

    hidden: {
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


    readonly property var indexers: {
        var inxs = [];

        var sLayout = layouts.startLayout;
        for (var i=0; i<sLayout.children.length; ++i){
            var appletItem = sLayout.children[i];
            if (indexerIsSupported(appletItem)) {
                inxs.push(appletItem.index);
            }
        }

        var mLayout = layouts.mainLayout;
        for (var i=0; i<mLayout.children.length; ++i){
            var appletItem = mLayout.children[i];
            if (indexerIsSupported(appletItem)) {
                inxs.push(appletItem.index);
            }
        }

        var eLayout = layouts.endLayout;
        for (var i=0; i<eLayout.children.length; ++i){
            var appletItem = eLayout.children[i];
            if (indexerIsSupported(appletItem)) {
                inxs.push(appletItem.index);
            }
        }

        return inxs;
    }

    readonly property var indexerBridges: {
        var inbdgs = [];

        var sLayout = layouts.startLayout;
        for (var i=0; i<sLayout.children.length; ++i){
            var appletItem = sLayout.children[i];
            if (indexerBridgeIsValid(appletItem)) {
                inbdgs.push(appletItem.communicator.bridge.indexer);
            }
        }

        var mLayout = layouts.mainLayout;
        for (var i=0; i<mLayout.children.length; ++i){
            var appletItem = mLayout.children[i];
            if (indexerBridgeIsValid(appletItem)) {
                inbdgs.push(appletItem.communicator.bridge.indexer);
            }
        }

        var eLayout = layouts.endLayout;
        for (var i=0; i<eLayout.children.length; ++i){
            var appletItem = eLayout.children[i];
            if (indexerBridgeIsValid(appletItem)) {
                inbdgs.push(appletItem.communicator.bridge.indexer);
            }
        }

        return inbdgs;
    }

    function indexerIsSupported(appletItem) {
        return (appletItem
                && appletItem.index>=0
                && appletItem.communicator
                && appletItem.communicator.indexerIsSupported);
    }

    function indexerBridgeIsValid(appletItem) {
        return (appletItem
                && appletItem.index>=0
                && appletItem.communicator
                && appletItem.communicator.indexerIsSupported
                && appletItem.communicator.bridge
                && appletItem.communicator.bridge.indexer);
    }
}
