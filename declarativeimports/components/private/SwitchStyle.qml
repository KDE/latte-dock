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

import QtQuick 2.2
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import QtQuick.Controls 1.2
import org.kde.plasma.core 2.0 as PlasmaCore

QtQuickControlStyle.SwitchStyle {
    id: styleRoot

    handle: PlasmaCore.FrameSvgItem {
        opacity: control.enabled ? 1.0 : 0.6
        width: height
        imagePath: "widgets/button"
        prefix: "shadow"

        PlasmaCore.FrameSvgItem {
            id: button
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "normal"
        }
    }

    groove: Item {
        implicitHeight: theme.mSize(theme.defaultFont).height
        implicitWidth: height * 2

        PlasmaCore.FrameSvgItem {
            anchors.fill: parent
            implicitHeight: parent.implicitHeight
            implicitWidth: parent.implicitWidth
            imagePath: "widgets/slider"
            prefix: "groove"
            opacity: control.checked ? 0 : 1
            visible: opacity > 0
            Behavior on opacity {
                PropertyAnimation { duration: units.shortDuration * 2 }
            }
        }

        PlasmaCore.FrameSvgItem {
            id: highlight
            anchors.fill: parent
            implicitHeight: parent.implicitHeight
            implicitWidth: parent.implicitWidth
            imagePath: "widgets/slider"
            prefix: "groove-highlight"

            opacity: control.checked ? 1 : 0
            visible: opacity > 0

            Behavior on opacity {
                PropertyAnimation { duration: units.shortDuration * 2 }
            }
        }
    }
}
