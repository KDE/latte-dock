/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0

Rectangle {
    property double proportion: 0

    property double previousProportion: 0

    property bool style3d: true

    property int numberValue
    property string textValue

    property bool fullCircle: true
    property bool showNumber: true
    property bool showText: false
    property bool textWithBackgroundColor: false

    property int radiusPerCentage: 100
    property int minimumWidth: 0
    property int maximumWidth: 9999

    property double circleOpacity: 1
    property double fontPixelSize: partSize // * 0.55

    property double stdThickness: partSize < 0 ? 0 : partSize  //  "/2.1"
    property double circleThicknessAttr: fullCircle ? 0 : stdThickness * 0.9
    property double partSize: height / 2
    property double pi2: Math.PI * 2

    width: Math.max(minimumWidth, valueText.width + 4*units.smallSpacing)

    color: theme.backgroundColor
    radius: (radiusPerCentage / 100) * (height / 2)
    border.width: 0 //Math.max(1,width/64)

    property int borderWidth: 1
    property real borderOpacity: 1
    property color borderColor: theme.textColor
    property color textColor: theme.textColor
    property color highlightedColor: theme.buttonFocusColor

    readonly property bool singleCharacter: (showNumber && numberValue<=9 && numberValue>=0)|| (showText && textValue.length===1)

    onProportionChanged: {
        if (proportion<0.03) {
            previousProportion = 0;
        }

        //console.log(previousProportion + " - "+proportion);
        var currentStep = (proportion - previousProportion);
        if ((currentStep >= 0.01) || (proportion>=1 && previousProportion !==1)) {
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

        width: parent.width - 2 * parent.borderWidth
        height: parent.height - 2 * parent.borderWidth
        opacity: proportion > 0 ? 1 : 0

        anchors.centerIn: parent

        property color drawColor: highlightedColor

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

        visible: proportion === 1 && showNumber
        radius: parent.radius
    }

    Text {
        id: valueText
        anchors.centerIn: canvas

        width: Math.min(maximumWidth - 4*units.smallSpacing, implicitWidth)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        elide: Text.ElideRight

        text: {
            if (showNumber) {
                if (numberValue > 9999) {
                    return i18nc("Over 9999 new messages, overlay, keep short", "9,999+");
                } else if (numberValue > 0) {
                    return numberValue.toLocaleString(Qt.locale(), 'f', 0);
                }
            }

            if (showText) {
                return textValue;
            }

            return "";
        }
        font.pixelSize: 0.62 * parent.height
        font.bold: true
        color: textWithBackgroundColor ? parent.color : parent.textColor
        visible: showNumber || showText
    }

    Rectangle{
        anchors.fill: parent
        anchors.topMargin: parent.borderWidth
        anchors.bottomMargin: parent.borderWidth
        anchors.leftMargin: parent.borderWidth
        anchors.rightMargin: parent.borderWidth
        color: "transparent"
        border.width: parent.borderWidth > 0 ? parent.borderWidth+1 : 0
        border.color: "black"
        radius: parent.radius
        opacity: 0.4

        visible: style3d
    }

    Rectangle{
        anchors.fill: parent
        border.width: parent.borderWidth
        border.color: {
            if (style3d) {
                return parent.borderColor
            }

            return proportion === 1 ? parent.highlightedColor : parent.color
        }
        color: "transparent"
        radius: parent.radius
        opacity: parent.borderOpacity
    }
}

