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
import QtQuick.Dialogs 1.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte

import "../../controls" as LatteExtraControls

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    Timer {
        id: syncGeometry

        running: false
        repeat: false
        interval: 400
        onTriggered: viewConfig.syncGeometry()
    }

    ColumnLayout {
        id: content

        width: (dialog.appliedWidth - units.smallSpacing * 2) - Layout.leftMargin * 2
        spacing: dialog.subGroupSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.leftMargin: units.smallSpacing * 2

        //! BEGIN: Layout
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            spacing: units.smallSpacing
            visible: false //dialog.highLevel

            LatteExtraControls.Header {
                text: i18n("Layout")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents3.ComboBox {
                    id: layoutCmb
                    Layout.fillWidth: true

                    property var layoutTexts: [];
                    property var layouts;

                    Component.onCompleted: loadLayouts();

                    Connections {
                        target:layoutManager
                        onMenuLayoutsChanged: layoutCmb.loadLayouts();
                    }

                    function loadLayouts(){
                        layouts = layoutManager.menuLayouts;
                        layoutTexts = [];

                        //if current layout isnt at the menu layouts
                        if (layouts.indexOf(layoutManager.currentLayoutName) === -1) {
                            if (Qt.application.layoutDirection === Qt.RightToLeft) {
                                layoutTexts.push( layoutManager.currentLayoutName + " ✔ ");
                            } else {
                                layoutTexts.push( " ✔ "+layoutManager.currentLayoutName);
                            }
                        }

                        var activeLayout = 0;

                        for(var i=0; i<layouts.length; ++i){
                            var selText1 = "     ";
                            var selText2 = "     ";

                            if (layouts[i] === layoutManager.currentLayoutName) {
                                selText1 = " ✔ ";
                                activeLayout = i;
                            }

                            if (Qt.application.layoutDirection === Qt.RightToLeft) {
                                layoutTexts.push(selText2+layouts[i]+selText1);
                            } else {
                                layoutTexts.push(selText1+layouts[i]+selText2);
                            }
                        }

                        model = layoutTexts;
                        currentIndex = activeLayout;
                    }

                    onActivated: {
                        layoutManager.switchToLayout(layouts[index]);
                    }
                }

                PlasmaComponents.Button {
                    text: i18nc("opens the layout manager window","Configure...")
                    iconSource: "document-edit"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    onClicked: layoutManager.showLatteSettingsDialog()
                }
            }
        }
        //! END: Layout

        //! BEGIN: Items
        ColumnLayout {
            Layout.fillWidth: true
            //Layout.topMargin: dialog.highLevel ? 0 : units.smallSpacing
            Layout.topMargin: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            spacing: units.smallSpacing

            LatteExtraControls.Header {
                text: i18n("Items")
            }

            LatteExtraControls.SubHeader {
                text: i18nc("items effects", "Size")
                isFirstSubCategory: true
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                enabled: proportionSizeSlider.value === 1

                PlasmaComponents.Label {
                    text: i18nc("absolute size","Absolute")
                    horizontalAlignment: Text.AlignLeft
                }

                LatteExtraControls.Slider {
                    id: appletsSizeSlider
                    Layout.fillWidth: true
                    value: plasmoid.configuration.iconSize
                    from: 16
                    to: 128
                    stepSize: dialog.highLevel || (plasmoid.configuration.iconSize % 8 !== 0) || dialog.viewIsPanel ? 1 : 8
                    wheelEnabled: false

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
                        valueChanged.connect(updateIconSize);

                        if (plasmoid.configuration.iconSize>128) {
                            to = plasmoid.configuration.iconSize + 64
                        } else {
                            to = 128
                        }
                    }

                    Component.onDestruction: {
                        valueChanged.disconnect(updateIconSize);
                    }
                }

                PlasmaComponents.Label {
                    text: appletsSizeSlider.value + " px."
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                visible: dialog.expertLevel || plasmoid.configuration.proportionIconSize>0

                PlasmaComponents.Label {
                    text: i18nc("relative size", "Relative")
                    horizontalAlignment: Text.AlignLeft
                    enabled: proportionSizeSlider.value !== proportionSizeSlider.from
                }

                LatteExtraControls.Slider {
                    id: proportionSizeSlider
                    Layout.fillWidth: true
                    value: plasmoid.configuration.proportionIconSize
                    from: 1.0
                    to: 10
                    stepSize: 0.5
                    wheelEnabled: false

                    function updateProportionIconSize() {
                        if (!pressed) {
                            if(value===1) {
                                plasmoid.configuration.proportionIconSize = -1;
                            } else {
                                plasmoid.configuration.proportionIconSize = value;
                            }
                        }
                    }

                    onPressedChanged: {
                        updateProportionIconSize();
                    }

                    Component.onCompleted: {
                        valueChanged.connect(updateProportionIconSize)
                    }

                    Component.onDestruction: {
                        valueChanged.disconnect(updateProportionIconSize)
                    }
                }

                PlasmaComponents.Label {
                    text: proportionSizeSlider.value !== proportionSizeSlider.from ?
                              proportionSizeSlider.value.toFixed(1) + " %" : "--- %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    enabled: proportionSizeSlider.value !== proportionSizeSlider.from
                }
            }

            LatteExtraControls.SubHeader {
                text: i18nc("items effects", "Effects")
                //isFirstSubCategory: true
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                enabled: plasmoid.configuration.durationTime > 0

                PlasmaComponents.Label {
                    text: i18n("Zoom On Hover")
                    horizontalAlignment: Text.AlignLeft
                }

                LatteExtraControls.Slider {
                    Layout.fillWidth: true
                    id: zoomSlider

                    value: Number(1 + plasmoid.configuration.zoomLevel / 20).toFixed(2)
                    from: 1
                    to: 2
                    stepSize: 0.05
                    wheelEnabled: false

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

                    Component.onDestruction: {
                        valueChanged.disconnect(updateZoomLevel)
                    }
                }

                PlasmaComponents.Label {
                    text: Number((zoomSlider.value * 100) - 100).toFixed(0) + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            /*
            LatteExtraControls.SubHeader {
                visible: dialog.expertLevel
                text: i18nc("automatic options", "Automatic")
                isFirstSubCategory: true
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Decrease size automatically when needed")
                checked: plasmoid.configuration.autoDecreaseIconSize
                tooltip: i18n("Items size is decreased automatically when the contents exceed the maximum length \n\nHint: this option is disabled when plasma taskmanagers are present")
                enabled: !(latteView.tasksPresent() && !latteView.latteTasksPresent());
                visible: dialog.expertLevel

                onClicked: {
                    plasmoid.configuration.autoDecreaseIconSize = checked
                }
            }*/
        }
        //! END: Items

        //! BEGIN: Length
        ColumnLayout {
            Layout.fillWidth: true
            Layout.rightMargin: units.smallSpacing * 2
            spacing: units.smallSpacing

            LatteExtraControls.Header {
                text: i18n("Length")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.Label {
                    text: i18n("Maximum")
                    horizontalAlignment: Text.AlignLeft
                }

                LatteExtraControls.Slider {
                    Layout.fillWidth: true
                    id: maxLengthSlider

                    value: plasmoid.configuration.maxLength
                    from: 30
                    to: 100
                    stepSize: 2
                    wheelEnabled: false

                    function updateMaxLength() {
                        if (!pressed) {
                            plasmoid.configuration.maxLength = value;
                            var newTotal = Math.abs(plasmoid.configuration.offset) + value;

                            //centered and justify alignments based on offset and get out of the screen in some cases
                            var centeredCheck = ((plasmoid.configuration.panelPosition === Latte.Types.Center)
                                                 || (plasmoid.configuration.panelPosition === Latte.Types.Justify))
                                    && ((Math.abs(plasmoid.configuration.offset) + value/2) > 50);

                            if (newTotal > 100 || centeredCheck) {
                                if ((plasmoid.configuration.panelPosition === Latte.Types.Center)
                                        || (plasmoid.configuration.panelPosition === Latte.Types.Justify)) {

                                    var suggestedValue = (plasmoid.configuration.offset<0) ? Math.min(0, -(100-value)): Math.max(0, 100-value);

                                    if ((Math.abs(suggestedValue) + value/2) > 50) {
                                        if (suggestedValue < 0) {
                                            suggestedValue = - (50 - value/2);
                                        } else {
                                            suggestedValue = 50 - value/2;
                                        }
                                    }

                                    plasmoid.configuration.offset = suggestedValue;
                                } else {
                                    plasmoid.configuration.offset = Math.max(0, 100-value);
                                }
                            }
                        }
                    }

                    onPressedChanged: {
                        updateMaxLength();
                    }

                    Component.onCompleted: {
                        valueChanged.connect(updateMaxLength)
                    }

                    Component.onDestruction: {
                        valueChanged.disconnect(updateMaxLength)
                    }
                }

                PlasmaComponents.Label {
                    text: maxLengthSlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                visible: dialog.expertLevel

                PlasmaComponents.Label {
                    text: i18n("Offset")
                    horizontalAlignment: Text.AlignLeft
                }

                LatteExtraControls.Slider {
                    Layout.fillWidth: true
                    id: offsetSlider

                    value: plasmoid.configuration.offset
                    from: ((plasmoid.configuration.panelPosition === Latte.Types.Center)
                           || (plasmoid.configuration.panelPosition === Latte.Types.Justify)) ? -20 :  0
                    to: ((plasmoid.configuration.panelPosition === Latte.Types.Center)
                         || (plasmoid.configuration.panelPosition === Latte.Types.Justify)) ? 20 :  40
                    stepSize: 2
                    wheelEnabled: false

                    function updateOffset() {
                        if (!pressed) {
                            plasmoid.configuration.offset = value;
                            var newTotal = Math.abs(value) + plasmoid.configuration.maxLength;

                            //centered and justify alignments based on offset and get out of the screen in some cases
                            var centeredCheck = ((plasmoid.configuration.panelPosition === Latte.Types.Center)
                                                 || (plasmoid.configuration.panelPosition === Latte.Types.Justify))
                                    && ((Math.abs(value) + plasmoid.configuration.maxLength/2) > 50);
                            if (newTotal > 100 || centeredCheck) {
                                plasmoid.configuration.maxLength = ((plasmoid.configuration.panelPosition === Latte.Types.Center)
                                                                    || (plasmoid.configuration.panelPosition === Latte.Types.Justify)) ?
                                            2*(50 - Math.abs(value)) :100 - Math.abs(value);
                            }
                        }
                    }

                    onPressedChanged: {
                        updateOffset();
                    }

                    Component.onCompleted: {
                        valueChanged.connect(updateOffset);
                    }

                    Component.onDestruction: {
                        valueChanged.disconnect(updateOffset);
                    }
                }

                PlasmaComponents.Label {
                    text: offsetSlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }
        }
        //! END: Length

        //! BEGIN: Margins
        ColumnLayout {
            id: marginsColumn
            Layout.fillWidth: true
            Layout.rightMargin: units.smallSpacing * 2
            spacing: units.smallSpacing
            visible: dialog.expertLevel

            readonly property int maxMargin: 25

            LatteExtraControls.Header {
                text: i18n("Margins")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.Label {
                    text: i18n("Length")
                    horizontalAlignment: Text.AlignLeft
                }

                LatteExtraControls.Slider {
                    id: lengthExtMarginSlider
                    Layout.fillWidth: true

                    value: plasmoid.configuration.lengthExtMargin
                    from: 0
                    to: marginsColumn.maxMargin
                    stepSize: 1
                    wheelEnabled: false

                    onPressedChanged: {
                        if (!pressed) {
                            plasmoid.configuration.lengthExtMargin = value;
                        }
                    }
                }

                PlasmaComponents.Label {
                    text: lengthExtMarginSlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            LatteExtraControls.HeaderSwitch {
                id: shrinkThickMargins
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight

                checked: !plasmoid.configuration.shrinkThickMargins
                level: 2
                text: i18n("Thickness")
                tooltip: i18n("Enable/disable thickness margins")

                onPressed: {
                    plasmoid.configuration.shrinkThickMargins = !plasmoid.configuration.shrinkThickMargins;
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                enabled: !plasmoid.configuration.shrinkThickMargins

                PlasmaComponents.Label {
                    text: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? i18n("Height") : i18n("Width")
                    horizontalAlignment: Text.AlignLeft
                }

                LatteExtraControls.Slider {
                    id: thickMarginSlider
                    Layout.fillWidth: true

                    value: plasmoid.configuration.thickMargin
                    from: 0
                    to: 40
                    stepSize: 1
                    wheelEnabled: false

                    onPressedChanged: {
                        if (!pressed) {
                            plasmoid.configuration.thickMargin = value;
                        }
                    }
                }

                PlasmaComponents.Label {
                    text: thickMarginSlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }
        }
        //! END: Margins

        //! BEGIN: Colors
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            visible: dialog.expertLevel

            LatteExtraControls.Header {
                Layout.columnSpan: 4
                text: i18n("Colors")
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                columnSpacing: 2
                rowSpacing: units.smallSpacing
                columns: 3

                property int themeColors: plasmoid.configuration.themeColors
                property int windowColors: plasmoid.configuration.windowColors

                ExclusiveGroup {
                    id: themeColorsGroup
                    onCurrentChanged: {
                        if (current.checked) {
                            plasmoid.configuration.themeColors = current.colors;
                        }
                    }
                }

                ExclusiveGroup {
                    id: windowColorsGroup
                    onCurrentChanged: {
                        if (current.checked) {
                            plasmoid.configuration.windowColors = current.colors;
                        }
                    }
                }

                LatteExtraControls.SubHeader {
                    Layout.columnSpan: 3
                    isFirstSubCategory: true
                    text: i18n("Theme")
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Plasma")
                    checked: parent.themeColors === colors
                    checkable: true
                    exclusiveGroup: themeColorsGroup
                    tooltip: i18n("Plasma theme color palette is going to be used")

                    readonly property int colors: Latte.Types.PlasmaThemeColors
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Reverse")
                    checked: parent.themeColors === colors
                    checkable: true
                    exclusiveGroup: themeColorsGroup
                    tooltip: i18n("Reverse color palette from plasma theme is going to be used")

                    readonly property int colors: Latte.Types.ReverseThemeColors
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Smart")
                    checked: parent.themeColors === colors
                    checkable: true
                    exclusiveGroup: themeColorsGroup
                    tooltip: i18n("Smart color palette is going to provide best contrast after taking into account the environment such as the underlying background")

                    readonly property int colors: Latte.Types.SmartThemeColors
                }

                LatteExtraControls.SubHeader {
                    Layout.columnSpan: 3
                    text: i18n("From Window")
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("None")
                    checked: parent.windowColors === colors
                    checkable: true
                    exclusiveGroup: windowColorsGroup
                    tooltip: i18n("Colors are not going to be based on any window")

                    readonly property int colors: Latte.Types.NoneWindowColors
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Active")
                    checked: parent.windowColors === colors
                    checkable: true
                    exclusiveGroup: windowColorsGroup
                    tooltip: i18n("Colors are going to be based on the active window")

                    readonly property int colors: Latte.Types.ActiveWindowColors
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Touching")
                    checked: parent.windowColors === colors
                    checkable: true
                    exclusiveGroup: windowColorsGroup
                    tooltip: i18n("Colors are going to be based on windows that are touching the view")

                    readonly property int colors: Latte.Types.TouchingWindowColors
                }
            }
        }
        //! END: Colors

        //! BEGIN: Background
        ColumnLayout {
            Layout.fillWidth: true
            Layout.rightMargin: units.smallSpacing * 2
            spacing: units.smallSpacing
            enabled: Latte.WindowSystem.compositingActive

            LatteExtraControls.HeaderSwitch {
                id: showBackground
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight
                Layout.bottomMargin: units.smallSpacing

                checked: plasmoid.configuration.useThemePanel
                text: i18n("Background")
                tooltip: i18n("Enable/disable background")

                onPressed: {
                    plasmoid.configuration.useThemePanel = !plasmoid.configuration.useThemePanel;
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2

                PlasmaComponents.Label {
                    enabled: showBackground.checked
                    text: i18n("Size")
                    horizontalAlignment: Text.AlignLeft
                }

                LatteExtraControls.Slider {
                    id: panelSizeSlider
                    Layout.fillWidth: true
                    enabled: showBackground.checked

                    value: plasmoid.configuration.panelSize
                    from: 0
                    to: 100
                    stepSize: 2
                    wheelEnabled: false

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

                    Component.onDestruction: {
                        valueChanged.disconnect(updatePanelSize)
                    }
                }

                PlasmaComponents.Label {
                    enabled: showBackground.checked
                    text: panelSizeSlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2

                PlasmaComponents.Label {
                    text: i18n("Opacity")
                    horizontalAlignment: Text.AlignLeft
                    enabled: transparencySlider.enabled
                }

                LatteExtraControls.Slider {
                    id: transparencySlider
                    Layout.fillWidth: true
                    enabled: showBackground.checked //&& !blockOpacityAdjustment

                    value: plasmoid.configuration.panelTransparency
                    from: 0
                    to: 100
                    stepSize: 1
                    wheelEnabled: false

                    /*property bool blockOpacityAdjustment: (plasmoid.configuration.solidBackgroundForMaximized && plasmoid.configuration.backgroundOnlyOnMaximized)
                                                          || (solidBackground.checked
                                                              && !plasmoid.configuration.solidBackgroundForMaximized
                                                              && !plasmoid.configuration.backgroundOnlyOnMaximized)*/

                    function updatePanelTransparency() {
                        if (!pressed)
                            plasmoid.configuration.panelTransparency = value
                    }

                    onPressedChanged: {
                        updatePanelTransparency();
                    }

                    Component.onCompleted: {
                        valueChanged.connect(updatePanelTransparency);
                    }

                    Component.onDestruction: {
                        valueChanged.disconnect(updatePanelTransparency);
                    }
                }

                PlasmaComponents.Label {
                    enabled: transparencySlider.enabled
                    text: transparencySlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            LatteExtraControls.SubHeader {
                visible: dialog.expertLevel
                isFirstSubCategory: true
                text: i18n("Options")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: 2
                visible: dialog.expertLevel

                PlasmaComponents.Button {
                    id: panelBlur
                    Layout.fillWidth: true
                    text: i18n("Blur")
                    checked: plasmoid.configuration.blurEnabled
                    checkable: true
                    enabled: showBackground.checked
                    tooltip: i18n("Background is blurred underneath")

                    onClicked: {
                        plasmoid.configuration.blurEnabled  = checked
                    }
                }

                PlasmaComponents.Button {
                    id: panelShadows
                    Layout.fillWidth: true
                    text: i18n("Shadows")
                    checked: plasmoid.configuration.panelShadows
                    checkable: true
                    enabled: showBackground.checked
                    tooltip: i18n("Background shows its shadows")

                    onClicked: {
                        plasmoid.configuration.panelShadows  = checked
                    }
                }

                PlasmaComponents.Button {
                    id: solidBackground
                    Layout.fillWidth: true
                    text: i18n("Solid Style")
                    checked: plasmoid.configuration.solidPanel
                    checkable: true
                    enabled: showBackground.checked
                    tooltip: i18n("Background uses a solid style image provided from plasma theme")

                    onClicked: {
                        plasmoid.configuration.solidPanel = checked
                    }
                }
            }

            LatteExtraControls.SubHeader {
                visible: dialog.expertLevel
                text: i18nc("dynamic visibility for background", "Dynamic Visibility")
            }

            PlasmaComponents.CheckBox {
                id: solidForMaximizedChk
                Layout.leftMargin: units.smallSpacing * 2
                Layout.maximumWidth: (dialog.appliedWidth - units.smallSpacing * 2) - 3*units.smallSpacing
                text: i18n("Prefer opaque background when touching any window")
                checked: plasmoid.configuration.solidBackgroundForMaximized
                tooltip: i18n("Background removes its transparency setting when a window is touching")
                style: LatteExtraControls.LatteCheckBoxStyle{}
                enabled: showBackground.checked
                visible: dialog.expertLevel

                onClicked: {
                    plasmoid.configuration.solidBackgroundForMaximized = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: onlyOnMaximizedChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Hide background when not needed")
                checked: plasmoid.configuration.backgroundOnlyOnMaximized
                tooltip: i18n("Background becomes hidden except when a window is touching or the desktop background is busy")
                enabled: showBackground.checked
                visible: dialog.expertLevel

                onClicked: {
                    plasmoid.configuration.backgroundOnlyOnMaximized = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: hideShadowsOnMaximizedChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Hide background shadow for maximized windows")
                checked: plasmoid.configuration.disablePanelShadowForMaximized
                tooltip: i18n("Background shadows become hidden when a maximized window is touching the view")
                enabled: showBackground.checked
                visible: dialog.expertLevel

                onClicked: {
                    plasmoid.configuration.disablePanelShadowForMaximized = checked;
                }
            }

            LatteExtraControls.SubHeader {
                visible: dialog.expertLevel
                text: i18n("Exceptions")
            }

            PlasmaComponents.CheckBox {
                id: solidForPopupsChk
                Layout.leftMargin: units.smallSpacing * 2
                Layout.maximumWidth: (dialog.appliedWidth - units.smallSpacing * 2) - 3*units.smallSpacing
                text: i18n("Prefer Plasma background and colors for expanded applets")
                checked: plasmoid.configuration.plasmaBackgroundForPopups
                tooltip: i18n("Background becomes opaque in plasma style when applets are expanded")
                style: LatteExtraControls.LatteCheckBoxStyle{}
                enabled: showBackground.checked
                visible: dialog.expertLevel

                onClicked: {
                    plasmoid.configuration.plasmaBackgroundForPopups = checked;
                }
            }
        }
        //! END: Background

        //! Bottom spacer
        PlasmaComponents.Label{
            id: bottomMarginSpacer
            text:" "
        }
    }
}
