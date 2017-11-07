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
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    id: root

    imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    //imagePath: "widgets/panel-background"
    //imagePath: ""
    prefix:""
    // onRepaintNeeded: adjustPrefix();

    property Item containment
    property Item dockLayout

    readonly property bool verticalPanel: containment && containment.formFactor === PlasmaCore.Types.Vertical

    /*  Rectangle{
        anchors.fill: parent
        color: "transparent"
        border.color: "blue"
        border.width: 1
    }*/

    function adjustPrefix() {
        if (!containment) {
            return "";
        }
        var pre;
        switch (containment.location) {
        case PlasmaCore.Types.LeftEdge:
            pre = "west";
            break;
        case PlasmaCore.Types.TopEdge:
            pre = "north";
            break;
        case PlasmaCore.Types.RightEdge:
            pre = "east";
            break;
        case PlasmaCore.Types.BottomEdge:
            pre = "south";
            break;
        default:
            prefix = "";
        }
        if (hasElementPrefix(pre)) {
            prefix = pre;
        } else {
            prefix = "";
        }
    }

    Component.onDestruction: {
        console.log("dock view qml source deleting...");

        if (containment) {
            containment.locationChanged.disconnect(adjustPrefix);
        }
    }

    onContainmentChanged: {
        console.log("dock view qml source - containment changed 1...");
        if (!containment) {
            return;
        }
        console.log("dock view qml source - containment changed 2...");

        containment.parent = containmentParent;
        containment.visible = true;
        containment.anchors.fill = containmentParent;
        containment.locationChanged.connect(adjustPrefix);
        adjustPrefix();

        for(var i=0; i<containment.children.length; ++i){
            if (containment.children[i].objectName === "dockLayoutView") {
                dockLayout = containment.children[i];
                dockLayout.dock = dock;
                dockLayout.universalSettings = universalSettings;
                dockLayout.universalLayoutManager = layoutManager;
            }
        }
    }

    Item {
        id: containmentParent
        anchors.fill: parent
    }

    //! it is used in order to check the right click position
    //! the only way to take into account the visual appearance
    //! of the applet (including its spacers)
    function appletContainsPos(appletId, pos) {
        if (dockLayout) {
            return dockLayout.appletContainsPos(appletId, pos);
        }

        return false;
    }
}
