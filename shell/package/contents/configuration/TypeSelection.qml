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
import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

Grid {
    id: typeRow

    width: horizontal ? content.width - 6*units.smallSpacing : maxWidth - 2*units.smallSpacing
    anchors.leftMargin: units.smallSpacing
    anchors.rightMargin: units.smallSpacing

    Layout.minimumWidth: width
    Layout.maximumWidth: width
    Layout.minimumHeight: height
    Layout.maximumHeight: height

    Layout.leftMargin: units.smallSpacing * 2
    Layout.rightMargin: units.smallSpacing * 2

    rows: horizontal ? 1 : 0
    columns: horizontal ? 0 : 1

    spacing: 1

    verticalItemAlignment: Grid.AlignVCenter

    property bool horizontal: false

    readonly property bool isPanel: (plasmoid.configuration.panelPosition === Latte.Dock.Justify)
                                    && (plasmoid.configuration.useThemePanel) && (plasmoid.configuration.panelSize===100)
                                    && (plasmoid.configuration.zoomLevel === 0)

    ExclusiveGroup {
        id: dockTypeGroup
    }

    PlasmaComponents.Button {
        id: dockTypeButton
        width: horizontal ? (parent.width - parent.spacing)/ 2 : parent.width

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
                plasmoid.configuration.shrinkThickMargins = false;
                plasmoid.configuration.activeIndicator = Latte.Dock.InternalsIndicator;
                plasmoid.configuration.autoDecreaseIconSize = true;

                //! Dynamic Background
                plasmoid.configuration.solidBackgroundForMaximized = false;
                plasmoid.configuration.colorizeTransparentPanels = false;
                plasmoid.configuration.backgroundOnlyOnMaximized = false;
                plasmoid.configuration.disablePanelShadowForMaximized = false;
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
                plasmoid.configuration.shrinkThickMargins = true;
                plasmoid.configuration.activeIndicator = Latte.Dock.NoneIndicator;
                plasmoid.configuration.autoDecreaseIconSize = false;

                //! Dynamic Background
                plasmoid.configuration.solidBackgroundForMaximized = (plasmoid.configuration.panelTransparency !== 100 ? true : false);
                plasmoid.configuration.colorizeTransparentPanels = false;
                plasmoid.configuration.backgroundOnlyOnMaximized = false;
                plasmoid.configuration.disablePanelShadowForMaximized = false;
            }
        }
    }
}

