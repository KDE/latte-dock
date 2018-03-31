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

import org.kde.latte 0.1 as Latte

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

        Latte.BadgeText {
            id: progressCircle
            anchors.centerIn: parent
            minimumWidth: 0.8 * parent.height
            height: 0.8 * parent.height
            numberValue: mainItemContainer.badgeIndicator > 0 ? mainItemContainer.badgeIndicator : centralItem.smartLauncherItem.count
            fullCircle: true
            showNumber: true

            textWithBackgroundColor: ( (mainItemContainer.badgeIndicator > 0)
                                      || (centralItem.smartLauncherItem.countVisible && !centralItem.smartLauncherItem.progressVisible) )
                                     && proportion>0

            proportion: {
                if (mainItemContainer.badgeIndicator > 0 ||
                        (centralItem.smartLauncherItem.countVisible && !centralItem.smartLauncherItem.progressVisible)) {
                    return 100;
                }

                if (centralItem.smartLauncherItem) {
                    return centralItem.smartLauncherItem.progress / 100;
                } else {
                    return 0;
                }
            }
        }
    }
}
