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

import org.kde.latte.abilities.definitions 0.1 as AbilityDefinition

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
