/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import Qt5Compat.GraphicalEffects

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmoidItem {
    id: mainItem

    width: childrenRect.width
    height: childrenRect.height

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property alias cfg_showBarLine: showBarLine.checked
    property alias cfg_useThemePanel: useThemePanel.checked
    property alias cfg_panelSize: panelSize.value
    property alias cfg_transparentPanel: transparentPanel.checked
    property alias cfg_isInNowDockPanel: mainItem.isInNowDockPanel

    property bool isInNowDockPanel

    ColumnLayout {

        id:mainColumn
        spacing: 15
        Layout.fillWidth: true

        GridLayout{
            enabled: !mainItem.isInNowDockPanel
            Layout.fillWidth: true
            columns: 3
            property bool panelConfigEnabled: showBarLine.checked && useThemePanel.checked

            Label{}

            CheckBox {
                id: showBarLine
                Layout.columnSpan: 3
                text: i18n("Show bar line for tasks")
                enabled: true
            }

            CheckBox {
                id: useThemePanel
                Layout.columnSpan: 3
                text: i18n("Use plasma theme panel")
                enabled: showBarLine.checked
            }

            CheckBox {
                id: transparentPanel
                Layout.columnSpan: 3
                text: i18n("Use transparency in the panel")
                enabled: parent.panelConfigEnabled
            }


            Label {
                id: panelLabel
                text: i18n("Size: ")
                enabled: parent.panelConfigEnabled
            }

            Slider {
                id: panelSize
                enabled: parent.panelConfigEnabled
                Layout.fillWidth: true
                minimumValue: 0
                maximumValue: 256
                stepSize: 2
                tickmarksEnabled: false
            }

            Label {
                enabled: parent.panelConfigEnabled
                Layout.minimumWidth: metricsLabel.width
                Layout.maximumWidth: metricsLabel.width
                Layout.alignment: Qt.AlignRight
                horizontalAlignment: Text.AlignRight

                text: ( panelSize.value + " px." )

                Label{
                    id:metricsLabel
                    visible: false
                    text: panelSize.maximumValue+" px."
                }
            }

            /*    Label{
                Layout.columnSpan: 3
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignRight
                Layout.maximumWidth: zoomLevel.width + zoomLevelText.width + panelLabel.width
                horizontalAlignment: Text.AlignRight
                text: i18n("in panel placement, themes that have set a <b>specific</b> panel transparent work better")
                wrapMode: Text.WordWrap
                font.italic: true
                enabled: parent.panelConfigEnabled
            }*/

            /////
            //spacer to set a minimumWidth for sliders
            //Layout.minimumWidth didn't work
            Label{}
            Label{Layout.minimumWidth: 280}
            Label{}

        }
    }

    DropShadow {
        id:shadowText
        anchors.fill: inNowDockLabel
        enabled: isInNowDockPanel
        fast: true
        radius: 3
        samples: 5
        color: "#cc080808"
        source: inNowDockLabel

        verticalOffset: 2
        horizontalOffset: -1
        visible: isInNowDockPanel
    }


    Label {
        id:inNowDockLabel
        anchors.horizontalCenter: mainItem.horizontalCenter
        anchors.verticalCenter: mainColumn.verticalCenter
      //  anchors.verticalCenterOffset:  (mainColumn.height / 4)

        width: 0.85 * mainItem.width
        text: i18n("For the disabled settings you should use the Latte Dock Configuration Window")
        visible: mainItem.isInNowDockPanel

        horizontalAlignment: Text.AlignHCenter
        //  font.bold: true
        font.italic: true
        font.pointSize: 1.2 * theme.defaultFont.pointSize

        wrapMode: Text.WordWrap
    }

}
