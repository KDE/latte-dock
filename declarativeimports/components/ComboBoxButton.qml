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

import QtQuick 2.1
import QtQuick.Layouts 1.3

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

Rectangle {
    id: root
    color: "transparent"

    implicitWidth: buttonMetrics.implicitWidth
    implicitHeight: buttonMetrics.implicitHeight

    readonly property Item comboBox: mainComboBox
    readonly property Item button: mainButton

    property bool buttonEnabled: true
    property string buttonText:""
    property string buttonIconSource:""
    property string buttonToolTip: ""
    property QtObject buttonExclusiveGroup: null

    property bool checked: false
    property bool checkable: false

    property bool comboBoxEnabled: true
    property bool comboBoxBlankSpaceForEmptyIcons: false
    property bool comboBoxForcePressed: false
    property int comboBoxMinimumPopUpWidth: 150
    property string comboBoxTextRole: ""
    property string comboBoxIconRole: ""

    PlasmaComponents.Button {
        id: mainButton
        anchors.left: Qt.application.layoutDirection === Qt.RightToLeft ? undefined : parent.left
        anchors.right: Qt.application.layoutDirection === Qt.RightToLeft ? parent.right : undefined
        LayoutMirroring.enabled: false
        enabled: buttonEnabled
        checked: root.checked
        checkable: root.checkable
        exclusiveGroup: buttonExclusiveGroup

        width: parent.width
        height: mainComboBox.height

        text: checkable ?  " " : buttonText
        iconSource: buttonIconSource
        tooltip: buttonToolTip
    }

    //overlayed combobox
    LatteComponents.ComboBox {
        id: mainComboBox
        anchors.right: mainButton.right
        anchors.top: parent.top

        width:  units.iconSizes.medium - units.smallSpacing
        height: parent.height

        enabled: comboBoxEnabled

        iconRole: comboBoxIconRole
        textRole: comboBoxTextRole

        blankSpaceForEmptyIcons: comboBoxBlankSpaceForEmptyIcons
        forcePressed: comboBoxForcePressed
        popUpRelativeX: Qt.application.layoutDirection === Qt.RightToLeft ? root.width : -(parent.width - width)

        minimumPopUpWidth: Math.max(comboBoxMinimumPopUpWidth, root.width)
    }

    Label{
        width: labelMetrics.exceeds ? parent.width-mainComboBox.width :  parent.width
        height: parent.height
        text: buttonText
        font: mainButton.font
        color: theme.buttonTextColor
        visible: root.checkable

        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Label{
        id: labelMetrics
        text: root.buttonText
        opacity: 0
        elide: Text.ElideNone
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        readonly property bool exceeds: width>(mainButton.width-mainComboBox.width)
    }
}
