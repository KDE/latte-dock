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

Loader {
    id: indicatorLoader
    anchors.bottom: (root.position === PlasmaCore.Types.BottomPositioned) ? parent.bottom : undefined
    anchors.top: (root.position === PlasmaCore.Types.TopPositioned) ? parent.top : undefined
    anchors.left: (root.position === PlasmaCore.Types.LeftPositioned) ? parent.left : undefined
    anchors.right: (root.position === PlasmaCore.Types.RightPositioned) ? parent.right : undefined

    anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
    anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

    width: {
        if (locked) {
            return visualLockedWidth;
        }

        return !root.vertical ? taskItem.wrapperAlias.width - 2*taskItem.wrapperAlias.mScale*root.lengthExtMargin : taskItem.wrapperAlias.width;
    }

    height: {
        if (locked) {
            return visualLockedHeight;
        }

        return root.vertical ? taskItem.wrapperAlias.height - 2*taskItem.wrapperAlias.mScale*root.lengthExtMargin : taskItem.wrapperAlias.height;
    }

    active: root.indicatorsEnabled

    readonly property bool locked: inAttentionAnimation || inNewWindowAnimation

    property real visualLockedWidth: root.iconSize + root.internalWidthMargins
    property real visualLockedHeight: root.iconSize + root.internalHeightMargins

    /* Indicators Properties in order use them*/
    readonly property bool isTask: true
    readonly property bool isApplet: false

    readonly property bool isLauncher: taskItem.isLauncher
    readonly property bool isStartup: taskItem.isStartup
    readonly property bool isWindow: taskItem.isWindow

    readonly property bool isActive: taskItem.hasActive || (root.showPreviews && windowsPreviewDlg.activeItem && (windowsPreviewDlg.activeItem === taskItem))
    readonly property bool isGroup: taskItem.isGroupParent
    readonly property bool isMinimized: taskItem.isMinimized
    readonly property bool inAttention: taskItem.inAttention
    readonly property bool inRemoving: taskItem.inRemoveStage

    readonly property bool hasActive: taskItem.hasActive
    readonly property bool hasMinimized: taskItem.hasMinimized
    readonly property bool hasShown: taskItem.hasShown
    readonly property int windowsCount: taskItem.windowsCount

    readonly property int iconSize: root.iconSize
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

    sourceComponent: {
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

    //! Used when the indicators need icon colors in orde to be painted
    //! properly, for example the Unity indicator
    Binding {
        target: taskItem
        property: "indicatorNeedsIconColors"
        value: {
            if (indicatorLoader.active
                    && indicatorLoader.item
                    && indicatorLoader.item.hasOwnProperty("needsIconColors")) {
                return indicatorLoader.item.needsIconColors;
            }

            return false;
        }
    }
}
