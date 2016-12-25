/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */


import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    width: childrenRect.width
    height: childrenRect.height

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property alias cfg_wheelEnabled: wheelEnabled.checked
    property alias cfg_middleClickAction: middleClickAction.currentIndex

    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_showOnlyCurrentDesktop: showOnlyCurrentDesktop.checked
    property alias cfg_showOnlyCurrentActivity: showOnlyCurrentActivity.checked

    property alias cfg_highlightWindows: highlightWindowsChk.checked
    property alias cfg_smartLaunchersEnabled: smartLaunchersChk.checked
    property alias cfg_showToolTips: showPreviewsChk.checked
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
                    id: showPreviewsChk
                    Layout.fillWidth: true
                    text: i18n("Preview windows on hovering")
                }

                CheckBox {
                    id: highlightWindowsChk
                    Layout.fillWidth: true
                    text: i18n("Highlight windows on hovering")
                }

                CheckBox {
                    id: windowActionsChk
                    Layout.fillWidth: true
                    text: i18n("Show window actions in the context menu")
                }

                CheckBox {
                    id: smartLaunchersChk
                    Layout.fillWidth: true
                    text: i18n("Show progress information in task buttons")
                }

                RowLayout {
                    Label {
                        text: i18n("On middle-click:")
                    }

                    ComboBox {
                        id: middleClickAction
                        Layout.fillWidth: true
                        model: [i18nc("The click action", "None"), i18n("Close Window or Group"), i18n("New Instance"), i18n("Minimize/Restore Window or Group")]
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
