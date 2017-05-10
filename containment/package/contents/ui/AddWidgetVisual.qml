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

import QtQuick 2.1

Item{
    id: newDroppedLauncherVisual
    anchors.fill: parent

    Rectangle{
        anchors.fill: parent

        radius: root.iconSize/10

        property color tempColor: "#aa222222"
        color: tempColor
        border.width: 1
        border.color: "#ff656565"

        property int crossSize: Math.min(parent.width/2, parent.height/2)

        Rectangle{width: parent.crossSize; height: 4; radius:2; anchors.centerIn: parent; color: theme.highlightColor}
        Rectangle{width: 4; height: parent.crossSize; radius:2; anchors.centerIn: parent; color: theme.highlightColor}
    }
}
