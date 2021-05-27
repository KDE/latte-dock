/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.latte.core 0.2 as LatteCore

PlasmaCore.FrameSvgItem {
    id: dialog

    imagePath: "widgets/panel-background"
    prefix:""
    enabledBorders: infoWindow ? infoWindow.enabledBorders : PlasmaCore.FrameSvg.AllBorders

    width: Screen.width + 1
    height: Math.min(Screen.height - units.gridUnit * 8, logo.height + messageLbl.height + 2 *units.gridUnit)

    property string message

    ColumnLayout {
        id: root

        anchors.centerIn: parent

        Item{
            id: iconItem
            Layout.fillWidth: true
            Layout.minimumHeight: logo.height

            LatteCore.IconItem {
                id: logo
                anchors.centerIn: parent

                width: units.iconSizes.huge
                height: width

                source: "latte-dock"
            }
        }

        PlasmaComponents.Label {
            id: messageLbl
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            text: dialog.message
            font.italic: true
        }
    }
}
