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

import "../../indicators/options" as TaskIndicator

Item {
    id: indicatorBridge
    property bool taskIsValid: true

    readonly property bool active: indicators ? indicators.isEnabled : false
    readonly property bool locked: taskIsValid ? (inAttentionAnimation || inNewWindowAnimation) : false

    /* Indicators Properties in order for indicators to use them*/
    readonly property bool isTask: true
    readonly property bool isApplet: false
    readonly property bool isEmptySpace: false /*since 0.9.3*/

    readonly property bool isLauncher: taskIsValid ? taskItem.isLauncher || root.disableAllWindowsFunctionality : true
    readonly property bool isStartup: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.isStartup : false
    readonly property bool isWindow: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.isWindow : false

    readonly property bool isActive: taskIsValid && !root.disableAllWindowsFunctionality ? (taskItem.hasActive
                                                                                             || (root.showPreviews
                                                                                                 && (taskItem.isWindow || taskItem.isGroupParent)
                                                                                                 && windowsPreviewDlg.activeItem
                                                                                                 && (windowsPreviewDlg.activeItem === taskItem)) ) : false

    readonly property bool isGroup: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.isGroupParent : false
    readonly property bool isHovered: taskIsValid ? taskItem.containsMouse : false
    readonly property bool isMinimized: taskIsValid && !root.disableAllWindowsFunctionality? taskItem.isMinimized : false
    readonly property bool isPressed: taskIsValid ? taskItem.pressed : false
    readonly property bool inAttention: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.inAttention : false
    readonly property bool inRemoving: taskIsValid ? taskItem.inRemoveStage : false

    readonly property bool isSquare: true

    readonly property bool hasActive: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.hasActive : false
    readonly property bool hasMinimized: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.hasMinimized : false
    readonly property bool hasShown: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.hasShown : false
    readonly property int windowsCount: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.windowsCount : 0
    readonly property int windowsMinimizedCount: taskIsValid && !root.disableAllWindowsFunctionality ? taskItem.windowsMinimizedCount : 0

    readonly property int currentIconSize: taskIsValid ? taskItem.metrics.iconSize : metrics.iconSize
    readonly property int maxIconSize: taskIsValid ? taskItem.metrics.maxIconSize : metrics.iconSize
    readonly property real scaleFactor: taskIsValid ? taskItem.wrapperAlias.mScale : 1
    readonly property real panelOpacity: root.currentPanelOpacity
    readonly property color shadowColor: root.appShadowColorSolid

    readonly property bool animationsEnabled: taskIsValid ? taskItem.animations.active : animations.active
    readonly property real durationTime: taskIsValid ? taskItem.animations.speedFactor.current : animations.speedFactor.current

    readonly property bool progressVisible: wrapper.progressVisible /*since 0.9.2*/
    readonly property real progress: wrapper.progress /*since 0.9.2*/

    readonly property int screenEdgeMargin: taskIsValid ? taskItem.metrics.margin.screenEdge : metrics.margin.screenEdge /*since 0.10*/

    readonly property variant svgs: indicators ? indicators.svgs : []

    readonly property QtObject palette: enforceLattePalette ? latteBridge.palette.applyTheme : theme

    //!icon colors
    property color iconBackgroundColor: taskIsValid ? taskItem.wrapperAlias.backgroundColor : "black"
    property color iconGlowColor: taskIsValid ? taskItem.wrapperAlias.glowColor : "white"

    //! grouped options
    readonly property Item shared: indicators ? indicators : emptyOptions
    readonly property QtObject configuration: indicators ? indicators.configuration : null
    readonly property QtObject resources: indicators ? indicators.resources : null

    Item{id: emptyOptions}
}
