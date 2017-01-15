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
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.plasmoid 2.0

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

import org.kde.latte 0.1 as Latte

PlasmaCore.FrameSvgItem {
    id: dialog
    imagePath: "dialogs/background"

    property int maxWidth: 34 * theme.defaultFont.pixelSize
    width: maxWidth + units.smallSpacing * 2
    height: content.height + units.smallSpacing * 2
    Layout.minimumWidth: width
    Layout.minimumHeight: height

    property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    FontLoader {
        id: tangerineFont
        name: "Tangerine"
        source: "../fonts/tangerine.ttf"
    }

    PlasmaComponents.ToolButton {
        id: pinButton

        anchors.right: parent.right
        anchors.top: parent.top

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

    ColumnLayout {
        id: content

        Layout.minimumWidth: width
        Layout.minimumHeight: height
        Layout.preferredWidth: width
        Layout.preferredHeight: height
        height: header.height + tabBar.height + pagesBackground.height + actionButtons.height + spacing * 3
        width: dialog.maxWidth

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
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
                
                text: "atte"
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
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true

                text: i18n("ver:") + "@VERSION@"
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
            Layout.minimumWidth: maxWidth - 2*units.smallSpacing
            Layout.minimumHeight:  height
            Layout.maximumHeight: height

            width: maxWidth - units.smallSpacing
            height: behaviorPage.Layout.maximumHeight + units.smallSpacing * 4
            
            property color bC: theme.backgroundColor
            property color transparentBack: Qt.rgba(bC.r, bC.g, bC.b, 0.7)

            color: transparentBack

            border.width: 1
            border.color: theme.backgroundColor

            PlasmaExtras.ScrollArea {
                id: scrollArea
                
                anchors.fill: parent
                verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                
                flickableItem.flickableDirection: Flickable.VerticalFlick
                
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
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            spacing: units.largeSpacing
            
            property int docksCount: dock.docksCount
            
            onDocksCountChanged: {
                addDock.enabled = docksCount < 4 && dock.freeEdges().length > 0
            }

            PlasmaComponents.Button {
                id: addDock
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true

                text: i18n("Add")
                iconSource: "list-add"

                onClicked: dock.addNewDock()

                Component.onCompleted: {
                    enabled = dock.freeEdges().length > 0
                }
            }
            PlasmaComponents.Button {
                id: removeDock
                
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                text: i18n("Remove")
                iconSource: "edit-delete"
                opacity: enabled ? 1 : 0
                enabled: dock.docksCount > 1

                onClicked: dock.removeDock()
            }

            PlasmaComponents.Button {
                id: quit
                
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight

                text: i18n("Quit")
                iconSource: "window-close"

                onClicked: dock.closeApplication()
            }
        }
    }
}
