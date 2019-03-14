/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte 0.2 as Latte

Loader{
    id: indicatorLoader
    anchors.fill: parent

    active: manager && manager.active && (isBackLayer || (!isBackLayer && indicatorProvidesFrontLayer))
    sourceComponent: manager.sourceComponent

    property bool isBackLayer: true
    property Item manager

    //! Used when the indicators require more thickness in the view mask
    //! e.g. when the Latte indicators are glowing in reverse order
    Binding {
        target: visibilityManager
        property: "indicatorsExtraThickMask"
        value: {
            if (indicatorLoader.isBackLayer
                    && indicatorLoader.active
                    && indicatorLoader.item
                    && indicatorLoader.item.hasOwnProperty("extraMaskThickness")) {
                return indicatorLoader.item.extraMaskThickness;
            }

            return 0;
        }
    }

    //! Used when the indicators need icon colors in orde to be painted
    //! properly, for example the Unity indicator
    Binding {
        target: appletItem
        property: "indicatorNeedsIconColors"
        value: {
            if (indicatorLoader.isBackLayer
                    && indicatorLoader.active
                    && indicatorLoader.item
                    && indicatorLoader.item.hasOwnProperty("needsIconColors")) {
                return indicatorLoader.item.needsIconColors;
            }

            return false;
        }
    }

    //! Used when the indicators property also a front layer
    //! to be drawn above the icon
    Binding {
        target: appletItem
        property: "indicatorProvidesFrontLayer"
        value: {
            if (indicatorLoader.isBackLayer
                    && indicatorLoader.active
                    && indicatorLoader.item
                    && indicatorLoader.item.hasOwnProperty("providesFrontLayer")) {
                return indicatorLoader.item.providesFrontLayer;
            }

            return false;
        }
    }
}
