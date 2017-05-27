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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    ColumnLayout {
        id: content

        width: dialog.maxWidth - Layout.leftMargin * 2
        spacing: dialog.subGroupSpacing
        anchors.centerIn: parent
        Layout.leftMargin: units.smallSpacing * 2
        Layout.rightMargin: units.smallSpacing * 2

        //! BEGIN: Appearance
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            Header {
                text: i18n("Appearance")
            }

            PlasmaComponents.CheckBox {
                id: blurPanel
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Blur for panel background")
                checked: plasmoid.configuration.blurEnabled

                onClicked: {
                    plasmoid.configuration.blurEnabled = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: shrinkThickness
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Shrink thickness margins to minimum")
                checked: plasmoid.configuration.shrinkThickMargins

                onClicked: {
                    plasmoid.configuration.shrinkThickMargins = checked
                }
            }

            PlasmaComponents.CheckBox {
                id: onlyOnMaximizedChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show background only for maximized windows")
                checked: plasmoid.configuration.backgroundOnlyOnMaximized
                tooltip: i18n("The panel background becomes transparent and is shown only\nwhen there is a maximized window")

                onClicked: {
                    plasmoid.configuration.backgroundOnlyOnMaximized = checked;
                    plasmoid.configuration.disablePanelShadowForMaximized = false;
                    hideShadowsOnMaximizedChk.checked = false;
                }
            }

            PlasmaComponents.CheckBox {
                id: hideShadowsOnMaximizedChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Hide panel shadow for maximized windows")
                checked: plasmoid.configuration.disablePanelShadowForMaximized

                onClicked: {
                    plasmoid.configuration.disablePanelShadowForMaximized = checked;
                    plasmoid.configuration.backgroundOnlyOnMaximized = false;
                    onlyOnMaximizedChk.checked = false;
                }
            }
        }
        //! END: Appearance

        //! BEGIN: Behavior
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            Header {
                text: i18n("Behavior")
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Enable autostart during startup")
                checked: globalSettings.autostart

                onClicked: {
                    globalSettings.autostart = checked
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Decrease applets size when it is needed")
                checked: plasmoid.configuration.autoDecreaseIconSize
                tooltip: i18n("Applets size is decreased automatically when the contents \nexceed the maximum length")

                onClicked: {
                    plasmoid.configuration.autoDecreaseIconSize = checked
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
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Behave as a normal dock window")
                checked: dock.dockWinBehavior
                enabled: !(dock.visibility.mode === Latte.Dock.AlwaysVisible
                           || dock.visibility.mode === Latte.Dock.WindowsGoBelow)

                tooltip: i18n("Remove the BypassWindowManagerHint flag from the window.\nThe dock wont be above windows which are set at 'Always On Top'")

                onCheckedChanged: {
                    dock.dockWinBehavior = checked
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show Alternative Session in the context menu")
                checked: globalSettings.exposeAltSession

                onClicked: {
                    globalSettings.exposeAltSession = checked
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Raise dock on desktop change")
                checked: dock.visibility.raiseOnDesktop
                enabled: dock.visibility.mode !== Latte.Dock.AlwaysVisible

                onClicked: {
                    dock.visibility.raiseOnDesktop = checked
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Raise dock on activity change")
                checked: dock.visibility.raiseOnActivity
                enabled: dock.visibility.mode !== Latte.Dock.AlwaysVisible

                onClicked: {
                    dock.visibility.raiseOnActivity = checked
                }
            }
        }
        //! END: Behavior

        //! BEGIN: Active Indicator
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            visible: plasmoid.configuration.advanced

            Header {
                text: i18n("Active Applet Indicator")
            }

            RowLayout{
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 2

                RowLayout {
                    Layout.fillWidth: true

                    spacing: units.smallSpacing

                    property int activeIndicator: plasmoid.configuration.activeIndicator

                    ExclusiveGroup {
                        id: activeIndicatorGroup
                        onCurrentChanged: {
                            if (current.checked)
                                plasmoid.configuration.activeIndicator = current.activeIndicator
                        }
                    }

                    PlasmaComponents.Button {
                        Layout.fillWidth: true

                        text: i18nc("active indicator to no applets", "None")
                        checked: parent.activeIndicator === activeIndicator
                        checkable: true
                        exclusiveGroup: activeIndicatorGroup

                        readonly property int activeIndicator: Latte.Dock.NoneIndicator
                    }
                    PlasmaComponents.Button {
                        Layout.fillWidth: true

                        text: i18nc("active indicator only to in-house latte applets", "Internals")
                        checked: parent.activeIndicator === activeIndicator
                        checkable: true
                        exclusiveGroup: activeIndicatorGroup

                        readonly property int activeIndicator: Latte.Dock.InternalsIndicator
                    }
                    PlasmaComponents.Button {
                        Layout.fillWidth: true

                        text: i18nc("active indicator to all applets", "All")
                        checked: parent.activeIndicator === activeIndicator
                        checkable: true
                        exclusiveGroup: activeIndicatorGroup

                        readonly property int activeIndicator: Latte.Dock.AllIndicator
                    }
                }
            }
        }
        //! END: Active Indicator


        //! BEGIN: Session
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            Header {
                text: i18n("Session")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: units.smallSpacing

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Alternative Session")
                    checked: globalSettings.currentSession === Latte.Dock.AlternativeSession
                    checkable: true
                    tooltip: i18n("Sometimes the current layout of your panels is not sufficient \nfor example when you are travelling. Latte provides you with a full \nalternative sessionn to work on.")

                    onClicked: {
                        if (globalSettings.currentSession === Latte.Dock.DefaultSession) {
                            globalSettings.currentSession = Latte.Dock.AlternativeSession
                        } else {
                            globalSettings.currentSession = Latte.Dock.DefaultSession
                        }
                        dockConfig.hideConfigWindow()
                    }
                }

            }
        }
        //! END: Session

        //! BEGIN: Extra Actions
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            Header {
                text: i18n("Extra Actions")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                spacing: 2

                PlasmaComponents.Button {
                    iconSource: "distribute-horizontal-x"
                    text: i18n("Add Spacer")
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft
                    tooltip: i18n("Add a spacer to separate applets")

                    onClicked: {
                        dockConfig.addPanelSpacer()
                    }
                }

                PlasmaComponents.Button {
                    iconSource: "edit-delete"
                    text: i18n("Remove Tasks")
                    enabled: dock.tasksPresent()
                    tooltip: i18n("Remove Latte plasmoid")

                    onClicked: {
                        dock.removeTasksPlasmoid();
                    }
                }
            }
        }
        //! END: Extra Actions

        PlasmaComponents.Label{
            id: bottomMarginSpacer
            text:" "
        }
    }
}
