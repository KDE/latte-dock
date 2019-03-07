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

import QtQuick 2.7
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.2 as Latte


Item{
    property color iconColor: "red"

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
