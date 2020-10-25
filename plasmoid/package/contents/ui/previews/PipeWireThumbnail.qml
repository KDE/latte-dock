/*
 *   Copyright 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
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
