/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.ParabolicEffect {
    id: apis

    readonly property bool directRenderingEnabled: _privates.directRenderingEnabled

    //! This needs to be rethought and rechecked in order for public api to provide
    //! functionality that functionality must already exist in the definition
    readonly property Item publicApi: Item {
        readonly property alias isEnabled: apis.isEnabled
        readonly property alias directRenderingEnabled: apis.directRenderingEnabled
        readonly property alias factor: apis.factor
        readonly property alias restoreZoomIsBlocked: apis.restoreZoomIsBlocked
        readonly property alias spread: apis.spread
        readonly property alias currentParabolicItem: apis.currentParabolicItem

        signal sglClearZoom();

        function startRestoreZoomTimer() {
            apis.startRestoreZoomTimer();
        }

        function stopRestoreZoomTimer() {
            apis.stopRestoreZoomTimer();
        }
    }

    Component.onCompleted: {
        apis.sglClearZoom.connect(publicApi.sglClearZoom)
    }

    Component.onDestruction: {
        apis.sglClearZoom.disconnect(publicApi.sglClearZoom)
    }
}
