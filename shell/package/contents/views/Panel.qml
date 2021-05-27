/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    property Item viewLayout

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
        console.log("latte view qml source deleting...");

        if (containment) {
            containment.locationChanged.disconnect(adjustPrefix);
        }
    }

    onContainmentChanged: {
        console.log("latte view qml source - containment changed 1...");
        if (!containment) {
            return;
        }
        console.log("latte view qml source - containment changed 2...");

        containment.parent = containmentParent;
        containment.visible = true;
        containment.anchors.fill = containmentParent;
        containment.locationChanged.connect(adjustPrefix);
        adjustPrefix();

        for(var i=0; i<containment.children.length; ++i){
            if (containment.children[i].objectName === "containmentViewLayout") {
                viewLayout = containment.children[i];
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
        if (viewLayout) {
            return viewLayout.appletContainsPos(appletId, pos);
        }

        return false;
    }
}
