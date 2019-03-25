/*
 *   Copyright (C) 2018 by Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

pragma Singleton

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

Popup {
    id: root

    property Item controlRoot
    parent: controlRoot ? controlRoot.Window.contentItem : undefined
    modal: false
    focus: false
    closePolicy: Popup.NoAutoClose

    x: controlRoot ? Math.min(Math.max(0, controlRoot.mapToItem(root.parent, controlRoot.positionToRectangle(controlRoot.selectionStart).x, 0).x - root.width/2), controlRoot.Window.contentItem.width - root.width) : 0

    y: {
        if (!controlRoot) {
            return false;
        }
        var desiredY = controlRoot.mapToItem(root.parent, 0, controlRoot.positionToRectangle(controlRoot.selectionStart).y).y  - root.height;

        if (desiredY >= 0) {
            return Math.min(desiredY, controlRoot.Window.contentItem.height - root.height);
        } else {
            return Math.min(Math.max(0, controlRoot.mapToItem(root.parent, 0, controlRoot.positionToRectangle(controlRoot.selectionEnd).y + Math.round(units.gridUnit*1.5)).y), controlRoot.Window.contentItem.height - root.height);
        }
    }


    visible: controlRoot ? Kirigami.Settings.tabletMode && (controlRoot.selectedText.length > 0 || controlRoot.canPaste) : false

    width: contentItem.implicitWidth + leftPadding + rightPadding

    contentItem: RowLayout {
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-cut"
            visible: controlRoot.selectedText.length > 0
            onClicked: {
                controlRoot.cut();
            }
        }
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-copy"
            visible: controlRoot.selectedText.length > 0
            onClicked: {
                controlRoot.copy();
            }
        }
        ToolButton {
            focusPolicy: Qt.NoFocus
            icon.name: "edit-paste"
            onClicked: {
                controlRoot.paste();
            }
        }
    }
}
