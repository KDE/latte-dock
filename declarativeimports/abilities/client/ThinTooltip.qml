/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.ThinTooltip {
    id: thinTooltip
    property Item bridge: null

    isEnabled: ref.thinTooltip.isEnabled
    showIsBlocked: local.showIsBlocked
    currentVisualParent: ref.thinTooltip.currentVisualParent
    currentText: ref.thinTooltip.currentText

    readonly property bool isActive: bridge !== null
    readonly property AbilityDefinition.ThinTooltip local: AbilityDefinition.ThinTooltip {}

    Item {
        id: ref
        readonly property Item thinTooltip: bridge ? bridge.thinTooltip.host : local
    }

    function show(visualParent, text) {
        if (bridge) {
            bridge.thinTooltip.host.show(visualParent, text);
        } else {
            local.show(visualParent, text);
        }
    }

    function hide(visualParent) {
        if (bridge) {
            bridge.thinTooltip.host.hide(visualParent);
        } else {
            local.hide(visualParent);
        }
    }

    onIsActiveChanged: {
        if (isActive) {
            bridge.thinTooltip.client = thinTooltip;
        }
    }

    Component.onCompleted: {
        if (isActive) {
            bridge.thinTooltip.client = thinTooltip;
        }
    }

    Component.onDestruction: {
        if (isActive) {
            bridge.thinTooltip.client = null;
        }
    }
}
