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
    id: indicatorBridge
    anchors.fill: parent

    readonly property bool active: true

    /* Indicators Properties in order use them*/
    readonly property bool isTask: false
    readonly property bool isApplet: false
    readonly property bool isEmptySpace: true /*since 0.9.3*/

    readonly property bool isLauncher: false
    readonly property bool isStartup: false
    readonly property bool isWindow: false

    readonly property bool isActive: false
    readonly property bool isGroup: false
    readonly property bool isHovered: false
    readonly property bool isMinimized: false
    readonly property bool isPressed: mainArea.pressed
    readonly property bool inAttention: false
    readonly property bool inRemoving: false

    readonly property bool isSquare: false

    readonly property bool hasActive: false
    readonly property bool hasMinimized: false
    readonly property bool hasShown: false
    readonly property int windowsCount: 0
    readonly property int windowsMinimizedCount: 0

    readonly property int currentIconSize: root.iconSize
    readonly property int maxIconSize: root.maxIconSize
    readonly property real scaleFactor: 1
    readonly property real panelOpacity: root.currentPanelOpacity
    readonly property color shadowColor: root.appShadowColorSolid

    readonly property bool animationsEnabled: root.animationsEnabled
    readonly property real durationTime: root.durationTime

    readonly property bool progressVisible: false /*since 0.9.2*/
    readonly property real progress: 0 /*since 0.9.2*/

    readonly property int screenEdgeMargin: 0 /*since 0.10*/

    readonly property QtObject palette: colorizerManager.applyTheme

    //!icon colors
    property color iconBackgroundColor: "brown"
    property color iconGlowColor: "pink"

    //! grouped options
    readonly property Item shared: indicators
    readonly property QtObject configuration: indicators.configuration
    readonly property QtObject resources: indicators.resources
}
