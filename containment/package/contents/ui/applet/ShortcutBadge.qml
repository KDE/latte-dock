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

import QtQuick 2.1
import QtGraphicalEffects 1.0

import org.kde.latte.components 1.0 as LatteComponents

Loader{
    id: appletNumberLoader

    active: appletItem.canShowAppletNumberBadge &&
            (appletItem.shortcuts.showPositionShortcutBadges
             || appletItem.shortcuts.showAppletShortcutBadges
             || appletItem.shortcuts.showMetaBadge && applet.id===appletItem.shortcuts.applicationLauncherId)

    asynchronous: true
    visible: badgeString!==""

    property int fixedIndex: -1
    property string badgeString: ""

    onActiveChanged: {
        if (active && appletItem.shortcuts.showPositionShortcutBadges && appletItem.shortcuts.unifiedGlobalShortcuts) {
            fixedIndex = appletItem.indexer.visibleIndex(index);
        } else {
            fixedIndex = -1;
        }
    }

    Component.onCompleted: {
        if (active && appletItem.shortcuts.showPositionShortcutBadges && appletItem.shortcuts.unifiedGlobalShortcuts) {
            fixedIndex = appletItem.indexer.visibleIndex(index);
        } else {
            fixedIndex = -1;
        }
    }

    Binding{
        target: appletNumberLoader
        property:"badgeString"
        when: appletItem.shortcuts.showMetaBadge || appletItem.shortcuts.showAppletShortcutBadges
        value: {
            if (appletItem.shortcuts.showMetaBadge && applet && applet.id === appletItem.shortcuts.applicationLauncherId) {
                return '\u2318';
            }

            if (appletItem.shortcuts.showAppletShortcutBadges) {
                var plasmaShortcut = applet ? shortcutsEngine.appletShortcutBadge(applet.id) : "";

                if (plasmaShortcut !== "") {
                    return plasmaShortcut;
                }
            }

             if (appletNumberLoader.fixedIndex>=1 && appletNumberLoader.fixedIndex<20) {
                return appletItem.shortcuts.badges[appletNumberLoader.fixedIndex-1];
            } else {
                return "";
            }
        }
    }

    sourceComponent: Item{
        Loader{
            anchors.fill: appletNumber
            active: root.enableShadows && graphicsSystem.isAccelerated

            sourceComponent: DropShadow{
                color: root.appShadowColor
                fast: true
                samples: 2 * radius
                source: appletNumber
                radius: root.appShadowSize/2
                verticalOffset: 2
            }
        }

        LatteComponents.BadgeText {
            id: appletNumber

            // when iconSize < 48, height is always = 24, height / iconSize > 50%
            // we prefer center aligned badges to top-left aligned ones
            property bool centerInParent: appletItem.metrics.iconSize < 48

            anchors.left: centerInParent? undefined : parent.left
            anchors.top: centerInParent? undefined : parent.top
            anchors.centerIn: centerInParent? parent : undefined
            minimumWidth: 0.4 * (wrapper.zoomScale * appletItem.metrics.iconSize)
            height: Math.max(24, 0.4 * (wrapper.zoomScale * appletItem.metrics.iconSize))

            borderColor: colorizerManager.originalLightTextColor
            proportion: 0
            radiusPerCentage: 100
            showNumber: false
            showText: true
            textValue: appletNumberLoader.badgeString

            style3d: root.badges3DStyle
        }
    }
}
