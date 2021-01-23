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

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents
import org.kde.latte.private.containment 0.1 as LatteContainment

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

            spacing: units.smallSpacing

            LatteComponents.HeaderSwitch {
                id: showAppletShadow
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight
                Layout.topMargin: units.smallSpacing

                checked: plasmoid.configuration.appletShadowsEnabled
                text: i18n("Shadows")
                tooltip: i18n("Enable/disable applet shadows")

                onPressed: plasmoid.configuration.appletShadowsEnabled = !plasmoid.configuration.appletShadowsEnabled;
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0

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
                        text: i18nc("number in percentage, e.g. 85 %","%0 %").arg(shadowSizeSlider.value)
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
                        text: i18nc("number in percentage, e.g. 85 %","%0 %").arg(shadowOpacitySlider.value)
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }

                LatteComponents.SubHeader {
                    isFirstSubCategory: true
                    text: i18n("Color")
                }

                RowLayout {
                    id: shadowColorRow
                    Layout.fillWidth: true
                    spacing: 2
                    enabled: showAppletShadow.checked

                    readonly property string defaultShadow: "080808"
                    readonly property string themeShadow: {
                        var strC = String(theme.textColor);

                        return strC.indexOf("#") === 0 ? strC.substr(1) : strC;
                    }

                    ExclusiveGroup {
                        id: shadowColorGroup
                    }

                    PlasmaComponents.Button {
                        id: defaultShadowBtn
                        Layout.fillWidth: true

                        text: i18nc("default shadow", "Default")
                        checked: plasmoid.configuration.shadowColorType === type
                        checkable: false
                        exclusiveGroup: shadowColorGroup
                        tooltip: i18n("Default shadow for applets")

                        readonly property int type: LatteContainment.Types.DefaultColorShadow

                        onPressedChanged: {
                            if (pressed) {
                                plasmoid.configuration.shadowColorType = type;
                            }
                        }
                    }

                    PlasmaComponents.Button {
                        id: themeShadowBtn
                        Layout.fillWidth: true

                        text: i18nc("theme shadow", "Theme")
                        checked: plasmoid.configuration.shadowColorType === type
                        checkable: false
                        exclusiveGroup: shadowColorGroup
                        tooltip: i18n("Shadow from theme color palette")

                        readonly property int type: LatteContainment.Types.ThemeColorShadow

                        onPressedChanged: {
                            if (pressed) {
                                plasmoid.configuration.shadowColorType = type;
                            }
                        }
                    }

                    //overlayed button
                    PlasmaComponents.Button {
                        id: userShadowBtn
                        Layout.fillWidth: true
                        height: parent.height
                        text: " "

                        checkable: false
                        checked: plasmoid.configuration.shadowColorType === type
                        tooltip: i18n("Use set shadow color")
                        exclusiveGroup: shadowColorGroup

                        readonly property int type: LatteContainment.Types.UserColorShadow

                        onPressedChanged: {
                            if (pressed) {
                                plasmoid.configuration.shadowColorType = type;
                            }
                        }

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
        }
        //! END: Shadows

        //! BEGIN: Animations
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: units.smallSpacing
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
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0
                enabled: plasmoid.configuration.animationsEnabled

                LatteComponents.SubHeader {
                    isFirstSubCategory: true
                    text: i18n("Speed")
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        property int duration: plasmoid.configuration.durationTime

                        ExclusiveGroup {
                            id: animationsGroup
                        }

                        PlasmaComponents.Button {
                            Layout.fillWidth: true
                            text: i18n("x1")
                            checked: parent.duration === duration
                            checkable: false
                            exclusiveGroup: animationsGroup

                            readonly property int duration: 3

                            onPressedChanged: {
                                if (pressed) {
                                    plasmoid.configuration.durationTime = duration;
                                }
                            }
                        }
                        PlasmaComponents.Button {
                            Layout.fillWidth: true
                            text: i18n("x2")
                            checked: parent.duration === duration
                            checkable: false
                            exclusiveGroup: animationsGroup

                            readonly property int duration: 2

                            onPressedChanged: {
                                if (pressed) {
                                    plasmoid.configuration.durationTime = duration;
                                }
                            }
                        }
                        PlasmaComponents.Button {
                            Layout.fillWidth: true
                            text: i18n("x3")
                            checked: parent.duration === duration
                            checkable: false
                            exclusiveGroup: animationsGroup

                            readonly property int duration: 1

                            onPressedChanged: {
                                if (pressed) {
                                    plasmoid.configuration.durationTime = duration;
                                }
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
                Layout.rightMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                enabled: indicatorsSwitch.checked

             /*   LatteComponents.SubHeader {
                    text: i18n("Style")
                }*/

                Item {
                    Layout.fillWidth: true
                    Layout.minimumHeight: tabBar.height

                    PlasmaComponents.TabBar {
                        id: tabBar
                        width: parent.width

                        property string type: latteView.indicator.type

                        PlasmaComponents.TabButton {
                            id: latteBtn
                            text: i18nc("latte indicator style", "Latte")
                            readonly property string type: "org.kde.latte.default"

                            onCheckedChanged: {
                                if (checked) {
                                    latteView.indicator.type = type;
                                }
                            }
                        }
                        PlasmaComponents.TabButton {
                            id: plasmaBtn
                            text: i18nc("plasma indicator style", "Plasma")
                            readonly property string type: "org.kde.latte.plasma"

                            onCheckedChanged: {
                                if (checked) {
                                    latteView.indicator.type = type;
                                }
                            }
                        }

                        PlasmaComponents.TabButton {
                            id: customBtn

                            onCheckedChanged: {
                                if (checked) {
                                    customIndicator.onButtonIsPressed();
                                }
                            }

                            LatteExtraControls.CustomIndicatorButton {
                                id: customIndicator
                                anchors.fill: parent
                                implicitWidth: latteBtn.implicitWidth
                                implicitHeight: latteBtn.implicitHeight

                                checked: parent.checked
                                comboBoxMinimumPopUpWidth: 1.5 * customIndicator.width
                            }
                        }
                    }

                    Rectangle {
                        anchors.bottom: tabBar.bottom
                        anchors.left: tabBar.left
                        anchors.leftMargin: 2
                        width: tabBar.width - 2*2
                        height: 2
                        color: theme.textColor
                        opacity: 0.25
                    }
                }

                //! BEGIN: Indicator specific sub-options
                ColumnLayout {
                    id: indicatorSpecificOptions
                    Layout.fillWidth: true
                    Layout.topMargin: units.smallSpacing * 2
                    spacing: 0
                    enabled: latteView.indicator.enabled

                    // @since 0.10.0
                    readonly property bool deprecatedOptionsAreHidden: true

                    readonly property int optionsWidth: dialog.optionsWidth

                    Component.onCompleted: {
                        viewConfig.indicatorUiManager.setParentItem(indicatorSpecificOptions);
                        viewConfig.indicatorUiManager.ui(latteView.indicator.type, latteView);
                    }

                    Connections {
                        target: latteView.indicator
                        onPluginChanged: viewConfig.indicatorUiManager.ui(latteView.indicator.type, latteView);
                    }

                    Connections {
                        target: viewConfig
                        onIsReadyChanged: {
                            if (viewConfig.isReady) {
                                viewConfig.indicatorUiManager.ui(latteView.indicator.type, latteView);
                            }
                        }
                    }
                }
                //! END: Indicator specific sub-options
            }
        }
        //! END: Active Indicator General Settings        
    }
}
