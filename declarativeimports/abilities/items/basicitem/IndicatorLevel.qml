/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.abilities.items 0.1 as AbilityItem

AbilityItem.IndicatorLevel {
    id: indicatorLevel
    anchors.bottom: (abilityItem.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (abilityItem.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (abilityItem.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (abilityItem.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    anchors.horizontalCenter: abilityItem.isHorizontal ? parent.horizontalCenter : undefined
    anchors.verticalCenter: abilityItem.isHorizontal ? undefined : parent.verticalCenter

    width: abilityItem.isHorizontal ? length : thickness
    height: abilityItem.isHorizontal ? thickness : length

    level.isDrawn: level.indicator && level.indicator.host && level.indicator.host.isEnabled && !abilityItem.isSeparator && !abilityItem.isHidden

    readonly property real length: abilityItem.preserveIndicatorInInitialPosition ?
                                       abilityItem.abilities.metrics.iconSize + abilityItem.abilities.metrics.totals.lengthPaddings :
                                       abilityItem.parabolicItem.length - 2*abilityItem.parabolicItem.zoom*abilityItem.abilities.metrics.margin.length
    readonly property real thickness: abilityItem.preserveIndicatorInInitialPosition ?
                                          abilityItem.abilities.metrics.thickness :
                                          abilityItem.parabolicItem.thickness

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

    Connections {
        target: abilityItem
        onTaskLauncherActivated: level.taskLauncherActivated();
        onTaskGroupedWindowAdded: level.taskGroupedWindowAdded();
        onTaskGroupedWindowRemoved: level.taskGroupedWindowRemoved();
    }
}
