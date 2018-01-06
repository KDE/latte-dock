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
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

Rectangle {
    width: typeRow.width + 2*marginWidth
    height: typeRow.height + 2*marginHeight

    property int marginHeight: 0.3 * typeRow.height
    property int marginWidth: 2 * typeRow.height

    color: transparentBackgroundColor
    border.width: 1
    border.color: theme.backgroundColor

    Row {
        id: typeRow
        anchors.centerIn: parent

        spacing: 1

        readonly property bool isPanel: (plasmoid.configuration.panelPosition === Latte.Dock.Justify)
                                        && (plasmoid.configuration.useThemePanel) && (plasmoid.configuration.panelSize===100)
                                        && (plasmoid.configuration.zoomLevel === 0) && !plasmoid.configuration.autoDecreaseIconSize

        /*layer.enabled: true
    layer.effect: DropShadow {
        radius: dockTypeButton.height / 10
        samples: radius
        color: "#0a0a0a"
        horizontalOffset: -1
        //opacity: 0.5
    }*/

        ExclusiveGroup {
            id: dockTypeGroup
        }

        PlasmaComponents.Button {
            id: dockTypeButton
            width: dialog.width / 6
            checkable: true
            checked: !typeRow.isPanel
            text: i18nc("dock type","Dock")
            exclusiveGroup: dockTypeGroup
            tooltip: i18n("Change the behavior and appearance to Dock type")

            onPressedChanged: {
                if (pressed && !checked) {
                    dock.visibility.mode = Latte.Dock.DodgeActive;
                    plasmoid.configuration.panelPosition = Latte.Dock.Center;
                    plasmoid.configuration.useThemePanel = true;
                    plasmoid.configuration.solidPanel = false;
                    plasmoid.configuration.panelSize = 0;
                    plasmoid.configuration.shadows = 2;
                    plasmoid.configuration.zoomLevel = 16;
                    plasmoid.configuration.autoDecreaseIconSize = true;
                    plasmoid.configuration.shrinkThickMargins = false;
                }
            }
        }

        PlasmaComponents.Button {
            id: panelTypeButton
            width: dockTypeButton.width
            checkable: true
            checked: typeRow.isPanel
            text: i18nc("panel type","Panel")
            exclusiveGroup: dockTypeGroup
            tooltip: i18n("Change the behavior and appearance to Panel type")

            onPressedChanged: {
                if (pressed && !checked) {
                    dock.visibility.mode = Latte.Dock.AlwaysVisible;
                    plasmoid.configuration.panelPosition = Latte.Dock.Justify;
                    plasmoid.configuration.useThemePanel = true;
                    plasmoid.configuration.solidPanel = false;
                    plasmoid.configuration.panelSize = 100;
                    plasmoid.configuration.panelShadows = true;
                    plasmoid.configuration.shadows = 0;
                    plasmoid.configuration.zoomLevel = 0;
                    plasmoid.configuration.autoDecreaseIconSize = false;
                    plasmoid.configuration.shrinkThickMargins = true;
                }
            }
        }
    }

}
