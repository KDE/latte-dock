/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.components 1.0 as LatteComponents

ColumnLayout {
    id: root
    Layout.fillWidth: true

    LatteComponents.SubHeader {
        text: i18nc("indicator style","Style")
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 2

        property int indicatorType: indicator.configuration.activeStyle

        readonly property int buttonsCount: 2
        readonly property int buttonSize: (dialog.optionsWidth - (spacing * buttonsCount-1)) / buttonsCount

        ExclusiveGroup {
            id: activeIndicatorTypeGroup
        }

        PlasmaComponents.Button {
            Layout.minimumWidth: parent.buttonSize
            Layout.maximumWidth: Layout.minimumWidth
            text: i18nc("line indicator","Line")
            checked: parent.indicatorType === indicatorType
            checkable: false
            exclusiveGroup: activeIndicatorTypeGroup
            tooltip: i18n("Show a line indicator for active items")

            readonly property int indicatorType: 0 /*Line*/

            onPressedChanged: {
                if (pressed) {
                    indicator.configuration.activeStyle = indicatorType;
                }
            }
        }

        PlasmaComponents.Button {
            Layout.minimumWidth: parent.buttonSize
            Layout.maximumWidth: Layout.minimumWidth
            text: i18nc("dots indicator", "Dots")
            checked: parent.indicatorType === indicatorType
            checkable: false
            exclusiveGroup: activeIndicatorTypeGroup
            tooltip: i18n("Show a dot indicator for active items")

            readonly property int indicatorType: 1 /*Dot*/

            onPressedChanged: {
                if (pressed) {
                    indicator.configuration.activeStyle = indicatorType;
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: units.smallSpacing

        PlasmaComponents.Label {
            text: i18n("Thickness")
            horizontalAlignment: Text.AlignLeft
        }

        LatteComponents.Slider {
            id: sizeSlider
            Layout.fillWidth: true

            value: Math.round(indicator.configuration.size * 100)
            from: 3
            to: 25
            stepSize: 1
            wheelEnabled: false

            onPressedChanged: {
                if (!pressed) {
                    indicator.configuration.size = Number(value / 100).toFixed(2);
                }
            }
        }

        PlasmaComponents.Label {
            text: i18nc("number in percentage, e.g. 85 %","%1 %", currentValue)
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

            readonly property int currentValue: sizeSlider.value
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: units.smallSpacing

        PlasmaComponents.Label {
            text: i18n("Position")
            horizontalAlignment: Text.AlignLeft
        }

        LatteComponents.Slider {
            id: thickMarginSlider
            Layout.fillWidth: true

            value: Math.round(indicator.configuration.thickMargin * 100)
            from: 0
            to: 30
            stepSize: 1
            wheelEnabled: false

            onPressedChanged: {
                if (!pressed) {
                    indicator.configuration.thickMargin = value / 100;
                }
            }
        }

        PlasmaComponents.Label {
            text: i18nc("number in percentage, e.g. 85 %","%1 %", currentValue)
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

            readonly property int currentValue: thickMarginSlider.value
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: units.smallSpacing

        PlasmaComponents.Label {
            text: i18n("Padding")
            horizontalAlignment: Text.AlignLeft
        }

        LatteComponents.Slider {
            id: lengthIntMarginSlider
            Layout.fillWidth: true

            value: Math.round(indicator.configuration.lengthPadding * 100)
            from: 0
            to: maxMargin
            stepSize: 1
            wheelEnabled: false

            readonly property int maxMargin: 80

            onPressedChanged: {
                if (!pressed) {
                    indicator.configuration.lengthPadding = value / 100;
                }
            }
        }

        PlasmaComponents.Label {
            text: i18nc("number in percentage, e.g. 85 %","%1 %", currentValue)
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

            readonly property int currentValue: lengthIntMarginSlider.value
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: units.smallSpacing

        PlasmaComponents.Label {
            text: i18n("Corner Margin")
            horizontalAlignment: Text.AlignLeft
        }

        LatteComponents.Slider {
            id: backgroundCornerMarginSlider
            Layout.fillWidth: true

            value: Math.round(indicator.configuration.backgroundCornerMargin * 100)
            from: 0
            to: 100
            stepSize: 1
            wheelEnabled: false

            onPressedChanged: {
                if (!pressed) {
                    indicator.configuration.backgroundCornerMargin = value / 100;
                }
            }
        }

        PlasmaComponents.Label {
            text: i18nc("number in percentage, e.g. 85 %","%1 %", currentValue)
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

            readonly property int currentValue: backgroundCornerMarginSlider.value
        }
    }

    LatteComponents.HeaderSwitch {
        id: glowEnabled
        Layout.fillWidth: true
        Layout.minimumHeight: implicitHeight
        Layout.bottomMargin: units.smallSpacing

        checked: indicator.configuration.glowEnabled
        level: 2
        text: i18n("Glow")
        tooltip: i18n("Enable/disable indicator glow")

        onPressed: {
            indicator.configuration.glowEnabled = !indicator.configuration.glowEnabled;
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 2
        enabled: indicator.configuration.glowEnabled

        property int option: indicator.configuration.glowApplyTo

        readonly property int buttonsCount: 2
        readonly property int buttonSize: (dialog.optionsWidth - (spacing * buttonsCount-1)) / buttonsCount

        ExclusiveGroup {
            id: glowGroup
        }

        PlasmaComponents.Button {
            Layout.minimumWidth: parent.buttonSize
            Layout.maximumWidth: Layout.minimumWidth
            text: i18nc("glow only to active task/applet indicators","On Active")
            checked: parent.option === option
            checkable: false
            exclusiveGroup:  glowGroup
            tooltip: i18n("Add glow only to active task/applet indicator")

            readonly property int option: 1 /*OnActive*/

            onPressedChanged: {
                if (pressed) {
                    indicator.configuration.glowApplyTo = option;
                }
            }
        }

        PlasmaComponents.Button {
            Layout.minimumWidth: parent.buttonSize
            Layout.maximumWidth: Layout.minimumWidth
            text: i18nc("glow to all task/applet indicators","All")
            checked: parent.option === option
            checkable: false
            exclusiveGroup: glowGroup
            tooltip: i18n("Add glow to all task/applet indicators")

            readonly property int option: 2 /*All*/

            onPressedChanged: {
                if (pressed) {
                    indicator.configuration.glowApplyTo = option;
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 2

        enabled: indicator.configuration.glowEnabled

        PlasmaComponents.Label {
            Layout.minimumWidth: implicitWidth
            horizontalAlignment: Text.AlignLeft
            Layout.rightMargin: units.smallSpacing
            text: i18n("Opacity")
        }

        LatteComponents.Slider {
            id: glowOpacitySlider
            Layout.fillWidth: true

            leftPadding: 0
            value: indicator.configuration.glowOpacity * 100
            from: 0
            to: 100
            stepSize: 5
            wheelEnabled: false

            function updateGlowOpacity() {
                if (!pressed)
                    indicator.configuration.glowOpacity = value/100;
            }

            onPressedChanged: {
                updateGlowOpacity();
            }

            Component.onCompleted: {
                valueChanged.connect(updateGlowOpacity);
            }

            Component.onDestruction: {
                valueChanged.disconnect(updateGlowOpacity);
            }
        }

        PlasmaComponents.Label {
            text: i18nc("number in percentage, e.g. 85 %","%1 %", glowOpacitySlider.value)
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
        }
    }

    ColumnLayout {
        spacing: 0
        visible: indicator.latteTasksArePresent

        LatteComponents.SubHeader {
            enabled: indicator.configuration.glowApplyTo!==0/*None*/
            text: i18n("Tasks")
        }

        LatteComponents.CheckBoxesColumn {
            LatteComponents.CheckBox {
                Layout.maximumWidth: dialog.optionsWidth
                text: i18n("Different color for minimized windows")
                value: indicator.configuration.minimizedTaskColoredDifferently

                onClicked: {
                    indicator.configuration.minimizedTaskColoredDifferently = !indicator.configuration.minimizedTaskColoredDifferently;
                }
            }

            LatteComponents.CheckBox {
                Layout.maximumWidth: dialog.optionsWidth
                text: i18n("Show an extra dot for grouped windows when active")
                tooltip: i18n("Grouped windows show both a line and a dot when one of them is active and the Line Active Indicator is enabled")
                enabled: indicator.configuration.activeStyle === 0 /*Line*/
                value: indicator.configuration.extraDotOnActive

                onClicked: {
                    indicator.configuration.extraDotOnActive = !indicator.configuration.extraDotOnActive;
                }
            }
        }
    }

    LatteComponents.SubHeader {
        enabled: indicator.configuration.glowApplyTo!==0/*None*/
        text: i18n("Options")
    }

    LatteComponents.CheckBox {
        Layout.maximumWidth: dialog.optionsWidth
        text: i18n("Show indicators for applets")
        tooltip: i18n("Indicators are shown for applets")
        value: indicator.configuration.enabledForApplets

        onClicked: {
            indicator.configuration.enabledForApplets = !indicator.configuration.enabledForApplets;
        }
    }

    LatteComponents.CheckBox {
        Layout.maximumWidth: dialog.optionsWidth
        text: i18n("Reverse indicator style")
        value: indicator.configuration.reversed

        onClicked: {
            indicator.configuration.reversed = !indicator.configuration.reversed;
        }
    }
}
