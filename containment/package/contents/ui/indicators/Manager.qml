/*
*  Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

import "../applet/indicator" as AppletIndicator

Item{
    id: managerIndicator

    readonly property QtObject configuration: latteView && latteView.indicator ? latteView.indicator.configuration : null

    readonly property bool isEnabled: latteView && latteView.indicator ? (latteView.indicator.enabled && latteView.indicator.pluginIsReady) : false
    readonly property bool enabledForApplets: latteView && latteView.indicator ? latteView.indicator.enabledForApplets : true
    readonly property bool reversed: latteView && latteView.indicator ? latteView.indicator.reversed : false
    readonly property real padding: latteView && latteView.indicator ? latteView.indicator.padding: 0.08
    readonly property string type: latteView && latteView.indicator ? latteView.indicator.type : "org.kde.latte.indicator.default"

    readonly property Component plasmaStyleComponent: latteView && latteView.indicator ? latteView.indicator.plasmaComponent : null
    readonly property Component indicatorComponent: latteView && latteView.indicator ? latteView.indicator.component : null

    readonly property Item info: Item{
        readonly property bool needsIconColors: metricsLoader.active && metricsLoader.item && metricsLoader.item.hasOwnProperty("needsIconColors")
                                                && metricsLoader.item.needsIconColors
        readonly property bool providesFrontLayer: metricsLoader.active && metricsLoader.item && metricsLoader.item.hasOwnProperty("providesFrontLayer")
                                                   && metricsLoader.item.providesFrontLayer

        readonly property int extraMaskThickness: {
            if (metricsLoader.active && metricsLoader.item && metricsLoader.item.hasOwnProperty("extraMaskThickness")) {
                return metricsLoader.item.extraMaskThickness;
            }

            return 0;
        }
    }

    //! Metrics and values provided from an invisible indicator
    Loader{
        id: metricsLoader
        opacity: 0
        active: managerIndicator.isEnabled

        readonly property bool isBackground: true
        readonly property bool isForeground: false
        readonly property Item bridge: AppletIndicator.Bridge{
            appletIsValid: false
        }

        sourceComponent: managerIndicator.indicatorComponent
    }
}

