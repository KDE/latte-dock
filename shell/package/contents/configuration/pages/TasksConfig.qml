/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    id: _tasksPage
    width: content.width + content.Layout.leftMargin * 2
    height: content.height + units.smallSpacing * 2

    property bool disableAllWindowsFunctionality: tasks.configuration.hideAllTasks

    readonly property bool isCurrentPage: (dialog.currentPage === _tasksPage)

    onIsCurrentPageChanged: {
        if (isCurrentPage && latteView.extendedInterface.latteTasksModel.count>1) {
            latteView.extendedInterface.appletRequestedVisualIndicator(tasks.id);
        }
    }

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
                    tooltip: i18n("Show unread messages or notifications from tasks")
                    value: tasks.configuration.showInfoBadge

                    onClicked: {
                        tasks.configuration.showInfoBadge = !tasks.configuration.showInfoBadge;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Progress information for tasks")
                    tooltip: i18n("Show a progress animation for tasks e.g. when copying files with Dolphin")
                    value: tasks.configuration.showProgressBadge

                    onClicked: {
                        tasks.configuration.showProgressBadge = !tasks.configuration.showProgressBadge;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Audio playing from tasks")
                    tooltip: i18n("Show audio playing from tasks")
                    value: tasks.configuration.showAudioBadge

                    onClicked: {
                        tasks.configuration.showAudioBadge = !tasks.configuration.showAudioBadge;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Prominent color for notification badge")
                    enabled: tasks.configuration.showInfoBadge
                    tooltip: i18n("Notification badge uses a more prominent background which is usually red")
                    value: tasks.configuration.infoBadgeProminentColorEnabled

                    onClicked: {
                        tasks.configuration.infoBadgeProminentColorEnabled = !tasks.configuration.infoBadgeProminentColorEnabled;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Change volume when scrolling audio badge")
                    enabled: tasks.configuration.showAudioBadge
                    tooltip: i18n("The user is able to mute/unmute with click or change the volume with mouse wheel")
                    value: tasks.configuration.audioBadgeActionsEnabled

                    onClicked: {
                        tasks.configuration.audioBadgeActionsEnabled = !tasks.configuration.audioBadgeActionsEnabled;
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
                    text: i18n("Launchers are added only in current tasks applet")
                    tooltip: i18n("Launchers are added only in current tasks applet and not as regular applets or in any other applet")
                    value:tasks.configuration.isPreferredForDroppedLaunchers

                    onClicked: {
                        tasks.configuration.isPreferredForDroppedLaunchers = !tasks.configuration.isPreferredForDroppedLaunchers;
                    }
                }

                LatteComponents.CheckBox {
                    id: windowActionsChk
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Window actions in the context menu")
                    visible: dialog.advancedLevel
                    enabled: !disableAllWindowsFunctionality
                    value: tasks.configuration.showWindowActions

                    onClicked: {
                        tasks.configuration.showWindowActions = !tasks.configuration.showWindowActions;
                    }
                }

                LatteComponents.CheckBox {
                    id: previewPopupChk
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Preview window behaves as popup")
                    visible: dialog.advancedLevel
                    enabled: !disableAllWindowsFunctionality
                    value: tasks.configuration.previewWindowAsPopup

                    onClicked: {
                        tasks.configuration.previewWindowAsPopup = !tasks.configuration.previewWindowAsPopup;
                    }
                }

                LatteComponents.CheckBox {
                    id: unifyGlobalShortcutsChk
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Based on position shortcuts apply only on current tasks")
                    // checked: tasks.configuration.isPreferredForPositionShortcuts //! Disabled because it was not updated between multiple Tasks
                    tooltip: i18n("Based on position global shortcuts are enabled only for current tasks and not for other applets")
                    visible: dialog.advancedLevel
                    enabled: latteView.isPreferredForShortcuts || (!latteView.layout.preferredForShortcutsTouched && latteView.isHighestPriorityView())
                    value: tasks.configuration.isPreferredForPositionShortcuts

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
                    enabled: !disableAllWindowsFunctionality
                    value: tasks.configuration.showOnlyCurrentScreen

                    onClicked: {
                        tasks.configuration.showOnlyCurrentScreen = !tasks.configuration.showOnlyCurrentScreen;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only tasks from the current desktop")
                    enabled: !disableAllWindowsFunctionality
                    value: tasks.configuration.showOnlyCurrentDesktop

                    onClicked: {
                        tasks.configuration.showOnlyCurrentDesktop = !tasks.configuration.showOnlyCurrentDesktop;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only tasks from the current activity")
                    enabled: !disableAllWindowsFunctionality
                    value: tasks.configuration.showOnlyCurrentActivity

                    onClicked: {
                        tasks.configuration.showOnlyCurrentActivity = !tasks.configuration.showOnlyCurrentActivity;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only tasks from launchers")
                    visible: dialog.advancedLevel
                    enabled: !disableAllWindowsFunctionality
                    value: tasks.configuration.showWindowsOnlyFromLaunchers

                    onClicked: {
                        tasks.configuration.showWindowsOnlyFromLaunchers = !tasks.configuration.showWindowsOnlyFromLaunchers;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only launchers and hide all tasks")
                    tooltip: i18n("Tasks become hidden and only launchers are shown")
                    visible: dialog.advancedLevel
                    value: tasks.configuration.hideAllTasks

                    onClicked: {
                        tasks.configuration.hideAllTasks = !tasks.configuration.hideAllTasks;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Show only grouped tasks for same application")
                    tooltip: i18n("By default group tasks of the same application")
                    visible: dialog.advancedLevel
                    enabled: !disableAllWindowsFunctionality
                    value: tasks.configuration.groupTasksByDefault

                    onClicked: {
                        tasks.configuration.groupTasksByDefault = !tasks.configuration.groupTasksByDefault;
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
                    value: tasks.configuration.animationLauncherBouncing
                    enabled: !latteView.indicator.info.providesTaskLauncherAnimation

                    onClicked: {
                        tasks.configuration.animationLauncherBouncing = !tasks.configuration.animationLauncherBouncing;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Bounce tasks that need attention")
                    value: tasks.configuration.animationWindowInAttention
                    enabled: !latteView.indicator.info.providesInAttentionAnimation

                    onClicked: {
                        tasks.configuration.animationWindowInAttention = !tasks.configuration.animationWindowInAttention;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Slide in and out single windows")
                    value: tasks.configuration.animationNewWindowSliding

                    onClicked: {
                        tasks.configuration.animationNewWindowSliding = !tasks.configuration.animationNewWindowSliding;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Grouped tasks bounce their new windows")
                    value: tasks.configuration.animationWindowAddedInGroup
                    enabled: !latteView.indicator.info.providesGroupedWindowAddedAnimation

                    onClicked: {
                        tasks.configuration.animationWindowAddedInGroup = !tasks.configuration.animationWindowAddedInGroup;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Grouped tasks slide out their closed windows")
                    value: tasks.configuration.animationWindowRemovedFromGroup
                    enabled: !latteView.indicator.info.providesGroupedWindowRemovedAnimation

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
                        text: i18nc("unique launchers group","Unique Group")
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
                        text: i18nc("layout launchers group","Layout Group")
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
                        text: i18nc("global launchers group","Global Group")
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
