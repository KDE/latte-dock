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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Item{
    property Item rootItem: parent

    readonly property bool needsIconColors: true

    Item{
        width: rootItem.isTask ? Math.min(parent.width, parent.height) : parent.width
        height: rootItem.isTask ? width : parent.height
        anchors.centerIn: parent

        property bool isActive: rootItem.isActive || (rootItem.isWindow && rootItem.hasActive)

        Rectangle {
            id: unityRect
            anchors.fill: parent
            visible: rootItem.isActive || (rootItem.isWindow && rootItem.hasShown)

            radius: rootItem.iconSize / 12
            color: rootItem.backgroundColor
            clip: true
        }

        RadialGradient{
            id: glowGradient
            anchors.verticalCenter: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - unityRect.anchors.margins * 2 - 1
            height: (width * 0.85) - unityRect.anchors.margins * 2 - 1
            visible: false

            gradient: Gradient {
                GradientStop { position: 0.0;
                    color: {
                        if (rootItem.isMinimized) {
                            return "#aafcfcfc";
                        }

                        return rootItem.glowColor;
                    }
                }
                GradientStop { position: 0.6; color: "transparent" }
            }
        }

        Item {
            id: gradientMask
            anchors.fill: glowGradient

            Rectangle {
                anchors.top: gradientMask.verticalCenter
                anchors.topMargin: unityRect.anchors.margins
                width: glowGradient.width
                height: glowGradient.height / 2
                radius: unityRect.radius
            }

            visible: false
        }

        OpacityMask {
            anchors.fill: glowGradient
            source: glowGradient
            maskSource: gradientMask
            visible: unityRect.visible || borderRectangle.visible
        }

        Rectangle {
            id: borderRectangle
            anchors.fill: parent
            visible: (rootItem.isTask && rootItem.isWindow) || (rootItem.isApplet && rootItem.isActive)
            color: "transparent"
            border.width: 1
            border.color: "#606060"
            radius: unityRect.radius
            clip: true

            Rectangle {
                anchors.fill: parent
                anchors.margins: parent.border.width
                radius: unityRect.radius
                color: "transparent"
                border.width: 1
                border.color: "#25dedede"
            }
        }
    }
}
