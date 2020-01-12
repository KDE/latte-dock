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
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

import org.kde.plasma.plasmoid 2.0

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

import org.kde.latte 0.2 as Latte

import "../controls" as LatteExtraControls

FocusScope {
    id: dialog

    width: typeSettings.width + units.smallSpacing * 4
    height: typeSettings.height + units.smallSpacing * 4
    Layout.minimumWidth: width
    Layout.minimumHeight: height
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    PlasmaCore.FrameSvgItem{
        anchors.fill: parent
        imagePath: "widgets/panel-background"
        enabledBorders: viewConfig.enabledBorders
    }


    LatteExtraControls.TypeSelection{
        id: typeSettings
        anchors.centerIn: parent

        Component.onCompleted: forceActiveFocus();

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                viewConfig.hideConfigWindow();
            }
        }
    }

    //! HACK FOR X11 environments
    //! show an inner shadow similar to Latte::View editShadow in order to
    //! not break the visual user experience
    LatteExtraControls.InnerShadow{
        width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? dialog.width + 2*shadowSize : shadowSize
        height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? shadowSize : dialog.height + 2*shadowSize

        shadowSize: latteView.effects.editShadow
        shadowOpacity: Math.max(0.35, maxOpacity)
        shadowDirection: plasmoid.location

        visible: !Latte.WindowSystem.isPlatformWayland && Latte.WindowSystem.compositingActive && latteView.effects.settingsMaskSubtracted

        readonly property real maxOpacity: Latte.WindowSystem.compositingActive && !plasmoid.configuration.inConfigureAppletsMode ?
                                               plasmoid.configuration.editBackgroundOpacity : 1
    }
}
