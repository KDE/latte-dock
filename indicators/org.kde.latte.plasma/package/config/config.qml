/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte.components 1.0 as LatteComponents

ColumnLayout {
    id: root
    Layout.fillWidth: true

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
            text: i18nc("number in percentage, e.g. 85 %","%1 %", currentValue)
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

            readonly property int currentValue: lengthIntMarginSlider.value
        }
    }

    LatteComponents.SubHeader {
        text: i18n("Options")
    }

    LatteComponents.CheckBoxesColumn {
        Layout.topMargin: 1.5 * units.smallSpacing

        LatteComponents.CheckBox {
            Layout.maximumWidth: dialog.optionsWidth
            text: i18n("Reverse indicator style")
            value: indicator.configuration.reversed

            onClicked: {
                indicator.configuration.reversed = !indicator.configuration.reversed;
            }
        }

        LatteComponents.CheckBox {
            Layout.maximumWidth: dialog.optionsWidth
            text: i18n("Growing circle animation when clicked")
            value: indicator.configuration.clickedAnimationEnabled

            onClicked: {
                indicator.configuration.clickedAnimationEnabled = !indicator.configuration.clickedAnimationEnabled;
            }
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
    }
}
