/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {    id: frame
    property string basePrefix: "normal"

    imagePath: "widgets/tabbar"
    rotation: 0

    opacity: 1

    prefix: {
        if (!indicator.isActive) {
            return "";
        }

        if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            return "west-active-tab";
        }

        if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            return "north-active-tab";
        }

        if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            return "east-active-tab";
        }

        if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
            return "south-active-tab";
        }

        return  "south-active-tab";
    }

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
