/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

Item{
    Rectangle{
        anchors.fill: parent

        anchors.bottom: (panel.position === PlasmaCore.Types.TopPositioned) ? parent.bottom : undefined
        anchors.top: (panel.position === PlasmaCore.Types.BottomPositioned) ? parent.top : undefined
        anchors.left: (panel.position === PlasmaCore.Types.RightPositioned) ? parent.left : undefined
        anchors.right: (panel.position === PlasmaCore.Types.LeftPositioned) ? parent.right : undefined

        radius: panel.iconSize/10

        property color tempColor: "#aa222222"
        color: tempColor
        border.width: 1
        border.color: "#ff656565"

        property int crossSize: Math.min(parent.width/2, parent.height/2)

        Rectangle{width: parent.crossSize; height: 4; anchors.centerIn: parent; color: theme.highlightColor}
        Rectangle{width: 4; height: parent.crossSize; anchors.centerIn: parent; color: theme.highlightColor}
    }
}
