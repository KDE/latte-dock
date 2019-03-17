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

Item{
    id: indicatorManager
    anchors.fill: parent

    readonly property bool active: (indicators.common.indicatorsEnabled && appletItem.communicatorAlias.activeIndicatorEnabled && indicators.common.indicatorsForApplets)
                                   || (!indicators.common.indicatorsForApplets && appletItem.communicatorAlias.overlayLatteIconIsActive)

    readonly property Component sourceComponent: {
        if (!indicators.common.indicatorsForApplets && appletItem.communicatorAlias.overlayLatteIconIsActive) {
            return plasmaStyleIndicator;
        }

        switch (indicators.common.indicatorStyle) {
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
    readonly property bool isHovered: appletMouseArea.containsMouse
    readonly property bool isMinimized: false
    readonly property bool inAttention: false
    readonly property bool inRemoving: false

    readonly property bool hasActive: isActive
    readonly property bool hasMinimized: false
    readonly property bool hasShown: false
    readonly property int windowsCount: 0
    readonly property int windowsMinimizedCount: 0

    readonly property int currentIconSize: root.iconSize
    readonly property int maxIconSize: root.maxIconSize
    readonly property int durationTime: root.durationTime
    readonly property real scaleFactor: appletItem.wrapperAlias.zoomScale
    readonly property color shadowColor: root.appShadowColorSolid

    //!icon colors
    property color backgroundColor: appletItem.wrapperAlias.overlayIconLoader.backgroundColor
    property color glowColor: appletItem.wrapperAlias.overlayIconLoader.glowColor

    //! grouped options
    readonly property Item common: indicators.common
    readonly property Item explicit: indicators.explicit

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
}
