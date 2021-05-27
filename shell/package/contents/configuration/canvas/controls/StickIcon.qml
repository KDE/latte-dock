/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore

GraphicIcon{
    readonly property int itemLength: 0.22*width

    Rectangle {
        anchors.fill: parent
        anchors.margins: parent.margin * 2
        color: "transparent"

        Rectangle{
            id: circle
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            radius: width/2
            color: iconColor
        }
    }
}
