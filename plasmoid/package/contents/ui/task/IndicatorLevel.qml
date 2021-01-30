/*
*  Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.abilities.items 0.1 as AbilityItem

AbilityItem.IndicatorLevel {
    id: indicatorLevel
    anchors.bottom: (root.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (root.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (root.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (root.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
    anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

    width: {
        if (locked) {
            return visualLockedWidth;
        }

        return !root.vertical ? taskItem.parabolicItem.width - 2*taskItem.parabolicItem.zoom*taskItem.abilities.metrics.margin.length: taskItem.parabolicItem.width;
    }

    height: {
        if (locked) {
            return visualLockedHeight;
        }

        return root.vertical ? taskItem.parabolicItem.height - 2*taskItem.parabolicItem.zoom*taskItem.abilities.metrics.margin.length : taskItem.parabolicItem.height;
    }

    level.isDrawn: level.indicator && level.indicator.host && level.indicator.host.isEnabled && !taskItem.isSeparator && !taskItem.isHidden

    readonly property bool locked: inAttentionAnimation || inNewWindowAnimation || inBouncingAnimation

    property real visualLockedWidth: root.vertical ? taskItem.abilities.metrics.margin.screenEdge + taskItem.abilities.metrics.iconSize + root.internalWidthMargins :
                                                     taskItem.abilities.metrics.iconSize + root.internalWidthMargins
    property real visualLockedHeight: !root.vertical ? taskItem.abilities.metrics.margin.screenEdge + taskItem.abilities.metrics.iconSize + root.internalHeightMargins :
                                                       taskItem.abilities.metrics.iconSize + root.internalHeightMargins

    Connections {
        target: taskItem.mouseArea
        enabled: indicatorLevel.level.indicator.host ? indicatorLevel.level.indicator.host.info.needsMouseEventCoordinates : false
        onPressed: {
            var fixedPos = indicatorLevel.mapFromItem(taskItem, mouse.x, mouse.y);
            level.mousePressed(Math.round(fixedPos.x), Math.round(fixedPos.y), mouse.button);
        }
        onReleased: {
            var fixedPos = indicatorLevel.mapFromItem(taskItem, mouse.x, mouse.y);
            level.mousePressed(Math.round(fixedPos.x), Math.round(fixedPos.y), mouse.button);
        }
    }
}
