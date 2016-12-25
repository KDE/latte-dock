/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: background
    anchors.fill: parent

    CircleText {
        id: progressCircle
        width: 0.42 * parent.height
        height: width
        numberValue: centralItem.smartLauncherItem.count
        fullCircle: true
        showNumber: true
        proportion: centralItem.smartLauncherItem.progress / 100

        states: [
            State {
                name: "left"
                when: (panel.position === PlasmaCore.Types.LeftPositioned)

                AnchorChanges {
                    target: progressCircle
                    anchors{ top:background.top; bottom:undefined; left:undefined; right:background.right;}
                }
            },
            State {
                name: "right"
                when: (panel.position === PlasmaCore.Types.RightPositioned)

                AnchorChanges {
                    target: progressCircle
                    anchors{ top:background.top; bottom:undefined; left:background.left; right:undefined;}
                }
            },
            State {
                name: "bottom"
                when: (panel.position === PlasmaCore.Types.BottomPositioned)

                AnchorChanges {
                    target: progressCircle
                    anchors{ top:background.top; bottom:undefined; left:undefined; right:background.right;}
                }
            },
            State {
                name: "top"
                when: (panel.position === PlasmaCore.Types.TopPositioned)

                AnchorChanges {
                    target: progressCircle
                    anchors{ top:background.top; bottom:undefined; left:undefined; right:background.right;}
                }
            }
        ]
    }
}
