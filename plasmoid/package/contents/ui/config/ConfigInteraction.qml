/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte.private.tasks 0.1 as LatteTasks

Item {
    width: childrenRect.width
    height: childrenRect.height

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property alias cfg_wheelEnabled: wheelEnabled.checked
    property alias cfg_middleClickAction: middleClickAction.currentIndex
    property alias cfg_hoverAction: hoverActionCmb.currentIndex

    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_showOnlyCurrentDesktop: showOnlyCurrentDesktop.checked
    property alias cfg_showOnlyCurrentActivity: showOnlyCurrentActivity.checked

    property alias cfg_showInfoBadge: showInfoBadgeChk.checked
    property alias cfg_showWindowActions: windowActionsChk.checked

    ColumnLayout{
        spacing: 15

        GroupBox {
            title: ""
            flat: true
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true

                CheckBox {
                    id: wheelEnabled
                    text: i18n("Cycle through tasks with mouse wheel")
                    enabled: false
                }

                CheckBox {
                    id: windowActionsChk
                    Layout.fillWidth: true
                    text: i18n("Show window actions in the context menu")
                }

                CheckBox {
                    id: showInfoBadgeChk
                    Layout.fillWidth: true
                    text: i18n("Show progress information for tasks")
                }

                GridLayout {
                    columns: 2

                    Label {
                        text: i18n("Middle Click")
                    }

                    ComboBox {
                        id: middleClickAction
                        Layout.fillWidth: true
                        model: [i18nc("The click action", "None"), i18n("Close Window or Group"), i18n("New Instance"), i18n("Minimize/Restore Window or Group")]
                    }

                    Label {
                        text: i18n("Hover")
                    }

                    ComboBox {
                        id: hoverActionCmb
                        Layout.fillWidth: true
                        model: [
                            i18nc("none action", "None"),
                            i18n("Preview Windows"),
                            i18n("Highlight Windows"),
                            i18n("Preview and Highlight Windows"),
                        ]

                        currentIndex: {
                            switch(plasmoid.configuration.hoverAction) {
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
                                plasmoid.configuration.hoverAction = LatteTasks.Types.NoneAction;
                                break;
                            case 1:
                                plasmoid.configuration.hoverAction = LatteTasks.Types.PreviewWindows;
                                break;
                            case 2:
                                plasmoid.configuration.hoverAction = LatteTasks.Types.HighlightWindows;
                                break;
                            case 3:
                                plasmoid.configuration.hoverAction = LatteTasks.Types.PreviewAndHighlightWindows;
                                break;
                            }
                        }
                    }

                }
            }
        }


        ColumnLayout {
            Layout.fillWidth: true


            Label {
                text: i18n("Filters")
               // Layout.fillWidth: true
                anchors.horizontalCenter: parent.horizontalCenter
               // anchors.centerIn: parent
                font.bold: true
                font.italic: true
            }


            CheckBox {
                id: showOnlyCurrentScreen
                text: i18n("Show only tasks from the current screen")
            }

            CheckBox {
                id: showOnlyCurrentDesktop
                text: i18n("Show only tasks from the current desktop")
            }

            CheckBox {
                id: showOnlyCurrentActivity
                text: i18n("Show only tasks from the current activity")
            }
        }

    }

}
