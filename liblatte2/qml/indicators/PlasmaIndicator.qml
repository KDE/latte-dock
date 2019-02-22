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

import org.kde.latte 0.2 as Latte

PlasmaCore.FrameSvgItem {
    id: frame

    imagePath: "widgets/tasks"
    property string basePrefix: "normal"

    prefix: taskPrefix(basePrefix)

    property Item rootItem: parent

    function taskPrefix(prefix) {
        var effectivePrefix;

        switch (plasmoid.location) {
        case PlasmaCore.Types.LeftEdge:
            effectivePrefix = "west-" + prefix;
            break;
        case PlasmaCore.Types.TopEdge:
            effectivePrefix = "north-" + prefix;
            break;
        case PlasmaCore.Types.RightEdge:
            effectivePrefix = "east-" + prefix;
            break;
        default:
            effectivePrefix = "south-" + prefix;
        }
        return [effectivePrefix, prefix];
    }

    states: [
        State {
            name: "launcher"
            when: rootItem.isLauncher || (rootItem.isApplet && !rootItem.isActive)

            PropertyChanges {
                target: frame
                basePrefix: ""
            }
        },
        State {
            name: "attention"
            when: rootItem.inAttention

            PropertyChanges {
                target: frame
                basePrefix: "attention"
            }
        },
        State {
            name: "minimized"
            when: rootItem.isMinimized

            PropertyChanges {
                target: frame
                basePrefix: "minimized"
            }
        },
        State {
            name: "active"
            when: rootItem.isActive

            PropertyChanges {
                target: frame
                basePrefix: "focus"
            }
        }
    ]
}
