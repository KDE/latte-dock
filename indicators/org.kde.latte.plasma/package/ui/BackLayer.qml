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

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    id: frame
    property string basePrefix: "normal"

    imagePath: "widgets/tasks"
    rotation: root.reversedEnabled ? 180 : 0

    opacity: 1

    prefix: root.taskPrefix(basePrefix)

    states: [
        State {
            name: "launcher"
            when: indicator.isLauncher || (indicator.isApplet && !indicator.isActive)

            PropertyChanges {
                target: frame
                basePrefix: ""
            }
        },
        State {
            name: "hovered"
            when: indicator.isHovered && frame.hasElementPrefix("hover")

            PropertyChanges {
                target: frame
                basePrefix: "hover"
            }
        },
        State {
            name: "attention"
            when: indicator.inAttention

            PropertyChanges {
                target: frame
                basePrefix: "attention"
            }
        },
        State {
            name: "minimized"
            when: indicator.isMinimized

            PropertyChanges {
                target: frame
                basePrefix: "minimized"
            }
        },
        State {
            name: "active"
            when: indicator.isActive

            PropertyChanges {
                target: frame
                basePrefix: "focus"
            }
        }
    ]
}
