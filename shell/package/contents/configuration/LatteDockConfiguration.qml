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

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

import org.kde.plasma.plasmoid 2.0

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

import "pages" as Pages
import "../controls" as LatteExtraControls

FocusScope {
    id: dialog

    readonly property bool basicLevel: viewConfig.complexity === Latte.Types.BasicSettings
    readonly property bool advancedLevel: viewConfig.complexity === Latte.Types.AdvancedSettings
    readonly property bool expertLevel: viewConfig.complexity === Latte.Types.ExpertSettings

    readonly property bool highLevel: advancedLevel || expertLevel

    readonly property bool inConfigureAppletsMode: plasmoid.configuration.inConfigureAppletsMode || !Latte.WindowSystem.compositingActive

    //! max size based on screen resolution
    //!    TODO: if we can access availableScreenGeometry.height this can be improved, currently
    //!    we use 100px. or 50px. in order to give space for othe views to be shown and to have also
    //!    some space around the settings window
    property int maxHeight: plasmoid.formFactor === PlasmaCore.Types.Horizontal ?
                                viewConfig.availableScreenGeometry.height - (latteView.editThickness - latteView.normalThickness) - 16 :
                                viewConfig.availableScreenGeometry.height - 2 * units.largeSpacing
    property int maxWidth: 0.6 * latteView.screenGeometry.width

    //! propose size based on font size
    property int proposedWidth: 0.82 * proposedHeight + units.smallSpacing * 2
    property int proposedHeight: 34 * theme.mSize(theme.defaultFont).height

    //! chosen size to be applied, if the user has set or not a different scale for the settings window
    property int chosenWidth: userScaleWidth !== 1 ? userScaleWidth * proposedWidth : proposedWidth
    property int chosenHeight: userScaleHeight !== 1 ? userScaleHeight * heightLevel * proposedHeight : heightLevel * proposedHeight

    //! user set scales based on its preference, e.g. 96% of the proposed size
    property real userScaleWidth: 1
    property real userScaleHeight: 1

    readonly property real heightLevel: (dialog.expertLevel ? 100 : 1)

    onHeightChanged: viewConfig.syncGeometry();

    //! applied size in order to not be out of boundaries
    //! width can be between 200px - maxWidth
    //! height can be between 400px - maxHeight
    property int appliedWidth: Math.min(maxWidth, Math.max(200, chosenWidth))
    property int appliedHeight: Math.min(maxHeight, Math.max(400, chosenHeight))

    width: appliedWidth
    height: appliedHeight

    Layout.minimumWidth: width
    Layout.minimumHeight: height
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    readonly property bool viewIsPanel: latteView.type === Latte.Types.PanelView

    property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical
    property int subGroupSpacing: units.largeSpacing + units.smallSpacing * 1.5

    property color bC: theme.backgroundColor
    property color transparentBackgroundColor: Qt.rgba(bC.r, bC.g, bC.b, 0.7)

    onHighLevelChanged: {
        //! switch to appearancePage when effectsPage becomes hidden because
        //! advancedLevel was disabled by the user
        if (!highLevel && tabGroup.currentTab === effectsPage) {
            tabGroup.currentTab = appearancePage;
            tabBar.currentTab = appearanceTabBtn;
        }
    }

    Component.onCompleted: {
        updateScales();
    }

    Connections {
        target: latteView.positioner
        onCurrentScreenNameChanged: dialog.updateScales();
    }

    function updateScales() {
        userScaleWidth = universalSettings.screenWidthScale(latteView.positioner.currentScreenName);
        userScaleHeight = universalSettings.screenHeightScale(latteView.positioner.currentScreenName);
    }

    PlasmaCore.FrameSvgItem{
        anchors.fill: parent
        imagePath: "dialogs/background"
        enabledBorders: viewConfig.enabledBorders
    }

    MouseArea{
        id: backgroundMouseArea
        anchors.fill: parent
        hoverEnabled: true

        property bool blockWheel: false
        property bool updatingWidthScale: false
        property bool updatingHeightScale: false
        property bool wheelTriggeredOnce: false
        property real scaleStep: 0.04

        onContainsMouseChanged: {
            if (!containsMouse) {
                wheelTriggeredOnce = false;
            }
        }

        onWheel: {
            var metaModifier = (wheel.modifiers & Qt.MetaModifier);
            var ctrlModifier = (wheel.modifiers & Qt.ControlModifier);

            if (blockWheel || !(metaModifier || ctrlModifier)){
                return;
            }

            updatingWidthScale = metaModifier || (dialog.expertLevel && ctrlModifier);
            updatingHeightScale = !dialog.expertLevel && ctrlModifier;

            blockWheel = true;
            wheelTriggeredOnce = true;
            scrollDelayer.start();

            var angle = wheel.angleDelta.y / 8;

            //positive direction
            if (angle > 12) {
                var scales;
                if (updatingWidthScale) {
                    userScaleWidth = userScaleWidth + scaleStep;
                }

                if (updatingHeightScale) {
                    userScaleHeight = userScaleHeight + scaleStep;
                }

                universalSettings.setScreenScales(latteView.positioner.currentScreenName, userScaleWidth, userScaleHeight);
                viewConfig.syncGeometry();
                //negative direction
            } else if (angle < -12) {
                if (updatingWidthScale) {
                    userScaleWidth = userScaleWidth - scaleStep;
                }

                if (updatingHeightScale) {
                    userScaleHeight = userScaleHeight - scaleStep;
                }
                universalSettings.setScreenScales(latteView.positioner.currentScreenName, userScaleWidth, userScaleHeight);
                viewConfig.syncGeometry();
            }
        }
    }

    PlasmaComponents.Label{
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: backgroundMouseArea.updatingWidthScale ?
                  i18nc("view settings width scale","Width scale at %0%").arg(userScaleWidth * 100) :
                  i18nc("view settings height scale","Height scale at %0%").arg(userScaleHeight * 100)
        visible: backgroundMouseArea.containsMouse && backgroundMouseArea.wheelTriggeredOnce
    }

    //! A timer is needed in order to handle also touchpads that probably
    //! send too many signals very fast. This way the signals per sec are limited.
    //! The user needs to have a steady normal scroll in order to not
    //! notice a annoying delay
    Timer{
        id: scrollDelayer
        interval: 75
        onTriggered: backgroundMouseArea.blockWheel = false;
    }

    ColumnLayout {
        id: content

        Layout.minimumWidth: width
        Layout.minimumHeight: calculatedHeight
        Layout.preferredWidth: width
        Layout.preferredHeight: calculatedHeight
        width: (dialog.appliedWidth - units.smallSpacing * 2)

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        spacing: units.smallSpacing

        property int calculatedHeight: header.height + headerSpacer.height+ tabBar.height + pagesBackground.height + actionButtons.height + spacing * 3

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                viewConfig.hideConfigWindow();
            }
        }

        Component.onCompleted: forceActiveFocus();

        RowLayout {
            id: header
            Layout.fillWidth: true

            spacing: 0

            Item {
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: false
                Layout.topMargin: units.smallSpacing
                Layout.preferredWidth: width
                Layout.preferredHeight: height

                width: Qt.application.layoutDirection !== Qt.RightToLeft ? logo.width + latteTxt.width + units.smallSpacing : logo.width + units.smallSpacing
                height: logo.height

                Latte.IconItem {
                    id: logo

                    width: Math.round(1.4 * latteTxtMetrics.font.pixelSize)
                    height: width

                    smooth: true
                    source: "latte-dock"
                    // animated: true
                    usesPlasmaTheme: false
                    active: aboutArea.hovered
                }

                PlasmaComponents.Label {
                    id: latteTxtMetrics
                    text: "Latte"
                    width: 0
                    font.pointSize: 2 * theme.defaultFont.pointSize
                    visible: false
                }

                PlasmaCore.SvgItem{
                    id: latteTxt

                    width: 2.2 * height
                    height: 0.4 * latteTxtMetrics.font.pixelSize

                    visible: Qt.application.layoutDirection !== Qt.RightToLeft

                    anchors.left: logo.right
                    anchors.verticalCenter: logo.verticalCenter

                    svg: PlasmaCore.Svg{
                        imagePath: universalSettings.trademarkIconPath()
                    }
                }

                PlasmaComponents.Button{
                    id: aboutArea
                    //! Used as tooltip
                    anchors.fill: parent
                    opacity: 0
                    tooltip: i18n("Open Latte settings window")
                    onPressedChanged: {
                        if (pressed) {
                            layoutManager.showLatteSettingsDialog(Latte.Types.PreferencesPage)
                        }
                    }
                }

                Rectangle {
                    anchors.top: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 2
                    width: parent.width + 4
                    height: 2
                    color: theme.highlightColor
                    visible: aboutArea.hovered
                }
            }

            Item{
                id: headerSpacer
                Layout.minimumHeight: complexitySettings.height + 2*units.smallSpacing
            }

            ColumnLayout {
                PlasmaComponents.ToolButton {
                    id: pinButton

                    Layout.fillWidth: false
                    Layout.fillHeight: false
                    Layout.preferredWidth: width
                    Layout.preferredHeight: height
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Layout.bottomMargin: units.smallSpacing
                    //!avoid editMode box shadow
                    Layout.topMargin: units.smallSpacing * 2
                    Layout.rightMargin: units.smallSpacing

                    iconSource: "window-pin"
                    checkable: true

                    width: Math.round(units.gridUnit * 1.25)
                    height: width

                    property bool inStartup: true

                    onClicked: {
                        plasmoid.configuration.configurationSticker = checked
                        viewConfig.setSticker(checked)
                    }

                    Component.onCompleted: {
                        checked = plasmoid.configuration.configurationSticker
                        viewConfig.setSticker(plasmoid.configuration.configurationSticker)
                    }
                }

                RowLayout {
                    id: complexitySettings
                    Layout.fillWidth: true
                    Layout.rightMargin: units.smallSpacing * 2
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop

                    function switchMode() {
                        if (dialog.basicLevel || dialog.advancedLevel) {
                            viewConfig.complexity = Latte.Types.ExpertSettings;
                        } else if (dialog.expertLevel) {
                            viewConfig.complexity = Latte.Types.BasicSettings;
                        }
                    }

                    PlasmaComponents.Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight
                    }

                    PlasmaComponents.Label {
                        id: complexityLbl
                        Layout.alignment: Qt.AlignRight
                      //  opacity: dialog.basicLevel ? basicOpacity : 1

                        //! TODO: the term here is not accurate because the expert settings mode
                        //! is used currently. In the future this term will be rethought if
                        //! it must remain or be changed
                        text: i18nc("advanced settings", "Advanced")

                        readonly property real textColorBrightness: colorBrightness(theme.textColor)
                        readonly property real basicOpacity: textColorBrightness > 127 ? 0.7 : 0.3

                        color: {
                            if (dialog.basicLevel) {
                                return textColorBrightness > 127 ? Qt.darker(theme.textColor, 1.4) : Qt.lighter(theme.textColor, 2.8);
                            }

                            return theme.textColor;
                        }

                        function colorBrightness(color) {
                            return colorBrightnessFromRGB(color.r * 255, color.g * 255, color.b * 255);
                        }

                        // formula for brightness according to:
                        // https://www.w3.org/TR/AERT/#color-contrast
                        function colorBrightnessFromRGB(r, g, b) {
                            return (r * 299 + g * 587 + b * 114) / 1000
                        }

                        MouseArea {
                            id: complexityMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                complexitySettings.switchMode();
                            }
                        }
                    }

                    LatteComponents.Switch {
                        id: complexitySwitch
                        checked: dialog.expertLevel

                        onPressedChanged: {
                            if(pressed){
                                complexitySettings.switchMode();
                            }
                        }
                    }
                }
            }
        }

        PlasmaComponents.TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.maximumWidth: (dialog.appliedWidth - units.smallSpacing * 2)

            PlasmaComponents.TabButton {
                id: behaviorTabBtn
                text: i18n("Behavior")
                tab: behaviorPage
            }
            PlasmaComponents.TabButton {
                id: appearanceTabBtn
                text: i18n("Appearance")
                tab: appearancePage
            }
            PlasmaComponents.TabButton {
                id: effectsTabBtn
                text: i18n("Effects")
                tab: effectsPage
                visible: dialog.highLevel
            }
            PlasmaComponents.TabButton {
                id: tasksTabBtn
                text: i18n("Tasks")
                tab: tasksPage

                visible: latteView.latteTasksArePresent
            }
        }

        Rectangle {
            id: pagesBackground
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.minimumWidth: dialog.appliedWidth - units.smallSpacing * 4
            Layout.minimumHeight: height
            Layout.maximumHeight: height

            width: dialog.appliedWidth - units.smallSpacing * 3
            height: availableFreeHeight + units.smallSpacing * 4

            color: transparentBackgroundColor
            border.width: 1
            border.color: theme.backgroundColor

            //fix the height binding loop when showing the configuration window
            property int availableFreeHeight: dialog.appliedHeight - header.height - headerSpacer.height - tabBar.height - actionButtons.height - 2 * units.smallSpacing

            PlasmaExtras.ScrollArea {
                id: scrollArea

                anchors.fill: parent
                verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                flickableItem.flickableDirection: Flickable.VerticalFlick

                PlasmaComponents.TabGroup {
                    id: tabGroup

                    width: currentTab.Layout.maximumWidth
                    height: currentTab.Layout.maximumHeight

                    Pages.BehaviorConfig {
                        id: behaviorPage
                    }

                    Pages.AppearanceConfig {
                        id: appearancePage
                    }

                    Pages.EffectsConfig {
                        id: effectsPage
                    }

                    Pages.TasksConfig {
                        id: tasksPage
                    }
                }
            }
        }

        RowLayout {
            id: actionButtons
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            spacing: units.largeSpacing

            Connections{
                target: latteView.managedLayout
                onViewsCountChanged: actionButtons.updateEnabled();
            }

            function updateEnabled() {
                var screenFreeEdges = latteView.managedLayout.qmlFreeEdges(latteView.positioner.currentScreenId);
                actionsComboBtn.buttonEnabled = screenFreeEdges.length > 0;
                if (actionsModel.count > 0) {
                    actionsModel.get(0).enabled = actionsComboBtn.buttonEnabled;
                }
                removeView.enabled = latteView.managedLayout.viewsCount>1 /*&& !(latteView.managedLayout.viewsWithTasks()===1 && latteView.tasksPresent())*/
            }

            LatteComponents.ComboBoxButton {
                id: actionsComboBtn
                Layout.fillWidth: true
                implicitWidth: removeView.implicitWidth
                implicitHeight: removeView.implicitHeight

                buttonEnabled: true
                buttonText: i18n("New Dock")
                buttonIconSource: "list-add"
                buttonToolTip: i18n("Add a new dock")

                comboBoxEnabled: true
                comboBoxBlankSpaceForEmptyIcons: true
                comboBoxPopUpAlignRight: Qt.application.layoutDirection === Qt.RightToLeft
                comboBoxEnabledRole: "enabled"
                comboBoxTextRole: "name"
                comboBoxIconRole: "icon"
                comboBoxMinimumPopUpWidth: actionsModel.count > 1 ? dialog.width / 2 : 150

                property var activeLayoutsNames;

                Component.onCompleted: {
                    comboBox.model = actionsModel;
                    actionButtons.updateEnabled();
                }

                ListModel {
                    id: actionsModel
                }

                Connections{
                    target: actionsComboBtn.comboBox

                    Component.onCompleted:{
                        actionsComboBtn.addModel();
                        actionButtons.updateEnabled();
                    }

                    onActivated: {
                        if (index==0) {
                            latteView.copyView();
                        } else if (index>=1) {
                            latteView.positioner.hideDockDuringMovingToLayout(activeLayoutsNames[index-1]);
                        }

                        actionsComboBtn.comboBox.currentIndex = -1;
                    }

                    onEnabledChanged: {
                        if (enabled) {
                            actionsComboBtn.addModel();
                        } else {
                            actionsComboBtn.emptyModel();
                        }
                    }
                }

                Connections{
                    target: actionsComboBtn.button

                    onClicked: latteView.managedLayout.addNewView();
                }

                Connections{
                    target: latteView
                    onTypeChanged: actionsComboBtn.updateCopyText()
                }

                function addModel() {
                    actionsModel.clear();

                    var copy = {actionId: 'copy:', enabled: true, name: '', icon: 'edit-copy'};
                    actionsModel.append(copy);

                    updateCopyText();

                    var tempActiveLayouts = layoutManager.activeLayoutsNames();
                    var currentLayoutIndex = tempActiveLayouts.indexOf(latteView.managedLayout.name);

                    tempActiveLayouts.splice(currentLayoutIndex,1);

                    if (tempActiveLayouts.length > 0) {
                        activeLayoutsNames = tempActiveLayouts;
                        var iconArrow = Qt.application.layoutDirection === Qt.RightToLeft ? 'arrow-left' : 'arrow-right';

                        for(var i=0; i<activeLayoutsNames.length; ++i) {
                            var layout = {actionId: 'move:', name: i18n("Move to: %0").arg(activeLayoutsNames[i]), icon: iconArrow};
                            actionsModel.append(layout);
                        }
                    }

                    actionsComboBtn.comboBox.currentIndex = -1;
                }

                function emptyModel() {
                    actionsModel.clear();
                    var copy = {actionId: 'copy:', enabled: true, name: '', icon: 'edit-copy'};
                    actionsModel.append(copy);
                    updateCopyText();
                    actionsComboBtn.comboBox.currentIndex = -1;
                }

                function updateCopyText() {
                    var copyText = latteView.type === Latte.Types.DockView ? i18n("Copy Dock") : i18n("Copy Panel")
                    actionsModel.get(0).name = copyText;
                }
            }

            PlasmaComponents.Button {
                id: removeView
                Layout.fillWidth: true

                text: i18n("Remove")
                iconSource: "delete"
                opacity: latteView.managedLayout.viewsCount > 1 ? 1 : 0
                tooltip: i18n("Remove current dock")

                onClicked: latteView.removeView()
            }

            PlasmaComponents.Button {
                id: closeButton
                Layout.fillWidth: true

                text: i18n("Close")
                iconSource: "dialog-close"
                tooltip: i18n("Close settings window")

                onClicked: viewConfig.hideConfigWindow();
            }
        }
    }

    //! HACK FOR X11 environments
    //! show an inner shadow similar to Latte::View editShadow in order to
    //! not break the visual user experience
    LatteExtraControls.InnerShadow{
        width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? dialog.width + 2*shadowSize : shadowSize
        height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? shadowSize : dialog.height + 2*shadowSize

        shadowSize: latteView.effects.editShadow
        shadowOpacity: Math.max(0.35, maxOpacity)
        shadowDirection: plasmoid.location

        visible: !Latte.WindowSystem.isPlatformWayland && Latte.WindowSystem.compositingActive && latteView.effects.settingsMaskSubtracted

        readonly property real maxOpacity: Latte.WindowSystem.compositingActive && !plasmoid.configuration.inConfigureAppletsMode ?
                                               plasmoid.configuration.editBackgroundOpacity : 1
    }
}
