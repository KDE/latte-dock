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

import QtQuick 2.2
import QtGraphicalEffects 1.0

Rectangle {
    property double proportion: 0

    property double previousProportion: 0

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

    property color alphaBackColor: Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b, 0.45)
    property color alphaBackColor2: Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b, 0.8)
    color: alphaBackColor
    radius: width
    border.width: Math.max(1,width/64)
    border.color: alphaBackColor2

    onProportionChanged: {
      //  console.log(previousProportion + " - "+proportion);
        if ((proportion - 0.03 >= previousProportion) || (proportion===1)) {
         //   console.log("request repaint...");
            previousProportion = proportion;
            repaint();
        }
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

        width: parent.width - 2 * parent.border.width
        height: parent.height - 2 * parent.border.width
        opacity: 0.7

        anchors.centerIn: parent

        onPaint: {
            var ctx = getContext('2d');
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.fillStyle = theme.highlightColor;

            var startRadian = - Math.PI / 2;

            var radians = pi2 * proportion;

            ctx.beginPath();
            ctx.arc(width/2, height/2, stdThickness, startRadian, startRadian + radians + filler, false);
            ctx.arc(width/2, height/2, circleThicknessAttr, startRadian + radians + filler, startRadian, true);

            ctx.closePath();
            ctx.fill();
        }
    }

    Text {
        id: valueText
        anchors.centerIn: parent
        text: numberValue
        font.pixelSize: fontPixelSize
        color: theme.textColor
        visible: showNumber
    }

    Rectangle{
        anchors.fill: parent
        border.width: parent.border.width
        border.color: parent.border.color
        color: "transparent"
        radius: parent.radius
    }
}

