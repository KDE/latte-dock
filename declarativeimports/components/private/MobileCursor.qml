/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

Item {
    id: root
    width: 1 //<-important that this is actually a single device pixel
    height: units.gridUnit

    property Item target

    property bool selectionStartHandle: false

    visible: Kirigami.Settings.tabletMode && ((target.activeFocus && !selectionStartHandle) || target.selectedText.length > 0)

    Rectangle {
        width: Math.round(units.devicePixelRatio * 3)
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            bottom: parent.bottom
        }
        color: Qt.tint(theme.highlightColor, Qt.rgba(1,1,1,0.4))
        radius: width
        Rectangle {
            width: Math.round(units.gridUnit/1.5)
            height: width
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.bottom
            }
            radius: width
            color: Qt.tint(theme.highlightColor, Qt.rgba(1,1,1,0.4))
        }
        MouseArea {
            anchors {
                fill: parent
                margins: -units.gridUnit
            }
            preventStealing: true
            onPositionChanged: {
                var pos = mapToItem(target, mouse.x, mouse.y);
                pos = target.positionAt(pos.x, pos.y);

                if (target.selectedText.length > 0) {
                    if (selectionStartHandle) {
                        target.select(Math.min(pos, target.selectionEnd - 1), target.selectionEnd);
                    } else {
                        target.select(target.selectionStart, Math.max(pos, target.selectionStart + 1));
                    }
                } else {
                    target.cursorPosition = pos;
                }
            }
        }
    }
}
