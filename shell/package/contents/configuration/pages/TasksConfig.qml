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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

import org.kde.latte.private.tasks 0.1 as LatteTasks

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    property bool disableAllWindowsFunctionality: tasks.configuration.hideAllTasks

    ColumnLayout {
        id: content

        width: (dialog.appliedWidth - units.smallSpacing * 2) - Layout.leftMargin * 2
        spacing: dialog.subGroupSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.leftMargin: units.smallSpacing * 2
        Layout.rightMargin: units.smallSpacing * 2

        //! BEGIN: Badges
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.topMargin: units.smallSpacing
            visible: dialog.advancedLevel

            LatteComponents.Header {
                text: i18n("Badges")
            }

            LatteComponents.CheckBoxesColumn {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Notifications from tasks")
                    checked: tasks.configuration.showInfoBadge
                    tooltip: i18n("Show unread messages or notifications from tasks")

                    onClicked: {
                        tasks.configuration.showInfoBadge = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Progress information for tasks")
                    checked: tasks.configuration.showProgressBadge
                    tooltip: i18n("Show a progress animation for tasks e.g. when copying files with Dolphin")

                    onClicked: {
                        tasks.configuration.showProgressBadge = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Audio playing from tasks")
                    checked: tasks.configuration.showAudioBadge
                    tooltip: i18n("Show audio playing from tasks")

                    onClicked: {
                        tasks.configuration.showAudioBadge = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Prominent color for notification badge")
                    checked: tasks.configuration.infoBadgeProminentColorEnabled
                    enabled: tasks.configuration.showInfoBadge
                    tooltip: i18n("Notification badge uses a more prominent background which is usually red")

                    onClicked: {
                        tasks.configuration.infoBadgeProminentColorEnabled = !tasks.configuration.infoBadgeProminentColorEnabled;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Change volume when scrolling audio badge")
                    checked: tasks.configuration.audioBadgeActionsEnabled
                    enabled: tasks.configuration.showAudioBadge
                    tooltip: i18n("The user is able to mute/unmute with click or change the volume with mouse wheel")

                    onClicked: {
                        tasks.configuration.audioBadgeActionsEnabled = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Use compact badges")
                    checked: tasks.configuration.badgesCompactStyle
                    tooltip: i18n("Hide badge borders, and notifications count from the notification badge")

                    onClicked: {
                        tasks.configuration.badgesCompactStyle = checked
                    }
                }
            }
        }
        //! END: Badges

        //! BEGIN: Tasks Interaction
        ColumnLayout {
            Layout.topMargin: dialog.basicLevel ? units.smallSpacing : 0
            spacing: units.smallSpacing

            LatteComponents.Header {
                text: i18n("Interaction")
            }

            LatteComponents.CheckBoxesColumn {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Add launchers only in the Tasks Area")
                    checked: plasmoid.configuration.addLaunchersInTaskManager
                    tooltip: i18n("Launchers are added only in the taskmanager and not as plasma applets")
                    enabled: latteView.extendedInterface.latteTasksModel.count === 1
                             && latteView.extendedInterface.plasmaTasksModel.count === 0

                    onClicked: {
                        plasmoid.configuration.addLaunchersInTaskManager = checked;
                    }
                }

                LatteComponents.CheckBox {
                    id: windowActionsChk
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Window actions in the context menu")
                    checked: tasks.configuration.showWindowActions
                    visible: dialog.advancedLevel
                    enabled: !disableAllWindowsFunctionality

                    onClicked: {
                        tasks.configuration.showWindowActions = checked
                    }
                }

                LatteComponents.CheckBox {
                    id: previewPopupChk
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Preview window behaves as popup")
                    checked: tasks.configuration.previewWindowAsPopup
                    visible: dialog.advancedLevel
                    enabled: !disableAllWindowsFunctionality

                    onClicked: {
                        tasks.configuration.previewWindowAsPopup = checked;
                    }
                }

                LatteComponents.CheckBox {
                    id: unifyGlobalShortcutsChk
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("➊ Based on position shortcuts apply only on tasks")
                    // checked: tasks.configuration.isPreferredForPositionShortcuts //! Disabled because it was not updated between multiple Tasks
                    tooltip: i18n("Based on position global shortcuts are enabled only for tasks and not for applets")
                    visible: dialog.advancedLevel
                    enabled: latteView.isPreferredForShortcuts || (!latteView.layout.preferredForShortcutsTouched && latteView.isHighestPriorityView())

                    readonly property bool isPreferredForShotcuts: tasks.configuration.isPreferredForPositionShortcuts

                    onIsPreferredForShotcutsChanged: checked = isPreferredForShotcuts;

                    onClicked: {
                        tasks.configuration.isPreferredForPositionShortcuts = !tasks.configuration.isPreferredForPositionShortcuts;
                    }
                }
            }
        }
        //! END: Tasks Interaction

        //! BEGIN: Tasks Filters
        ColumnLayout {
            spacing: units.smallSpacing


            LatteComponents.Header {
                text: i18n("Filters")
            }

            LatteComponents.CheckBoxesColumn {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only tasks from the current screen")
                    checked: tasks.configuration.showOnlyCurrentScreen
                    enabled: !disableAllWindowsFunctionality

                    onClicked: {
                        tasks.configuration.showOnlyCurrentScreen = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only tasks from the current desktop")
                    checked: tasks.configuration.showOnlyCurrentDesktop
                    enabled: !disableAllWindowsFunctionality

                    onClicked: {
                        tasks.configuration.showOnlyCurrentDesktop = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only tasks from the current activity")
                    checked: tasks.configuration.showOnlyCurrentActivity
                    enabled: !disableAllWindowsFunctionality

                    onClicked: {
                        tasks.configuration.showOnlyCurrentActivity = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only tasks from launchers")
                    checked: tasks.configuration.showWindowsOnlyFromLaunchers
                    visible: dialog.advancedLevel
                    enabled: !disableAllWindowsFunctionality

                    onClicked: {
                        tasks.configuration.showWindowsOnlyFromLaunchers = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only launchers and hide all tasks")
                    checked: tasks.configuration.hideAllTasks
                    tooltip: i18n("Tasks become hidden and only launchers are shown")
                    visible: dialog.advancedLevel

                    onClicked: {
                        tasks.configuration.hideAllTasks = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only grouped tasks for same application")
                    checked: tasks.configuration.groupTasksByDefault
                    tooltip: i18n("By default group tasks of the same application")
                    visible: dialog.advancedLevel
                    enabled: !disableAllWindowsFunctionality

                    onClicked: {
                        tasks.configuration.groupTasksByDefault = checked
                    }
                }
            }
        }

        //! END: Tasks Filters

        //! BEGIN: Animations
        ColumnLayout {
            spacing: units.smallSpacing
            enabled: plasmoid.configuration.animationsEnabled
            visible: dialog.advancedLevel

            LatteComponents.Header {
                text: i18n("Animations")
            }

            LatteComponents.CheckBoxesColumn {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Bounce launchers when triggered")
                    checked: tasks.configuration.animationLauncherBouncing

                    onClicked: {
                        tasks.configuration.animationLauncherBouncing = !tasks.configuration.animationLauncherBouncing;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Bounce tasks that need attention")
                    checked: tasks.configuration.animationWindowInAttention

                    onClicked: {
                        tasks.configuration.animationWindowInAttention = !tasks.configuration.animationWindowInAttention;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Slide in and out single windows")
                    checked: tasks.configuration.animationNewWindowSliding

                    onClicked: {
                        tasks.configuration.animationNewWindowSliding = !tasks.configuration.animationNewWindowSliding;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Grouped tasks bounce their new windows")
                    checked: tasks.configuration.animationWindowAddedInGroup

                    onClicked: {
                        tasks.configuration.animationWindowAddedInGroup = !tasks.configuration.animationWindowAddedInGroup;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Grouped tasks slide out their closed windows")
                    checked: tasks.configuration.animationWindowRemovedFromGroup

                    onClicked: {
                        tasks.configuration.animationWindowRemovedFromGroup = !tasks.configuration.animationWindowRemovedFromGroup;
                    }
                }
            }
        }
        //! END: Animations


        //! BEGIN: Launchers Group
        ColumnLayout {
            spacing: units.smallSpacing


            LatteComponents.Header {
                text: i18n("Launchers")
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0

                LatteComponents.SubHeader {
                    isFirstSubCategory: true
                    text: i18nc("launchers group", "Group");
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 2

                    property int group: tasks.configuration.launchersGroup

                    readonly property int buttonsCount: layoutGroupButton.visible ? 3 : 2
                    readonly property int buttonSize: (dialog.optionsWidth - (spacing * buttonsCount-1)) / buttonsCount

                    ExclusiveGroup {
                        id: launchersGroup
                    }

                    PlasmaComponents.Button {
                        Layout.minimumWidth: parent.buttonSize
                        Layout.maximumWidth: Layout.minimumWidth
                        text: i18nc("unique launchers group","Unique")
                        checked: parent.group === group
                        checkable: false
                        exclusiveGroup: launchersGroup
                        tooltip: i18n("Use a unique set of launchers for this view which is independent from any other view")

                        readonly property int group: LatteCore.Types.UniqueLaunchers

                        onPressedChanged: {
                            if (pressed) {
                                tasks.configuration.launchersGroup = group;
                            }
                        }
                    }

                    PlasmaComponents.Button {
                        id: layoutGroupButton
                        Layout.minimumWidth: parent.buttonSize
                        Layout.maximumWidth: Layout.minimumWidth
                        text: i18nc("layout launchers group","Layout")
                        checked: parent.group === group
                        checkable: false
                        exclusiveGroup: launchersGroup
                        tooltip: i18n("Use the current layout set of launchers for this latteView. This group provides launchers <b>synchronization</b> between different views in the <b>same layout</b>")
                        //! it is shown only when the user has activated that option manually from the text layout file
                        visible: tasks.configuration.launchersGroup === group

                        readonly property int group: LatteCore.Types.LayoutLaunchers

                        onPressedChanged: {
                            if (pressed) {
                                tasks.configuration.launchersGroup = group;
                            }
                        }
                    }

                    PlasmaComponents.Button {
                        Layout.minimumWidth: parent.buttonSize
                        Layout.maximumWidth: Layout.minimumWidth
                        text: i18nc("global launchers group","Global")
                        checked: parent.group === group
                        checkable: false
                        exclusiveGroup: launchersGroup
                        tooltip: i18n("Use the global set of launchers for this latteView. This group provides launchers <b>synchronization</b> between different views and between <b>different layouts</b>")

                        readonly property int group: LatteCore.Types.GlobalLaunchers

                        onPressedChanged: {
                            if (pressed) {
                                tasks.configuration.launchersGroup = group;
                            }
                        }
                    }
                }
            }
        }
        //! END: Launchers Group

        //! BEGIN: Scrolling
        ColumnLayout {
            spacing: units.smallSpacing
            visible: dialog.advancedLevel

            LatteComponents.HeaderSwitch {
                id: scrollingHeader
                Layout.minimumWidth: dialog.optionsWidth + 2 *units.smallSpacing
                Layout.maximumWidth: Layout.minimumWidth
                Layout.minimumHeight: implicitHeight
                Layout.bottomMargin: units.smallSpacing
                enabled: LatteCore.WindowSystem.compositingActive

                checked: tasks.configuration.scrollTasksEnabled
                text: i18n("Scrolling")
                tooltip: i18n("Enable tasks scrolling when they overflow and exceed the available space");

                onPressed: {
                    tasks.configuration.scrollTasksEnabled = !tasks.configuration.scrollTasksEnabled;;
                }
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0
                enabled: scrollingHeader.checked

                GridLayout {
                    columns: 2
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth

                    Layout.topMargin: units.smallSpacing

                    PlasmaComponents.Label {
                        Layout.fillWidth: true
                        text: i18n("Manual")
                    }

                    LatteComponents.ComboBox {
                        id: manualScrolling
                        Layout.minimumWidth: leftClickAction.width
                        Layout.maximumWidth: leftClickAction.width
                        model: [i18nc("disabled manual scrolling", "Disabled scrolling"),
                            dialog.panelIsVertical ? i18n("Only vertical scrolling") : i18n("Only horizontal scrolling"),
                            i18n("Horizontal and vertical scrolling")]

                        currentIndex: tasks.configuration.manualScrollTasksType
                        onCurrentIndexChanged: tasks.configuration.manualScrollTasksType = currentIndex;
                    }

                    PlasmaComponents.Label {
                        id: autoScrollText
                        Layout.fillWidth: true
                        text: i18n("Automatic")
                    }

                    LatteComponents.ComboBox {
                        id: autoScrolling
                        Layout.minimumWidth: leftClickAction.width
                        Layout.maximumWidth: leftClickAction.width
                        model: [
                            i18n("Disabled"),
                            i18n("Enabled")
                        ]

                        currentIndex: tasks.configuration.autoScrollTasksEnabled
                        onCurrentIndexChanged: {
                            if (currentIndex === 0) {
                                tasks.configuration.autoScrollTasksEnabled = false;
                            } else {
                                tasks.configuration.autoScrollTasksEnabled = true;
                            }
                        }
                    }
                }
            }
        }
        //! END: Scolling


        //! BEGIN: Actions
        ColumnLayout {
            spacing: units.smallSpacing
            visible: dialog.advancedLevel

            LatteComponents.Header {
                text: i18n("Actions")
            }

            ColumnLayout {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0

                GridLayout {
                    columns: 2
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth

                    Layout.topMargin: units.smallSpacing
                    enabled: !disableAllWindowsFunctionality

                    PlasmaComponents.Label {
                        id: leftClickLbl
                        text: i18n("Left Click")
                    }

                    LatteComponents.ComboBox {
                        id: leftClickAction
                        Layout.fillWidth: true
                        model: [i18nc("present windows action", "Present Windows"),
                            i18n("Cycle Through Tasks"),
                            i18n("Preview Windows")]

                        currentIndex: {
                            switch(tasks.configuration.leftClickAction) {
                            case LatteTasks.Types.PresentWindows:
                                return 0;
                            case LatteTasks.Types.CycleThroughTasks:
                                return 1;
                            case LatteTasks.Types.PreviewWindows:
                                return 2;
                            }

                            return 0;
                        }

                        onCurrentIndexChanged: {
                            switch(currentIndex) {
                            case 0:
                                tasks.configuration.leftClickAction = LatteTasks.Types.PresentWindows;
                                break;
                            case 1:
                                tasks.configuration.leftClickAction = LatteTasks.Types.CycleThroughTasks;
                                break;
                            case 2:
                                tasks.configuration.leftClickAction = LatteTasks.Types.PreviewWindows;
                                break;
                            }
                        }
                    }

                    PlasmaComponents.Label {
                        id: middleClickText
                        text: i18n("Middle Click")
                    }

                    LatteComponents.ComboBox {
                        id: middleClickAction
                        Layout.fillWidth: true
                        model: [
                            i18nc("The click action", "None"),
                            i18n("Close Window or Group"),
                            i18n("New Instance"),
                            i18n("Minimize/Restore Window or Group"),
                            i18n("Cycle Through Tasks"),
                            i18n("Toggle Task Grouping")
                        ]

                        currentIndex: tasks.configuration.middleClickAction
                        onCurrentIndexChanged: tasks.configuration.middleClickAction = currentIndex
                    }

                    PlasmaComponents.Label {
                        text: i18n("Hover")
                    }

                    LatteComponents.ComboBox {
                        id: hoverAction
                        Layout.fillWidth: true
                        model: [
                            i18nc("none action", "None"),
                            i18n("Preview Windows"),
                            i18n("Highlight Windows"),
                            i18n("Preview and Highlight Windows"),
                        ]

                        currentIndex: {
                            switch(tasks.configuration.hoverAction) {
                            case LatteTasks.Types.NoneAction:
                                return 0;
                            case LatteTasks.Types.PreviewWindows:
                                return 1;
                            case LatteTasks.Types.HighlightWindows:
                                return 2;
                            case LatteTasks.Types.PreviewAndHighlightWindows:
                                return 3;
                            }

                            return 0;
                        }

                        onCurrentIndexChanged: {
                            switch(currentIndex) {
                            case 0:
                                tasks.configuration.hoverAction = LatteTasks.Types.NoneAction;
                                break;
                            case 1:
                                tasks.configuration.hoverAction = LatteTasks.Types.PreviewWindows;
                                break;
                            case 2:
                                tasks.configuration.hoverAction = LatteTasks.Types.HighlightWindows;
                                break;
                            case 3:
                                tasks.configuration.hoverAction = LatteTasks.Types.PreviewAndHighlightWindows;
                                break;
                            }
                        }
                    }

                    PlasmaComponents.Label {
                        text: i18n("Wheel")
                    }

                    LatteComponents.ComboBox {
                        id: wheelAction
                        Layout.fillWidth: true
                        model: [
                            i18nc("none action", "None"),
                            i18n("Cycle Through Tasks"),
                            i18n("Cycle And Minimize Tasks")
                        ]

                        currentIndex: tasks.configuration.taskScrollAction
                        onCurrentIndexChanged: tasks.configuration.taskScrollAction = currentIndex
                    }

                    RowLayout {
                        spacing: units.smallSpacing
                        enabled: !disableAllWindowsFunctionality

                        Layout.minimumWidth: middleClickText.width
                        Layout.maximumWidth: middleClickText.width

                        LatteComponents.ComboBox {
                            id: modifier
                            Layout.fillWidth: true
                            model: ["Shift", "Ctrl", "Alt", "Meta"]

                            currentIndex: tasks.configuration.modifier
                            onCurrentIndexChanged: tasks.configuration.modifier = currentIndex
                        }

                        PlasmaComponents.Label {
                            text: "+"
                        }
                    }

                    RowLayout {
                        spacing: units.smallSpacing
                        enabled: !disableAllWindowsFunctionality

                        readonly property int maxSize: 0.4 * dialog.optionsWidth

                        LatteComponents.ComboBox {
                            id: modifierClick
                            Layout.preferredWidth: 0.7 * parent.maxSize
                            Layout.maximumWidth: parent.maxSize
                            model: [i18n("Left Click"), i18n("Middle Click"), i18n("Right Click")]

                            currentIndex: tasks.configuration.modifierClick
                            onCurrentIndexChanged: tasks.configuration.modifierClick = currentIndex
                        }

                        PlasmaComponents.Label {
                            text: "="
                        }

                        LatteComponents.ComboBox {
                            id: modifierClickAction
                            Layout.fillWidth: true
                            model: [i18nc("The click action", "None"), i18n("Close Window or Group"),
                                i18n("New Instance"), i18n("Minimize/Restore Window or Group"),  i18n("Cycle Through Tasks"), i18n("Toggle Task Grouping")]

                            currentIndex: tasks.configuration.modifierClickAction
                            onCurrentIndexChanged: tasks.configuration.modifierClickAction = currentIndex
                        }
                    }
                }

                RowLayout {
                    Layout.minimumWidth: dialog.optionsWidth
                    Layout.maximumWidth: Layout.minimumWidth
                    Layout.topMargin: units.smallSpacing
                    spacing: units.smallSpacing
                    enabled: !disableAllWindowsFunctionality

                }
            }
        }
        //! END: Actions

        //! BEGIN: Recycling
       /* ColumnLayout {
            spacing: units.smallSpacing
            visible: dialog.advancedLevel

            LatteComponents.Header {
                text: i18n("Recycling")
            }

            PlasmaComponents.Button {
                Layout.minimumWidth: dialog.optionsWidth
                Layout.maximumWidth: Layout.minimumWidth
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                Layout.topMargin: units.smallSpacing

                text: i18n("Remove Latte Tasks Applet")
                enabled: latteView.latteTasksArePresent
                tooltip: i18n("Remove Latte Tasks plasmoid")

                onClicked: {
                    latteView.removeTasksPlasmoid();
                }
            }
        }*/
    }
}
