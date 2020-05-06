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
    id: parabolic
    property Item bridge: null
    readonly property bool isActive: bridge !== null

    factor: ref.parabolic.factor
    restoreZoomIsBlocked: bridge ? (bridge.parabolic.host.restoreZoomIsBlocked || local.restoreZoomIsBlocked) : local.restoreZoomIsBlocked

    //! private properties can not go to definition because can not be made readonly in there
    //! special care must be taken in order to be redefined in local properties
    readonly property bool directRenderingEnabled: ref.parabolic._privates.directRenderingEnabled
    readonly property int lastIndex: local._privates.lastIndex /*special property that needs to remain the same*/

    readonly property AbilityDefinition.ParabolicEffect local: AbilityDefinition.ParabolicEffect {
        id: _localref
        readonly property bool directRenderingEnabled: _localref._privates.directRenderingEnabled
        readonly property int lastIndex: _localref._privates.lastIndex
    }

    Item {
        id: ref
        readonly property Item parabolic: bridge ? bridge.parabolic.host : local
    }

    onIsActiveChanged: {
        if (isActive) {
            bridge.parabolic.client = parabolic;
        }
    }

    Component.onCompleted: {
        if (isActive) {
            bridge.parabolic.client = parabolic;
        }
    }

    Component.onDestruction: {
        if (isActive) {
            bridge.parabolic.client = null;
        }
    }
}
