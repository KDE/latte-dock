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

    active: (root.indicatorsEnabled && appletItem.communicatorAlias.activeIndicatorEnabled && root.indicatorsForApplets)
            || (!root.indicatorsForApplets && appletItem.communicatorAlias.overlayLatteIconIsActive)

    sourceComponent: {
        if (!root.indicatorsForApplets && appletItem.communicatorAlias.overlayLatteIconIsActive) {
            return plasmaStyleIndicator;
        }

        switch (root.indicatorStyle) {
        case Latte.Types.LatteIndicator:
            return latteStyleIndicator;
        case Latte.Types.PlasmaIndicator:
            return plasmaStyleIndicator;
        case Latte.Types.UnityIndicator:
            return unityStyleIndicator;
        default:
            return latteStyleIndicator;
        };
    }

    /* Indicators Properties in order use them*/
    readonly property bool isTask: false
    readonly property bool isApplet: true

    readonly property bool isLauncher: false
    readonly property bool isStartup: false
    readonly property bool isWindow: false

    readonly property bool isActive: appletItem.isActive
    readonly property bool isGroup: false
    readonly property bool isMinimized: false
    readonly property bool inAttention: false

    readonly property bool hasActive: isActive
    readonly property bool hasMinimized: false
    readonly property bool hasShown: false

    readonly property int iconSize: root.iconSize
    readonly property int durationTime: root.durationTime
    readonly property real scaleFactor: appletItem.wrapperAlias.zoomScale
    readonly property color shadowColor: root.appShadowColorSolid

    readonly property bool dotsOnActive: root.dotsOnActive
    readonly property bool multiColorEnabled: root.threeColorsWindows
    readonly property bool reversedEnabled: root.reverseLinesPosition
    readonly property int activeIndicatorType: root.activeIndicatorType
    readonly property bool usePlasmaTabsStyle: !root.indicatorsForApplets

    //!glow options
    readonly property bool glowEnabled: root.showGlow
    readonly property int glowOption: root.glowOption
    readonly property real glowOpacity: root.glowOpacity
    readonly property bool glow3D: root.glow3D

    //!icon colors
    property color backgroundColor: appletItem.wrapperAlias.overlayIconLoader.backgroundColor
    property color glowColor: appletItem.wrapperAlias.overlayIconLoader.glowColor

    Component {
        id: latteStyleIndicator
        Latte.LatteIndicator{}
    }

    Component {
        id: plasmaStyleIndicator
        Latte.PlasmaIndicator{}
    }

    Component{
        id:unityStyleIndicator
        Latte.UnityIndicator{}
    }

    //! Used when the indicators require more thickness in the view mask
    //! e.g. when the Latte indicators are glowing in reverse order
    Binding {
        target: visibilityManager
        property: "indicatorsExtraThickMask"
        value: {
            if (indicatorLoader.active
                    && indicatorLoader.item
                    && indicatorLoader.item.hasOwnProperty("extraMaskThickness")) {
                return indicatorLoader.item.extraMaskThickness;
            }

            return 0;
        }
    }
}
