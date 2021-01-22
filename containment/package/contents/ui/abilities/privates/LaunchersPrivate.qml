/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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
    id: _launchersprivate
    property Item layouts: null
    property bool updateIsBlocked: false

    readonly property bool hasStealingApplet: appletStealingDroppedLaunchers !== null

    property int appletIdStealingDroppedLaunchers: -1
    property Item appletStealingDroppedLaunchers: null

    signal currentAppletStealingDroppedLaunchers(int id);

    Binding {
        target: _launchersprivate
        property: "appletIdStealingDroppedLaunchers"
        when: !updateIsBlocked
        value: {
            var sLayout = layouts.startLayout;
            for (var i=0; i<sLayout.children.length; ++i){
                var appletItem = sLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.launchersAreSupported
                        && appletItem.communicator.bridge.launchers.client.isStealingDroppedLaunchers) {
                    return appletItem.index;
                }
            }

            var mLayout = layouts.mainLayout;
            for (var i=0; i<mLayout.children.length; ++i){
                var appletItem = mLayout.children[i];

                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.launchersAreSupported
                        && appletItem.communicator.bridge.launchers.client.isStealingDroppedLaunchers) {
                    return appletItem.index;
                }
            }

            var eLayout = layouts.endLayout;
            for (var i=0; i<eLayout.children.length; ++i){
                var appletItem = eLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.launchersAreSupported
                        && appletItem.communicator.bridge.launchers.client.isStealingDroppedLaunchers) {
                    return appletItem.index;
                }
            }

            return -1;
        }
    }


    Binding {
        target: _launchersprivate
        property: "appletStealingDroppedLaunchers"
        when: !updateIsBlocked
        value: {
            var sLayout = layouts.startLayout;
            for (var i=0; i<sLayout.children.length; ++i){
                var appletItem = sLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.launchersAreSupported
                        && appletItem.communicator.bridge.launchers.client.isStealingDroppedLaunchers) {
                    return appletItem.communicator.bridge.launchers.client;
                }
            }

            var mLayout = layouts.mainLayout;
            for (var i=0; i<mLayout.children.length; ++i){
                var appletItem = mLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.launchersAreSupported
                        && appletItem.communicator.bridge.launchers.client.isStealingDroppedLaunchers) {
                    return appletItem.communicator.bridge.launchers.client;
                }
            }

            var eLayout = layouts.endLayout;
            for (var i=0; i<eLayout.children.length; ++i){
                var appletItem = eLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.launchersAreSupported
                        && appletItem.communicator.bridge.launchers.client.isStealingDroppedLaunchers) {
                    return appletItem.communicator.bridge.launchers.client;
                }
            }

            return null;
        }
    }
}
