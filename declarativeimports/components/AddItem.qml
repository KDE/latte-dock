/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.plasma.plasmoid 2.0

import "code/ColorizerTools.js" as ColorizerTools

Item{
    id: addItem

    property real backgroundOpacity: 1

    Rectangle{
        width: Math.min(parent.width, parent.height)
        height: width
        anchors.centerIn: parent

        radius: 0.05 * Math.max(width,height)

        color: Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b, backgroundOpacity)
        border.width: 1
        border.color: outlineColor

        property int crossSize: Math.min(0.4*parent.width, 0.4 * parent.height)

        readonly property color outlineColorBase: theme.backgroundColor
        readonly property real outlineColorBaseBrightness: ColorizerTools.colorBrightness(outlineColorBase)
        readonly property color outlineColor: {
            if (outlineColorBaseBrightness > 127.5) {
                return Qt.darker(outlineColorBase, 1.5);
            } else {
                return Qt.lighter(outlineColorBase, 2.2);
            }
        }

        Rectangle{width: parent.crossSize; height: 4; radius:2; anchors.centerIn: parent; color: theme.highlightColor}
        Rectangle{width: 4; height: parent.crossSize; radius:2; anchors.centerIn: parent; color: theme.highlightColor}
    }
}
