/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

ColumnLayout {
    id: root
    Layout.fillWidth: true

    LatteComponents.SubHeader {
        text: i18nc("active indicator style","Style For Active")
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 2

        property int indicatorType: indicator.configuration.activeStyle

        readonly property int buttonsCount: 2
        readonly property int buttonSize: (dialog.optionsWidth - (spacing * buttonsCount-1)) / buttonsCount

        ExclusiveGroup {
            id: activeIndicatorTypeGroup
            onCurrentChanged: {
                if (current.checked) {
                    indicator.configuration.activeStyle = current.indicatorType;
                }
            }
        }

        PlasmaComponents.Button {
            Layout.minimumWidth: parent.buttonSize
            Layout.maximumWidth: Layout.minimumWidth
            text: i18nc("line indicator","Line")
            checked: parent.indicatorType === indicatorType
            checkable: true
            exclusiveGroup: activeIndicatorTypeGroup
            tooltip: i18n("Show a line indicator for active items")

            readonly property int indicatorType: 0 /*Line*/
        }

        PlasmaComponents.Button {
            Layout.minimumWidth: parent.buttonSize
            Layout.maximumWidth: Layout.minimumWidth
            text: i18nc("dot indicator", "Dot")
            checked: parent.indicatorType === indicatorType
            checkable: true
            exclusiveGroup: activeIndicatorTypeGroup
            tooltip: i18n("Show a dot indicator for active items")

            readonly property int indicatorType: 1 /*Dot*/
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
            onCurrentChanged: {
                if (current.checked)
                    indicator.configuration.glowApplyTo = current.option
            }
        }

        PlasmaComponents.Button {
            Layout.minimumWidth: parent.buttonSize
            Layout.maximumWidth: Layout.minimumWidth
            text: i18nc("glow only to active task/applet indicators","On Active")
            checked: parent.option === option
            checkable: true
            exclusiveGroup:  glowGroup
            tooltip: i18n("Add glow only to active task/applet indicator")

            readonly property int option: 1 /*OnActive*/
        }

        PlasmaComponents.Button {
            Layout.minimumWidth: parent.buttonSize
            Layout.maximumWidth: Layout.minimumWidth
            text: i18nc("glow to all task/applet indicators","All")
            checked: parent.option === option
            checkable: true
            exclusiveGroup: glowGroup
            tooltip: i18n("Add glow to all task/applet indicators")

            readonly property int option: 2 /*All*/
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
            text: i18nc("number in percentage, e.g. 85 %","%0 %").arg(glowOpacitySlider.value)
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
        }
    }

    LatteComponents.SubHeader {
        text: i18n("Padding")
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: units.smallSpacing

        PlasmaComponents.Label {
            text: i18n("Length")
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
            text: i18nc("number in percentage, e.g. 85 %","%0 %").arg(currentValue)
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

            readonly property int currentValue: lengthIntMarginSlider.value
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
                checked: indicator.configuration.minimizedTaskColoredDifferently

                onClicked: {
                    indicator.configuration.minimizedTaskColoredDifferently = checked;
                }
            }

            LatteComponents.CheckBox {
                Layout.maximumWidth: dialog.optionsWidth
                text: i18n("Show an extra dot for grouped windows when active")
                checked: indicator.configuration.extraDotOnActive
                tooltip: i18n("Grouped windows show both a line and a dot when one of them is active and the Line Active Indicator is enabled")
                enabled: indicator.configuration.activeStyle === 0 /*Line*/

                onClicked: {
                    indicator.configuration.extraDotOnActive = checked;
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
        checked: indicator.configuration.enabledForApplets
        tooltip: i18n("Indicators are shown for applets")

        onClicked: {
            indicator.configuration.enabledForApplets = !indicator.configuration.enabledForApplets;
        }
    }

    LatteComponents.CheckBox {
        Layout.maximumWidth: dialog.optionsWidth
        text: i18n("Reverse indicator style")
        checked: indicator.configuration.reversed

        onClicked: {
            indicator.configuration.reversed = !indicator.configuration.reversed;
        }
    }
}
