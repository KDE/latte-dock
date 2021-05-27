/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.PositionShortcuts {
    id: _shortcutsprivate
    property Item layouts: null
    property bool updateIsBlocked: false

    readonly property bool unifiedGlobalShortcuts: appletIdStealingPositionShortcuts === -1

    property bool showAppletShortcutBadges: false
    property bool showMetaBadge: false
    property int applicationLauncherId: -1

    property int appletIdStealingPositionShortcuts: -1

    signal currentAppletStealingPositionShortcuts(int id);

    Binding {
        target: _shortcutsprivate
        property: "badges"
        when: !updateIsBlocked && shortcutsEngine
        value: shortcutsEngine.badgesForActivate
    }

    Binding {
        target: _shortcutsprivate
        property: "appletIdStealingPositionShortcuts"
        when: !updateIsBlocked
        value: {
            var sLayout = layouts.startLayout;
            for (var i=0; i<sLayout.children.length; ++i){
                var appletItem = sLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.positionShortcutsAreSupported
                        && appletItem.communicator.bridge.shortcuts.client.isStealingGlobalPositionShortcuts) {
                    return appletItem.index;
                }
            }

            var mLayout = layouts.mainLayout;
            for (var i=0; i<mLayout.children.length; ++i){
                var appletItem = mLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.positionShortcutsAreSupported
                        && appletItem.communicator.bridge.shortcuts.client.isStealingGlobalPositionShortcuts) {
                    return appletItem.index;
                }
            }

            var eLayout = layouts.endLayout;
            for (var i=0; i<eLayout.children.length; ++i){
                var appletItem = eLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.positionShortcutsAreSupported
                        && appletItem.communicator.bridge.shortcuts.client.isStealingGlobalPositionShortcuts) {
                    return appletItem.index;
                }
            }

            return -1;
        }
    }
}
