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
import QtQuick.Dialogs 1.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

import "../../controls" as LatteExtraControls

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    ColumnLayout {
        id: content

        width: (dialog.appliedWidth - units.smallSpacing * 2) - Layout.leftMargin * 2
        spacing: dialog.subGroupSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.leftMargin: units.smallSpacing * 2

        //! BEGIN: Shadows
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            spacing: units.smallSpacing

            LatteComponents.HeaderSwitch {
                id: showAppletShadow
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight
                Layout.topMargin: units.smallSpacing

                checked: plasmoid.configuration.shadows !== Latte.Types.NoneShadow
                text: i18n("Shadows")
                tooltip: i18n("Enable/disable applet shadows")

                onPressed: {
                    if(plasmoid.configuration.shadows !== Latte.Types.AllAppletsShadow){
                        plasmoid.configuration.shadows = Latte.Types.AllAppletsShadow;
                    } else {
                        plasmoid.configuration.shadows = Latte.Types.NoneShadow;
                    }
                }
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2

                RowLayout{
                    enabled: showAppletShadow.checked

                    PlasmaComponents.Label {
                        enabled: showAppletShadow.checked
                        text: i18n("Size")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
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


                RowLayout{
                    enabled: showAppletShadow.checked

                    PlasmaComponents.Label {
                        enabled: showAppletShadow.checked
                        text: i18n("Opacity")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: shadowOpacitySlider
                        Layout.fillWidth: true

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
            }

            LatteComponents.SubHeader {
                isFirstSubCategory: true
                text: i18n("Color")
            }

            RowLayout {
                id: shadowColorRow
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: 2
                enabled: showAppletShadow.checked

                readonly property string defaultShadow: "080808"
                readonly property string themeShadow: {
                    var strC = String(theme.textColor);

                    return strC.indexOf("#") === 0 ? strC.substr(1) : strC;
                }

                ExclusiveGroup {
                    id: shadowColorGroup

                    property bool inStartup: true

                    onCurrentChanged: {
                        if (inStartup) {
                            return;
                        }

                        if (current === defaultShadowBtn) {
                            plasmoid.configuration.shadowColorType = Latte.Types.DefaultColorShadow;
                        } else if (current === themeShadowBtn) {
                            plasmoid.configuration.shadowColorType = Latte.Types.ThemeColorShadow;
                        } else if (current === userShadowBtn) {
                            plasmoid.configuration.shadowColorType = Latte.Types.UserColorShadow;
                        }
                    }

                    Component.onCompleted: inStartup = false;
                }

                PlasmaComponents.Button {
                    id: defaultShadowBtn
                    Layout.fillWidth: true

                    text: i18nc("default shadow", "Default")
                    checked: plasmoid.configuration.shadowColorType === Latte.Types.DefaultColorShadow
                    checkable: true
                    exclusiveGroup: shadowColorGroup
                    tooltip: i18n("Default shadow for applets")
                }

                PlasmaComponents.Button {
                    id: themeShadowBtn
                    Layout.fillWidth: true

                    text: i18nc("theme shadow", "Theme")
                    checked: plasmoid.configuration.shadowColorType === Latte.Types.ThemeColorShadow
                    checkable: true
                    exclusiveGroup: shadowColorGroup
                    tooltip: i18n("Shadow from theme color palette")
                }

                //overlayed button
                PlasmaComponents.Button {
                    id: userShadowBtn
                    Layout.fillWidth: true
                    height: parent.height
                    text: " "

                    checkable: true
                    checked: plasmoid.configuration.shadowColorType === Latte.Types.UserColorShadow
                    tooltip: i18n("Use set shadow color")
                    exclusiveGroup: shadowColorGroup

                    Rectangle{
                        anchors.fill: parent
                        anchors.margins: 1.5*units.smallSpacing

                        color: "#" + plasmoid.configuration.shadowColor;

                        opacity: shadowColorRow.enabled ? 1 : 0.6

                        Rectangle{
                            anchors.fill: parent
                            color: "transparent"
                            border.width: 1
                            border.color: theme.textColor
                            opacity: parent.opacity - 0.4
                        }

                        MouseArea{
                            anchors.fill: parent
                            onClicked: {
                                shadowColorGroup.current = userShadowBtn;
                                viewConfig.setSticker(true);
                                colorDialogLoader.showDialog = true;
                            }
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
                                if (strC.indexOf("#") === 0) {
                                    plasmoid.configuration.shadowColor = strC.substr(1);
                                }

                                colorDialogLoader.showDialog = false;
                                viewConfig.setSticker(false);
                            }
                            onRejected: {
                                colorDialogLoader.showDialog = false;
                                viewConfig.setSticker(false);
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
        //! END: Shadows

        //! BEGIN: Animations
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            spacing: units.smallSpacing

            LatteComponents.HeaderSwitch {
                id: animationsHeader
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight
                Layout.topMargin: units.smallSpacing

                checked: plasmoid.configuration.animationsEnabled
                text: i18n("Animations")
                tooltip: i18n("Enable/disable all animations")

                onPressed: {
                    plasmoid.configuration.animationsEnabled = !plasmoid.configuration.animationsEnabled;
                }
            }

            ColumnLayout {
                spacing: 0
                enabled: plasmoid.configuration.animationsEnabled

                LatteComponents.SubHeader {
                    Layout.leftMargin: units.smallSpacing * 2
                    isFirstSubCategory: true
                    text: i18n("Speed")
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: units.smallSpacing * 2
                    spacing: 0

                    RowLayout {
                        Layout.fillWidth: true
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
                            text: i18n("x1")
                            checked: parent.duration === duration
                            checkable: true
                            exclusiveGroup: animationsGroup

                            readonly property int duration: 3
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

                            readonly property int duration: 1
                        }
                    }

                    ColumnLayout {
                        spacing: units.smallSpacing
                        visible: latteView.latteTasksArePresent

                        LatteComponents.SubHeader {
                            Layout.leftMargin: units.smallSpacing * 2
                            text: i18n("Tasks")
                        }

                        PlasmaComponents.CheckBox {
                            text: i18n("Bounce launchers when triggered")
                            checked: plasmoid.configuration.animationLauncherBouncing

                            onClicked: {
                                plasmoid.configuration.animationLauncherBouncing = !plasmoid.configuration.animationLauncherBouncing;
                            }
                        }

                        PlasmaComponents.CheckBox {
                            text: i18n("Bounce tasks that need attention")
                            checked: plasmoid.configuration.animationWindowInAttention

                            onClicked: {
                                plasmoid.configuration.animationWindowInAttention = !plasmoid.configuration.animationWindowInAttention;
                            }
                        }

                        PlasmaComponents.CheckBox {
                            text: i18n("Slide in and out single windows")
                            checked: plasmoid.configuration.animationNewWindowSliding

                            onClicked: {
                                plasmoid.configuration.animationNewWindowSliding = !plasmoid.configuration.animationNewWindowSliding;
                            }
                        }

                        PlasmaComponents.CheckBox {
                            text: i18n("Grouped tasks bounce their new windows")
                            checked: plasmoid.configuration.animationWindowAddedInGroup

                            onClicked: {
                                plasmoid.configuration.animationWindowAddedInGroup = !plasmoid.configuration.animationWindowAddedInGroup;
                            }
                        }

                        PlasmaComponents.CheckBox {
                            text: i18n("Grouped tasks slide out their closed windows")
                            checked: plasmoid.configuration.animationWindowRemovedFromGroup

                            onClicked: {
                                plasmoid.configuration.animationWindowRemovedFromGroup = !plasmoid.configuration.animationWindowRemovedFromGroup;
                            }
                        }
                    }
                }
            }
        }
        //! END: Animations

        //! BEGIN: Active Indicator General Settings
        ColumnLayout{
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            LatteComponents.HeaderSwitch {
                id: indicatorsSwitch
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight

                checked: latteView.indicator.enabled
                text: i18n("Indicators")
                tooltip: i18n("Enable/disable indicators")

                onPressed: {
                    latteView.indicator.enabled = !latteView.indicator.enabled;
                }
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                enabled: indicatorsSwitch.checked

                LatteComponents.SubHeader {
                    text: i18n("Style")
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    property string type: latteView.indicator.type

                    ExclusiveGroup {
                        id: indicatorStyleGroup
                        onCurrentChanged: {
                            if (current.checked) {
                                latteView.indicator.type = current.type
                            }
                        }
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18nc("latte indicator style", "Latte")
                        checked: parent.type === type
                        checkable: true
                        exclusiveGroup:  indicatorStyleGroup
                        tooltip: i18n("Use Latte style for your indicators")

                        readonly property string type: "org.kde.latte.default"
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18nc("plasma indicator style", "Plasma")
                        checked: parent.type === type
                        checkable: true
                        exclusiveGroup:  indicatorStyleGroup
                        tooltip: i18n("Use Plasma style for your indicators")

                        readonly property string type: "org.kde.latte.plasma"
                    }

                    LatteExtraControls.CustomIndicatorButton {
                        Layout.fillWidth: true
                        checked: parent.type === type
                        checkable: true
                        buttonExclusiveGroup:  indicatorStyleGroup
                        buttonToolTip: i18n("Use %0 style for your indicators and download more custom indicators to use").arg(buttonText)
                    }


                    /*PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18nc("unity indicator style", "Unity")
                        checked: parent.type === type
                        checkable: true
                        exclusiveGroup:  indicatorStyleGroup
                        tooltip: i18n("Use Unity style for your indicators")

                        readonly property string type: "org.kde.latte.unity"
                    }*/
                }

                LatteComponents.SubHeader {
                    Layout.topMargin: units.smallSpacing
                    isFirstSubCategory: true
                    text: i18n("Paddings")
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: units.smallSpacing

                    PlasmaComponents.Label {
                        text: i18n("Internal")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteComponents.Slider {
                        id: lengthIntMarginSlider
                        Layout.fillWidth: true

                        value: Math.round(latteView.indicator.padding * 100)
                        from: 0
                        to: maxMargin
                        stepSize: 1
                        wheelEnabled: false

                        readonly property int maxMargin: 25

                        onPressedChanged: {
                            if (!pressed) {
                                latteView.indicator.padding = value / 100;
                            }
                        }
                    }

                    PlasmaComponents.Label {
                        text: lengthIntMarginSlider.value + " %"
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }

                LatteComponents.SubHeader {
                    Layout.topMargin: units.smallSpacing
                    isFirstSubCategory: true
                    text: i18n("Options")
                }

                PlasmaComponents.CheckBox {
                    text: i18n("Show indicators for applets")
                    checked: latteView.indicator.enabledForApplets
                    tooltip: i18n("Indicators are shown for applets")

                    onClicked: {
                        latteView.indicator.enabledForApplets = !latteView.indicator.enabledForApplets;
                    }
                }

                PlasmaComponents.CheckBox {
                    text: i18n("Reverse indicator style")
                    checked: latteView.indicator.reversed
                    tooltip: i18n("Reverse indicator style e.g. from bottom to top")

                    onClicked: {
                        latteView.indicator.reversed = !latteView.indicator.reversed;
                    }
                }
            }
        }
        //! END: Active Indicator General Settings

        //! BEGIN: Indicator specific sub-options
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: units.smallSpacing
            Layout.rightMargin: units.smallSpacing
            spacing: units.smallSpacing
            visible: latteView.indicator.providesConfigUi
            enabled: latteView.indicator.enabled

            LatteComponents.Header {
                text: i18n("%0 Indicator Options").arg(indicatorStyleGroup.current.text)
            }

            ColumnLayout {
                id: indicatorSpecificOptions
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: 0

                Component.onCompleted: {
                    latteView.indicator.configUiFor(latteView.indicator.type, indicatorSpecificOptions);
                }

                Connections {
                    target: latteView.indicator
                    onPluginChanged: latteView.indicator.configUiFor(latteView.indicator.type, indicatorSpecificOptions);
                }
            }
        }
        //! END: Indicator specific sub-options
    }
}
