/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.TextField {
    id: textField

    validator: IntValidator {
        bottom: minValue
        top: maxValue
    }

    onTextChanged: {
        if (text.trim() === minValue.toString())
            text = ""
    }

    font.italic: true
    inputMethodHints: Qt.ImhDigitsOnly
    placeholderText: i18n("none")
    horizontalAlignment: Text.AlignLeft

    readonly property int implicitWidth: internalContent.width + theme.mSize(theme.defaultFont).width * 3.5

    readonly property int value: text === "" ? minValue : parseInt(text)
    property int step: 100
    property int minValue: 0
    property int maxValue: 3000

    function increment() {
        var val = text === "" ? minValue : parseInt(text)
        text = Math.min(val + step, maxValue).toString()
    }

    function decrement() {
        var val = text === "" ? minValue : parseInt(text)
        val = Math.max(val - step, minValue)
        text = val === minValue ? "" : val.toString()
    }

    RowLayout {
        id: internalContent
        spacing: 0
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        PlasmaComponents.Label {
            Layout.alignment: Qt.AlignVCenter
            color: textField.textColor
            text: i18n("ms.")
            font.italic: true
            opacity: value === 0 ? 0 : 0.6
        }
        PlasmaComponents.Button {
            id: downButton

            Layout.fillHeight: true
            Layout.preferredWidth: height
            Layout.maximumWidth: height
            Layout.leftMargin: Qt.application.layoutDirection === Qt.RightToLeft ? 0 : 0.7 * theme.mSize(theme.defaultFont).width
            Layout.rightMargin: Qt.application.layoutDirection === Qt.RightToLeft ? 0.7 * theme.mSize(theme.defaultFont).width : 0

            text: "-"
            onClicked: decrement()
        }
        PlasmaComponents.Button {
            id: upButton

            Layout.fillHeight: true
            Layout.preferredWidth: height
            Layout.maximumWidth: height
            text: "+"
            onClicked: increment()
        }
    }

    Timer {
        id: holdPressed
        running: upButton.pressed || downButton.pressed
        interval: 200
        repeat: true

        onRunningChanged: {
            if (!running)
                interval = 200
        }

        onTriggered: {
            if (interval === 200)
                interval = 150
            else if (upButton.pressed)
                increment()
            else
                decrement()
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.MiddleButton

        onWheel: {
            var angle = wheel.angleDelta.y / 8

            if (angle > 0) {
                increment()
            } else if (angle < 0) {
                decrement()
            }
        }
    }
}
