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

    readonly property Item parentItem: parent.manager
    property string basePrefix: "normal"

    imagePath: parentItem.usePlasmaTabsStyle ? "widgets/tabbar" : "widgets/tasks"

    prefix: {
        if (parentItem.usePlasmaTabsStyle) {
            if (!parentItem.isActive) {
                return "";
            }

            if ((plasmoid.location === PlasmaCore.Types.LeftEdge && !parentItem.reversedEnabled)
                    || (plasmoid.location === PlasmaCore.Types.RightEdge && parentItem.reversedEnabled)) {
                return "west-active-tab";
            }

            if ((plasmoid.location === PlasmaCore.Types.TopEdge && !parentItem.reversedEnabled)
                    || (plasmoid.location === PlasmaCore.Types.BottomEdge && parentItem.reversedEnabled)) {
                return "north-active-tab";
            }

            if ((plasmoid.location === PlasmaCore.Types.RightEdge && !parentItem.reversedEnabled)
                    || (plasmoid.location === PlasmaCore.Types.LeftEdge && parentItem.reversedEnabled)) {
                return "east-active-tab";
            }

            if ((plasmoid.location === PlasmaCore.Types.BottomEdge && !parentItem.reversedEnabled)
                    || (plasmoid.location === PlasmaCore.Types.TopEdge && parentItem.reversedEnabled)) {
                return "south-active-tab";
            }

            return  "south-active-tab";
        } else {
            return taskPrefix(basePrefix);
        }
    }

    function taskPrefix(prefix) {
        var effectivePrefix;

        if ((plasmoid.location === PlasmaCore.Types.LeftEdge && !parentItem.reversedEnabled)
                || (plasmoid.location === PlasmaCore.Types.RightEdge && parentItem.reversedEnabled)) {
            effectivePrefix = "west-" + prefix;
        }

        if ((plasmoid.location === PlasmaCore.Types.TopEdge && !parentItem.reversedEnabled)
                || (plasmoid.location === PlasmaCore.Types.BottomEdge && parentItem.reversedEnabled)) {
            effectivePrefix = "north-" + prefix;
        }

        if ((plasmoid.location === PlasmaCore.Types.RightEdge && !parentItem.reversedEnabled)
                || (plasmoid.location === PlasmaCore.Types.LeftEdge && parentItem.reversedEnabled)) {
            effectivePrefix = "east-" + prefix;
        }

        if ((plasmoid.location === PlasmaCore.Types.BottomEdge && !parentItem.reversedEnabled)
                || (plasmoid.location === PlasmaCore.Types.TopEdge && parentItem.reversedEnabled)) {
            effectivePrefix = "south-" + prefix;
        }

        return [effectivePrefix, prefix];
    }

    states: [
        State {
            name: "launcher"
            when: parentItem.isLauncher || (parentItem.isApplet && !parentItem.isActive)

            PropertyChanges {
                target: frame
                basePrefix: ""
            }
        },
        State {
            name: "attention"
            when: parentItem.inAttention

            PropertyChanges {
                target: frame
                basePrefix: "attention"
            }
        },
        State {
            name: "minimized"
            when: parentItem.isMinimized

            PropertyChanges {
                target: frame
                basePrefix: "minimized"
            }
        },
        State {
            name: "active"
            when: parentItem.isActive

            PropertyChanges {
                target: frame
                basePrefix: "focus"
            }
        }
    ]
}
