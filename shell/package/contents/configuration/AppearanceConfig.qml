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
        spacing: dialog.subGroupSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.leftMargin: units.smallSpacing * 2

        //! BEGIN: Layout
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.topMargin: units.smallSpacing

            Header {
                text: i18n("Layout")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.ComboBox {
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
                            layoutTexts.push( "✔ "+layoutManager.currentLayoutName);
                        }

                        var activeLayout = 0;

                        for(var i=0; i<layouts.length; ++i){
                            var selText = "    ";

                            if (layouts[i] === layoutManager.currentLayoutName) {
                                selText = Qt.application.layoutDirection === Qt.RightToLeft ? " ✔" : "✔ ";
                                activeLayout = i;
                            }

                            if (Qt.application.layoutDirection === Qt.RightToLeft) {
                                   layoutTexts.push("    " + layouts[i] + selText);
                            } else {
                                layoutTexts.push(selText+layouts[i]);
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

                    onClicked: layoutManager.showLayoutConfigDialog()
                }
            }
        }
        //! END: Layout

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
                enabled: plasmoid.configuration.proportionIconSize === -1

                PlasmaComponents.Slider {
                    id: appletsSizeSlider
                    Layout.fillWidth: true
                    value: plasmoid.configuration.iconSize
                    minimumValue: 16
                    maximumValue: 128
                    stepSize: !plasmoid.configuration.autoDecreaseIconSize
                              && (plasmoid.configuration.advanced || (plasmoid.configuration.iconSize % 8 !== 0)) ? 1 : 8

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
                    }

                    Component.onDestruction: {
                        valueChanged.disconnect(updateIconSize);
                    }
                }

                PlasmaComponents.Label {
                    text: appletsSizeSlider.value + " px."
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                enabled: plasmoid.configuration.durationTime > 0

                PlasmaComponents.Label {
                    text: i18n("Zoom On Hover:")
                    horizontalAlignment: Text.AlignLeft
                }

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

                    Component.onDestruction: {
                        valueChanged.disconnect(updateZoomLevel)
                    }
                }

                PlasmaComponents.Label {
                    text: Number((zoomSlider.value * 100) - 100).toFixed(0) + "%"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                visible: plasmoid.configuration.advanced || plasmoid.configuration.proportionIconSize>0

                PlasmaComponents.Label {
                    text: i18n("Screen Height Proportion:")
                    horizontalAlignment: Text.AlignLeft
                    enabled: proportionSizeSlider.value >= proportionSizeSlider.realMinimum
                }

                PlasmaComponents.Slider {
                    id: proportionSizeSlider
                    Layout.fillWidth: true
                    value: plasmoid.configuration.proportionIconSize
                    minimumValue: 1.0
                    maximumValue: 10
                    stepSize: 0.5
                    property real realMinimum: minimumValue + 0.5

                    function updateProportionIconSize() {
                        if (!pressed) {
                            if(value<realMinimum) {
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
                    text: proportionSizeSlider.value>=proportionSizeSlider.realMinimum ?
                              proportionSizeSlider.value.toFixed(1) + "%" : "---%"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    enabled: proportionSizeSlider.value >= proportionSizeSlider.realMinimum
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                visible: plasmoid.configuration.advanced

                PlasmaComponents.Label {
                    text: i18n("Applets Distance:")
                    horizontalAlignment: Text.AlignLeft
                    enabled: iconMarginSlider.value > 0
                }

                PlasmaComponents.Slider {
                    id: iconMarginSlider
                    Layout.fillWidth: true
                    value: plasmoid.configuration.iconMargin
                    minimumValue: 0
                    maximumValue: 100
                    stepSize: 5

                    onPressedChanged: {
                        if (!pressed) {
                            plasmoid.configuration.iconMargin = value;
                        }
                    }
                }

                PlasmaComponents.Label {
                    text: iconMarginSlider.value.toFixed(1) + "%"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    enabled: iconMarginSlider.value > 0
                }
            }
        }
        //! END: Applet Size

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

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                PlasmaComponents.CheckBox {
                    id: showBackground
                    text: i18nc("show panel","Show")
                    checked: plasmoid.configuration.useThemePanel

                    onClicked: {
                        plasmoid.configuration.useThemePanel = checked
                    }
                }

                PlasmaComponents.Label {
                    text: " | "
                    horizontalAlignment: Text.AlignLeft
                    opacity: 0.35
                }

                PlasmaComponents.Label {
                    enabled: showBackground.checked
                    text: i18n("Size: ")
                    horizontalAlignment: Text.AlignLeft
                }

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

                    Component.onDestruction: {
                        valueChanged.disconnect(updatePanelSize)
                    }
                }

                PlasmaComponents.Label {
                    enabled: showBackground.checked
                    text: panelSizeSlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                visible: plasmoid.configuration.advanced

                PlasmaComponents.Label {
                    text: i18n("Opacity: ")
                    horizontalAlignment: Text.AlignLeft
                    enabled: showBackground.checked && !solidBackground.checked
                }

                PlasmaComponents.Slider {
                    id: transparencySlider
                    Layout.fillWidth: true
                    enabled: showBackground.checked && !solidBackground.checked

                    value: plasmoid.configuration.panelTransparency
                    minimumValue: 0
                    maximumValue: 100
                    stepSize: 5

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
                    enabled: showBackground.checked && !solidBackground.checked
                    text: transparencySlider.value + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 2
                visible: plasmoid.configuration.advanced

                PlasmaComponents.Button {
                    id: panelShadows
                    Layout.fillWidth: true
                    text: i18n("Shadows")
                    checked: plasmoid.configuration.panelShadows
                    checkable: true
                    enabled: showBackground.checked

                    onClicked: {
                        plasmoid.configuration.panelShadows  = checked
                    }
                }

                PlasmaComponents.Button {
                    id: solidBackground
                    Layout.fillWidth: true
                    text: i18n("Solid")
                    checked: plasmoid.configuration.solidPanel
                    checkable: true
                    enabled: showBackground.checked

                    onClicked: {
                        plasmoid.configuration.solidPanel = checked
                    }
                }
            }
        }
        //! END: Background

        //! BEGIN: Shadows
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            visible: plasmoid.configuration.advanced

            Header {
                text: i18n("Applet shadows")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 2

                ColumnLayout{
                    Layout.leftMargin: units.smallSpacing * 2
                    Layout.rightMargin: units.smallSpacing * 2
                    PlasmaComponents.CheckBox {
                        id: showAppletShadow
                        text: i18nc("show applet shadow","Show")
                        checked: plasmoid.configuration.shadows>0

                        onClicked: {
                            if (checked)
                                plasmoid.configuration.shadows = 2;
                            else
                                plasmoid.configuration.shadows = 0;
                        }
                    }


                    PlasmaComponents.Button{
                        id: backColorBtn
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                        Layout.maximumWidth: showAppletShadow.width
                        text:" "
                        enabled: showAppletShadow.checked

                        PlasmaComponents.ComboBox {
                            id: restoreCmb
                            anchors.fill: parent
                            enabled: backColorBtn.enabled

                            function addModel() {
                                var actions = [];
                                actions.push(i18nc("Use theme shadow","Theme"));
                                actions.push(i18nc("Clear applet shadow settings","Clear"));
                                restoreCmb.model = actions;
                                restoreCmb.currentIndex = -1;
                            }

                            function emptyModel() {
                                var actions = []
                                actions.push("  ");
                                restoreCmb.model = actions;
                                restoreCmb.currentIndex = -1;
                            }

                            Component.onCompleted:{
                                addModel();
                            }

                            onActivated: {
                                if (index===0) {
                                    var strC = String(theme.textColor);
                                    if (strC.indexOf("#") === 0)
                                        plasmoid.configuration.shadowColor = strC.substr(1);
                                }else if (index===1){
                                    plasmoid.configuration.shadowColor = "080808";
                                }

                                if (index===0 || index===1) {
                                    plasmoid.configuration.shadowSize = 20;
                                    plasmoid.configuration.shadowOpacity = 100;
                                }

                                restoreCmb.currentIndex = -1;
                            }

                            onCurrentIndexChanged: {
                                if (currentIndex === 0)
                                    currentIndex = -1;
                            }

                            onEnabledChanged: {
                                if (enabled)
                                    addModel();
                                else
                                    emptyModel();
                            }
                        }


                        //overlayed button
                        PlasmaComponents.Button {
                            id: colorBtn
                            width: parent.width - units.iconSizes.medium + 2*units.smallSpacing
                            height: parent.height
                            text: " "
                            enabled: showAppletShadow.checked

                            onClicked: {
                                dockConfig.setSticker(true);
                                colorDialogLoader.showDialog = true;
                            }

                            Rectangle{
                                anchors.fill: parent
                                anchors.margins: 1.5*units.smallSpacing

                                color: "#" + plasmoid.configuration.shadowColor;
                                opacity: colorBtn.enabled ? 1 : 0.4

                                Rectangle{
                                    anchors.fill: parent
                                    color: "transparent"
                                    border.width: 1
                                    border.color: theme.textColor
                                    opacity: 0.7
                                }
                            }

                            Loader{
                                id:colorDialogLoader
                                property bool showDialog: false
                                active: showDialog

                                sourceComponent: ColorDialog {
                                    title: i18n("Please choose shadow color")
                                    showAlphaChannel: false

                                    onAccepted: {
                                        //console.log("You chose: " + String(color));
                                        var strC = String(color);
                                        if (strC.indexOf("#") === 0)
                                            plasmoid.configuration.shadowColor = strC.substr(1);

                                        colorDialogLoader.showDialog = false;
                                        dockConfig.setSticker(false);
                                    }
                                    onRejected: {
                                        colorDialogLoader.showDialog = false;
                                        dockConfig.setSticker(false);
                                    }
                                    Component.onCompleted: {
                                        color = String("#" + plasmoid.configuration.shadowColor);
                                        visible = true;
                                    }
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    RowLayout{
                        PlasmaComponents.Label {
                            text: " | "
                            horizontalAlignment: Text.AlignLeft
                            opacity: 0.35
                        }

                        PlasmaComponents.Label {
                            enabled: showAppletShadow.checked
                            text: i18n("Opacity: ")
                            horizontalAlignment: Text.AlignLeft
                        }

                        PlasmaComponents.Slider {
                            id: shadowOpacitySlider
                            Layout.fillWidth: true
                            enabled: showAppletShadow.checked

                            value: plasmoid.configuration.shadowOpacity
                            minimumValue: 0
                            maximumValue: 100
                            stepSize: 5

                            function updateShadowOpacity() {
                                if (!pressed)
                                    plasmoid.configuration.shadowOpacity = value;
                            }

                            onPressedChanged: {
                                updateShadowOpacity();
                            }

                            Component.onCompleted: {
                                valueChanged.connect(updateShadowOpacity);
                            }

                            Component.onDestruction: {
                                valueChanged.disconnect(updateShadowOpacity);
                            }
                        }

                        PlasmaComponents.Label {
                            enabled: showAppletShadow.checked
                            text: shadowOpacitySlider.value + " %"
                            horizontalAlignment: Text.AlignRight
                            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        }
                    }

                    RowLayout{
                        PlasmaComponents.Label {
                            text: " | "
                            horizontalAlignment: Text.AlignLeft
                            opacity: 0.35
                        }

                        PlasmaComponents.Label {
                            enabled: showAppletShadow.checked
                            text: i18n("Size: ")
                            horizontalAlignment: Text.AlignLeft
                        }

                        PlasmaComponents.Slider {
                            id: shadowSizeSlider
                            Layout.fillWidth: true
                            enabled: showAppletShadow.checked

                            value: plasmoid.configuration.shadowSize
                            minimumValue: 0
                            maximumValue: 100
                            stepSize: 5

                            function updateShadowSize() {
                                if (!pressed)
                                    plasmoid.configuration.shadowSize = value;
                            }

                            onPressedChanged: {
                                updateShadowSize();
                            }

                            Component.onCompleted: {
                                valueChanged.connect(updateShadowSize);
                            }

                            Component.onDestruction: {
                                valueChanged.disconnect(updateShadowSize);
                            }
                        }

                        PlasmaComponents.Label {
                            enabled: showAppletShadow.checked
                            text: shadowSizeSlider.value + " %"
                            horizontalAlignment: Text.AlignRight
                            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        }
                    }
                }
            }
        }
        //! END: Shadows

        //! BEGIN: Length
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            visible: plasmoid.configuration.advanced

            Header {
                text: i18n("Length")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.Label {
                    text: i18n("Maximum: ")
                    horizontalAlignment: Text.AlignLeft
                }

                PlasmaComponents.Slider {
                    Layout.fillWidth: true
                    id: maxLengthSlider

                    valueIndicatorText: i18n("Length")
                    valueIndicatorVisible: true

                    value: plasmoid.configuration.maxLength
                    minimumValue: 30
                    maximumValue: 100
                    stepSize: 2

                    function updateMaxLength() {
                        if (!pressed) {
                            plasmoid.configuration.maxLength = value;
                            var newTotal = Math.abs(plasmoid.configuration.offset) + value;

                            //centered and justify alignments based on offset and get out of the screen in some cases
                            var centeredCheck = ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                                                 || (plasmoid.configuration.panelPosition === Latte.Dock.Justify))
                                    && ((Math.abs(plasmoid.configuration.offset) + value/2) > 50);

                            if (newTotal > 100 || centeredCheck) {
                                if ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                                        || (plasmoid.configuration.panelPosition === Latte.Dock.Justify)) {

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
                    text: maxLengthSlider.value + "%"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.Label {
                    text: i18n("Offset: ")
                    horizontalAlignment: Text.AlignLeft
                }

                PlasmaComponents.Slider {
                    Layout.fillWidth: true
                    id: offsetSlider

                    valueIndicatorText: i18n("Offset")
                    valueIndicatorVisible: true

                    value: plasmoid.configuration.offset
                    minimumValue: ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                                   || (plasmoid.configuration.panelPosition === Latte.Dock.Justify)) ? -20 :  0
                    maximumValue: ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                                   || (plasmoid.configuration.panelPosition === Latte.Dock.Justify)) ? 20 :  40
                    stepSize: 2

                    function updateOffset() {
                        if (!pressed) {
                            plasmoid.configuration.offset = value;
                            var newTotal = Math.abs(value) + plasmoid.configuration.maxLength;

                            //centered and justify alignments based on offset and get out of the screen in some cases
                            var centeredCheck = ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                                                 || (plasmoid.configuration.panelPosition === Latte.Dock.Justify))
                                    && ((Math.abs(value) + plasmoid.configuration.maxLength/2) > 50);
                            if (newTotal > 100 || centeredCheck) {
                                plasmoid.configuration.maxLength = ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                                                                    || (plasmoid.configuration.panelPosition === Latte.Dock.Justify)) ?
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
                    text: offsetSlider.value + "%"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }
        }
        //! END: Length

        PlasmaComponents.Label{
            id: bottomMarginSpacer
            text:" "
        }
    }
}
