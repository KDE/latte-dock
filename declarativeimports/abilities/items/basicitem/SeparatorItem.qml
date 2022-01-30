/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.8
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

Loader {
    anchors.bottom: (abilityItem.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (abilityItem.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (abilityItem.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (abilityItem.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    anchors.horizontalCenter: abilityItem.isHorizontal ? parent.horizontalCenter : undefined
    anchors.verticalCenter: abilityItem.isHorizontal ? undefined : parent.verticalCenter

    anchors.bottomMargin: (abilityItem.location === PlasmaCore.Types.BottomEdge) ? margin : 0
    anchors.topMargin: (abilityItem.location === PlasmaCore.Types.TopEdge) ? margin : 0
    anchors.leftMargin: (abilityItem.location === PlasmaCore.Types.LeftEdge) ? margin : 0
    anchors.rightMargin: (abilityItem.location === PlasmaCore.Types.RightEdge) ? margin : 0

    width: abilityItem.isVertical ?
               abilityItem.abilities.metrics.iconSize :
               (isSeparatorInRealLength ? LatteCore.Environment.separatorLength+abilityItem.abilities.metrics.totals.lengthEdges: 2)
    height: abilityItem.isHorizontal ?
                abilityItem.abilities.metrics.iconSize :
                (isSeparatorInRealLength  ? LatteCore.Environment.separatorLength+abilityItem.abilities.metrics.totals.lengthEdges: 2)

    opacity: 0.4

    readonly property int margin: abilityItem.abilities.metrics.margin.screenEdge + abilityItem.abilities.metrics.margin.tailThickness

    active: abilityItem.isSeparatorVisible

    sourceComponent: Item {
        ///Shadow for separator
        Loader{
            anchors.fill: separatorItem
            active: abilityItem.isSeparatorVisible
                    && abilityItem.abilities.myView.itemShadow.isEnabled
                    && abilityItem.abilities.environment.isGraphicsSystemAccelerated
            opacity: 1.0

            Behavior on opacity {
                NumberAnimation { duration: abilityItem.abilities.animations.speedFactor.current * abilityItem.abilities.animations.duration.large }
            }

            sourceComponent: DropShadow{
                anchors.fill: parent
                color: abilityItem.abilities.myView.itemShadow.shadowColor
                fast: true
                samples: 2 * radius
                source: separatorItem
                radius: abilityItem.abilities.myView.itemShadow.size
                verticalOffset: 2
            }
        }

        Item{
            id:separatorItem
            anchors.fill: parent

            Behavior on opacity {
                NumberAnimation { duration: abilityItem.abilities.animations.speedFactor.current * abilityItem.abilities.animations.duration.large }
            }

            Rectangle {
                anchors.centerIn: parent
                width: abilityItem.isVertical ? abilityItem.abilities.metrics.iconSize - 4  : 1
                height: abilityItem.isHorizontal ? abilityItem.abilities.metrics.iconSize - 4 : 1
                color: abilityItem.abilities.myView.palette.textColor
            }
        }
    }
}
