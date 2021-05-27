/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore

Canvas {
    property color color: "lightblue"
    property bool isHorizontal: true
    property int spriteMargin: 1
    property int spritePosition: PlasmaCore.Types.CenterPositioned
    property int spriteSize: 5

    readonly property int length: isHorizontal ? width : height
    readonly property int currentSprite: Math.min(spriteSize, maxSpriteSize)

    readonly property int maxSpriteSize: (length - 2*spriteMargin)/7

    readonly property int centeredrectstart: {
        if (spritePosition === PlasmaCore.Types.LeftPositioned
                || spritePosition === PlasmaCore.Types.TopPositioned) {
            return spriteMargin;
        } else if (spritePosition === PlasmaCore.Types.RightPositioned
                   || spritePosition === PlasmaCore.Types.BottomPositioned) {
            return isHorizontal ? (width - (7*currentSprite) - spriteMargin) : (height - (7*currentSprite) - spriteMargin)
        }

        return isHorizontal ? (width - 7*currentSprite) / 2 : (height - 7*currentSprite)/2
    }

    readonly property int a1: 0
    readonly property int b1: centeredrectstart + 4*currentSprite
    readonly property int c1: centeredrectstart
    readonly property int d1: 0

    readonly property int a2: b1 + currentSprite
    readonly property int b2: a2 + currentSprite
    readonly property int c2: c1 + 2 * currentSprite
    readonly property int d2: c1 + currentSprite

    readonly property int a3: b2 + currentSprite
    readonly property int b3: isHorizontal ? width : height
    readonly property int c3: isHorizontal ? width : height
    readonly property int d3: c2 + currentSprite

    onColorChanged: requestPaint()
    onSpriteMarginChanged: requestPaint()
    onSpritePositionChanged: requestPaint()
    onSpriteSizeChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    onPaint:{
        var ctx = getContext("2d");

        ctx.beginPath();
        ctx.clearRect(0,0,width,height);
        ctx.fill();

        ctx.fillStyle = color;
        ctx.lineWidth=1;

        var start = isHorizontal ? 0 : width;
        var end = isHorizontal ? height : 0;

        if (isHorizontal) {
            ctx.beginPath();
            ctx.moveTo(a1,start);
            ctx.lineTo(b1,start);
            ctx.lineTo(c1,end);
            ctx.lineTo(d1,end);
            ctx.fill();

            ctx.beginPath ();
            ctx.moveTo(a2,start);
            ctx.lineTo(b2,start);
            ctx.lineTo(c2,end);
            ctx.lineTo(d2,end);
            ctx.fill();

            ctx.beginPath ();
            ctx.moveTo(a3,start);
            ctx.lineTo(b3,start);
            ctx.lineTo(c3,end);
            ctx.lineTo(d3,end);
            ctx.fill();
        } else {
            ctx.beginPath ();
            ctx.moveTo(start,a1);
            ctx.lineTo(start,b1);
            ctx.lineTo(end, c1);
            ctx.lineTo(end, d1);
            ctx.fill();

            ctx.beginPath ();
            ctx.moveTo(start,a2);
            ctx.lineTo(start,b2);
            ctx.lineTo(end,c2);
            ctx.lineTo(end,d2);
            ctx.fill();

            ctx.beginPath ();
            ctx.moveTo(start,a3);
            ctx.lineTo(start,b3);
            ctx.lineTo(end,c3);
            ctx.lineTo(end,d3);
            ctx.fill();
        }
    }
}
