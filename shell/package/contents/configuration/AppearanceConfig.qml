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

import org.kde.latte 0.1 as Latte

import "../controls" as LatteExtraControls

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
            visible: plasmoid.configuration.advanced

            Header {
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
            Layout.topMargin: plasmoid.configuration.advanced ? 0 : units.smallSpacing
            spacing: units.smallSpacing

            Header {
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
                               && (plasmoid.configuration.advanced || (plasmoid.configuration.iconSize % 8 !== 0)))
                              || dialog.dockIsPanel ? 1 : 8
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
                    text: Number((zoomSlider.value * 100) - 100).toFixed(0) + "%"
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
                visible: plasmoid.configuration.advanced || plasmoid.configuration.proportionIconSize>0

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
                              proportionSizeSlider.value.toFixed(1) + "%" : "---%"
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
                visible: plasmoid.configuration.advanced

                PlasmaComponents.Label {
                    text: i18n("Applets Distance")
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
                    text: iconMarginSlider.value.toFixed(1) + "%"
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    enabled: iconMarginSlider.value > 0
                }
            }
        }
        //! END: Applet Size

        //! BEGIN: Background
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            enabled: Latte.WindowSystem.compositingActive

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
                visible: plasmoid.configuration.advanced

                PlasmaComponents.Label {
                    text: i18n("Opacity")
                    horizontalAlignment: Text.AlignLeft
                    enabled: transparencySlider.enabled
                }

                LatteExtraControls.Slider {
                    id: transparencySlider
                    Layout.fillWidth: true
                    //! transparency can be set when the user doesnt want solidness all the time
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

        //! BEGIN: Animations
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            enabled: Latte.WindowSystem.compositingActive

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

        //! BEGIN: Active Indicator
        ColumnLayout{
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            Header {
                text: i18n("Active Indicator")
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                rowSpacing: units.smallSpacing * 2
                columnSpacing: 1

                columns: 5

                property int indicatorType: plasmoid.configuration.activeIndicatorType
                property int activeIndicator: plasmoid.configuration.activeIndicator

                ExclusiveGroup {
                    id: activeIndicatorTypeGroup
                    onCurrentChanged: {
                        if (current.checked)
                            plasmoid.configuration.activeIndicatorType = current.indicatorType;
                    }
                }

                ExclusiveGroup {
                    id: activeIndicatorGroup
                    onCurrentChanged: {
                        if (current.checked)
                            plasmoid.configuration.activeIndicator = current.activeIndicator
                    }
                }

                PlasmaComponents.Label {
                    text: i18nc("active indicator style","Style")
                    horizontalAlignment: Text.AlignLeft
                    visible: plasmoid.configuration.advanced
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true

                    text: i18nc("line indicator","Line")
                    checked: parent.indicatorType === indicatorType
                    checkable: true
                    exclusiveGroup: activeIndicatorTypeGroup
                    tooltip: i18n("Show a line indicator for active tasks/applets")

                    readonly property int indicatorType: Latte.Dock.LineIndicator
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true

                    text: i18nc("dot indicator", "Dot")
                    checked: parent.indicatorType === indicatorType
                    checkable: true
                    exclusiveGroup: activeIndicatorTypeGroup
                    tooltip: i18n("Show a dot indicator for active tasks/applets")

                    readonly property int indicatorType: Latte.Dock.DotIndicator
                }

                PlasmaComponents.Label{
                    text:"  |  "
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true

                    text: i18nc("reverse the position of the active indicator e.g. from bottom to top", "Reverse")
                    checked: plasmoid.configuration.reverseLinesPosition
                    checkable: true
                    tooltip: i18n("Reverse the position of the active indicator e.g. from bottom to top")

                    onClicked: {
                        plasmoid.configuration.reverseLinesPosition = checked;
                    }
                }

                PlasmaComponents.Label {
                    text: i18n("Applets") + " "
                    horizontalAlignment: Text.AlignLeft
                    visible: plasmoid.configuration.advanced
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true

                    text: i18nc("active indicator to no applets", "None")
                    checked: parent.activeIndicator === activeIndicator
                    checkable: true
                    exclusiveGroup: activeIndicatorGroup
                    visible: plasmoid.configuration.advanced
                    tooltip: i18n("Latte will not show any active applet indicator on its own\n except those the plasma theme provides")

                    readonly property int activeIndicator: Latte.Dock.NoneIndicator
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true

                    text: i18nc("active indicator only to in-house latte applets", "Internals")
                    checked: parent.activeIndicator === activeIndicator
                    checkable: true
                    exclusiveGroup: activeIndicatorGroup
                    visible: plasmoid.configuration.advanced
                    tooltip: i18n("Latte will show active applet indicators only for applets that have been adjusted\n by it for hovering capabilities e.g. folderview")

                    readonly property int activeIndicator: Latte.Dock.InternalsIndicator
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2

                    text: i18nc("active indicator to all applets", "All")
                    checked: parent.activeIndicator === activeIndicator
                    checkable: true
                    exclusiveGroup: activeIndicatorGroup
                    visible: plasmoid.configuration.advanced
                    tooltip: i18n("Latte will show active applet indicators for all applets")

                    readonly property int activeIndicator: Latte.Dock.AllIndicator
                }
            }
        }
        //! END: Active Indicator

        //! BEGIN: Glow
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            visible: plasmoid.configuration.advanced

            Header {
                text: i18n("Glow")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 2

                ColumnLayout{
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        PlasmaComponents.CheckBox {
                            id: showGlowChk
                            text: i18nc("show glow","Show")
                            checked: plasmoid.configuration.showGlow

                            onClicked: {
                                plasmoid.configuration.showGlow = checked
                            }
                        }

                        PlasmaComponents.Label {
                            text: " | "
                            horizontalAlignment: Text.AlignLeft
                            opacity: 0.35
                            enabled: showGlowChk.checked
                        }

                        property int option: plasmoid.configuration.glowOption

                        ExclusiveGroup {
                            id: glowGroup
                            onCurrentChanged: {
                                if (current.checked)
                                    plasmoid.configuration.glowOption = current.option
                            }
                        }

                        PlasmaComponents.Button {
                            Layout.fillWidth: true
                            text: i18nc("add glow only to active task/applet indicators","Only On Active")
                            checked: parent.option === option
                            checkable: true
                            enabled: showGlowChk.checked
                            exclusiveGroup:  glowGroup
                            tooltip: i18n("Add glow only to active task/applet indicator")

                            readonly property int option: Latte.Dock.GlowOnlyOnActive
                        }
                        PlasmaComponents.Button {
                            Layout.fillWidth: true
                            text: i18nc("Add glow to all task/applet indicators","All")
                            checked: parent.option === option
                            checkable: true
                            enabled: showGlowChk.checked
                            exclusiveGroup: glowGroup
                            tooltip: i18n("Add glow to all task/applet indicators")

                            readonly property int option: Latte.Dock.GlowAll
                        }
                    }

                    RowLayout{
                        PlasmaComponents.Label {
                            enabled: showGlowChk.checked
                            text: i18n("Opacity")
                            horizontalAlignment: Text.AlignLeft
                        }

                        LatteExtraControls.Slider {
                            id: glowOpacitySlider
                            Layout.fillWidth: true
                            enabled: showGlowChk.checked

                            value: plasmoid.configuration.glowOpacity
                            from: 0
                            to: 100
                            stepSize: 5
                            wheelEnabled: false

                            function updateGlowOpacity() {
                                if (!pressed)
                                    plasmoid.configuration.glowOpacity = value;
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
                            enabled: showGlowChk.checked
                            text: glowOpacitySlider.value + " %"
                            horizontalAlignment: Text.AlignRight
                            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                        }

                        PlasmaComponents.Label {
                            text: " | "
                            horizontalAlignment: Text.AlignLeft
                            enabled: showGlowChk.checked
                            opacity: 0.35
                        }

                        PlasmaComponents.CheckBox {
                            id: showGlow3D
                            Layout.leftMargin: units.smallSpacing * 2
                            text: " " + i18n("3D")
                            checked: plasmoid.configuration.glow3D
                            enabled: showGlowChk.checked
                            tooltip: i18n("Use a 3D style glow")

                            onClicked: {
                                plasmoid.configuration.glow3D = checked;
                            }
                        }
                    }
                }
            }
        }
        //! END: Glow

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

                        PlasmaComponents3.ComboBox {
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
                            text: i18n("Opacity")
                            horizontalAlignment: Text.AlignLeft
                        }

                        LatteExtraControls.Slider {
                            id: shadowOpacitySlider
                            Layout.fillWidth: true
                            enabled: showAppletShadow.checked

                            value: plasmoid.configuration.shadowOpacity
                            from: 0
                            to: 100
                            stepSize: 5
                            wheelEnabled: false

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
                            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
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
                            text: i18n("Size")
                            horizontalAlignment: Text.AlignLeft
                        }

                        LatteExtraControls.Slider {
                            id: shadowSizeSlider
                            Layout.fillWidth: true
                            enabled: showAppletShadow.checked

                            value: plasmoid.configuration.shadowSize
                            from: 0
                            to: 100
                            stepSize: 5
                            wheelEnabled: false

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
                            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
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
                    from: ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                           || (plasmoid.configuration.panelPosition === Latte.Dock.Justify)) ? -20 :  0
                    to: ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                         || (plasmoid.configuration.panelPosition === Latte.Dock.Justify)) ? 20 :  40
                    stepSize: 2
                    wheelEnabled: false

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
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
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
