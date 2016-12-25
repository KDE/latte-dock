/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: mainItem

    width: childrenRect.width
    height: childrenRect.height

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property alias cfg_showBarLine: showBarLine.checked
    property alias cfg_useThemePanel: useThemePanel.checked
    property alias cfg_panelSize: panelSize.value
    property alias cfg_transparentPanel: transparentPanel.checked
    property alias cfg_plasmoidPosition: panelPositionCmb.currentIndex
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


            Label {
                text: i18n("Position: ")
            }

            ComboBox {
                // 16, 22, 32, 48, 64,128, 256
                id: panelPositionCmb

                Layout.fillWidth: true
                model: [i18n("Center"), i18n("Left"), i18n("Right"), i18n("Top"), i18n("Bottom")]
            }
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
            //Layout.minimumWidth didnt work
            Label{}
            Label{Layout.minimumWidth: 280}
            Label{}

        }
    }

    DropShadow {
        id:shadowText
        anchors.fill: inNowDockLabel
        enabled: isInNowDockPanel
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
        text: i18n("For the disabled settings you should use the Now Dock Panel Configuration Window")
        visible: mainItem.isInNowDockPanel

        horizontalAlignment: Text.AlignHCenter
        //  font.bold: true
        font.italic: true
        font.pointSize: 1.2 * theme.defaultFont.pointSize

        wrapMode: Text.WordWrap
    }

}
