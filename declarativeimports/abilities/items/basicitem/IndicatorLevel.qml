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
    anchors.bottom: (plasmoid.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (plasmoid.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (plasmoid.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (plasmoid.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    anchors.horizontalCenter: abilityItem.isHorizontal ? parent.horizontalCenter : undefined
    anchors.verticalCenter: abilityItem.isHorizontal ? undefined : parent.verticalCenter

    width: abilityItem.isHorizontal ? length : thickness
    height: abilityItem.isHorizontal ? thickness : length

    level.isDrawn: level.indicator && level.indicator.host && level.indicator.host.isEnabled && !abilityItem.isSeparator && !abilityItem.isHidden

    readonly property real length: abilityItem.parabolicItem.length - 2*abilityItem.parabolicItem.zoom*abilityItem.abilities.metrics.margin.length
    readonly property real thickness: abilityItem.parabolicItem.thickness

    Connections {
        target: abilityItem
        enabled: indicatorLevel.level.indicator.host ? indicatorLevel.level.indicator.host.info.needsMouseEventCoordinates : false
        onMousePressed: {
            var fixedPos = indicatorLevel.mapFromItem(abilityItem, x, y);
            level.mousePressed(Math.round(fixedPos.x), Math.round(fixedPos.y), button);
        }
        onMouseReleased: {
            var fixedPos = indicatorLevel.mapFromItem(abilityItem, x, y);
            level.mouseReleased(Math.round(fixedPos.x), Math.round(fixedPos.y), button);
        }
    }
}
