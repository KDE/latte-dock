/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick.Window 2.6
import org.kde.taskmanager 0.1 as TaskManager

TaskManager.PipeWireSourceItem {
    visible: waylandItem.nodeId > 0
    nodeId: waylandItem.nodeId

    anchors.fill: parent

    TaskManager.ScreencastingRequest {
        id: waylandItem
        uuid: !windowsPreviewDlg.visible ? "" : thumbnailSourceItem.winId
    }
}
