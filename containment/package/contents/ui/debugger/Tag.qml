/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Item {
    id: tag
    width: _label.width + 4
    height: _label.height + 4

    readonly property alias background: _background
    readonly property alias label: _label

    Rectangle {
        id: _background
        anchors.fill: parent
        color: "black"
    }

    Text {
        id: _label
        anchors.centerIn: parent
        text: " --- "
        color: "white"
        font.pointSize: 11
    }
}
