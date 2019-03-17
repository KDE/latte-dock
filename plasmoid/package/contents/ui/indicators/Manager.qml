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

import "options" as Options

Item{
    id: managerIndicator

    readonly property Item common: commonOptions
    readonly property Item explicit: explicitOptions

    readonly property Component indicatorComponent: latteStyleIndicator

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

    Options.Common {
        id: commonOptions
    }

    Options.Latte {
        id : explicitOptions
    }

    //! Indicators Components
    Component {
        id: latteStyleIndicator
        Latte.LatteIndicator{}
    }

    //! Metrics and values provided from an invisible indicator
    Loader{
        id: metricsLoader
        opacity: 0

        readonly property bool isBackLayer: true
        readonly property Item manager: managerIndicator

        sourceComponent: managerIndicator.indicatorComponent
    }
}

