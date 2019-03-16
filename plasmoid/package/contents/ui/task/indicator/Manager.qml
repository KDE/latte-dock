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

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Item {
    id: indicatorManager
    readonly property bool active: root.indicatorsEnabled
    readonly property bool locked: inAttentionAnimation || inNewWindowAnimation

    /* Indicators Properties in order for indicators to use them*/
    readonly property bool isTask: true
    readonly property bool isApplet: false

    readonly property bool isLauncher: taskItem.isLauncher
    readonly property bool isStartup: taskItem.isStartup
    readonly property bool isWindow: taskItem.isWindow

    readonly property bool isActive: taskItem.hasActive || (root.showPreviews && (taskItem.isWindow || taskItem.isGroupParent)
                                                            && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === taskItem))
    readonly property bool isGroup: taskItem.isGroupParent
    readonly property bool isHovered: taskItem.containsMouse
    readonly property bool isMinimized: taskItem.isMinimized
    readonly property bool inAttention: taskItem.inAttention
    readonly property bool inRemoving: taskItem.inRemoveStage

    readonly property bool hasActive: taskItem.hasActive
    readonly property bool hasMinimized: taskItem.hasMinimized
    readonly property bool hasShown: taskItem.hasShown
    readonly property int windowsCount: taskItem.windowsCount
    readonly property int windowsMinimizedCount: taskItem.windowsMinimizedCount

    readonly property int currentIconSize: root.iconSize
    readonly property int maxIconSize: root.maxIconSize
    readonly property int durationTime: root.durationTime
    readonly property real scaleFactor: taskItem.wrapperAlias.mScale
    readonly property color shadowColor: root.appShadowColorSolid

    readonly property bool dotsOnActive: root.dotsOnActive
    readonly property bool multiColorEnabled: root.threeColorsWindows
    readonly property bool reversedEnabled: root.reverseLinesPosition
    readonly property int activeIndicatorType: root.activeIndicatorType
    readonly property bool usePlasmaTabsStyle: false

    //!glow options
    readonly property bool glowEnabled: root.showGlow
    readonly property int glowOption: root.glowOption
    readonly property real glowOpacity: root.glowOpacity
    readonly property bool glow3D: root.glow3D

    //!icon colors
    property color backgroundColor: taskItem.wrapperAlias.backgroundColor
    property color glowColor: taskItem.wrapperAlias.glowColor

    readonly property Component sourceComponent: {
        switch (root.indicatorStyle) {
        case Latte.Types.LatteIndicator:
            return latteIndicatorComponent;
        case Latte.Types.PlasmaIndicator:
            return plasmaIndicatorComponent;
        case Latte.Types.UnityIndicator:
            return unityIndicatorComponent;
        default:
            return latteIndicatorComponent;
        };
    }

    Component{
        id:latteIndicatorComponent
        Latte.LatteIndicator{}
    }

    Component{
        id: plasmaIndicatorComponent
        Latte.PlasmaIndicator{}
    }

    Component{
        id:unityIndicatorComponent
        Latte.UnityIndicator{}
    }
}
