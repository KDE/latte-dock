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

import QtQuick 2.0

import org.kde.latte.abilities.definitions 0.1 as AbilityDefinition

AbilityDefinition.ParabolicEffect {
    id: apis

    readonly property bool directRenderingEnabled: _privates.directRenderingEnabled
    readonly property int lastIndex: _privates.lastIndex

    //! This needs to be rethought and rechecked in order for public api to provide
    //! functionality that functionality must already exist in the definition
    readonly property Item publicApi: Item {
        readonly property alias directRenderingEnabled: apis.directRenderingEnabled
        readonly property alias factor: apis.factor
        readonly property alias lastIndex: apis.lastIndex
        readonly property alias restoreZoomIsBlocked: apis.restoreZoomIsBlocked

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
