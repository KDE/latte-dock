/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Containment {
    id: apis

    property int appletIndex: -1
    property Item myView: null

    alignment: {
        if (!myView) {
            return LatteCore.Types.Center;
        }

        if (myView.alignment === LatteCore.Types.Justify) {
            if (appletIndex>=0 && appletIndex<100) {
                return plasmoid.formFactor === PlasmaCore.Types.Horizontal ? LatteCore.Types.Left : LatteCore.Types.Top;
            } else if (appletIndex>=100 && appletIndex<200) {
                return LatteCore.Types.Center;
            } else if (appletIndex>=200) {
                return plasmoid.formFactor === PlasmaCore.Types.Horizontal ? LatteCore.Types.Right : LatteCore.Types.Bottom;
            }

            return LatteCore.Types.Center;
        }

        return myView.alignment;
    }

    readonly property Item publicApi: Item {
        readonly property alias isFirstAppletInContainment: apis.isFirstAppletInContainment
        readonly property alias isLastAppletInContainment: apis.isLastAppletInContainment

        readonly property alias alignment: apis.alignment
    }
}
