/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.latte.components 1.0 as LatteComponents

Loader{
    id: shorcutBadge
    active: abilityItem.abilities.shortcuts.showPositionShortcutBadges && !abilityItem.isSeparator && !abilityItem.isHidden && abilityItem.abilities.shortcuts.isEnabled
    asynchronous: true
    visible: badgeString !== ""

    property int fixedIndex:-1

    readonly property int maxFixedIndex: abilityItem.abilities.shortcuts.badges.length
    readonly property real textColorBrightness: colorBrightness(theme.textColor)
    readonly property string badgeString: (shorcutBadge.fixedIndex>=1 && shorcutBadge.fixedIndex<=maxFixedIndex) ?
                                              abilityItem.abilities.shortcuts.badges[shorcutBadge.fixedIndex-1] : ""
    readonly property color lightTextColor: textColorBrightness > 127.5 ? theme.textColor : theme.backgroundColor

    onActiveChanged: updateShorcutIndex();

    Connections {
        target: abilityItem
        onItemIndexChanged: shortcutBadge.updateShorcutIndex();
    }

    function updateShorcutIndex() {
        if (shorcutBadge.active && abilityItem.abilities.shortcuts.showPositionShortcutBadges) {
            fixedIndex = abilityItem.abilities.shortcuts.shortcutIndex(abilityItem.itemIndex);
        } else {
            fixedIndex = -1;
        }
    }

    function colorBrightness(color) {
        return colorBrightnessFromRGB(color.r * 255, color.g * 255, color.b * 255);
    }

    // formula for brightness according to:
    // https://www.w3.org/TR/AERT/#color-contrast
    function colorBrightnessFromRGB(r, g, b) {
        return (r * 299 + g * 587 + b * 114) / 1000
    }

    sourceComponent: Item{
        Loader{
            anchors.fill: taskNumber
            active: abilityItem.abilities.myView.itemShadow.isEnabled
                    && abilityItem.abilities.environment.isGraphicsSystemAccelerated

            sourceComponent: DropShadow{
                color: abilityItem.abilities.myView.itemShadow.shadowColor
                fast: true
                samples: 2 * radius
                source: taskNumber
                radius: abilityItem.abilities.myView.itemShadow.size/2
                verticalOffset: 2
            }
        }

        LatteComponents.BadgeText {
            id: taskNumber
            // when iconSize < 48, height is always = 24, height / iconSize > 50%
            // we prefer center aligned badges to top-left aligned ones
            property bool centerInParent: abilityItem.abilities.metrics.iconSize < 48

            anchors.left: centerInParent? undefined : parent.left
            anchors.top: centerInParent? undefined : parent.top
            anchors.centerIn: centerInParent? parent : undefined
            minimumWidth: 0.4 * (abilityItem.parabolicItem.zoom * abilityItem.abilities.metrics.iconSize)
            height: Math.max(24, 0.4 * (abilityItem.parabolicItem.zoom * abilityItem.abilities.metrics.iconSize))

            style3d: abilityItem.abilities.myView.badgesIn3DStyle
            textValue: shorcutBadge.badgeString
            borderColor: shortcutBadge.lightTextColor

            showNumber: false
            showText: true

            proportion: 0
            radiusPerCentage: 100
        }
    }
}
