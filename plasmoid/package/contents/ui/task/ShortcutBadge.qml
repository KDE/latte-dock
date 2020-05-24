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
import QtGraphicalEffects 1.0

import org.kde.latte.components 1.0 as LatteComponents

Loader{
    id: shorcutBadge
    anchors.fill: iconImageBuffer
    active: taskItem.shortcuts.showPositionShortcutBadges && !taskItem.isSeparator && !taskItem.isHidden && taskItem.shortcuts.isEnabled
    asynchronous: true
    visible: badgeString !== ""

    property int fixedIndex:-1
    property string badgeString: (shorcutBadge.fixedIndex>=1 && shorcutBadge.fixedIndex<20 && taskItem.shortcuts.badges.length===19) ?
                                     taskItem.shortcuts.badges[shorcutBadge.fixedIndex-1] : ""

    onActiveChanged: updateShorcutIndex();

    Connections {
        target: taskItem
        onItemIndexChanged: shortcutBadge.updateShorcutIndex();
    }

    function updateShorcutIndex() {
        if (shorcutBadge.active && taskItem.shortcuts.showPositionShortcutBadges) {
            fixedIndex = taskItem.shortcuts.shortcutIndex(taskItem.itemIndex);
        } else {
            fixedIndex = -1;
        }
    }

    sourceComponent: Item{
        Loader{
            anchors.fill: taskNumber
            active: root.enableShadows && graphicsSystem.isAccelerated

            sourceComponent: DropShadow{
                color: root.appShadowColor
                fast: true
                samples: 2 * radius
                source: taskNumber
                radius: root.appShadowSize/2
                verticalOffset: 2
            }
        }

        LatteComponents.BadgeText {
            id: taskNumber
            // when iconSize < 48, height is always = 24, height / iconSize > 50%
            // we prefer center aligned badges to top-left aligned ones
            property bool centerInParent: taskItem.metrics.iconSize < 48

            anchors.left: centerInParent? undefined : parent.left
            anchors.top: centerInParent? undefined : parent.top
            anchors.centerIn: centerInParent? parent : undefined
            minimumWidth: 0.4 * (wrapper.mScale * taskItem.metrics.iconSize)
            height: Math.max(24, 0.4 * (wrapper.mScale * taskItem.metrics.iconSize))

            style3d: root.badges3DStyle
            textValue: shorcutBadge.badgeString
            borderColor: root.lightTextColor

            showNumber: false
            showText: true

            proportion: 0
            radiusPerCentage: 100
        }
    }
}
