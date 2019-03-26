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

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

Loader{
    id: appletNumberLoader

    active: appletItem.canShowAppletNumberBadge &&
            (root.showLatteShortcutBadges
             || root.showAppletShortcutBadges
             || root.showMetaBadge && applet.id===applicationLauncherId)

    asynchronous: true
    visible: badgeString!==""

    property int fixedIndex: -1
    property string badgeString: ""

    onActiveChanged: {
        if (active && root.showLatteShortcutBadges && root.unifiedGlobalShortcuts) {
            fixedIndex = parabolicManager.pseudoAppletIndex(index);
        } else {
            fixedIndex = -1;
        }
    }

    Component.onCompleted: {
        if (active && root.showLatteShortcutBadges && root.unifiedGlobalShortcuts) {
            fixedIndex = parabolicManager.pseudoAppletIndex(index);
        } else {
            fixedIndex = -1;
        }
    }

    Binding{
        target: appletNumberLoader
        property:"badgeString"
        value: {
            //! don't change value on hiding/releasing
            if (!root.showMetaBadge && !root.showAppletShortcutBadges) {
                return;
            }

            if (root.showAppletShortcutBadges) {
                var plasmaShortcut = applet ? shortcutsEngine.appletShortcutBadge(applet.id) : "";

                if (plasmaShortcut !== "") {
                    return plasmaShortcut;
                }
            }

            if (root.showMetaBadge && applet && applet.id === applicationLauncherId) {
                return '\u2318';
            } else if (appletNumberLoader.fixedIndex>=1 && appletNumberLoader.fixedIndex<20) {
                return root.badgesForActivate[appletNumberLoader.fixedIndex-1];
            } else {
                return "";
            }
        }
    }

    sourceComponent: Item{
        Loader{
            anchors.fill: appletNumber
            active: root.enableShadows

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
            anchors.top: parent.top
            anchors.left: parent.left
            //anchors.centerIn: parent

            minimumWidth: 0.4 * (wrapper.zoomScale * root.iconSize)
            height: Math.max(24, 0.4 * (wrapper.zoomScale * root.iconSize))

            border.color: root.minimizedDotColor
            proportion: 0
            radiusPerCentage: 100
            showNumber: false
            showText: true
            textValue: appletNumberLoader.badgeString
        }
    }
}
