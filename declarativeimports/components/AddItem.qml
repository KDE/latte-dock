/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

        radius: Math.max(width,height)/2

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
