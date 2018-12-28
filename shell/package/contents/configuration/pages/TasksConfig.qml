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
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte

import "../../controls" as LatteExtraControls

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    property bool disableAllWindowsFunctionality: plasmoid.configuration.showWindowsOnlyFromLaunchers
                                                  && plasmoid.configuration.activeIndicator === Latte.Types.NoneIndicator

    ColumnLayout {
        id: content

        width: (dialog.appliedWidth - units.smallSpacing * 2) - Layout.leftMargin * 2
        spacing: dialog.subGroupSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.leftMargin: units.smallSpacing * 2
        Layout.rightMargin: units.smallSpacing * 2

        //! BEGIN: Tasks Appearance
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.topMargin: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            enabled: !disableAllWindowsFunctionality

            LatteExtraControls.Header {
                text: i18n("Appearance")
            }

            PlasmaComponents.CheckBox {
                id: threeColorsWindows
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Different color for minimized windows")
                checked: plasmoid.configuration.threeColorsWindows

                onClicked: {
                    plasmoid.configuration.threeColorsWindows = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: dotsOnActive
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Indicator for active window group")
                checked: plasmoid.configuration.dotsOnActive
                tooltip: i18n("Grouped windows show both a line and a dot when \none of them is active and the Line Active Indicator \nis enabled")
                visible: dialog.highLevel
                enabled: plasmoid.configuration.activeIndicatorType === Latte.Types.LineIndicator

                onClicked: {
                    plasmoid.configuration.dotsOnActive = checked
                }
            }
        }
        //! END: Tasks Appearance

        //! BEGIN: Tasks Interaction
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            LatteExtraControls.Header {
                text: i18n("Interaction")
            }

            PlasmaComponents.CheckBox {
                id: showPreviewsChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Preview windows on hovering")
                checked: plasmoid.configuration.showToolTips
                enabled: !disableAllWindowsFunctionality

                onClicked: {
                    plasmoid.configuration.showToolTips = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: highlightWindowsChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Highlight windows on hovering")
                checked: plasmoid.configuration.highlightWindows
                visible: dialog.highLevel
                enabled: !disableAllWindowsFunctionality

                onClicked: {
                    plasmoid.configuration.highlightWindows = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: smartLaunchersChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show progress information for tasks")
                checked: plasmoid.configuration.smartLaunchersEnabled
                tooltip: i18n("Show a beautiful progress animation e.g. when copying \nfiles with Dolphin")

                onClicked: {
                    plasmoid.configuration.smartLaunchersEnabled = checked
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Add launchers only in the corresponding area")
                checked: plasmoid.configuration.addLaunchersInTaskManager
                tooltip: i18n("Launchers are added only in the taskmanager and not as plasma applets")

                onClicked: {
                    plasmoid.configuration.addLaunchersInTaskManager = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: windowActionsChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show window actions in the context menu")
                checked: plasmoid.configuration.showWindowActions
                visible: dialog.highLevel
                enabled: !disableAllWindowsFunctionality

                onClicked: {
                    plasmoid.configuration.showWindowActions = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: audioIndicatorChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show indicator for audio streams")
                checked: plasmoid.configuration.indicateAudioStreams
                tooltip: i18n("Audio indicator from which the user can mute/unmute an app")
                visible: dialog.highLevel

                onClicked: {
                    plasmoid.configuration.indicateAudioStreams = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: groupTasksChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Group tasks of the same application")
                checked: plasmoid.configuration.groupTasksByDefault
                tooltip: i18n("By default group tasks of the same application")
                visible: dialog.highLevel

                onClicked: {
                    plasmoid.configuration.groupTasksByDefault = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: unifyGlobalShortcutsChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Unify global shortcuts with applets")
                checked: plasmoid.configuration.unifiedGlobalShortcuts
                tooltip: i18n("Global shortcuts are used both from applets and tasks")
                visible: dialog.highLevel

                onClicked: {
                    plasmoid.configuration.unifiedGlobalShortcuts = checked
                }
            }
        }
        //! END: Tasks Interaction

        //! BEGIN: Tasks Filters
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            LatteExtraControls.Header {
                text: i18n("Filters")
            }

            PlasmaComponents.CheckBox {
                id: showOnlyCurrentScreen
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show only tasks from the current screen")
                checked: plasmoid.configuration.showOnlyCurrentScreen

                onClicked: {
                    plasmoid.configuration.showOnlyCurrentScreen = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: showOnlyCurrentDesktop
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show only tasks from the current desktop")
                checked: plasmoid.configuration.showOnlyCurrentDesktop

                onClicked: {
                    plasmoid.configuration.showOnlyCurrentDesktop = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: showOnlyCurrentActivity
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show only tasks from the current activity")
                checked: plasmoid.configuration.showOnlyCurrentActivity

                onClicked: {
                    plasmoid.configuration.showOnlyCurrentActivity = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: showWindowsOnlyFromLaunchersChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show only tasks from launchers")
                checked: plasmoid.configuration.showWindowsOnlyFromLaunchers
                visible: dialog.highLevel

                onClicked: {
                    plasmoid.configuration.showWindowsOnlyFromLaunchers = checked
                }
            }
        }
        //! END: Tasks Filters

        //! BEGIN: Launchers Group
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            LatteExtraControls.Header {
                text: i18n("Launchers")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: 2

                property int group: plasmoid.configuration.launchersGroup

                ExclusiveGroup {
                    id: launchersGroup
                    onCurrentChanged: {
                        if (current.checked) {
                            viewConfig.updateLaunchersForGroup(current.group);
                            plasmoid.configuration.launchersGroup = current.group;
                        }
                    }
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("unique launchers group","Unique")
                    checked: parent.group === group
                    checkable: true
                    exclusiveGroup: launchersGroup
                    tooltip: i18n("Use a unique set of launchers for this view which is independent from any other view")

                    readonly property int group: Latte.Types.UniqueLaunchers
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("layout launchers group","Layout")
                    checked: parent.group === group
                    checkable: true
                    exclusiveGroup: launchersGroup
                    tooltip: i18n("Use the current layout set of launchers for this latteView. This group provides launchers <b>synchronization</b> between different views in the <b>same layout</b>")
                    //! it is shown only when the user has activated that option manually from the text layout file
                    visible: plasmoid.configuration.launchersGroup === group

                    readonly property int group: Latte.Types.LayoutLaunchers
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("global launchers group","Global")
                    checked: parent.group === group
                    checkable: true
                    exclusiveGroup: launchersGroup
                    tooltip: i18n("Use the global set of launchers for this latteView. This group provides launchers <b>synchronization</b> between different views and between <b>different layouts</b>")

                    readonly property int group: Latte.Types.GlobalLaunchers
                }
            }
        }
        //! END: Launchers Group

        //! BEGIN: Actions
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            visible: dialog.expertLevel

            LatteExtraControls.Header {
                text: i18n("Actions")
            }

            RowLayout {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.topMargin: units.smallSpacing
                enabled: !disableAllWindowsFunctionality

                PlasmaComponents.Label {
                    text: i18n("On middle-click")
                    Layout.alignment: Qt.AlignRight
                }

                PlasmaComponents3.ComboBox {
                    id: middleClickAction
                    Layout.fillWidth: true
                    model: [i18nc("The click action", "None"), i18n("Close Window or Group"),
                        i18n("New Instance"), i18n("Minimize/Restore Window or Group"), i18n("Cycle Through Tasks"), i18n("Toggle Task Grouping")]

                    currentIndex: plasmoid.configuration.middleClickAction
                    onCurrentIndexChanged: plasmoid.configuration.middleClickAction = currentIndex
                }
            }

            RowLayout {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.topMargin: units.smallSpacing
                spacing: units.smallSpacing
                enabled: !disableAllWindowsFunctionality

                PlasmaComponents3.ComboBox {
                    id: modifier
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 5
                    model: ["Shift", "Ctrl", "Alt", "Meta"]

                    currentIndex: plasmoid.configuration.modifier
                    onCurrentIndexChanged: plasmoid.configuration.modifier = currentIndex
                }

                PlasmaComponents.Label {
                    text: "+"
                }

                PlasmaComponents3.ComboBox {
                    id: modifierClick
                    Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 10
                    model: [i18n("Left Click"), i18n("Middle Click"), i18n("Right Click")]

                    currentIndex: plasmoid.configuration.modifierClick
                    onCurrentIndexChanged: plasmoid.configuration.modifierClick = currentIndex
                }

                PlasmaComponents.Label {
                    text: "="
                }

                PlasmaComponents3.ComboBox {
                    id: modifierClickAction
                    Layout.fillWidth: true
                    model: [i18nc("The click action", "None"), i18n("Close Window or Group"),
                        i18n("New Instance"), i18n("Minimize/Restore Window or Group"),  i18n("Cycle Through Tasks"), i18n("Toggle Task Grouping")]

                    currentIndex: plasmoid.configuration.modifierClickAction
                    onCurrentIndexChanged: plasmoid.configuration.modifierClickAction = currentIndex
                }
            }

            PlasmaComponents.Button {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.topMargin: units.smallSpacing
                Layout.fillWidth: true

                text: i18n("Remove Latte Tasks Applet")
                enabled: latteView.latteTasksPresent()
                tooltip: i18n("Remove Latte Tasks plasmoid")

                onClicked: {
                    latteView.removeTasksPlasmoid();
                }
            }
        }
        //! END: Actions

        //! Bottom spacer
        PlasmaComponents.Label{
            id: bottomMarginSpacer
            text:" "
        }
    }
}
