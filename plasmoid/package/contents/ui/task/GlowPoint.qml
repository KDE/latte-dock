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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components

import QtGraphicalEffects 1.0

Item{
    //   property string color
    id: glowItem

    property bool roundCorners: true
    property bool showAttention: false

    property bool showGlow: false
    property int animation: Math.max(1.65*3*units.longDuration,root.durationTime*3*units.longDuration)

    property color attentionColor: colorScopePalette.negativeTextColor // "#ffff1717"
    property color basicColor: "blue"

    /*onShowAttentionChanged: {
        if(!showAttention){
            smallCircle.color = basicColor;
        }
    }*/

    Item{
        id:mainGlow
        anchors.fill: parent

        Rectangle {
            id: smallCircle
            anchors.centerIn: parent
            anchors.fill: parent

            color: glowItem.basicColor
            radius: glowItem.roundCorners ? Math.min(width,height) / 2 : 0
        }

        Loader{
            anchors.centerIn: parent
            anchors.fill: parent

            active: glowItem.showAttention

            sourceComponent:Rectangle {
                id: smallCircleInAttention

                color: glowItem.basicColor
                radius: smallCircle.radius

                SequentialAnimation{
                    running: glowItem.showAttention
                    loops: Animation.Infinite
                    alwaysRunToEnd: true

                    PropertyAnimation {
                        target: smallCircleInAttention
                        property: "color"
                        to: glowItem.attentionColor
                        duration: glowItem.animation
                        easing.type: Easing.InOutQuad
                    }

                    PropertyAnimation {
                        target: smallCircleInAttention
                        property: "color"
                        to: glowItem.basicColor
                        duration: glowItem.animation
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        RectangularGlow {
            id:recGlow
            anchors.fill: smallCircle
            glowRadius: 2 * Math.min(smallCircle.width, smallCircle.height)
            spread: 0.2
            color: glowItem.showAttention ? smallCircleInAttention.color : smallCircle.color
            opacity: root.showBarLine ? 0.25 : 0.45
            visible: glowItem.showGlow
        }
    }
}
