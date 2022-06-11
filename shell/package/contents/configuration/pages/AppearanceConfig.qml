/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents
import org.kde.latte.private.containment 0.1 as LatteContainment

PlasmaComponents.Page {
    id: page
    width: content.width + content.Layout.leftMargin * 2
    height: content.height + units.smallSpacing * 2

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

        //! BEGIN: Items
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: units.smallSpacing

            spacing: units.smallSpacing

            LatteComponents.Header {
                text: i18n("Items")
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    spacing: units.smallSpacing
                    enabled: proportionSizeSlider.value === 1

                    PlasmaComponents.Label {
                        text: i18nc("absolute size","Absolute size")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: appletsSizeSlider
                        Layout.fillWidth: true
                        //!round to nearest odd number
                        value: 2 * Math.round(plasmoid.configuration.iconSize / 2)
                        from: 16
                        to: 512
                        stepSize: dialog.advancedLevel || (plasmoid.configuration.iconSize % 8 !== 0) || dialog.viewIsPanel ? 2 : 8
                        wheelEnabled: false

                        function updateIconSize() {
                            if (!pressed) {
                                plasmoid.configuration.iconSize = value;
                                syncGeometry.restart();
                            }
                        }

                        onPressedChanged: {
                            updateIconSize()
                        }

                        Component.onCompleted: {
                            valueChanged.connect(updateIconSize);
                        }

                        Component.onDestruction: {
                            valueChanged.disconnect(updateIconSize);
                        }
                    }

                    PlasmaComponents.Label {
                        text: i18nc("number in pixels, e.g. 12 px.", "%1 px.", appletsSizeSlider.value)//.arg(appletsSizeSlider.value)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    spacing: units.smallSpacing
                    visible: dialog.advancedLevel || plasmoid.configuration.proportionIconSize>0

                    PlasmaComponents.Label {
                        text: i18nc("relative size", "Relative size")
                        horizontalAlignment: Text.AlignLeft
                        enabled: proportionSizeSlider.value !== proportionSizeSlider.from
                    }

                    LatteComponents.Slider {
                        id: proportionSizeSlider
                        Layout.fillWidth: true
                        value: plasmoid.configuration.proportionIconSize
                        from: 1.0
                        to: (latteView.visibility.mode === LatteCore.Types.SidebarOnDemand || latteView.visibility.mode === LatteCore.Types.SidebarAutoHide)  ? 25 : 12
                        stepSize: 0.1
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
                        id: absoluteSizeLbl
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

                        text: proportionSizeSlider.value !== proportionSizeSlider.from ?
                                  (absoluteSizeLblMouseArea.containsMouse ?
                                       i18nc("number in pixels, e.g. 64 px.","%1 px.", latteView.metrics.maxIconSize) :
                                       i18nc("number in percentage, e.g. 85 %","%1 %", proportionSizeSlider.value.toFixed(1))) :
                                  i18nc("no value in percentage","--- %")
                        horizontalAlignment: Text.AlignRight
                        enabled: proportionSizeSlider.value !== proportionSizeSlider.from

                        MouseArea {
                            id: absoluteSizeLblMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    spacing: units.smallSpacing
                    enabled: LatteCore.WindowSystem.compositingActive && plasmoid.configuration.animationsEnabled

                    PlasmaComponents.Label {
                        text: i18n("Zoom on hover")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: zoomSlider
                        Layout.fillWidth: true
                        value: Number(1 + plasmoid.configuration.zoomLevel / 20).toFixed(2)
                        from: 1
                        to: 2.25
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
                        text: i18nc("number in percentage, e.g. 85 %","%1 %", Number((zoomSlider.value * 100) - 100).toFixed(0))
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }
            }
        }
        //! END: Items

        //! BEGIN: Length
        ColumnLayout {
            Layout.fillWidth: true

            spacing: units.smallSpacing

            LatteComponents.Header {
                text: i18n("Length")
            }

            ColumnLayout {
                id: lengthColumn
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0

                readonly property int labelsMaxWidth: Math.max(maxLengthLbl.implicitWidth,
                                                               minLengthLbl.implicitWidth,
                                                               offsetLbl.implicitWidth)

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    spacing: units.smallSpacing

                    PlasmaComponents.Label {
                        id: maxLengthLbl
                        text: i18n("Maximum")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: maxLengthSlider
                        Layout.fillWidth: true

                        value: plasmoid.configuration.maxLength
                        from: 0
                        to: 100
                        stepSize: 1
                        wheelEnabled: false

                        readonly property int localMinValue: 1

                        function updateMaxLength() {
                            if (!pressed && viewConfig.isReady) {
                                plasmoid.configuration.maxLength = Math.max(value, plasmoid.configuration.minLength, localMinValue);
                                var newTotal = Math.abs(plasmoid.configuration.offset) + value;

                                //centered and justify alignments based on offset and get out of the screen in some cases
                                var centeredCheck = ((plasmoid.configuration.alignment === LatteCore.Types.Center)
                                                     || (plasmoid.configuration.alignment === LatteCore.Types.Justify))
                                        && ((Math.abs(plasmoid.configuration.offset) + value/2) > 50);

                                if (newTotal > 100 || centeredCheck) {
                                    if ((plasmoid.configuration.alignment === LatteCore.Types.Center)
                                            || (plasmoid.configuration.alignment === LatteCore.Types.Justify)) {

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

                                if (plasmoid.configuration.maxLength < plasmoid.configuration.minLength) {
                                    minLengthSlider.updateMinLength();
                                }
                            } else {
                                if ((value < plasmoid.configuration.minLength) || (value < localMinValue)) {
                                    value = Math.max(plasmoid.configuration.minLength, localMinValue);
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
                        text: i18nc("number in percentage, e.g. 85 %","%1 %", maxLengthSlider.value)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

                        LatteComponents.ScrollArea {
                            anchors.fill: parent
                            delayIsEnabled: false

                            readonly property real smallStep: 0.1

                            onScrolledUp:  {
                                var ctrlModifier = (wheel.modifiers & Qt.ControlModifier);
                                if (ctrlModifier) {
                                    plasmoid.configuration.maxLength = plasmoid.configuration.maxLength + smallStep;
                                }
                            }

                            onScrolledDown: {
                                var ctrlModifier = (wheel.modifiers & Qt.ControlModifier);
                                if (ctrlModifier) {
                                    plasmoid.configuration.maxLength = plasmoid.configuration.maxLength - smallStep;
                                }
                            }

                            onClicked: {
                                plasmoid.configuration.maxLength = Math.round(plasmoid.configuration.maxLength);
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    spacing: units.smallSpacing
                    visible: dialog.advancedLevel
                    enabled: (plasmoid.configuration.alignment !== LatteCore.Types.Justify)

                    PlasmaComponents.Label {
                        id: minLengthLbl
                        text: i18n("Minimum")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: minLengthSlider
                        Layout.fillWidth: true

                        value: plasmoid.configuration.minLength
                        from: 0
                        to: 100
                        stepSize: 1
                        wheelEnabled: false

                        function updateMinLength() {
                            if (!pressed  && viewConfig.isReady) {
                                plasmoid.configuration.minLength = value; //Math.min(value, plasmoid.configuration.maxLength);

                                if (plasmoid.configuration.minLength > maxLengthSlider.value) {
                                    maxLengthSlider.updateMaxLength();
                                }
                            } else {
                                if (value > plasmoid.configuration.maxLength) {
                                    value = plasmoid.configuration.maxLength
                                }
                            }
                        }

                        onPressedChanged: {
                            updateMinLength();
                        }

                        Component.onCompleted: {
                            valueChanged.connect(updateMinLength)
                        }

                        Component.onDestruction: {
                            valueChanged.disconnect(updateMinLength)
                        }
                    }

                    PlasmaComponents.Label {
                        text: i18nc("number in percentage, e.g. 85 %","%1 %", minLengthSlider.value)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

                        LatteComponents.ScrollArea {
                            anchors.fill: parent
                            delayIsEnabled: false

                            readonly property real smallStep: 0.1

                            onScrolledUp:  {
                                var ctrlModifier = (wheel.modifiers & Qt.ControlModifier);
                                if (ctrlModifier) {
                                    plasmoid.configuration.minLength = plasmoid.configuration.minLength + smallStep;
                                }
                            }

                            onScrolledDown: {
                                var ctrlModifier = (wheel.modifiers & Qt.ControlModifier);
                                if (ctrlModifier) {
                                    plasmoid.configuration.minLength = plasmoid.configuration.minLength - smallStep;
                                }
                            }

                            onClicked: {
                                plasmoid.configuration.minLength = Math.round(plasmoid.configuration.minLength);
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    spacing: units.smallSpacing
                    visible: dialog.advancedLevel
                    enabled: offsetSlider.to > offsetSlider.from

                    PlasmaComponents.Label {
                        id: offsetLbl
                        text: i18n("Offset")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: offsetSlider
                        Layout.fillWidth: true
                        stepSize: 1
                        wheelEnabled: false

                        readonly property int screenLengthMaxFactor: (100 - plasmoid.configuration.maxLength) / 2

                        //! these properties are used in order to not update view_offset incorrectly when the primary config view
                        //! is changing between different views
                        property bool userInputIsValid: false
                        readonly property bool sliderIsReady: viewConfig.isReady && (from===fromValue) && (to===toValue)

                        readonly property int fromValue: ((plasmoid.configuration.alignment === LatteCore.Types.Center)
                                                          || (plasmoid.configuration.alignment === LatteCore.Types.Justify)) ? -offsetSlider.screenLengthMaxFactor :  0

                        readonly property int toValue: ((plasmoid.configuration.alignment === LatteCore.Types.Center)
                                                        || (plasmoid.configuration.alignment === LatteCore.Types.Justify)) ? offsetSlider.screenLengthMaxFactor :  2*offsetSlider.screenLengthMaxFactor

                        property real offsetValue: plasmoid.configuration.offset

                        Binding {
                            target: offsetSlider
                            property: "from"
                            when: viewConfig.isReady
                            value: offsetSlider.fromValue
                        }

                        Binding {
                            target: offsetSlider
                            property: "to"
                            when: viewConfig.isReady
                            value: offsetSlider.toValue
                        }

                        function updateOffset() {
                            if (!pressed && sliderIsReady) {
                                if (userInputIsValid) {
                                    plasmoid.configuration.offset = value;
                                } else {
                                    value = Math.min(Math.max(from, plasmoid.configuration.offset), to);
                                    plasmoid.configuration.offset = value;
                                }

                                var newTotal = Math.abs(value) + plasmoid.configuration.maxLength;

                                //centered and justify alignments based on offset and get out of the screen in some cases
                                var centeredCheck = ((plasmoid.configuration.alignment === LatteCore.Types.Center)
                                                     || (plasmoid.configuration.alignment === LatteCore.Types.Justify))
                                        && ((Math.abs(value) + plasmoid.configuration.maxLength/2) > 50);
                                if (newTotal > 100 || centeredCheck) {
                                    plasmoid.configuration.maxLength = ((plasmoid.configuration.alignment === LatteCore.Types.Center)
                                                                        || (plasmoid.configuration.alignment === LatteCore.Types.Justify)) ?
                                                2*(50 - Math.abs(value)) :100 - Math.abs(value);
                                }
                            }
                        }

                        onPressedChanged: {
                            if (pressed) {
                                userInputIsValid = true;
                            } else {
                                updateOffset();
                                userInputIsValid = false;
                            }
                        }

                        Component.onCompleted: {
                            offsetValueChanged.connect(updateOffset);
                            fromChanged.connect(updateOffset);
                            toChanged.connect(updateOffset);
                            sliderIsReadyChanged.connect(updateOffset);

                            updateOffset();
                        }

                        Component.onDestruction: {
                            offsetValueChanged.disconnect(updateOffset);
                            fromChanged.disconnect(updateOffset);
                            toChanged.disconnect(updateOffset);
                            sliderIsReadyChanged.disconnect(updateOffset);
                        }
                    }

                    PlasmaComponents.Label {
                        text: i18nc("number in percentage, e.g. 85 %","%1 %", offsetSlider.value)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

                        LatteComponents.ScrollArea {
                            anchors.fill: parent
                            delayIsEnabled: false

                            readonly property real smallStep: 0.1

                            onScrolledUp:  {
                                var ctrlModifier = (wheel.modifiers & Qt.ControlModifier);
                                if (ctrlModifier) {
                                    plasmoid.configuration.offset= plasmoid.configuration.offset + smallStep;
                                }
                            }

                            onScrolledDown: {
                                var ctrlModifier = (wheel.modifiers & Qt.ControlModifier);
                                if (ctrlModifier) {
                                    plasmoid.configuration.offset = plasmoid.configuration.offset - smallStep;
                                }
                            }

                            onClicked: {
                                plasmoid.configuration.offset = Math.round(plasmoid.configuration.offset);
                            }
                        }
                    }
                }
            }
        }
        //! END: Length

        //! BEGIN: Margins
        ColumnLayout {
            id: marginsColumn
            Layout.fillWidth: true

            spacing: units.smallSpacing
            visible: dialog.advancedLevel

            readonly property int maxMargin: 25

            LatteComponents.Header {
                text: i18n("Margins")
            }

            ColumnLayout{
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth

                    spacing: units.smallSpacing

                    PlasmaComponents.Label {
                        text: i18n("Length")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
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
                        text: lengthMarginLblMouseArea.containsMouse ?
                                  i18nc("number in pixels, e.g. 8 px.","%1 px.", currentValueInPixels) :
                                  i18nc("number in percentage, e.g. 85 %","%1 %", lengthExtMarginSlider.value)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

                        readonly property int currentValueInPixels: (lengthExtMarginSlider.value/100) * latteView.metrics.maxIconSize

                        MouseArea {
                            id: lengthMarginLblMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    spacing: units.smallSpacing

                    PlasmaComponents.Label {
                        text: i18n("Thickness")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: thickMarginSlider
                        Layout.fillWidth: true

                        value: plasmoid.configuration.thickMargin
                        from: 0
                        to: 60
                        stepSize: 1
                        wheelEnabled: false
                        minimumInternalValue: latteView.indicator.info.minThicknessPadding * 100

                        onPressedChanged: {
                            if (!pressed) {
                                plasmoid.configuration.thickMargin = value;
                            }
                        }
                    }

                    PlasmaComponents.Label {
                        text: thickMarginLblMouseArea.containsMouse ?
                                  i18nc("number in pixels, e.g. 8 px.","%1 px.", currentValueInPixels) :
                                  i18nc("number in percentage, e.g. 85 %","%1 %", currentValue)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

                        readonly property int currentValue: Math.max(thickMarginSlider.minimumInternalValue, thickMarginSlider.value)
                        readonly property int currentValueInPixels: (currentValue/100) * latteView.metrics.maxIconSize

                        MouseArea {
                            id: thickMarginLblMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    spacing: units.smallSpacing

                    PlasmaComponents.Label {
                        text: i18n("Floating gap")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: screenEdgeMarginSlider
                        Layout.fillWidth: true

                        value: plasmoid.configuration.screenEdgeMargin
                        from: -1
                        to: 256
                        stepSize: 1
                        wheelEnabled: false

                        onPressedChanged: {
                            if (!pressed) {
                                plasmoid.configuration.screenEdgeMargin = value;
                            }
                        }
                    }

                    PlasmaComponents.Label {
                        text: currentValue < 0 ? "---" : i18nc("number in pixels, e.g. 85 px.","%1 px.", currentValue)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4

                        readonly property int currentValue: screenEdgeMarginSlider.value
                    }
                }
            }
        }
        //! END: Margins

        //! BEGIN: Colors
        ColumnLayout {
            spacing: units.smallSpacing
            visible: dialog.advancedLevel

            LatteComponents.Header {
                Layout.columnSpan: 4
                text: i18n("Colors")
            }

            GridLayout {
                id: colorsGridLayout
                Layout.minimumWidth: dialog.optionsWidth
                Layout.maximumWidth: Layout.minimumWidth
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                Layout.topMargin: units.smallSpacing
                columnSpacing: units.smallSpacing
                rowSpacing: units.smallSpacing
                columns: 2

                readonly property bool colorsScriptIsPresent: universalSettings.colorsScriptIsPresent

                PlasmaComponents.Label {
                    text: i18n("Palette")
                }

                LatteComponents.ComboBox {
                    Layout.fillWidth: true
                    model: [
                        {
                            name: i18nc("plasma theme colors", "Plasma Theme Colors"),
                            value: LatteContainment.Types.PlasmaThemeColors
                        },{
                            name: i18nc("dark theme colors", "Dark Colors"),
                            value: LatteContainment.Types.DarkThemeColors
                        },{
                            name: i18nc("light theme colors", "Light Colors"),
                            value: LatteContainment.Types.LightThemeColors
                        },{
                            name: i18nc("layout custom colors", "Layout Custom Colors"),
                            value: LatteContainment.Types.LayoutThemeColors
                        },
                        /*,{
                            name: i18nc("reverse plasma theme colors", "Reverse"),
                            value: LatteContainment.Types.ReverseThemeColors
                        }*/{
                            name: i18nc("smart theme colors", "Smart Colors Based On Desktop Background"),
                            value: LatteContainment.Types.SmartThemeColors
                        }
                    ]

                    currentIndex: colorsToIndex(plasmoid.configuration.themeColors)
                    textRole: "name"
                    onCurrentIndexChanged: plasmoid.configuration.themeColors = model[currentIndex].value

                    function colorsToIndex(colors) {
                        if (colors === LatteContainment.Types.PlasmaThemeColors) {
                            return 0;
                        } else if (colors === LatteContainment.Types.DarkThemeColors) {
                            return 1;
                        } else if (colors === LatteContainment.Types.LightThemeColors) {
                            return 2;
                        } else if (colors === LatteContainment.Types.ReverseThemeColors) {
                            return 3;
                        } else if (colors === LatteContainment.Types.LayoutThemeColors) {
                            return 3;
                        } else if (colors === LatteContainment.Types.SmartThemeColors) {
                            return 4;
                        }
                    }
                }

                PlasmaComponents.Label {
                    text: i18n("From Window")
                }

                LatteComponents.ComboBox {
                    Layout.fillWidth: true
                    model: [
                        {
                            name:i18n("Disabled"),
                            icon: "",
                            toolTip: "Colors are not going to take into account any windows"
                        },{
                            name:i18n("Current Active Window"),
                            icon: !colorsGridLayout.colorsScriptIsPresent ? "state-warning" : "",
                            toolTip: colorsGridLayout.colorsScriptIsPresent ?
                                         i18n("Colors are going to be based on the active window") :
                                         i18n("Colors are going to be based on the active window.\nWarning: You need to install Colors KWin Script from KDE Store")
                        },{
                            name: i18n("Any Touching Window"),
                            icon: !colorsGridLayout.colorsScriptIsPresent ? "state-warning" : "",
                            toolTip: colorsGridLayout.colorsScriptIsPresent ?
                                         i18n("Colors are going to be based on windows that are touching the view") :
                                         i18n("Colors are going to be based on windows that are touching the view.\nWarning: You need to install Colors KWin Script from KDE Store")
                        }
                    ]


                    textRole: "name"
                    iconRole: "icon"
                    toolTipRole: "toolTip"
                    blankSpaceForEmptyIcons: !colorsGridLayout.colorsScriptIsPresent
                    popUpAlignRight: Qt.application.layoutDirection !== Qt.RightToLeft

                    currentIndex: plasmoid.configuration.windowColors
                    onCurrentIndexChanged: plasmoid.configuration.windowColors = currentIndex
                }
            }
        }
        //! END: Colors

        //! BEGIN: Background
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            LatteComponents.HeaderSwitch {
                id: showBackground
                Layout.minimumWidth: dialog.optionsWidth + 2 *units.smallSpacing
                Layout.maximumWidth: Layout.minimumWidth
                Layout.minimumHeight: implicitHeight
                Layout.bottomMargin: units.smallSpacing
                enabled: LatteCore.WindowSystem.compositingActive

                checked: plasmoid.configuration.useThemePanel
                text: i18n("Background")
                tooltip: i18n("Enable/disable background")

                onPressed: {
                    plasmoid.configuration.useThemePanel = !plasmoid.configuration.useThemePanel;
                }
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    enabled: LatteCore.WindowSystem.compositingActive

                    PlasmaComponents.Label {
                        enabled: showBackground.checked
                        text: i18n("Thickness")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: panelSizeSlider
                        Layout.fillWidth: true
                        enabled: showBackground.checked

                        value: plasmoid.configuration.panelSize
                        from: 0
                        to: 100
                        stepSize: 1
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
                        text: i18nc("number in percentage, e.g. 85 %","%1 %", panelSizeSlider.value)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    enabled: LatteCore.WindowSystem.compositingActive

                    PlasmaComponents.Label {
                        text: plasmoid.configuration.backgroundOnlyOnMaximized && plasmoid.configuration.solidBackgroundForMaximized ?
                                  i18nc("opacity when desktop background is busy from contrast point of view","Busy Opacity") : i18n("Opacity")
                        horizontalAlignment: Text.AlignLeft
                        enabled: transparencySlider.enabled
                    }

                    LatteComponents.Slider {
                        id: transparencySlider
                        Layout.fillWidth: true
                        enabled: showBackground.checked //&& !blockOpacityAdjustment

                        value: plasmoid.configuration.panelTransparency
                        from: -1
                        to: 100
                        stepSize: 1
                        wheelEnabled: false

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
                        text: transparencySlider.value >= 0 ? i18nc("number in percentage, e.g. 85 %","%1 %", transparencySlider.value) : i18nc("Default word abbreviation", "Def.")
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    visible: dialog.advancedLevel && dialog.kirigamiLibraryIsFound

                    PlasmaComponents.Label {
                        text: i18n("Radius")
                        horizontalAlignment: Text.AlignLeft
                        enabled: radiusSlider.enabled
                    }

                    LatteComponents.Slider {
                        id: radiusSlider
                        Layout.fillWidth: true
                        enabled: showBackground.checked

                        value: plasmoid.configuration.backgroundRadius
                        from: -1
                        to: 50
                        stepSize: 1
                        wheelEnabled: false

                        function updateBackgroundRadius() {
                            if (!pressed) {
                                plasmoid.configuration.backgroundRadius = value
                            }
                        }

                        onPressedChanged: updateBackgroundRadius();
                        Component.onCompleted: valueChanged.connect(updateBackgroundRadius);
                        Component.onDestruction: valueChanged.disconnect(updateBackgroundRadius);
                    }

                    PlasmaComponents.Label {
                        enabled: radiusSlider.enabled
                        text: radiusSlider.value >= 0 ? i18nc("number in percentage, e.g. 85 %","%1 %", radiusSlider.value) : i18nc("Default word abbreviation", "Def.")
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    enabled: LatteCore.WindowSystem.compositingActive
                    visible: dialog.advancedLevel && dialog.kirigamiLibraryIsFound

                    PlasmaComponents.Label {
                        text: i18n("Shadow")
                        horizontalAlignment: Text.AlignLeft
                        enabled: shadowSlider.enabled
                    }

                    LatteComponents.Slider {
                        id: shadowSlider
                        Layout.fillWidth: true
                        enabled: showBackground.checked && panelShadows.checked

                        value: plasmoid.configuration.backgroundShadowSize
                        from: -1
                        to: 50
                        stepSize: 1
                        wheelEnabled: false

                        function updateBackgroundShadowSize() {
                            if (!pressed) {
                                plasmoid.configuration.backgroundShadowSize = value
                            }
                        }

                        onPressedChanged: updateBackgroundShadowSize();
                        Component.onCompleted: valueChanged.connect(updateBackgroundShadowSize);
                        Component.onDestruction: valueChanged.disconnect(updateBackgroundShadowSize);
                    }

                    PlasmaComponents.Label {
                        enabled: shadowSlider.enabled
                        text: shadowSlider.value >= 0 ? i18nc("number in pixels, e.g. 12 px.", "%1 px.", shadowSlider.value ) : i18nc("Default word abbreviation", "Def.")
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    Layout.topMargin: units.smallSpacing
                    spacing: 2
                    visible: dialog.advancedLevel

                    readonly property int buttonSize: (dialog.optionsWidth - (2 * spacing)) / children.length

                    PlasmaComponents.Button {
                        id: panelBlur
                        Layout.minimumWidth: parent.buttonSize
                        Layout.maximumWidth: Layout.minimumWidth
                        text: i18n("Blur")
                        checkable: true
                        enabled: showBackground.checked && LatteCore.WindowSystem.compositingActive
                        tooltip: i18n("Background is blurred underneath")

                        readonly property int blurEnabled: plasmoid.configuration.blurEnabled

                        onBlurEnabledChanged: checked = blurEnabled;

                        onClicked: {
                            plasmoid.configuration.blurEnabled  = checked
                        }
                    }

                    PlasmaComponents.Button {
                        id: panelShadows
                        Layout.minimumWidth: parent.buttonSize
                        Layout.maximumWidth: Layout.minimumWidth
                        text: i18n("Shadows")
                        checkable: true
                        enabled: showBackground.checked && LatteCore.WindowSystem.compositingActive && themeExtended.hasShadow
                        tooltip: i18n("Background shows its shadows")

                        readonly property int panelShadows: plasmoid.configuration.panelShadows

                        onPanelShadowsChanged: checked = panelShadows

                        onClicked: {
                            plasmoid.configuration.panelShadows  = checked
                        }
                    }

                    PlasmaComponents.Button {
                        id: solidBackground
                        Layout.minimumWidth: parent.buttonSize
                        Layout.maximumWidth: Layout.minimumWidth
                        text: i18n("Outline")
                        checkable: true
                        checked: plasmoid.configuration.panelOutline
                        enabled: showBackground.checked
                        tooltip: i18n("Background draws a line for its borders. You can set the line size from Latte Preferences")

                        onClicked: {
                            plasmoid.configuration.panelOutline = checked;
                        }
                    }

                    PlasmaComponents.Button {
                        id: allCorners
                        Layout.minimumWidth: parent.buttonSize
                        Layout.maximumWidth: Layout.minimumWidth
                        text: i18n("All Corners")
                        checkable: true
                        checked: plasmoid.configuration.backgroundAllCorners
                        enabled: showBackground.checked
                                 && ((plasmoid.configuration.screenEdgeMargin===-1) /*no-floating*/
                                     || (plasmoid.configuration.screenEdgeMargin > -1 /*floating with justify alignment and 100% maxlength*/
                                         && plasmoid.configuration.alignment ===LatteCore.Types.Justify
                                         && plasmoid.configuration.maxLength===100))
                        tooltip: i18n("Background draws all corners at all cases.")

                        onClicked: {
                            plasmoid.configuration.backgroundAllCorners = checked;
                        }
                    }
                }

                LatteComponents.SubHeader {
                    visible: dialog.advancedLevel
                    text: i18nc("dynamic visibility for background", "Dynamic Visibility")
                    enabled: LatteCore.WindowSystem.compositingActive
                }

                LatteComponents.CheckBoxesColumn {
                    enabled: LatteCore.WindowSystem.compositingActive
                    LatteComponents.CheckBox {
                        id: solidForMaximizedChk
                        Layout.maximumWidth: dialog.optionsWidth
                        text: i18n("Prefer opaque background when touching any window")
                        tooltip: i18n("Background removes its transparency setting when a window is touching")
                        enabled: showBackground.checked
                        visible: dialog.advancedLevel
                        value: plasmoid.configuration.solidBackgroundForMaximized

                        onClicked: {
                            plasmoid.configuration.solidBackgroundForMaximized = !plasmoid.configuration.solidBackgroundForMaximized;
                        }
                    }

                    LatteComponents.CheckBox {
                        id: onlyOnMaximizedChk
                        Layout.maximumWidth: dialog.optionsWidth
                        text: i18n("Hide background when not needed")
                        tooltip: i18n("Background becomes hidden except when a window is touching or the desktop background is busy")
                        enabled: showBackground.checked
                        visible: dialog.advancedLevel
                        value: plasmoid.configuration.backgroundOnlyOnMaximized

                        onClicked: {
                            plasmoid.configuration.backgroundOnlyOnMaximized = !plasmoid.configuration.backgroundOnlyOnMaximized;
                        }
                    }

                    LatteComponents.CheckBox {
                        id: hideShadowsOnMaximizedChk
                        Layout.maximumWidth: dialog.optionsWidth
                        text: i18n("Hide background shadow for maximized windows")
                        tooltip: i18n("Background shadows become hidden when an active maximized window is touching the view")
                        enabled: showBackground.checked
                        visible: dialog.advancedLevel
                        value: plasmoid.configuration.disablePanelShadowForMaximized

                        onClicked: {
                            plasmoid.configuration.disablePanelShadowForMaximized = !plasmoid.configuration.disablePanelShadowForMaximized;
                        }
                    }
                }

                LatteComponents.SubHeader {
                    visible: dialog.advancedLevel
                    text: i18n("Exceptions")
                    enabled: LatteCore.WindowSystem.compositingActive
                }

                LatteComponents.CheckBox {
                    id: solidForPopupsChk
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Prefer Plasma background and colors for expanded applets")
                    tooltip: i18n("Background becomes opaque in plasma style when applets are expanded")
                    enabled: showBackground.checked
                    visible: dialog.advancedLevel
                    value: plasmoid.configuration.plasmaBackgroundForPopups

                    onClicked: {
                        plasmoid.configuration.plasmaBackgroundForPopups = !plasmoid.configuration.plasmaBackgroundForPopups;
                    }
                }
            }
        }
        //! END: Background
    }
}
