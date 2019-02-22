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

import QtQuick 2.0
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Rectangle {
    id: unityRect
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 2
    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width - 4
    height: parent.height - 4
    radius: 4
    //height: (root.iconSize + root.thickMargin)/6 - 4
    //radius: height/2
    color: rootItem.backgroundColor
    clip: true

   property Item rootItem: parent

    RadialGradient{
        anchors.verticalCenter: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: width

        gradient: Gradient {
            GradientStop { position: 0.0; color: rootItem.glowColor }
            GradientStop { position: 0.6; color: "transparent" }
        }
    }

    visible: rootItem.isActive || rootItem.hasShown
}
