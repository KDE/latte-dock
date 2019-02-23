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

    Item{
        width: Math.min(parent.width, parent.height)
        height: width
        anchors.centerIn: parent

        Rectangle {
            id: unityRect
            anchors.fill: parent
            anchors.margins: 4
            visible: rootItem.isActive || (rootItem.isWindow && rootItem.hasShown)

            radius: 8
            //height: (root.iconSize + root.thickMargin)/6 - 4
            //radius: height/2
            color: {
                if (rootItem.inAttention) {
                    return theme.negativeTextColor;
                }

                if (isActive) {
                    return theme.buttonFocusColor;
                }

                return rootItem.backgroundColor;
            }
            clip: true

            property bool isActive: rootItem.isActive || (rootItem.isWindow && rootItem.hasActive)

            RadialGradient{
                anchors.verticalCenter: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: width * 0.85

                gradient: Gradient {
                    GradientStop { position: 0.0;
                        color: {
                            if (rootItem.inAttention) {
                                return Qt.lighter(theme.negativeTextColor, 1.5)
                            }

                            if (isActive) {
                                return Qt.lighter(theme.buttonFocusColor, 1.5)
                            }

                            return rootItem.glowColor;
                        }
                    }
                    GradientStop { position: 0.6; color: "transparent" }
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            visible: (rootItem.isTask && rootItem.isWindow) || (rootItem.isApplet && rootItem.isActive)
            color: "transparent"
            border.width: Math.max(1, rootItem.iconSize / 64)
            border.color: "#606060"
            radius: 8
            clip: true

            RadialGradient{
                anchors.verticalCenter: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: width * 0.85
                visible: !unityRect.visible

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#aafcfcfc" }
                    GradientStop { position: 0.6; color: "transparent" }
                }
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: parent.border.width
                radius: 8
                color: "transparent"
                border.width: parent.border.width
                border.color: "#25dedede"
            }
        }
    }
}
