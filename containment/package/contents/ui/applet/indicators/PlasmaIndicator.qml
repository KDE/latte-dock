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
    anchors.fill: parent

    imagePath: "widgets/tasks"

    property bool isActive: (appletItem.isExpanded
                             && communicator.overlayLatteIconIsActive
                             && !appletItem.isSystray
                             && applet.pluginName !== root.plasmoidName
                             && applet.pluginName !== "org.kde.activeWindowControl"
                             && applet.pluginName !== "org.kde.plasma.appmenu")


    property string basePrefix: ""

    prefix: taskPrefix(basePrefix)

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
            name: "active"
            when: frame.isActive

            PropertyChanges {
                target: frame
                basePrefix: "focus"
            }
        }
    ]
}
