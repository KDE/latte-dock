/*
*   Copyright 2013 by Sebastian Kügler <sebas@kde.org>
*   Copyright 2014 by Martin Gräßlin <mgraesslin@kde.org>
*   Copyright 2016 by Kai Uwe Broulik <kde@privat.broulik.de>
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

import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents

MouseArea {
    property var modelIndex
    property int winId // FIXME Legacy
    property Item thumbnailItem

    acceptedButtons: Qt.LeftButton
    hoverEnabled: true
    enabled: winId != 0

    onClicked: {
        tasksModel.requestActivate(modelIndex);
        windowsPreviewDlg.hide();
        //toolTip.hideToolTip();
    }

    onContainsMouseChanged: {
        tooltipContentItem.checkMouseInside();

        panel.windowsHovered([winId], containsMouse);
    }

    PlasmaComponents.ToolButton {
        anchors {
            top: parent.top
            topMargin: thumbnailItem ? (thumbnailItem.height - thumbnailItem.paintedHeight) / 2 : 0
            right: parent.right
            rightMargin: thumbnailItem ? (thumbnailItem.width - thumbnailItem.paintedWidth) / 2 : 0
        }

        iconSource: "window-close"
        visible: parent.containsMouse && winId != 0
        tooltip: i18nc("close this window", "Close")

        onClicked: tasksModel.requestClose(modelIndex);
    }
}
