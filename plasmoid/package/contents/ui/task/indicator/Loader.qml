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

Loader {
    id: indicatorLoader
    anchors.bottom: (root.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (root.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (root.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (root.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
    anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

    active: level.bridge && level.bridge.active && (level.isBackground || (level.isForeground && indicators.info.providesFrontLayer))
    sourceComponent: {
        if (!indicators) {
            return;
        }
        return indicators.indicatorComponent;
    }

    width: {
        if (locked) {
            return visualLockedWidth;
        }

        return !root.vertical ? taskItem.wrapperAlias.width - 2*taskItem.wrapperAlias.mScale*taskItem.metrics.margin.length: taskItem.wrapperAlias.width;
    }

    height: {
        if (locked) {
            return visualLockedHeight;
        }

        return root.vertical ? taskItem.wrapperAlias.height - 2*taskItem.wrapperAlias.mScale*taskItem.metrics.margin.length : taskItem.wrapperAlias.height;
    }

    readonly property bool locked: inAttentionAnimation || inNewWindowAnimation || inBouncingAnimation

    property real visualLockedWidth: root.vertical ? taskItem.metrics.margin.screenEdge + taskItem.metrics.iconSize + root.internalWidthMargins :
                                                     taskItem.metrics.iconSize + root.internalWidthMargins
    property real visualLockedHeight: !root.vertical ? taskItem.metrics.margin.screenEdge + taskItem.metrics.iconSize + root.internalHeightMargins :
                                                       taskItem.metrics.iconSize + root.internalHeightMargins

    //! Connections !//

    property Item level

    Connections {
        target: taskItem
        enabled: indicators ? indicators.info.needsMouseEventCoordinates : false
        onPressed: {
            var fixedPos = indicatorLoader.mapFromItem(taskItem, mouse.x, mouse.y);
            level.mousePressed(Math.round(fixedPos.x), Math.round(fixedPos.y), mouse.button);
        }
        onReleased: {
            var fixedPos = indicatorLoader.mapFromItem(taskItem, mouse.x, mouse.y);
            level.mousePressed(Math.round(fixedPos.x), Math.round(fixedPos.y), mouse.button);
        }
    }
}
