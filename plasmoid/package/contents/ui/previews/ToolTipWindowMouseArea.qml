/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte.core 0.2 as LatteCore

MouseArea {
    property var modelIndex
    property int winId // FIXME Legacy
    property Item rootTask

    acceptedButtons: Qt.LeftButton | Qt.MiddleButton | Qt.RightButton
    hoverEnabled: true
    enabled: LatteCore.WindowSystem.isPlatformWayland ||
             (!LatteCore.WindowSystem.isPlatformWayland && winId != 0)

    onClicked: {
        //!used mainly to not close the previews window when the user closes many windows simultaneously
        var keepVisibility = false;

        if (mouse.button == Qt.LeftButton) {
            tasksModel.requestActivate(modelIndex);
        } else if (mouse.button == Qt.MiddleButton) {
            if (isGroup) {
                keepVisibility = true;
            }

            tasksModel.requestClose(modelIndex);
        } else {
            root.createContextMenu(rootTask, modelIndex).show();
        }

        if (!keepVisibility) {
            windowsPreviewDlg.visible = false;
            root.forcePreviewsHiding(14.5);
        }
    }

    onContainsMouseChanged: {
        root.windowsHovered([winId], containsMouse);
    }
}

