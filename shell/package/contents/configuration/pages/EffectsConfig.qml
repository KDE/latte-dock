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
            spacing: units.smallSpacing

            LatteExtraControls.Header {
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
                        visible: dialog.highLevel

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
                            visible: dialog.highLevel

                            onClicked: {
                                viewConfig.setSticker(true);
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
                        visible: dialog.highLevel
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

        //! BEGIN: Glow
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            LatteExtraControls.Header {
                text: i18n("Indicators Glow")
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

                            readonly property int option: Latte.Types.GlowOnlyOnActive
                        }
                        PlasmaComponents.Button {
                            Layout.fillWidth: true
                            text: i18nc("Add glow to all task/applet indicators","All")
                            checked: parent.option === option
                            checkable: true
                            enabled: showGlowChk.checked
                            exclusiveGroup: glowGroup
                            tooltip: i18n("Add glow to all task/applet indicators")

                            readonly property int option: Latte.Types.GlowAll
                        }
                    }

                    RowLayout{
                        visible: dialog.highLevel

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

        //! BEGIN: Animations
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: units.smallSpacing
            spacing: units.smallSpacing

            LatteExtraControls.Header {
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
    }
}
