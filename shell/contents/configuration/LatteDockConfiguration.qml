
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

    //old way to count the dialog width
    //Math.max(420,appearancePage.noneShadow.width + appearancePage.lockedAppletsShadow.width + appearancePage.allAppletsShadow.width)
    width: content.width + units.largeSpacing * 2
    height: content.height + units.smallSpacing * 2

    property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    property int windowSpace: 8

    FontLoader {
        id: tangerineFont
        name: "Tangerine"
        source: "../fonts/tangerine.ttf"
    }

    ColumnLayout {
        id: content

        Layout.minimumWidth: width
        Layout.minimumHeight: height
        width: implicitWidth
        height: implicitHeight

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
                Layout.alignment: Qt.AlignLeft
            }

            PlasmaComponents.Label {
                id: verLabel
                font.family: "monospace"
                font.pointSize: 0.8 * theme.defaultFont.pointSize
                font.bold: true
                opacity: 0.4

                Layout.topMargin: 1.4 * units.iconSizes.small
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true

                text: i18n("ver:") + "@VERSION@"
            }
            
            PlasmaComponents.ToolButton {
                id: pinButton
    
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
    
                iconSource: "window-pin"
                checkable: true
    
                width: Math.round(units.gridUnit * 1.25)
                height: width
    
                property bool inStartup: true
    
                onCheckedChanged: {
                    if (!inStartup) {
                        plasmoid.configuration.configurationSticker = checked
                        dockConfig.setSticker(checked)
                    }
                }
    
                Component.onCompleted: {
                    checked = plasmoid.configuration.configurationSticker
                    dockConfig.setSticker(plasmoid.configuration.configurationSticker)
                    inStartup = false
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
            height: childrenRect.height

            property color bC: theme.backgroundColor
            property color transparentBack: Qt.rgba(bC.r, bC.g, bC.b, 0.7)

            color: transparentBack

            border.width: 1
            border.color: theme.backgroundColor

            PlasmaExtras.ScrollArea {
                anchors.fill: parent
                anchors.margins: 3
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                PlasmaComponents.TabGroup {
                    width: pagesBackground.width - 6
                    //FIXME: this creates a binding loop but I havent found any alternative yet
                    height: currentTab.childrenRect.height

                    privateContents: [
                        BehaviorConfig {
                            id: behaviorPage
                        },
                        AppearanceConfig {
                            id: appearancePage
                        },
                        TasksConfig {
                            id: tasksPage
                        }
                    ]
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
