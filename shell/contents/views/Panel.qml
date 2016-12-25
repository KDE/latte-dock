/*
 *  Copyright 2012 Marco Martin <mart@kde.org>
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
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    id: root

    //imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    //imagePath: "widgets/panel-background"
    imagePath: ""
    prefix:""
    // onRepaintNeeded: adjustPrefix();

    //enabledBorders: panel.enabledBorders

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

    onContainmentChanged: {
        if (!containment) {
            return;
        }
        containment.parent = containmentParent;
        containment.visible = true;
        containment.anchors.fill = containmentParent;
        containment.locationChanged.connect(adjustPrefix);
        adjustPrefix();

        for(var i=0; i<containment.children.length; ++i){
            if (containment.children[i].objectName === "dockLayoutView") {
                dockLayout = containment.children[i];
                dockLayout.dockView = panel;
            }
        }
    }

    Binding {
        target: panel
        property: "length"
        when: containment
        value: {
            if (!containment) {
                return;
            }
            if (verticalPanel) {
                return containment.Layout.preferredHeight
            } else {
                return containment.Layout.preferredWidth
            }
        }
    }


    Item {
        id: containmentParent
        anchors.fill: parent
    }
} 
