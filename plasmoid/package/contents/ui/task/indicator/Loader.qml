/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Loader {
    id: indicatorLoader
    anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
    anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
    anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
    anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

    anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
    anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

    active: level.bridge && level.bridge.active && (level.isBackground || (level.isForeground && indicators.info.providesFrontLayer))
    sourceComponent: indicators.indicatorComponent

    width: {
        if (locked) {
            return visualLockedWidth;
        }

        return !root.vertical ? taskItem.wrapperAlias.width - 2*taskItem.wrapperAlias.mScale*root.lengthExtMargin : taskItem.wrapperAlias.width;
    }

    height: {
        if (locked) {
            return visualLockedHeight;
        }

        return root.vertical ? taskItem.wrapperAlias.height - 2*taskItem.wrapperAlias.mScale*root.lengthExtMargin : taskItem.wrapperAlias.height;
    }

    readonly property bool locked: inAttentionAnimation || inNewWindowAnimation

    property real visualLockedWidth: root.iconSize + root.internalWidthMargins
    property real visualLockedHeight: root.iconSize + root.internalHeightMargins

    property Item level
}
