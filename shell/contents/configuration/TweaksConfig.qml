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
        spacing: units.largeSpacing
        anchors.centerIn: parent
        Layout.leftMargin: units.smallSpacing * 2

        //! BEGIN: Behavior
        ColumnLayout {
            spacing: units.smallSpacing

            Header {
                text: i18n("Behavior")
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Enable autostart during startup")
                checked: dockConfig.autostart

                onClicked: {
                    dockConfig.autostart = checked;
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Raise docks on desktop or actitivy change")
                checked: dockConfig.raiseDocksTemporary

                onClicked: {
                    dockConfig.raiseDocksTemporary = checked;
                }
            }
        }

        //! BEGIN: Session
        ColumnLayout {
            spacing: units.smallSpacing

            Header {
                text: i18n("Session")
            }

            PlasmaComponents.Button {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                Layout.fillWidth: true
                text: i18n("Alternative Session")
                checked: dockConfig.currentSession === Latte.Dock.AlternativeSession
                checkable: true

                onClicked: {
                    if (dockConfig.currentSession === Latte.Dock.DefaultSession){
                        dockConfig.currentSession = Latte.Dock.AlternativeSession;
                    } else {
                        dockConfig.currentSession = Latte.Dock.DefaultSession;
                    }
                }

            }
        }

        //! BEGIN: Extra Actions
        ColumnLayout {
            spacing: units.smallSpacing

            Header {
                text: i18n("Extra Actions")
            }

            GridLayout {
                width: parent.width
                rowSpacing: 1
                columnSpacing: 1
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                columns: 2

                PlasmaComponents.Button {
                    iconSource: "distribute-horizontal-x"
                    text: i18n("Add Spacer")
                    Layout.alignment: Qt.AlignLeft
                    onClicked: {
                        dockConfig.addPanelSpacer();
                    }
                }

                PlasmaComponents.Label{}
            }
        }
    }
}
