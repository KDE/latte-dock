/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
