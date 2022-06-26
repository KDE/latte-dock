/*
    SPDX-FileCopyrightText: 2022 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    id: frame
    imagePath: "widgets/tabbar"
    opacity: indicator.isActive ? 1 : 0
    visible: indicator.isApplet

    prefix: {
        let prefix;
        switch (Plasmoid.location) {
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

    Behavior on opacity {
        NumberAnimation {
            duration: PlasmaCore.Units.shortDuration
            easing.type: Easing.InOutQuad
        }
    }
}
