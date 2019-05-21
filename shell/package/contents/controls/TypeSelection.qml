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

import org.kde.latte 0.2 as Latte

Grid {
    id: typeRow

    width: horizontal ? content.width - 4*units.smallSpacing : 9 * theme.defaultFont.pixelSize
    anchors.leftMargin: horizontal ? units.smallSpacing : 0
    anchors.rightMargin: horizontal ? units.smallSpacing : 0

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

    ExclusiveGroup {
        id: viewTypeGroup
    }

    PlasmaComponents.Button {
        id: dockTypeButton
        width: horizontal ? (parent.width - parent.spacing)/ 2 : parent.width

        checkable: true
        checked: latteView.type === Latte.Types.DockView
        text: i18nc("dock type","Dock")
        exclusiveGroup: viewTypeGroup
        tooltip: i18n("Change the behavior and appearance to Dock type")

        onPressedChanged: {
            if (pressed && !checked) {
                latteView.visibility.mode = Latte.Types.DodgeActive;
                plasmoid.configuration.panelPosition = Latte.Types.Center;
                plasmoid.configuration.useThemePanel = true;
                plasmoid.configuration.solidPanel = false;
                plasmoid.configuration.panelSize = 5;
                plasmoid.configuration.shadows = 2;
                plasmoid.configuration.zoomLevel = 16;
                plasmoid.configuration.titleTooltips = true;
                plasmoid.configuration.activeIndicator = Latte.Types.InternalsIndicator;
                //plasmoid.configuration.autoDecreaseIconSize = true;

                //! Empty Areas
                plasmoid.configuration.dragActiveWindowEnabled = false;
                plasmoid.configuration.scrollAction = Latte.Types.ScrollNone;

                //! Animations
                plasmoid.configuration.animationLauncherBouncing = true;
                plasmoid.configuration.animationWindowInAttention = true;
                plasmoid.configuration.animationWindowAddedInGroup = true;

                //! Dynamic Background
                plasmoid.configuration.solidBackgroundForMaximized = false;
                plasmoid.configuration.colorizeTransparentPanels = false;
                plasmoid.configuration.backgroundOnlyOnMaximized = false;
                plasmoid.configuration.disablePanelShadowForMaximized = false;
                plasmoid.configuration.plasmaBackgroundForPopups = false;
            }
        }
    }

    PlasmaComponents.Button {
        id: panelTypeButton
        width: dockTypeButton.width

        checkable: true
        checked: latteView.type === Latte.Types.PanelView
        text: i18nc("panel type","Panel")
        exclusiveGroup: viewTypeGroup
        tooltip: i18n("Change the behavior and appearance to Panel type")

        onPressedChanged: {
            if (pressed && !checked) {
                latteView.visibility.mode = Latte.Types.AlwaysVisible;
                plasmoid.configuration.panelPosition = Latte.Types.Justify;
                plasmoid.configuration.useThemePanel = true;
                plasmoid.configuration.solidPanel = false;
                plasmoid.configuration.panelSize = 100;
                plasmoid.configuration.panelShadows = true;
                plasmoid.configuration.shadows = 0;
                plasmoid.configuration.zoomLevel = 0;
                plasmoid.configuration.titleTooltips = false;
                plasmoid.configuration.activeIndicator = Latte.Types.NoneIndicator;
                //plasmoid.configuration.autoDecreaseIconSize = false;

                //! Empty Areas
                plasmoid.configuration.dragActiveWindowEnabled = true;

                //! Animations
                plasmoid.configuration.animationLauncherBouncing = false;
                plasmoid.configuration.animationWindowInAttention = false;
                plasmoid.configuration.animationWindowAddedInGroup = false;

                //! Dynamic Background
                plasmoid.configuration.solidBackgroundForMaximized = (plasmoid.configuration.panelTransparency !== 100 ? true : false);
                plasmoid.configuration.colorizeTransparentPanels = false;
                plasmoid.configuration.backgroundOnlyOnMaximized = false;
                plasmoid.configuration.disablePanelShadowForMaximized = false;
                plasmoid.configuration.plasmaBackgroundForPopups = true;
            }
        }
    }
}

