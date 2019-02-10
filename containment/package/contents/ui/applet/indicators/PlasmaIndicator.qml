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
    id: expandedItem
    anchors.fill: parent
    imagePath: "widgets/tabbar"
    visible: opacity > 0
    prefix: {
        var prefix;
        switch (plasmoid.location) {
        case PlasmaCore.Types.LeftEdge:
            prefix = "west-active-tab";
            break;
        case PlasmaCore.Types.TopEdge:
            prefix = "north-active-tab";
            break;
        case PlasmaCore.Types.RightEdge:
            prefix = "east-active-tab";
            break;
        default:
            prefix = "south-active-tab";
        }
        if (!hasElementPrefix(prefix)) {
            prefix = "active-tab";
        }
        return prefix;
    }

    opacity: isActive ? 1 : 0

    property bool isActive: (appletItem.isExpanded
                             && communicator.overlayLatteIconIsActive
                             && !appletItem.isSystray
                             && applet.pluginName !== root.plasmoidName
                             && applet.pluginName !== "org.kde.activeWindowControl"
                             && applet.pluginName !== "org.kde.plasma.appmenu")

    Behavior on opacity {
        NumberAnimation {
            duration: units.shortDuration
            easing.type: Easing.InOutQuad
        }
    }
}
