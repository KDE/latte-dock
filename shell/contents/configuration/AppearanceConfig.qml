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
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    Timer {
        id: syncGeometry

        running: false
        repeat: false
        interval: 400
        onTriggered: dockConfig.syncGeometry()
    }

    ColumnLayout {
        id: content

        width: dialog.maxWidth - Layout.leftMargin * 2
        spacing: units.largeSpacing
        anchors.centerIn: parent
        Layout.leftMargin: units.smallSpacing * 2

        //! BEGIN: Applet Size
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            Header {
                text: i18n("Applets Size")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.Slider {
                    id: appletsSizeSlider
                    Layout.fillWidth: true
                    value: plasmoid.configuration.iconSize
                    minimumValue: 16
                    maximumValue: 128
                    stepSize: 8

                    function updateIconSize() {
                        if (!pressed) {
                            plasmoid.configuration.iconSize = value
                            syncGeometry.restart()
                        }
                    }

                    onPressedChanged: {
                        updateIconSize()
                    }

                    Component.onCompleted: {
                        valueChanged.connect(updateIconSize)
                    }
                }

                PlasmaComponents.Label {
                    text: appletsSizeSlider.value + " px."
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }
        }
        //! END: Applet Size

        //! BEGIN: Zoom On Hover
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            enabled: plasmoid.configuration.durationTime > 0

            Header {
                text: i18n("Zoom On Hover")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.Slider {
                    Layout.fillWidth: true
                    id: zoomSlider

                    valueIndicatorText: i18n("Zoom Factor")
                    valueIndicatorVisible: true

                    value: Number(1 + plasmoid.configuration.zoomLevel / 20).toFixed(2)
                    minimumValue: 1
                    maximumValue: 2
                    stepSize: 0.05

                    function updateZoomLevel() {
                        if (!pressed) {
                            var result = Math.round((value - 1) * 20)
                            plasmoid.configuration.zoomLevel = result
                        }
                    }

                    onPressedChanged: {
                        updateZoomLevel()
                    }

                    Component.onCompleted: {
                        valueChanged.connect(updateZoomLevel)
                    }
                }

                PlasmaComponents.Label {
                    text: Number((zoomSlider.value * 100) - 100).toFixed(0) + "%"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }
        }
        //! END: Zoom On Hover

        //! BEGIN: Animations
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            Header {
                text: i18n("Animations")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 2

                property int duration: plasmoid.configuration.durationTime

                ExclusiveGroup {
                    id: animationsGroup
                    onCurrentChanged: {
                        if (current.checked)
                            plasmoid.configuration.durationTime = current.duration
                    }
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("None")
                    checked: parent.duration === duration
                    checkable: true
                    exclusiveGroup: animationsGroup

                    readonly property int duration: 0
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("x1")
                    checked: parent.duration === duration
                    checkable: true
                    exclusiveGroup: animationsGroup

                    readonly property int duration: 1
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("x2")
                    checked: parent.duration === duration
                    checkable: true
                    exclusiveGroup: animationsGroup

                    readonly property int duration: 2
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("x3")
                    checked: parent.duration === duration
                    checkable: true
                    exclusiveGroup: animationsGroup

                    readonly property int duration: 3
                }
            }
        }
        //! END: Animations

        //! BEGIN: Background
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            Header {
                text: i18n("Background")
            }

            GridLayout {
                width: parent.width
                rowSpacing: 1
                columnSpacing: 1
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                columns: 2

                PlasmaComponents.CheckBox {
                    id: showBackground
                    Layout.fillWidth: true
                    text: i18n("Show Panel Background")
                    checked: plasmoid.configuration.useThemePanel

                    onClicked: {
                        plasmoid.configuration.useThemePanel = checked
                    }
                }

                PlasmaComponents.CheckBox {
                    id: solidBackground
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter

                    text: i18n("Solid Background")
                    checked: plasmoid.configuration.solidPanel
                    enabled: showBackground.checked

                    onClicked: {
                        plasmoid.configuration.solidPanel = checked
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                PlasmaComponents.Slider {
                    id: panelSizeSlider
                    Layout.fillWidth: true
                    enabled: showBackground.checked

                    value: plasmoid.configuration.panelSize
                    minimumValue: 0
                    maximumValue: 100
                    stepSize: 5

                    function updatePanelSize() {
                        if (!pressed)
                            plasmoid.configuration.panelSize = value
                    }

                    onPressedChanged: {
                        updatePanelSize();
                    }

                    Component.onCompleted: {
                        valueChanged.connect(updatePanelSize)
                    }
                }

                PlasmaComponents.Label {
                    enabled: showBackground.checked
                    text: panelSizeSlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }
        }
        //! END: Background

        //! BEGIN: Max Length
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            Header {
                text: i18n("Length")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.Slider {
                    Layout.fillWidth: true
                    id: maxLengthSlider

                    valueIndicatorText: i18n("Length")
                    valueIndicatorVisible: true

                    value: plasmoid.configuration.maxLength
                    minimumValue: 30
                    maximumValue: 100
                    stepSize: 5

                    function updateMaxLength() {
                        if (!pressed) {
                            plasmoid.configuration.maxLength = value;
                        }
                    }

                    onPressedChanged: {
                        updateMaxLength();
                    }

                    Component.onCompleted: {
                        valueChanged.connect(updateMaxLength)
                    }
                }

                PlasmaComponents.Label {
                    text: maxLengthSlider.value + "%"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }
        }
        //! END: Zoom On Hover


        //! BEGIN: Shadows
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            Header {
                text: i18n("Applet shadows")
            }

            RowLayout{
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing / 2

                RowLayout {
                    Layout.fillWidth: true

                    spacing: units.smallSpacing

                    property int shadows: plasmoid.configuration.shadows

                    ExclusiveGroup {
                        id: shadowsGroup
                        onCurrentChanged: {
                            if (current.checked)
                                plasmoid.configuration.shadows = current.shadow
                        }
                    }

                    PlasmaComponents.RadioButton {
                        Layout.fillWidth: true

                        text: i18n("None")
                        checked: parent.shadows === shadow
                        exclusiveGroup: shadowsGroup

                        readonly property int shadow: 0
                    }
                    PlasmaComponents.RadioButton {
                        Layout.fillWidth: true

                        text: i18n("Locked")
                        checked: parent.shadows === shadow
                        exclusiveGroup: shadowsGroup

                        readonly property int shadow: 1
                    }
                    PlasmaComponents.RadioButton {
                        Layout.fillWidth: true

                        text: i18n("All")
                        checked: parent.shadows === shadow
                        exclusiveGroup: shadowsGroup

                        readonly property int shadow: 2
                    }
                }
            }
        }
        //! END: Shadows
    }
}
