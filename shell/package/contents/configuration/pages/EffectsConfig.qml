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

import QtQuick.Controls.Styles.Plasma 2.0 as Styles

import org.kde.latte 0.2 as Latte

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

            LatteExtraControls.HeaderSwitch {
                id: showAppletShadow
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight

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


                RowLayout{
                    enabled: showAppletShadow.checked

                    PlasmaComponents.Label {
                        enabled: showAppletShadow.checked
                        text: i18n("Opacity")
                        horizontalAlignment: Text.AlignLeft
                    }

                    LatteExtraControls.Slider {
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

            LatteExtraControls.SubHeader {
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

            LatteExtraControls.Header {
                text: i18n("Animations")
            }

            LatteExtraControls.SubHeader {
                Layout.leftMargin: units.smallSpacing * 2
                isFirstSubCategory: true
                text: i18n("Duration")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
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
                    text: i18n("Instant")
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

            LatteExtraControls.HeaderSwitch {
                id: indicatorsSwitch
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight

                checked: plasmoid.configuration.indicatorsEnabled
                text: i18n("Indicators")
                tooltip: i18n("Enable/disable indicators")

                onPressed: {
                    plasmoid.configuration.indicatorsEnabled = !plasmoid.configuration.indicatorsEnabled;
                }
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing
                enabled: indicatorsSwitch.checked

                LatteExtraControls.SubHeader {
                    text: i18n("Style")
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    property int style: plasmoid.configuration.indicatorStyle

                    ExclusiveGroup {
                        id: indicatorStyleGroup
                        onCurrentChanged: {
                            if (current.checked) {
                                plasmoid.configuration.indicatorStyle = current.style
                            }
                        }
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18nc("latte indicator style", "Latte")
                        checked: parent.style === style
                        checkable: true
                        exclusiveGroup:  indicatorStyleGroup
                        tooltip: i18n("Use Latte style for your indicators")

                        readonly property int style: Latte.Types.LatteIndicator
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18nc("plasma indicator style", "Plasma")
                        checked: parent.style === style
                        checkable: true
                        exclusiveGroup:  indicatorStyleGroup
                        tooltip: i18n("Use Plasma style for your indicators")

                        readonly property int style: Latte.Types.PlasmaIndicator
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18nc("unity indicator style", "Unity")
                        checked: parent.style === style
                        checkable: true
                        exclusiveGroup:  indicatorStyleGroup
                        tooltip: i18n("Use Unity style for your indicators")

                        readonly property int style: Latte.Types.UnityIndicator
                    }
                }

                LatteExtraControls.SubHeader {
                    Layout.topMargin: units.smallSpacing * 3
                    isFirstSubCategory: true
                    text: i18n("Placement")
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    visible: dialog.expertLevel

                    PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18nc("reversed indicators", "Reverse")
                        checked: plasmoid.configuration.reverseLinesPosition
                        checkable: true
                        tooltip: i18n("Reverse indicators position e.g. from bottom to top")

                        onClicked: {
                            plasmoid.configuration.reverseLinesPosition = !plasmoid.configuration.reverseLinesPosition;
                        }
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18n("Applets")
                        checked: plasmoid.configuration.indicatorsForApplets
                        checkable: true
                        tooltip: i18n("Show or hide indicators for applets")

                        onClicked: {
                            plasmoid.configuration.indicatorsForApplets = !plasmoid.configuration.indicatorsForApplets;
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.topMargin: units.smallSpacing * 4
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator

                    Rectangle{
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 0.35
                        height: 1
                        color: theme.textColor
                        opacity: 0.3
                    }
                }

                LatteExtraControls.HeaderSwitch {
                    id: showGlow
                    Layout.fillWidth: true
                    Layout.minimumHeight: implicitHeight
                    Layout.topMargin: units.smallSpacing * 3
                    Layout.bottomMargin: units.smallSpacing
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator

                    checked: plasmoid.configuration.showGlow
                    level: 2
                    text: i18n("Glow")
                    tooltip: i18n("Enable/disable thickness margins")

                    onPressed: {
                        plasmoid.configuration.showGlow = !plasmoid.configuration.showGlow;
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator
                    enabled: plasmoid.configuration.showGlow

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
                        text: i18nc("glow only to active task/applet indicators","On Active")
                        checked: parent.option === option
                        checkable: true
                        exclusiveGroup:  glowGroup
                        tooltip: i18n("Add glow only to active task/applet indicator")

                        readonly property int option: Latte.Types.GlowOnlyOnActive
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true
                        text: i18nc("glow to all task/applet indicators","All")
                        checked: parent.option === option
                        checkable: true
                        exclusiveGroup: glowGroup
                        tooltip: i18n("Add glow to all task/applet indicators")

                        readonly property int option: Latte.Types.GlowAll
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: units.smallSpacing
                    spacing: 2
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator
                    enabled: plasmoid.configuration.showGlow

                    PlasmaComponents.Label {
                        Layout.minimumWidth: implicitWidth
                        horizontalAlignment: Text.AlignLeft
                        Layout.rightMargin: units.smallSpacing
                        text: i18n("Opacity")
                    }

                    LatteExtraControls.Slider {
                        id: glowOpacitySlider
                        Layout.fillWidth: true

                        leftPadding: 0
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
                        text: glowOpacitySlider.value + " %"
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                        Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
                    }
                }

                LatteExtraControls.SubHeader {
                    text: i18nc("active indicator style","Style For Active")
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator

                    property int indicatorType: plasmoid.configuration.activeIndicatorType

                    ExclusiveGroup {
                        id: activeIndicatorTypeGroup
                        onCurrentChanged: {
                            if (current.checked) {
                                plasmoid.configuration.activeIndicatorType = current.indicatorType;
                            }
                        }
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true

                        text: i18nc("line indicator","Line")
                        checked: parent.indicatorType === indicatorType
                        checkable: true
                        exclusiveGroup: activeIndicatorTypeGroup
                        tooltip: i18n("Show a line indicator for active items")

                        readonly property int indicatorType: Latte.Types.LineIndicator
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true

                        text: i18nc("dot indicator", "Dot")
                        checked: parent.indicatorType === indicatorType
                        checkable: true
                        exclusiveGroup: activeIndicatorTypeGroup
                        tooltip: i18n("Show a dot indicator for active items")

                        readonly property int indicatorType: Latte.Types.DotIndicator
                    }
                }

                LatteExtraControls.SubHeader {
                    enabled: plasmoid.configuration.glowOption!==Latte.Types.GlowNone
                    text: i18n("Improvements")
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator && latteView.latteTasksPresent()
                }

                PlasmaComponents.CheckBox {
                    id: threeColorsWindows
                    text: i18n("Different color for minimized windows")
                    checked: plasmoid.configuration.threeColorsWindows
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator && latteView.latteTasksPresent()

                    onClicked: {
                        plasmoid.configuration.threeColorsWindows = checked
                    }
                }

                PlasmaComponents.CheckBox {
                    id: dotsOnActive
                    text: i18n("Show an extra dot for grouped windows when active")
                    checked: plasmoid.configuration.dotsOnActive
                    tooltip: i18n("Grouped windows show both a line and a dot when one of them is active and the Line Active Indicator is enabled")
                    enabled: plasmoid.configuration.activeIndicatorType === Latte.Types.LineIndicator
                    visible: plasmoid.configuration.indicatorStyle === Latte.Types.LatteIndicator && latteView.latteTasksPresent()

                    onClicked: {
                        plasmoid.configuration.dotsOnActive = checked
                    }
                }
            }
        }
        //! END: Active Indicator
    }
}
