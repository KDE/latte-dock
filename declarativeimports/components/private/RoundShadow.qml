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

/**Documented API
Inherits:
        Item
Imports:
        QtQuick 2.1
        org.kde.plasma.core
Description:
        It is a simple Radio button which is using the plasma theme.
        TODO Do we need more info?
Properties:
        TODO needs more info??
**/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: main
    state: parent.state
    property alias imagePath: shadowSvg.imagePath
    property string hoverElement: "hover"
    property string focusElement: "focus"
    property alias shadowElement: shadow.elementId

    //used to tell apart this implementation with the touch components one
    property bool hasOverState: true

    PlasmaCore.Svg {
        id: shadowSvg
        imagePath: "widgets/actionbutton"
    }

    PlasmaCore.SvgItem {
        id: hover
        svg: shadowSvg
        elementId: "hover"

        anchors.fill: parent

        opacity: 0
    }

    PlasmaCore.SvgItem {
        id: shadow
        svg: shadowSvg
        elementId: "shadow"

        anchors.fill: parent
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
                elementId: hoverElement
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
                elementId: hoverElement
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
                elementId: focusElement
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
                elementId: hoverElement
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
