/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Templates 2.2 as T
import QtQuick.Controls 2.2 as Controls
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kirigami 2.2 as Kirigami
import "private" as Private

T.ComboBox {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding) + indicator.implicitWidth + rightPadding
    implicitHeight: units.gridUnit * 1.6
    baselineOffset: contentItem.y + contentItem.baselineOffset

    hoverEnabled: true
    topPadding: surfaceNormal.margins.top
    leftPadding: surfaceNormal.margins.left
    rightPadding: surfaceNormal.margins.right + units.gridUnit * 2
    bottomPadding: surfaceNormal.margins.bottom

    wheelEnabled: false

    property bool hideDisplayText: false
    property bool hideSelectedItemIcon: false

    property bool blankSpaceForEmptyIcons: false
    property bool forcePressed: false
    property bool popUpAlignRight: true
    property bool buttonIsTransparent: false
    property int minimumPopUpWidth: 150
    property int popUpRelativeX: 0
    property int popUpTextHorizontalAlignment: Text.AlignLeft

    property string enabledRole
    property string iconRole
    property string iconToolTipRole
    property string iconOnlyWhenHoveredRole
    property string isSeparatorRole
    property string toolTipRole

    signal iconClicked(int index);

    delegate: ItemDelegate {
        width: control.popup.width
        enabled: !isSeparator && (control.enabledRole.length>0 ? (isArray ? modelData[control.enabledRole] : model[control.enabledRole]) : true)
        text: control.textRole.length>0 ? (isArray ? modelData[control.textRole] : model[control.textRole]) : modelData
        icon: control.iconRole.length>0 ? (isArray ? modelData[control.iconRole] : model[control.iconRole]) : ''
        iconToolTip: control.iconToolTipRole.length>0 ? (isArray ? modelData[control.iconToolTipRole] : model[control.iconToolTipRole]) : ''
        iconOnlyWhenHovered: control.iconOnlyWhenHoveredRole.length>0 ? (isArray ? modelData[control.iconOnlyWhenHoveredRole] : model[control.iconOnlyWhenHoveredRole]) : ''
        isSeparator: control.isSeparatorRole.length>0 ? (isArray ? modelData[control.isSeparatorRole] : model[control.isSeparatorRole]) : false
        toolTip: control.toolTipRole.length>0 ? (isArray ? modelData[control.toolTipRole] : model[control.toolTipRole]) : ''

        highlighted: mouseArea.pressed ? listView.currentIndex == index : control.currentIndex == index
        blankSpaceForEmptyIcons: control.blankSpaceForEmptyIcons
        textHorizontalAlignment: popUpTextHorizontalAlignment

        readonly property bool isArray: Array.isArray(control.model)
        property bool separatorVisible: false

        PlasmaComponents.Button {
            id: tooltipBtn
            anchors.fill: parent
            opacity: 0
            tooltip: parent.toolTip
            visible: tooltip !== ''

            onPressedChanged: {
                if (!pressed) {
                    control.currentIndex = index;
                    control.down = false;
                    control.pressed = false;
                    control.popup.visible = false;
                }
            }
        }
    }

    indicator: PlasmaCore.SvgItem {
        implicitWidth: units.iconSizes.small
        implicitHeight: implicitWidth
        anchors {
            right: parent.right
            rightMargin: control.buttonIsTransparent ? 0 : surfaceNormal.margins.right
            verticalCenter: parent.verticalCenter
        }
        svg: PlasmaCore.Svg {
            imagePath: "widgets/arrows"
            colorGroup: PlasmaCore.Theme.ButtonColorGroup
        }
        elementId: "down-arrow"
    }

    //     contentItem: Label {
    //         text: control.displayText
    //         font: control.font
    //         color: theme.buttonTextColor
    //         horizontalAlignment: Text.AlignLeft
    //         verticalAlignment: Text.AlignVCenter
    //         elide: Text.ElideRight
    //     }
    contentItem: MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        preventStealing: true
        property int indexUnderMouse: -1
        onWheel: {
            if (!control.wheelEnabled) {
                return;
            }

            if (wheel.pixelDelta.y < 0 || wheel.angleDelta.y < 0) {
                control.currentIndex = Math.min(control.currentIndex + 1, delegateModel.count -1);
            } else {
                control.currentIndex = Math.max(control.currentIndex - 1, 0);
            }
            control.activated(control.currentIndex);
        }
        onPressed: onGenericPressed()
        onReleased: onGenericReleased()
        onCanceled: {
            control.down = false;
            control.pressed = false;
        }
        onPositionChanged: {
            var pos = listView.mapFromItem(this, mouse.x, mouse.y);
            indexUnderMouse = listView.indexAt(pos.x, pos.y);
            listView.currentIndex = indexUnderMouse;
            control.activated(indexUnderMouse);
        }

        Connections {
            target: popup
            onClosed: {
                control.down = false;
                control.pressed = false;
            }
        }

        function onGenericPressed() {
            indexUnderMouse = -1;
            listView.currentIndex = control.highlightedIndex;
            control.down = true;
            control.pressed = true;
            control.popup.visible = !control.popup.visible;
        }

        function onGenericReleased() {
            if (!containsMouse && !hiddenTooltipButton.hovered) {
                control.down = false;
                control.pressed = false;
                control.popup.visible = false;
            }
            if (indexUnderMouse > -1) {
                control.currentIndex = indexUnderMouse;
            }
        }

        PlasmaComponents.Button {
            id: hiddenTooltipButton
            anchors.fill: parent
            opacity: 0
            visible: control && control.currentIndex>=0 && control.toolTipRole.length>0
            tooltip: {
                if (!visible) {
                    return "";
                }

                if (Array.isArray(control.model)) {
                    return control.model[control.currentIndex][control.toolTipRole];
                } else {
                    return control.model.get(control.currentIndex)[control.toolTipRole];
                }
            }

            onPressedChanged: {
                if (pressed) {
                    mouseArea.onGenericPressed();
                } else {
                    mouseArea.onGenericReleased();
                }
            }
        }

        RowLayout {
            anchors.fill: parent
            spacing: 0

            anchors {
                leftMargin: !control.mirrored ? 1 : 0
                rightMargin: control.mirrored ? 1 : 0
            }

            PlasmaCore.IconItem {
                id: selectedIcon
                implicitWidth: textLabel.height
                implicitHeight: textLabel.height

                colorGroup: PlasmaCore.Theme.ButtonColorGroup
                source: {
                    if (control
                            && control.currentIndex>=0
                            && control.iconRole.length>0) {

                        if (Array.isArray(control.model)) {
                            return control.model[control.currentIndex][control.iconRole];
                        } else {
                            return control.model.get(control.currentIndex)[control.iconRole];
                        }
                    }

                    return "";
                }

                visible: source !== '' && !control.hideSelectedItemIcon
            }

            Label {
                id: textLabel
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: !selectedIcon.visible && !control.mirrored ? units.smallSpacing : 0
                Layout.rightMargin: !selectedIcon.visible && control.mirrored ? units.smallSpacing : 0

                text: control.displayText
                font: control.font
                color: control.pressed ? theme.highlightedTextColor : theme.buttonTextColor
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                opacity: control.enabled ? 1 : 0.6

                visible: !control.hideDisplayText
            }

         /*   T.TextField {
                id: textField
                padding: 0
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: control.editable ? control.editText : control.displayText

                enabled: control.editable
                autoScroll: control.editable

                readOnly: control.down || !control.hasOwnProperty("editable") || !control.editable
                inputMethodHints: control.inputMethodHints
                validator: control.validator

                // Work around Qt bug where NativeRendering breaks for non-integer scale factors
                // https://bugreports.qt.io/browse/QTBUG-67007
                renderType: Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering
                color: theme.buttonTextColor //control.enabled ? theme.textColor : theme.disabledTextColor
                selectionColor: Kirigami.Theme.highlightColor
                selectedTextColor: Kirigami.Theme.highlightedTextColor

                selectByMouse: !Kirigami.Settings.tabletMode
                cursorDelegate: Kirigami.Settings.tabletMode ? mobileCursor : undefinedCursor

                font: control.font
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                opacity: control.enabled ? 1 : 0.6
                onFocusChanged: {
                    if (focus) {
                        Private.MobileTextActionsToolBar.controlRoot = textField;
                    }
                }

                onPressAndHold: {
                    if (!Kirigami.Settings.tabletMode) {
                        return;
                    }
                    forceActiveFocus();
                    cursorPosition = positionAt(event.x, event.y);
                    selectWord();
                }
            }*/
        }
    }

  /*  Component {
        id: mobileCursor
        Private.MobileCursor {
            target: textField
        }
    }*/

    Component {
        id: undefinedCursor
        Item{}
    }

 /*   Private.MobileCursor {
        target: textField
        selectionStartHandle: true
        property var rect: textField.positionToRectangle(textField.selectionStart)
        //FIXME: this magic values seem to be always valid, for every font,every dpi, every scaling
        x: rect.x + 5
        y: rect.y + 6
    }*/

    background: PlasmaCore.FrameSvgItem {
        id: surfaceNormal
        //retrocompatibility with old controls
        implicitWidth: units.gridUnit * 6
        width: parent.width
        height: parent.height

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: control.buttonIsTransparent ? -margins.right : 0
        readonly property bool editable: control.hasOwnProperty("editable") && control.editable
        imagePath: editable ? "widgets/lineedit" : "widgets/button"
        prefix: editable
                ? "base"
                : (control.pressed || control.forcePressed ? "pressed" : "normal")

        opacity: control.buttonIsTransparent && prefix !== "pressed" && textFieldPrivate.state !== "hover" && !control.popup.visible ? 0 : 1

        Private.TextFieldFocus {
            id: textFieldPrivate
            visible: parent.editable
            z: -1
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "hidden")
            anchors.fill: parent
        }
        Private.ButtonShadow {
            z: -1
            visible: !parent.editable
            anchors.fill: parent
            state: {
                if (control.pressed) {
                    return "hidden"
                } else if (control.hovered) {
                    return "hover"
                } else if (control.activeFocus) {
                    return "focus"
                } else {
                    return "shadow"
                }
            }
        }

        MouseArea {
            anchors {
                fill: parent
                leftMargin: control.leftPadding
                rightMargin: control.rightPadding
            }
            acceptedButtons: Qt.NoButton
            onWheel: {
                if (!control.wheelEnabled) {
                    return;
                }

                if (wheel.pixelDelta.y < 0 || wheel.angleDelta.y < 0) {
                    control.currentIndex = Math.min(control.currentIndex + 1, delegateModel.count -1);
                } else {
                    control.currentIndex = Math.max(control.currentIndex - 1, 0);
                }
                control.activated(control.currentIndex);
            }
        }
    }

    popup: T.Popup {
        x: {
            if (!control.mirrored) {
                if (popUpRelativeX !== 0) {
                    var adjustedX = exceedsContent && control.popUpAlignRight ? -(width - control.width) : popUpRelativeX;
                    return  adjustedX;
                } else {
                    return 0;
                }
            } else {
                //! mirrored case
                if (exceedsContent && control.popUpAlignRight) {
                    var adjustedX = width - control.width - popUpRelativeX;
                    return  -adjustedX;
                } else {
                    return 0;
                }
            }
        }
        y: control.height
        width: Math.max(control.width, control.minimumPopUpWidth)
        implicitHeight: contentItem.implicitHeight
        topMargin: 6
        bottomMargin: 6

        readonly property bool exceedsContent: control.width < width

        /*onVisibleChanged: {
            if (visible) {
                console.log("  mirrored:" + control.mirrored);
                console.log("  exceeds: " + exceedsContent);
                console.log("  popupAR: " + control.popUpAlignRight);
                console.log("  popupRX: " + popUpRelativeX);
            }
        }*/

        contentItem: ListView {
            id: listView
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            highlightRangeMode: ListView.ApplyRange
            highlightMoveDuration: 0
            // HACK: When the ComboBox is not inside a top-level Window, it's Popup does not inherit
            // the LayoutMirroring options. This is a workaround to fix this by enforcing
            // the LayoutMirroring options properly.
            // QTBUG: https://bugreports.qt.io/browse/QTBUG-66446
            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true
            T.ScrollBar.vertical: Controls.ScrollBar { }

            signal iconClicked(int index);

            onIconClicked: control.iconClicked(index);
        }
        background: Rectangle {
            anchors {
                fill: parent
                margins: -1
            }
            radius: 2
            color: theme.viewBackgroundColor
            border.color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.3)
            layer.enabled: true

            layer.effect: DropShadow {
                transparentBorder: true
                radius: 4
                samples: 8
                horizontalOffset: 2
                verticalOffset: 2
                color: Qt.rgba(0, 0, 0, 0.3)
            }
        }
    }
}
