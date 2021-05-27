/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.PositionShortcuts {
    id: shortcuts
    property Item bridge: null
    property Item indexer: null

    badges: ref.shortcuts.badges
    showPositionShortcutBadges: ref.shortcuts.showPositionShortcutBadges

    property bool isStealingGlobalPositionShortcuts: false

    readonly property bool isActive: bridge !== null

    readonly property bool isEnabled: {
        if (bridge) {
            return bridge.shortcuts.host.unifiedGlobalShortcuts
                    || (!bridge.shortcuts.host.unifiedGlobalShortcuts && bridge.shortcuts.appletIndex === bridge.shortcuts.host.appletIdStealingPositionShortcuts);
        }

        return true;
    }

    signal disabledIsStealingGlobalPositionShortcuts();

    readonly property AbilityDefinition.PositionShortcuts local: AbilityDefinition.PositionShortcuts {}

    Item {
        id: ref
        readonly property Item shortcuts: bridge ? bridge.shortcuts.host : local
    }

    onIsActiveChanged: {
        if (isActive) {
            bridge.shortcuts.client = shortcuts;
        }
    }

    Component.onCompleted: {
        if (isActive) {
            bridge.shortcuts.client = shortcuts;
        }
    }

    Component.onDestruction: {
        if (isActive) {
            bridge.shortcuts.client = null;
        }
    }

    function shortcutIndex(entryIndex) {
        if (!bridge || bridge.shortcuts.host.unifiedGlobalShortcuts) {
            return indexer.visibleIndex(entryIndex);
        }

        var base = bridge.indexer.host.visibleIndex(bridge.shortcuts.appletIndex);

        //!visible indexes start counting from 1
        return (indexer.visibleIndex(entryIndex) - base + 1);
    }
}

