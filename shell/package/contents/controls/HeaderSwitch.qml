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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte

import "." as LatteExtraControls

import "private" as Private

Item {
    id: item

    property bool isHeader: true
    property bool checked: false

    readonly property int implicitWidth: isHeader ? headerText.width + itemSwitch.width : labelText.width + itemSwitch.width
    readonly property int implicitHeight: isHeader ? Math.max(headerText.implicitHeight, itemSwitch.implicitHeight) :
                                                     Math.max(labelText.implicitHeight, itemSwitch.implicitHeight)

    property string text:""
    property string tooltip:""

    signal pressed();

    RowLayout {
        id: row

        width: parent.width
        anchors.verticalCenter: parent.verticalCenter

        LatteExtraControls.Header {
            id: headerText
            Layout.fillWidth: true
            text: item.text
            enabled: item.checked
            visible: item.isHeader
        }

        PlasmaComponents.Label {
            id: labelText
            Layout.fillWidth: true
            text: item.text
            enabled: item.checked
            visible: !item.isHeader
        }

        Switch {
            id: itemSwitch
            checked: item.checked

            style: Private.SwitchStyle {}
        }
    }

    MouseArea {
        id: itemMouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: item.pressed();
    }

    ToolTip{
        parent: itemSwitch
        text: item.tooltip
        visible: itemMouseArea.containsMouse && text !==""
        delay: 7 * units.longDuration
    }
}
