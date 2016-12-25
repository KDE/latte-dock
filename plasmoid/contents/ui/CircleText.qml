/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.2
import QtGraphicalEffects 1.0

Item {
    property double proportion

    property string valueLabel
    property string numberValue

    property bool fullCircle
    property bool showNumber
    property bool showLabel

    property double circleOpacity: 1

    property double fontPixelSize: partSize // * 0.55

    property double stdThickness: partSize < 0 ? 0 : partSize  //  "/2.1"
    property double circleThicknessAttr: fullCircle ? 0 : stdThickness * 0.9
    property double partSize: height / 2
    property double pi2: Math.PI * 2

    onProportionChanged: {
        repaint()
    }

    function repaint() {
        canvas.requestPaint()
    }

    Canvas {
        id: canvas

        property int lineWidth: 1
        property bool fill: true
        property bool stroke: true
        property real alpha: 1.0

        // edge bleeding fix
        readonly property double filler: 0.01

        width: parent.width
        height: parent.height
        antialiasing: true
        opacity: 1.0

        onPaint: {
            var ctx = getContext('2d')
            ctx.clearRect(0, 0, canvas.width, canvas.height)
            ctx.fillStyle = theme.highlightColor

            var startRadian = - Math.PI / 2

            var radians = pi2 * proportion

            ctx.beginPath();
            ctx.arc(width/2, height/2, stdThickness, startRadian, startRadian + radians + filler, false)
            ctx.arc(width/2, height/2, circleThicknessAttr, startRadian + radians + filler, startRadian, true)

            ctx.closePath()
            ctx.fill()
        }
    }

    DropShadow {
        anchors.fill: canvas
        radius: 4
        samples: 8
        spread: 0.5
        fast: true
        color: theme.backgroundColor
        source: canvas
    }

    Text {
        id: valueText
        anchors.centerIn: parent
        text: numberValue
        font.pixelSize: fontPixelSize
        color: theme.textColor
        visible: showNumber
    }

    DropShadow {
        anchors.fill: valueText
        radius: 3
        samples: 8
        spread: 0.6
        fast: true
        color: theme.backgroundColor
        source: valueText
        visible: showNumber
    }

    Text {
        id: valueTextLabel
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -2
        anchors.right: parent.right
        text: valueLabel
        font.pixelSize: fontPixelSize * 0.65
        color: theme.textColor
        visible: true
    }
}

