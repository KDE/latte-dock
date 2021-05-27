/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

GraphicIcon{
    readonly property int itemLength: 0.22*width

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        color: "transparent"

        Rectangle{
            anchors.top: parent.top
            anchors.left: parent.left
            width: itemLength
            height: parent.height
            radius: width/2
            color: iconColor
        }

        Rectangle{
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: itemLength
            height: parent.height
            radius: width/2
            color: iconColor
        }

        Rectangle{
            anchors.top: parent.top
            anchors.right: parent.right
            width: itemLength
            height: parent.height
            radius: width/2
            color: iconColor
        }
    }
}
