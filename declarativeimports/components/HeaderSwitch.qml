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

    Layout.rightMargin: {
        if (level === 1) {
            return Qt.application.layoutDirection === Qt.RightToLeft ? 0 : 2 * units.smallSpacing
        }

        return 0;
    }
    Layout.leftMargin: {
        if (level === 1) {
            return Qt.application.layoutDirection === Qt.RightToLeft ? 2 * units.smallSpacing : 0
        }

        return 0;
    }

    property int level:1
    property bool checked: false
    property bool isFirstSubCategory: false

    readonly property int implicitWidth: row.width

    readonly property int implicitHeight: {
        if (level === 1) {
            return Math.max(headerText.implicitHeight, itemSwitch.implicitHeight);
        } else if (level === 2) {
            return Math.max(subHeaderText.implicitHeight, itemSwitch.implicitHeight)
        }

        return Math.max(labelText.implicitHeight, itemSwitch.implicitHeight);
    }

    property string text:""
    property string tooltip:""

    signal pressed();

    Item {
        id: row
        width: parent.width
        height: textElement.height
        anchors.verticalCenter: parent.verticalCenter

        RowLayout {
            id: textElement
            anchors.left: level !== 2 ? parent.left : undefined
            anchors.horizontalCenter: level === 2 ? parent.horizontalCenter : undefined
            anchors.verticalCenter: parent.verticalCenter

            LatteExtraControls.Header {
                id: headerText
                text: item.text
                enabled: item.checked
                visible: level === 1
            }

            LatteExtraControls.SubHeader {
                id: subHeaderText
                text: item.text
                enabled: item.checked
                visible: level === 2
                isFirstSubCategory: item.isFirstSubCategory
            }

            PlasmaComponents.Label {
                id: labelText
                text: item.text
                enabled: item.checked
                visible: level > 2
            }
        }

        PlasmaComponents.Button {
            //tooltip ghost
            anchors.fill: textElement
            tooltip: item.tooltip
            opacity: 0
            onPressedChanged: {
                if (pressed) {
                    item.pressed();
                }
            }
        }
    }

    Switch {
        id: itemSwitch
        anchors.verticalCenter: row.verticalCenter
        anchors.right: row.right
        checked: item.checked

        style: Private.SwitchStyle {}

        PlasmaComponents.Button {
            //tooltip ghost
            anchors.fill: parent
            tooltip: item.tooltip
            opacity: 0
            onPressedChanged: {
                if (pressed) {
                    item.pressed();
                }
            }
        }
    }
}
