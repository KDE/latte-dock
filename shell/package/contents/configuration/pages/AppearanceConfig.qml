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
            spacing: units.smallSpacing
            Layout.topMargin: units.smallSpacing
            visible: dialog.highLevel

            LatteExtraControls.Header {
                text: i18n("Layout")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
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

        //! BEGIN: Applet Size
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: dialog.highLevel ? 0 : units.smallSpacing
            spacing: units.smallSpacing

            LatteExtraControls.Header {
                text: i18n("Applets")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                enabled: plasmoid.configuration.proportionIconSize === -1

                PlasmaComponents.Label {
                    text: i18n("Size")
                    horizontalAlignment: Text.AlignLeft
                }

                LatteExtraControls.Slider {
                    id: appletsSizeSlider
                    Layout.fillWidth: true
                    value: plasmoid.configuration.iconSize
                    from: 16
                    to: 128
                    stepSize: (!plasmoid.configuration.autoDecreaseIconSize
                               && (dialog.highLevel || (plasmoid.configuration.iconSize % 8 !== 0)))
                              || dialog.viewIsPanel ? 1 : 8
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
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                visible: dialog.expertLevel || plasmoid.configuration.proportionIconSize>0

                PlasmaComponents.Label {
                    text: i18n("Screen Height Proportion")
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

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                visible: dialog.highLevel

                PlasmaComponents.Label {
                    text: i18n("Distance")
                    horizontalAlignment: Text.AlignLeft
                    enabled: iconMarginSlider.value > 0
                }

                LatteExtraControls.Slider {
                    id: iconMarginSlider
                    Layout.fillWidth: true
                    value: plasmoid.configuration.iconMargin
                    from: 0
                    to: 100
                    stepSize: 5
                    wheelEnabled: false

                    onPressedChanged: {
                        if (!pressed) {
                            plasmoid.configuration.iconMargin = value;
                        }
                    }
                }

                PlasmaComponents.Label {
                    text: iconMarginSlider.value.toFixed(1) + " %"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    enabled: iconMarginSlider.value > 0
                }
            }

            PlasmaComponents.CheckBox {
                id: shrinkThickness
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Shrink thickness margins to minimum")
                checked: plasmoid.configuration.shrinkThickMargins

                onClicked: {
                    plasmoid.configuration.shrinkThickMargins = checked
                }
            }
        }
        //! END: Applet Size

        //! BEGIN: Background
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            enabled: Latte.WindowSystem.compositingActive

            LatteExtraControls.Header {
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
                    stepSize: 5
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
                Layout.rightMargin: units.smallSpacing * 2
                visible: dialog.highLevel

                PlasmaComponents.Label {
                    text: i18n("Opacity")
                    horizontalAlignment: Text.AlignLeft
                    enabled: transparencySlider.enabled
                }

                LatteExtraControls.Slider {
                    id: transparencySlider
                    Layout.fillWidth: true
                    //! transparency can be set when the user doesn't want solidness all the time
                    //! or when the user wants solidness only for maximized windows
                    enabled: showBackground.checked && !blockOpacityAdjustment

                    value: plasmoid.configuration.panelTransparency
                    from: 0
                    to: 100
                    stepSize: 5
                    wheelEnabled: false

                    property bool blockOpacityAdjustment: (plasmoid.configuration.solidBackgroundForMaximized && plasmoid.configuration.backgroundOnlyOnMaximized)
                                                          || (solidBackground.checked
                                                              && !plasmoid.configuration.solidBackgroundForMaximized
                                                              && !plasmoid.configuration.backgroundOnlyOnMaximized)

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

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 2
                visible: dialog.expertLevel

                PlasmaComponents.Button {
                    id: panelBlur
                    Layout.fillWidth: true
                    text: i18n("Blur")
                    checked: plasmoid.configuration.blurEnabled
                    checkable: true
                    enabled: showBackground.checked

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

        //! BEGIN: Length
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            visible: dialog.expertLevel

            LatteExtraControls.Header {
                text: i18n("Length")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
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
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

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

        //! BEGIN: Dynamic Background
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            enabled: Latte.WindowSystem.compositingActive
            visible: dialog.expertLevel

            LatteExtraControls.Header {
                text: i18n("Dynamic Background")
            }

            PlasmaComponents.CheckBox {
                id: solidForMaximizedChk
                Layout.leftMargin: units.smallSpacing * 2
                Layout.maximumWidth: (dialog.appliedWidth - units.smallSpacing * 2) - 3*units.smallSpacing
                text: i18n("Force solid background for maximized or snapped windows")
                checked: plasmoid.configuration.solidBackgroundForMaximized
                tooltip: i18n("The panel background removes its transparency setting \n when there is a maximized or snapped window")
                style: LatteExtraControls.LatteCheckBoxStyle{}

                onClicked: {
                    plasmoid.configuration.solidBackgroundForMaximized = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: onlyOnMaximizedChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Hide background for not maximized windows")
                checked: plasmoid.configuration.backgroundOnlyOnMaximized
                tooltip: i18n("The panel background becomes transparent except if \nthere is a maximized or snapped window")

                onClicked: {
                    plasmoid.configuration.backgroundOnlyOnMaximized = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: colorizeTransparentPanelsChk
                Layout.leftMargin: units.smallSpacing * 2
                Layout.maximumWidth: (dialog.appliedWidth - units.smallSpacing * 2) - units.smallSpacing * 3
                text: i18n("Monochrome contents when panel is transparent")
                checked: plasmoid.configuration.colorizeTransparentPanels
                tooltip: i18n("The panel contents are colorized in order to improve contrast \nwith the underlying desktop background when the panel is transparent")
                style: LatteExtraControls.LatteCheckBoxStyle{}

                enabled: solidForMaximizedChk.checked || onlyOnMaximizedChk.checked

                onClicked: {
                    plasmoid.configuration.colorizeTransparentPanels = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: colorizeFromActiveWindowSchemeChk
                Layout.leftMargin: units.smallSpacing * 2
                Layout.maximumWidth: (dialog.appliedWidth - units.smallSpacing * 2) - units.smallSpacing * 3
                text: i18n("Paint contents based on active window scheme")
                checked: plasmoid.configuration.colorizeFromActiveWindowScheme
                tooltip: i18n("The panel contents are colorized from active window scheme \nwhen that window is maximized or is touching the panel.")
                style: LatteExtraControls.LatteCheckBoxStyle{}

                enabled: solidForMaximizedChk.checked || onlyOnMaximizedChk.checked

                onClicked: {
                    plasmoid.configuration.colorizeFromActiveWindowScheme = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: hideShadowsOnMaximizedChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Hide panel shadow for maximized windows")
                checked: plasmoid.configuration.disablePanelShadowForMaximized

                onClicked: {
                    plasmoid.configuration.disablePanelShadowForMaximized = checked;
                }
            }
        }
        //! END: Dynamic Background

        //! Bottom spacer
        PlasmaComponents.Label{
            id: bottomMarginSpacer
            text:" "
        }
    }
}
