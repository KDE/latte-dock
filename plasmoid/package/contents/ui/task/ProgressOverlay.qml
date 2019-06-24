/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

Item {
    id: background

    readonly property int contentWidth: progressCircle.width + 0.1*height

    Item {
        id: subRectangle
        width: contentWidth
        height: parent.height / 2

        states: [
            State {
                name: "default"
                when: (root.position !== PlasmaCore.Types.RightPositioned)

                AnchorChanges {
                    target: subRectangle
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;}
                }
            },
            State {
                name: "right"
                when: (root.position === PlasmaCore.Types.RightPositioned)

                AnchorChanges {
                    target: subRectangle
                    anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;}
                }
            }
        ]

        LatteComponents.BadgeText {
            id: progressCircle
            anchors.centerIn: parent
            minimumWidth: Math.min(0.8 * parent.height, maximumWidth)
            maximumWidth: {
                if (showsAudioBadge) {
                    return height;
                } else if ( index === parabolicManager.firstRealTaskIndex && !taskItem.containsMouse) {
                    return (wrapper.mScale * (root.iconSize - (root.enableShadows ? shadowSize/2 : 0)));
                } else {
                    return 999999;
                }
            }
            height: 0.8 * parent.height
            numberValue: {
                if (taskIcon.smartLauncherItem && (taskIcon.smartLauncherItem.countVisible || taskIcon.smartLauncherItem.progressVisible)) {
                    return taskIcon.smartLauncherItem.count;
                } else if (taskItem.badgeIndicator > 0) {
                    return taskItem.badgeIndicator;
                }

                return 0;
            }
            fullCircle: true
            showNumber: true

            color: theme.backgroundColor
            textColor: {
                if (showsInfoBadge) {
                    return root.infoBadgeProminentColorEnabled ? prominentTextColor : theme.highlightedTextColor
                }

                return theme.textColor;
            }
            borderColor: root.lightTextColor

            highlightedColor: {
                if (showsInfoBadge) {
                    return root.infoBadgeProminentColorEnabled ? prominentBackColor : theme.highlightColor
                }

                return theme.buttonFocusColor;
            }


            textWithBackgroundColor: false

            proportion: {
                if (taskIcon.smartLauncherItem && taskIcon.smartLauncherItem.progressVisible) {
                    return taskIcon.smartLauncherItem.progress / 100;
                }

                if (taskItem.badgeIndicator > 0 || (taskIcon.smartLauncherItem && taskIcon.smartLauncherItem.countVisible)) {
                    return 100;
                }

                return 0;
            }

            readonly property color prominentBackColor: "#cc0000" //redish  (deprecated: theme.negativeTextColor)
            readonly property color prominentTextColor: "#f3f3f3" //whitish (deprecated: root.lightTextColor)

            readonly property bool showsInfoBadge: ((taskItem.badgeIndicator > 0)
                                                    || (taskIcon.smartLauncherItem && taskIcon.smartLauncherItem.countVisible && !taskIcon.smartLauncherItem.progressVisible))

            readonly property bool showsAudioBadge: root.showAudioBadge && taskItem.hasAudioStream && taskItem.playingAudio && !taskItem.isSeparator
        }
    }
}
