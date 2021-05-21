/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/**Documented API
Inherits:
        Item
Imports:
        QtQuick 2.1
        org.kde.plasma.core
Description:
TODO i need more info here
Properties:
**/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: main
    state: parent.state
    //used to tell apart this implementation with the touch components one
    property bool hasOverState: true
    property alias enabledBorders: shadow.enabledBorders

    PlasmaCore.FrameSvgItem {
        id: hover

        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        opacity: 0
        imagePath: "widgets/button"
        prefix: "hover"
    }

    PlasmaCore.FrameSvgItem {
        id: shadow

        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        imagePath: "widgets/button"
        prefix: "shadow"
    }

    states: [
        State {
            name: "shadow"
            PropertyChanges {
                target: shadow
                opacity: 1
            }
            PropertyChanges {
                target: hover
                opacity: 0
                prefix: "hover"
            }
        },
        State {
            name: "hover"
            PropertyChanges {
                target: shadow
                opacity: 0
            }
            PropertyChanges {
                target: hover
                opacity: 1
                prefix: "hover"
            }
        },
        State {
            name: "focus"
            PropertyChanges {
                target: shadow
                opacity: 0
            }
            PropertyChanges {
                target: hover
                opacity: 1
                prefix: "focus"
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                target: shadow
                opacity: 0
            }
            PropertyChanges {
                target: hover
                opacity: 0
                prefix: "hover"
            }
        }
    ]

    transitions: [
        Transition {
            PropertyAnimation {
                properties: "opacity"
                duration: units.longDuration
                easing.type: Easing.OutQuad
            }
        }
    ]
}
