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

import org.kde.latte 0.1 as Latte
import "../controls" as LatteExtraControls

FocusScope {
    id: dialog

    property int maxWidth: 34 * theme.defaultFont.pixelSize
    width: maxWidth + units.smallSpacing * 2
    height: Math.min(content.height + units.smallSpacing * 2,
                     dock.screenGeometry.height - dock.normalThickness - 2*units.largeSpacing)
    //! for small resolutions we better limit the size of the settings in order to be shown properly

    Layout.minimumWidth: width
    Layout.minimumHeight: height
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    readonly property bool dockIsPanel: behaviorPage.dockTypeSelection.isPanel

    property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical
    property int subGroupSpacing: units.largeSpacing + units.smallSpacing * 1.5

    property color bC: theme.backgroundColor
    property color transparentBackgroundColor: Qt.rgba(bC.r, bC.g, bC.b, 0.7)

    PlasmaCore.FrameSvgItem{
        anchors.fill: parent
        imagePath: "dialogs/background"
        enabledBorders: dockConfig.enabledBorders
    }

    ColumnLayout {
        id: content

        Layout.minimumWidth: width
        Layout.minimumHeight: calculatedHeight
        Layout.preferredWidth: width
        Layout.preferredHeight: calculatedHeight
        width: dialog.maxWidth

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        spacing: units.smallSpacing

        property int calculatedHeight: header.height + headerSpacer.height+ tabBar.height + pagesBackground.height + actionButtons.height + spacing * 3

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                dockConfig.hideConfigWindow();
            } else if (event.key === Qt.Key_Left) {
                //
                if (tabGroup.currentTab === behaviorPage) {
                    if (plasmoid.configuration.advanced) {
                        tabGroup.currentTab = tweaksPage;
                        tabBar.currentTab = tweaksTabBtn;
                    } else if (tasksTabBtn.visible) {
                        tabGroup.currentTab = tasksPage;
                        tabBar.currentTab = tasksTabBtn;
                    } else {
                        tabGroup.currentTab = appearancePage;
                        tabBar.currentTab = appearanceTabBtn;
                    }
                } else if (tabGroup.currentTab === tweaksPage) {
                    if (tasksTabBtn.visible) {
                        tabGroup.currentTab = tasksPage;
                        tabBar.currentTab = tasksTabBtn;
                    } else {
                        tabGroup.currentTab = appearancePage;
                        tabBar.currentTab = appearanceTabBtn;
                    }
                } else if (tabGroup.currentTab === tasksPage) {
                    tabGroup.currentTab = appearancePage;
                    tabBar.currentTab = appearanceTabBtn;
                } else if (tabGroup.currentTab === appearancePage) {
                    tabGroup.currentTab = behaviorPage;
                    tabBar.currentTab = behaviorTabBtn;
                }
                //
            } else if (event.key === Qt.Key_Right) {
                //
                if (tabGroup.currentTab === behaviorPage) {
                    tabGroup.currentTab = appearancePage;
                    tabBar.currentTab = appearanceTabBtn;
                } else if (tabGroup.currentTab === appearancePage) {
                    if (tasksTabBtn.visible) {
                        tabGroup.currentTab = tasksPage;
                        tabBar.currentTab = tasksTabBtn;
                    } else if (plasmoid.configuration.advanced) {
                        tabGroup.currentTab = tweaksPage;
                        tabBar.currentTab = tweaksTabBtn;
                    } else {
                        tabGroup.currentTab = behaviorPage;
                        tabBar.currentTab = behaviorTabBtn;
                    }
                } else if (tabGroup.currentTab === tasksPage) {
                    if (plasmoid.configuration.advanced) {
                        tabGroup.currentTab = tweaksPage;
                        tabBar.currentTab = tweaksTabBtn;
                    } else {
                        tabGroup.currentTab = behaviorPage;
                        tabBar.currentTab = behaviorTabBtn;
                    }
                } else if (tabGroup.currentTab === tweaksPage) {
                    tabGroup.currentTab = behaviorPage;
                    tabBar.currentTab = behaviorTabBtn;
                }
                //
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

                LatteExtraControls.ToolTip{
                    parent: logo
                    text: i18n("Open Latte settings window")
                    visible: aboutMouseArea.containsMouse
                    delay: 7 * units.longDuration
                }

                Latte.IconItem {
                    id: logo

                    width: Math.round(1.4 * latteTxtMetrics.font.pixelSize)
                    height: width

                    smooth: true
                    source: "latte-dock"
                    // animated: true
                    usesPlasmaTheme: false
                    active: aboutMouseArea.containsMouse
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

                MouseArea {
                    id: aboutMouseArea
                    acceptedButtons: Qt.LeftButton
                    anchors.fill: parent
                    hoverEnabled: true

                    readonly property int preferencesPage: Latte.Dock.PreferencesPage
                    onClicked: layoutManager.showLatteSettingsDialog(preferencesPage)
                }
            }

            Item{
                id: headerSpacer
                Layout.minimumHeight: advancedSettings.height + 2*units.smallSpacing
            }

            ColumnLayout {
                PlasmaComponents.ToolButton {
                    id: pinButton

                    Layout.fillWidth: false
                    Layout.fillHeight: false
                    Layout.preferredWidth: width
                    Layout.preferredHeight: height
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Layout.bottomMargin: units.smallSpacing * 3
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
                        dockConfig.setSticker(checked)
                    }

                    Component.onCompleted: {
                        checked = plasmoid.configuration.configurationSticker
                        dockConfig.setSticker(plasmoid.configuration.configurationSticker)
                    }
                }

                RowLayout {
                    id: advancedSettings
                    Layout.fillWidth: true
                    Layout.rightMargin: units.smallSpacing * 2
                    Layout.alignment: Qt.AlignRight | Qt.AlignBottom

                    PlasmaComponents.Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight
                    }

                    PlasmaComponents.Label {
                        text: i18n("Advanced")
                        Layout.alignment: Qt.AlignRight
                        opacity: plasmoid.configuration.advanced ? 1 : 0.3

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                plasmoid.configuration.advanced = !advancedSwitch.checked;
                                advancedSwitch.checked = plasmoid.configuration.advanced;
                            }
                        }
                    }

                    Switch {
                        id: advancedSwitch
                        checked: plasmoid.configuration.advanced

                        onPressedChanged: {
                            if(pressed)
                                plasmoid.configuration.advanced = !checked;
                        }

                        style: Styles.SwitchStyle {
                            property bool checked: advancedSwitch.checked
                        }

                        onCheckedChanged: {
                            if (!checked && tabGroup.currentTab === tweaksPage) {
                                if (tasksTabBtn.visible) {
                                    tabGroup.currentTab = tasksPage;
                                    tabBar.currentTab = tasksTabBtn;
                                } else {
                                    tabGroup.currentTab = appearancePage;
                                    tabBar.currentTab = appearanceTabBtn;
                                }
                            }

                            if (checked) {
                                dockConfig.createSecondaryWindow();
                            } else {
                                dockConfig.deleteSecondaryWindow();
                            }
                        }
                    }
                }
            }
        }

        PlasmaComponents.TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.maximumWidth: maxWidth

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
                id: tasksTabBtn
                text: i18n("Tasks")
                tab: tasksPage

                visible: dock.latteTasksPresent()
            }
            PlasmaComponents.TabButton {
                id: tweaksTabBtn
                text: i18n("Tweaks")
                tab: tweaksPage

                visible: plasmoid.configuration.advanced
            }
        }

        Rectangle {
            id: pagesBackground
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.minimumWidth: maxWidth - 2*units.smallSpacing
            Layout.minimumHeight: height
            Layout.maximumHeight: height

            width: maxWidth - units.smallSpacing
            height: availableFreeHeight + units.smallSpacing * 4

            color: transparentBackgroundColor
            border.width: 1
            border.color: theme.backgroundColor

            //fix the height binding loop when showing the configuration window

            property int availableFreeHeight: Math.min(29*theme.mSize(theme.defaultFont).height + units.smallSpacing * 2, //calculatios according to font size
                                                       dock.screenGeometry.height - dock.normalThickness - 2*units.largeSpacing - header.height - headerSpacer.height
                                                       - tabBar.height - actionButtons.height - 36) //calculations according to screen geometry and elements heights

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

                    BehaviorConfig {
                        id: behaviorPage
                    }

                    AppearanceConfig {
                        id: appearancePage
                    }

                    TasksConfig {
                        id: tasksPage
                    }

                    TweaksConfig {
                        id: tweaksPage
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
                target: dock
                onDocksCountChanged: actionButtons.updateEnabled();
            }

            function updateEnabled() {
                addDock.enabled = dock.docksCount < 4 && dock.freeEdges().length > 0
                removeDock.enabled = dock.docksCount>1 && !(dock.docksWithTasks()===1 && dock.tasksPresent())
            }

            PlasmaComponents.Button {
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true
                text:" "

                PlasmaComponents3.ComboBox {
                    id: actionsCmb
                    anchors.fill: parent
                    enabled: addDock.enabled

                    property var activeLayoutsNames;

                    function addModel() {
                        var actions = []
                        actions.push("    " + i18n("Copy Dock"));

                        var tempActiveLayouts = layoutManager.activeLayoutsNames();
                        var currentLayoutIndex = tempActiveLayouts.indexOf(dock.managedLayout.name);

                        tempActiveLayouts.splice(currentLayoutIndex,1);

                        if (tempActiveLayouts.length > 0) {
                            activeLayoutsNames = tempActiveLayouts;
                            actions.push("  ------  ");
                            for(var i=0; i<activeLayoutsNames.length; ++i) {
                                actions.push("    " + i18n("Move to:") + " " + activeLayoutsNames[i]);
                            }
                        }

                        actionsCmb.model = actions;
                        actionsCmb.currentIndex = -1;
                    }

                    function emptyModel() {
                        var actions = []
                        actions.push("  ");
                        actionsCmb.model = actions;
                        actionsCmb.currentIndex = -1;
                    }

                    Component.onCompleted:{
                        addModel();
                    }

                    onActivated: {
                        if (index==0) {
                            dock.copyDock();
                        } else if (index>=2) {
                            dock.hideDockDuringMovingToLayout(activeLayoutsNames[index-2]);
                        }

                        actionsCmb.currentIndex = -1;
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
                    id: addDock
                    anchors.left: Qt.application.layoutDirection === Qt.RightToLeft ? undefined : parent.left
                    anchors.right: Qt.application.layoutDirection === Qt.RightToLeft ? parent.right : undefined
                    LayoutMirroring.enabled: false

                    width: parent.width - units.iconSizes.medium + 2*units.smallSpacing
                    height: parent.height

                    text: i18n("New Dock")
                    iconSource: "list-add"
                    tooltip: i18n("Add a new dock")

                    onClicked: dock.addNewDock()

                    Component.onCompleted: {
                        enabled = dock.freeEdges().length > 0
                    }
                }
            }

            PlasmaComponents.Button {
                id: removeDock

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                text: i18n("Remove")
                iconSource: "edit-delete"
                opacity: dock.totalDocksCount > 1 ? 1 : 0
                tooltip: i18n("Remove current dock")

                onClicked: dock.removeDock()
            }

            PlasmaComponents.Button {
                id: closeButton

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight

                text: i18n("Close")
                iconSource: "dialog-close"
                tooltip: i18n("Close settings window")

                onClicked: dockConfig.hideConfigWindow();
            }
        }
    }
}
