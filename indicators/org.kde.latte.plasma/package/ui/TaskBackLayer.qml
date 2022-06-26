/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    id: frame
    property string basePrefix: "normal"

    imagePath: "widgets/tasks"
    //rotation: root.reversedEnabled ? 180 : 0
    opacity: 1
    prefix: indicator.isHovered ? root.taskPrefixHovered(basePrefix) : root.taskPrefix(basePrefix)
    visible: indicator.isTask || indicator.isLauncher || indicator.isGroup

    states: [
        State {
            name: "launcher"
            when: indicator.isLauncher

            PropertyChanges {
                target: frame
                basePrefix: ""
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
