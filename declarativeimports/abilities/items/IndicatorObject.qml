/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

Item{
    id: _indicator
    //! abilities
    property Item metrics: null
    property Item animations: null
    property Item host: null

    //! options
    property bool isTask: false
    property bool isApplet: false
    property bool isEmptySpace: false /*since 0.9.3*/

    property bool isLauncher: false
    property bool isStartup: false
    property bool isWindow: false

    property bool isActive: false
    property bool isGroup: false
    property bool isHovered: false
    property bool isMinimized: false
    property bool isPressed: false
    property bool inAttention: false
    property bool inRemoving: false

    property bool isSquare: true

    property bool hasActive: false
    property bool hasMinimized: false
    property bool hasShown: false
    property int windowsCount: 0
    property int windowsMinimizedCount: 0

    property int currentIconSize: _indicator.metrics ? _indicator.metrics.iconSize : 48
    property int maxIconSize: _indicator.metrics ? _indicator.metrics.maxIconSize : 48
    property real scaleFactor: 1
    property real panelOpacity: 1.0
    property color shadowColor: "black"

    property bool animationsEnabled: _indicator.animations ? _indicator.animations.active : true
    property real durationTime: _indicator.animations ? _indicator.animations.speedFactor.current : 2

    property bool progressVisible: false /*since 0.9.2*/
    property real progress: 0 /*since 0.9.2*/

    property int screenEdgeMargin: _indicator.metrics ? Math.max(0, _indicator.metrics.margin.screenEdge) : 0 /*since 0.10*/

    property QtObject palette: null

    //!icon colors
    property color iconBackgroundColor: "black"
    property color iconGlowColor: "white"

    //! grouped options
    property Item shared: host
    property QtObject configuration: shared ? shared.configuration : null
    property QtObject resources: shared ?  shared.resources : null


    //! Public Exposed API for Indicator Object
    readonly property Item publicApi: Item {
        readonly property alias isTask: _indicator.isTask
        readonly property alias isApplet: _indicator.isApplet
        readonly property alias isEmptySpace: _indicator.isEmptySpace

        readonly property alias isLauncher: _indicator.isLauncher
        readonly property alias isStartup: _indicator.isStartup
        readonly property alias isWindow: _indicator.isWindow

        readonly property alias isActive: _indicator.isActive
        readonly property alias isGroup: _indicator.isGroup
        readonly property alias isHovered: _indicator.isHovered
        readonly property alias isMinimized: _indicator.isMinimized
        readonly property alias isPressed: _indicator.isPressed
        readonly property alias inAttention: _indicator.inAttention
        readonly property alias inRemoving: _indicator.inRemoving

        readonly property alias isSquare: _indicator.isSquare

        readonly property alias hasActive: _indicator.hasActive
        readonly property alias hasMinimized: _indicator.hasMinimized
        readonly property alias hasShown: _indicator.hasShown
        readonly property alias windowsCount: _indicator.windowsCount
        readonly property alias windowsMinimizedCount: _indicator.windowsMinimizedCount

        readonly property alias currentIconSize: _indicator.currentIconSize
        readonly property alias maxIconSize: _indicator.maxIconSize
        readonly property alias scaleFactor: _indicator.scaleFactor
        readonly property alias panelOpacity: _indicator.panelOpacity
        readonly property alias shadowColor: _indicator.shadowColor

        readonly property alias animationsEnabled: _indicator.animationsEnabled
        readonly property alias durationTime: _indicator.durationTime

        readonly property alias progressVisible: _indicator.progressVisible
        readonly property alias progress: _indicator.progress

        readonly property alias screenEdgeMargin: _indicator.screenEdgeMargin

        readonly property QtObject palette: _indicator.palette ? _indicator.palette : theme

        //!icon colors
        readonly property alias iconBackgroundColor: _indicator.iconBackgroundColor
        readonly property alias iconGlowColor: _indicator.iconGlowColor

        //! grouped options
        readonly property alias shared: _indicator.shared
        readonly property alias configuration: _indicator.configuration
        readonly property alias resources: _indicator.resources
    }
}
