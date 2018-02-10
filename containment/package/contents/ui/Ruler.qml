/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item{
    property color foregroundColor: "#d7e3ff"
    opacity: behaveAsPlasmaPanel ? 0.7 : 1

    RowLayout{
        width: parent.width

        layer.enabled: true
        layer.effect: DropShadow{
            radius: 6
            fast: true
            samples: 2 * radius
            color: "#040404"
        }

        Rectangle{
            width: 2
            height: theme.defaultFont.pixelSize

            color: foregroundColor
        }

        Item{
            width: 0.6 * theme.defaultFont.pixelSize
            height: theme.defaultFont.pixelSize
            clip:true

            Rectangle{
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.right
                width: 0.75*theme.defaultFont.pixelSize
                height: width
                rotation: 45

                color: foregroundColor
            }
        }

        Rectangle{
            Layout.fillWidth: true
            height: 2
            color: foregroundColor
        }

        PlasmaComponents.Label{
            id: maxLengthLbl
            text: i18n("Maximum Length")
            color: foregroundColor
        }

        Rectangle{
            Layout.fillWidth: true
            height: 2
            color: foregroundColor
        }

        Item{
            width: 0.6 * theme.defaultFont.pixelSize
            height: theme.defaultFont.pixelSize
            clip:true

            Rectangle{
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.left
                width: 0.75*theme.defaultFont.pixelSize
                height: width
                rotation: 45

                color: foregroundColor
            }
        }

        Rectangle{
            width: 2
            height: theme.defaultFont.pixelSize

            color: foregroundColor
        }
    }

}
