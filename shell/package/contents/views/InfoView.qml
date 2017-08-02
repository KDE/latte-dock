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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.latte 0.1 as Latte

PlasmaCore.FrameSvgItem {
    id: dialog

    imagePath: "widgets/panel-background"
    prefix:""
    enabledBorders: infoWindow ? infoWindow.enabledBorders : PlasmaCore.FrameSvg.AllBorders

    width: Screen.width + 1
    height: Math.min(Screen.height - units.gridUnit * 8, logo.height + messageLbl.height + 2 *units.gridUnit)

    property string message

    ColumnLayout {
        id: root

        anchors.centerIn: parent

        Item{
            id: iconItem
            Layout.fillWidth: true
            Layout.minimumHeight: logo.height

            Latte.IconItem {
                id: logo
                anchors.centerIn: parent

                width: units.iconSizes.huge
                height: width

                source: "latte-dock"
            }
        }

        PlasmaComponents.Label {
            id: messageLbl
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            text: dialog.message
            font.italic: true
        }
    }
}
