/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*
*  This file is part of Candil-Dock
*
*  Candil-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 3 of
*  the License, or (at your option) any later version.
*
*  Candil-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles.Plasma 2.0 as Styles
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

//import org.candildock.shell 1.0

//import "../controls" as Controls

PlasmaCore.FrameSvgItem {
    id: root

    imagePath: "dialogs/background"

    Component.onCompleted: {
        console.log("showing candil dock configuration")
    }
    
    property Item dock;

    width: content.width + units.largeSpacing * 2
    height: content.height + units.smallSpacing * 2


    //! BEGIN: UI Components
    GridLayout {
        id: content

        anchors.centerIn: parent
        width: implicitWidth
        height: implicitHeight
        Layout.minimumWidth: width
        Layout.minimumHeight: height

        rowSpacing: units.smallSpacing
        columnSpacing: units.largeSpacing

        columns: 2

        PlasmaExtras.Heading {
            id: appearanceHeading
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: i18nc("@title:group config ui", "Appearance")
            level: 2
        }

        //! BEGIN: Location
        PlasmaComponents.Label {
            Layout.row: 1
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:listbox config ui", "Location:")
        }

        PlasmaComponents.ButtonRow {
            Layout.fillWidth: true
            spacing: 1
            exclusive: true
            PlasmaComponents.ToolButton {
                text: i18nc("@item:inlistbox config ui, location", "Top")
                flat: false
                checked: dock.location === edge
                checkable: true
                property int edge: PlasmaCore.Types.TopEdge
            }
            PlasmaComponents.ToolButton {
                text: i18nc("@item:inlistbox config ui, location", "Bottom")
                flat: false
                checked: dock.location === edge
                checkable: true
                property int edge: PlasmaCore.Types.BottomEdge
            }
            PlasmaComponents.ToolButton {
                text: i18nc("@item:inlistbox config ui, location", "Left")
                flat: false
                checked: dock.location === edge
                checkable: true
                property int edge: PlasmaCore.Types.LeftEdge
            }
            PlasmaComponents.ToolButton {
                text: i18nc("@item:inlistbox config ui, location", "Right")
                flat: false
                checked: dock.location === edge
                checkable: true
                property int edge: PlasmaCore.Types.RightEdge
            }
            onCheckedButtonChanged: {
                dock.location = checkedButton.edge
            }
        }
        //! END: Location

        //! BEGIN: Alignment
        PlasmaComponents.Label {
            Layout.row: 2
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:listbox config ui", "Alignment:")
        }

        PlasmaComponents.ButtonRow {
            Layout.fillWidth: true
            spacing: 1
            exclusive: true
            PlasmaComponents.ToolButton {
                text: i18nc("@item:inlistbox config ui, alignment", "Begin")
                flat: false
                checked: dock.alignment === align
                checkable: true
                property int align: Dock.Begin
            }
            PlasmaComponents.ToolButton {
                text: i18nc("@item:inlistbox config ui, alignment", "Center")
                flat: false
                checked: dock.alignment === align
                checkable: true
                property int align: Dock.Center
            }
            PlasmaComponents.ToolButton {
                text: i18nc("@item:inlistbox config ui, alignment", "End")
                flat: false
                checked: dock.alignment === align
                checkable: true
                property int align: Dock.End
            }
            PlasmaComponents.ToolButton {
                text: i18nc("@item:inlistbox config ui, alignment", "Fill")
                flat: false
                checked: dock.alignment === align
                checkable: true
                property int align: Dock.Fill
            }
            onCheckedButtonChanged: {
                dock.alignment = checkedButton.align
            }
        }

        PlasmaComponents.Label {
            Layout.row: 3
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:slider config ui, align icons",
                        "Alignment of icons:")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: units.largeSpacing

            readonly property int maxOffset: (dock.maxLength - dock.length) / 2

            PlasmaComponents.Slider {
                id: alignmentSlider
                Layout.fillWidth: true
                enabled: dock.alignment === Dock.Center
                maximumValue: 200
                minimumValue: 0
                stepSize: 2

                Component.onCompleted: {
                    value = 100 * (dock.offset / parent.maxOffset + 1)
                    dock.offset = Qt.binding(function () {
                        return parent.maxOffset * (value / 100 - 1)
                    })
                }

                updateValueWhileDragging: true
            }
            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignLeft
                Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                Layout.maximumWidth: Layout.minimumWidth

                horizontalAlignment: Text.AlignLeft
                text: i18nc("@label:slider percent", "%1%", (100 * dock.offset / parent.maxOffset) | 0)
            }
        }
        //! END: Alignment

        //! BEGIN: Icons
        PlasmaComponents.Label {
            Layout.row: 4
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:spinbox config ui, size icons", "Size icons:")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: units.largeSpacing

        /*    Controls.SpinBox {
                suffix: i18nc(
                            "@item:spinbox config ui, size icons, suffix in pixels",
                            " pixels")
                maximumValue: 128
                minimumValue: 48
                stepSize: 2
                value: dock.iconSize

                onValueChanged: {
                    dock.iconSize = value
                }
            }*/

            PlasmaComponents.Label {
                text: i18nc("@label:slider config ui, icons zoom", "Zoom:")
                Layout.alignment: Qt.AlignRight
            }

            PlasmaComponents.Slider {
                Layout.fillWidth: true
                value: 100
                maximumValue: 200
                minimumValue: 100
                stepSize: 1
                updateValueWhileDragging: true

                Component.onCompleted: {
                    value = (dock.zoomFactor * 100) | 0
                    dock.zoomFactor = Qt.binding(function(){
                        return value / 100
                    })
                }
            }
        }
        //! END: Icons
        PlasmaExtras.Heading {
            Layout.row: 5
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: i18nc("@title:group config ui", "Behavior")
            level: 2
        }
        //! BEGIN: Visibility
        PlasmaComponents.Label {
            Layout.row: 6
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:listbox config ui", "Visibility:")
        }

        PlasmaComponents.ComboBox {
            Layout.fillWidth: true
            model: [i18nc("@item:inlistbox config ui, visibility",
                          "Normal"), i18nc(
                    "@item:inlistbox config ui, visibility",
                    "Auto hide"), i18nc(
                    "@item:inlistbox config ui, visibility",
                    "Dodge active window"), i18nc(
                    "@item:inlistbox config ui, visibility", "Dodge windows")]
            currentIndex: dock.visibility.mode
            onActivated: {
                dock.visibility.mode = index
            }
        }

        RowLayout {
            Layout.row: 7
            Layout.column: 1
            Layout.fillWidth: true
            spacing: units.largeSpacing

            PlasmaComponents.Label {
                Layout.fillWidth: false
                horizontalAlignment: Text.AlignRight
                text: i18nc("@label:spinbox config ui, timers",
                            "Delay in show:")
            }
         /*   Controls.SpinBox {
                enabled: dock.visibility.mode !== Dock.Normal
                Layout.fillWidth: false
                Layout.maximumWidth: implicitWidth
                maximumValue: 3000
                minimumValue: 0
                value: dock.visibility.timerShow
                stepSize: 100

                onValueChanged: {
                    dock.visibility.timerShow = value
                }

                suffix: i18nc(
                            "@item:spinbox config ui, suffix in milliseconds",
                            " ms")
            }*/
            PlasmaComponents.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18nc("@label:spinbox config ui, timers",
                            "Delay in hidding:")
            }
         /*   Controls.SpinBox {
                enabled: dock.visibility.mode !== Dock.Normal
                Layout.fillWidth: false
                Layout.maximumWidth: implicitWidth
                maximumValue: 3000
                minimumValue: 0
                value: dock.visibility.timerHide
                stepSize: 100

                onValueChanged: {
                    dock.visibility.timerHide = value
                }

                suffix: i18nc(
                            "@item:spinbox config ui, suffix in milliseconds",
                            " ms")
            }*/
        }

        RowLayout {
            Layout.row: 8
            Layout.column: 1
            Layout.fillWidth: true
            spacing: units.largeSpacing

            PlasmaComponents.Label {
                text: i18nc("@option:check config ui", "Show in all screens:")
                Layout.alignment: Qt.AlignRight
            }
            Switch {
                id: switchScreen
                // TODO: Show on all screens
                style: Styles.SwitchStyle {
                    property bool checked: switchScreen.checked
                }
            }
            PlasmaComponents.ComboBox {
                Layout.fillWidth: true
                Component.onCompleted: {
                    var screens = []

                    for (var i = 0; i < dock.screens.length; i++) {
                        screens.push(dock.screens[i].name)
                    }

                    model = screens
                }
            }
        }
        //! END: Visibility
    }
    //! END: UI Components
}
