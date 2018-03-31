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

import org.kde.plasma.plasmoid 2.0

Rectangle {
    property double proportion: 0

    property double previousProportion: 0

    property int numberValue
    property string textValue

    property bool fullCircle: true
    property bool showNumber: true
    property bool showText: false
    property bool textWithBackgroundColor: false

    property int radiusPerCentage: 100
    property int minimumWidth: 0

    property double circleOpacity: 1
    property double fontPixelSize: partSize // * 0.55

    property double stdThickness: partSize < 0 ? 0 : partSize  //  "/2.1"
    property double circleThicknessAttr: fullCircle ? 0 : stdThickness * 0.9
    property double partSize: height / 2
    property double pi2: Math.PI * 2

    property color alphaBackColor: Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b, 0.45)
    property color alphaBackColor2: Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b, 0.8)

    width: Math.max(minimumWidth, valueText.width + 4*units.smallSpacing)

    color: theme.backgroundColor //mainItemContainer.badgeIndicator > 0 ? alphaBackColor2 : alphaBackColor
    radius: (radiusPerCentage / 100) * (height / 2)
    border.width: Math.max(1,width/64)
    border.color: root.minimizedDotColor //alphaBackColor2

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
        opacity: proportion > 0 ? 1 : 0

        anchors.centerIn: parent

        property color drawColor:  theme.buttonFocusColor; //mainItemContainer.badgeIndicator > 0 ? theme.buttonFocusColor : theme.highlightColor;

        onDrawColorChanged: requestPaint();

        onPaint: {
            var ctx = getContext('2d');
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.fillStyle = drawColor;

            var startRadian = - Math.PI / 2;

            var radians = pi2 * proportion;

            ctx.beginPath();
            ctx.arc(width/2, height/2, stdThickness, startRadian, startRadian + radians + filler, false);
            ctx.arc(width/2, height/2, circleThicknessAttr, startRadian + radians + filler, startRadian, true);

            ctx.closePath();
            ctx.fill();
        }
    }

    Rectangle {
        id: badgerBackground
        anchors.fill: canvas
        color: canvas.drawColor

        visible: proportion === 100 && showNumber
        radius: parent.radius
    }

    Text {
        id: valueText
        anchors.centerIn: parent
        text: {
            if (showNumber) {
                if (numberValue > 9999) {
                    return i18nc("Over 9999 new messages, overlay, keep short", "9,999+");
                } else if (numberValue > 0) {
                    return numberValue.toLocaleString(Qt.locale(), 'f', 0);
                } else {
                    return "";
                }
            }

            if (showText) {
                return textValue;
            }
        }
        font.pixelSize: 0.55 * parent.height
        font.bold: true
        color: textWithBackgroundColor ? theme.backgroundColor : theme.textColor
        visible: showNumber || showText
    }


    Rectangle{
        anchors.fill: parent
        anchors.topMargin: parent.border.width
        anchors.bottomMargin: parent.border.width
        anchors.leftMargin: parent.border.width
        anchors.rightMargin: parent.border.width
        color: "transparent"
        border.width: parent.border.width
        border.color: parent.alphaBackColor2
        radius: parent.radius
    }

    Rectangle{
        anchors.fill: parent
        border.width: parent.border.width
        border.color: parent.border.color
        color: "transparent"
        radius: parent.radius
    }
}

