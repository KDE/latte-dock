/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.plasmoid 2.0

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

import org.kde.latte 0.1 as Latte

PlasmaCore.FrameSvgItem {
    id: dialog
    imagePath: "dialogs/background"

    property int maxWidth: 420 
    width: content.width + units.smallSpacing * 2
    height: content.height + units.smallSpacing * 2
    Layout.minimumWidth: width
    Layout.minimumHeight: height

    property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    FontLoader {
        id: tangerineFont
        name: "Tangerine"
        source: "../fonts/tangerine.ttf"
    }

    ColumnLayout {
        id: content

        Layout.minimumWidth: width
        Layout.minimumHeight: height
        Layout.preferredWidth: width
        Layout.preferredHeight: height
        height: header.height + tabBar.height + pagesBackground.height + actionButtons.height + spacing * 3

        anchors.centerIn: parent
        spacing: units.smallSpacing

        RowLayout {
            id: header
            Layout.fillWidth: true
            spacing: 0

            KQuickControlAddons.QIconItem {
                id: logo
                
                Layout.alignment: Qt.AlignLeft | Qt.AlignHCenter
                Layout.fillWidth: false
                width: 1.5 * latteTxt.font.pixelSize
                height: width

                icon: "latte-dock"
            }

            PlasmaComponents.Label {
                id: latteTxt
                Layout.alignment: Qt.AlignLeft | Qt.AlignHCenter
                Layout.fillWidth: false
                
                text: i18n("atte")
                font.family: tangerineFont.name
                font.pointSize: 2 * theme.defaultFont.pointSize
                font.italic: true
            }

            PlasmaComponents.Label {
                id: verLabel
                font.family: "monospace"
                font.pointSize: 0.8 * theme.defaultFont.pointSize
                font.bold: true
                opacity: 0.4

                Layout.rightMargin: units.smallSpacing
                Layout.alignment: Qt.AlignRight | Qt.AlignHCenter
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true

                text: i18n("ver:") + "@VERSION@"
            }
            
            PlasmaComponents.ToolButton {
                id: pinButton
    
                Layout.fillWidth: false
                Layout.fillHeight: false
                Layout.preferredWidth: width
                Layout.preferredHeight: height
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
    
                iconSource: "window-pin"
                checkable: true
    
                width: Math.round(units.gridUnit * 1.25)
                height: width
    
                property bool inStartup: true
                
                onClicked: {
                    plasmoid.configuration.configurationSticker = checked
                    dockConfig.setSticker(checked)
                }
    
                Component.onCompleted: {
                    checked = plasmoid.configuration.configurationSticker
                    dockConfig.setSticker(plasmoid.configuration.configurationSticker)
                }
            }
        }

        PlasmaComponents.TabBar {
            id: tabBar
            Layout.fillWidth: true

            PlasmaComponents.TabButton {
                text: i18n("Behavior")
                tab: behaviorPage
            }
            PlasmaComponents.TabButton {
                text: i18n("Appearance")
                tab: appearancePage
            }
            PlasmaComponents.TabButton {
                text: i18n("Tasks")
                tab: tasksPage

                visible: dock.tasksPresent()
            }
        }

        Rectangle {
            id: pagesBackground
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.minimumWidth: maxWidth
            Layout.maximumHeight: height
            width: maxWidth + units.smallSpacing * 4
            height: behaviorPage.Layout.maximumHeight + units.smallSpacing * 2
            
            property color bC: theme.backgroundColor
            property color transparentBack: Qt.rgba(bC.r, bC.g, bC.b, 0.7)

            color: transparentBack

            border.width: 1
            border.color: theme.backgroundColor

            PlasmaExtras.ScrollArea {
                id: scrollArea
                
                anchors.fill: parent
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                
                PlasmaComponents.TabGroup {
                    id: tabGroup
                    
                    width: currentTab.Layout.maximumWidth
                    height: currentTab.Layout.maximumHeight
                    
                    BehaviorConfig {
                        id: behaviorPage
                    }
                    
                    AppearanceConfig {
                        id: appearancePage
                    }
                    
                    TasksConfig {
                        id: tasksPage
                    }
                }
            }
        }

        RowLayout {
            id: actionButtons

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter

            spacing: units.largeSpacing

            PlasmaComponents.Button {
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true

                text: i18n("Add")
                iconSource: "list-add"

                onClicked: dock.addNewDock()

                Component.onCompleted: {
                    var edges = dock.freeEdges()
                    if (edges.length === 0) {
                        enabled = false
                    }
                }
            }
            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                text: i18n("Remove")
                iconSource: "edit-delete"
                visible: dock.docksCount > 1 ? true : false

                onClicked: dock.removeDock()
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight

                text: i18n("Quit")
                iconSource: "window-close"

                onClicked: dock.closeApplication()
            }
        }
    }
}
