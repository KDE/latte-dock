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

AbilityDefinition.Indexer {
    id: indexer
    property Item bridge: null
    readonly property bool isActive: bridge !== null

    onIsActiveChanged: {
        if (isActive) {
            bridge.indexer.client = indexer;
        }
    }

    Component.onCompleted: {
        if (isActive) {
            bridge.indexer.client = indexer;
        }
    }

    Component.onDestruction: {
        if (isActive) {
            bridge.indexer.client = null;
        }
    }
}
