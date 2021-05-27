/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick.Controls 1.4
import QtQuick 2.2
import QtQuick.Layouts 1.3

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.latte.components 1.0 as LatteComponents

Rectangle {
    id: root
    color: "transparent"

    implicitWidth: buttonMetrics.implicitWidth
    implicitHeight: buttonMetrics.implicitHeight

    property ExclusiveGroup exclusiveGroup: null
    property bool checked: false
    property bool checkable: false

    readonly property alias comboBox: mainComboBox
    readonly property alias button: mainButton

    property bool buttonEnabled: true
    property bool buttonIsTransparent: false
    property bool buttonIsTriggeringMenu: false
    property string buttonText:""
    property string buttonIconSource:""
    property string buttonToolTip: ""

    property bool comboBoxEnabled: true
    property bool comboBoxBlankSpaceForEmptyIcons: false
    property bool comboBoxForcePressed: false
    property bool comboBoxPopUpAlignRight: true
    property bool comboBoxButtonIsTransparent: false
    property bool comboBoxButtonIsVisible: true
    property int comboBoxMinimumPopUpWidth: 150
    property int comboBoxPopupTextHorizontalAlignment: Text.AlignLeft
    property string comboBoxEnabledRole: ""
    property string comboBoxTextRole: ""
    property string comboBoxIconRole: ""
    property string comboBoxIconToolTipRole: ""
    property string comboBoxIconOnlyWhenHoveredRole: ""
    property string comboBoxIsSeparatorRole: ""

    readonly property bool isButtonIndicatingMenuPopup: buttonIsTriggeringMenu && !comboBoxButtonIsVisible && mainComboBox.popup.visible

    signal iconClicked(int index);

    onExclusiveGroupChanged: {
        if (exclusiveGroup) {
            exclusiveGroup.bindCheckable(root);
        }
    }

    ExclusiveGroup {
        id: hiddenExclusiveGroup
    }

    PlasmaComponents.Button {
        id: mainButton
        anchors.left: Qt.application.layoutDirection === Qt.RightToLeft ? undefined : parent.left
        anchors.right: Qt.application.layoutDirection === Qt.RightToLeft ? parent.right : undefined
        LayoutMirroring.enabled: false
        enabled: buttonEnabled
        checked: root.checked || (buttonIsTriggeringMenu && mainComboBox.popup.visible)
        opacity: buttonIsTransparent && !isButtonIndicatingMenuPopup ? 0 : 1

        /*workaround in order to replicate the proper Buttons Exclusive Group Behavior*/
        checkable: root.checkable && !parent.exclusiveGroup
        /*workaround in order to replicate the proper Buttons Exclusive Group Behavior*/
        exclusiveGroup: parent.exclusiveGroup ? hiddenExclusiveGroup : null

        width: parent.width
        height: mainComboBox.height

        text: root.checkable ?  " " : buttonText
        iconSource: buttonIconSource
        tooltip: buttonToolTip

        onClicked: {
            if (buttonIsTriggeringMenu) {
                //! hiding combobox is triggered by default behavior
                mainComboBox.popup.visible = !mainComboBox.popup.visible;
                mainComboBox.down = mainComboBox.popup.visible;
                mainComboBox.pressed = mainComboBox.popup.visible;
            }
        }

        //! WORKAROUND in order to miss one Clicked event from parent button,
        //! when combobox menu is shown and the user clicks the button in order to hide
        //! menu, this is enough in order to be dismissed. Without the workaround
        //! the menu is reshown because the Clicked event is triggered after
        //! the menu became hidden
        MouseArea {
            anchors.fill: parent
            visible: parent.enabled && buttonIsTriggeringMenu && mainComboBox.popup.visible
        }
    }

    //overlayed combobox
    LatteComponents.ComboBox {
        id: mainComboBox
        anchors.right: mainButton.right
        anchors.top: parent.top

        width:  units.iconSizes.medium - 2 * units.smallSpacing
        height: parent.height

        enabled: comboBoxEnabled
        visible: comboBoxButtonIsVisible

        enabledRole: comboBoxEnabledRole
        iconRole: comboBoxIconRole
        textRole: comboBoxTextRole
        iconToolTipRole: comboBoxIconToolTipRole
        iconOnlyWhenHoveredRole: comboBoxIconOnlyWhenHoveredRole
        isSeparatorRole: comboBoxIsSeparatorRole
        buttonIsTransparent: comboBoxButtonIsTransparent

        blankSpaceForEmptyIcons: comboBoxBlankSpaceForEmptyIcons
        forcePressed: comboBoxForcePressed
        popUpAlignRight: comboBoxPopUpAlignRight
        popUpRelativeX: Qt.application.layoutDirection === Qt.RightToLeft ?
                            (popUpAlignRight ? root.width - width : 0) :
                            (popUpAlignRight ? width : -(root.width - width))
        popUpTextHorizontalAlignment: comboBoxPopupTextHorizontalAlignment

        hideDisplayText: true
        hideSelectedItemIcon: true

        minimumPopUpWidth: Math.max(comboBoxMinimumPopUpWidth, root.width)

        onIconClicked: root.iconClicked(index);
    }

    Label{
        width: labelMetrics.exceeds ? parent.width-mainComboBox.width :  parent.width
        height: parent.height
        text: buttonText
        font: mainButton.font
        color: buttonIsTransparent ? theme.textColor : theme.buttonTextColor
        visible: root.checkable || (mainButton.opacity === 0)

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

        readonly property bool exceeds: width>(mainButton.width-2*mainComboBox.width)
    }
}
