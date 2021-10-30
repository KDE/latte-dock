/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Item {
    readonly property Item level: parent && parent.hasOwnProperty("level") ? parent.level : null
    readonly property Item indicator: {
        if (!level) {
            return null;
        }

        if (level.hasOwnProperty("bridge")) {
            return level.bridge;
        } else if (level.hasOwnProperty("indicator")) {
            return level.indicator.publicApi;
        }

        return null;
    }

    //! indicator is using main colors from icon e.g. unity style
    property bool needsIconColors: false
    //! indicator is using the mouse events coordinates e.g. for animations
    property bool needsMouseEventCoordinates: false
    //! indicator provides also foreground layer that should be drawn on top of the Items icons
    property bool providesFrontLayer: false
    //! indicator provides its own hovering animations. Latte should not provide
    //! any hovering animation in that case
    property bool providesHoveredAnimation: false
    //! indicator provides its own clicked animations. Latte should not provide
    //! any clicked animation in that case
    property bool providesClickedAnimation: false
    //! indicator provides its own inattention animation. Latte should not provide
    //! any inattention animation in that case.
    //! [since v0.10.4]
    property bool providesInAttentionAnimation: false
    //! indicator provides its own window added in group animation. Latte should not provide
    //! any window added in group animation in that case.
    //! [since v0.10.4]
    property bool providesGroupedWindowAddedAnimation: false
    //! indicator provides its own window removed from group animation. Latte should not provide
    //! any window removed from group animation in that case.
    //! [since v0.10.4]
    property bool providesGroupedWindowRemovedAnimation: false
    //! indicator provides its own task launcher triggered animation. Latte should not provide
    //! any task launcher triggered animation in that case.
    //! [since v0.10.4]
    property bool providesTaskLauncherAnimation: false

    //! this indicator will draw its own indicators for applets
    //! [since v0.10.0]
    property bool enabledForApplets: true

    //! this indicator visuals go out of the regular mask thickness and this variable sets
    //! the pixels needed outside of the mask thickness e.g. a glow
    property int extraMaskThickness: 0

    //! this indicator specifies a minimum thickness padding in order to be drawn correctly.
    //! Values below the specified value are ignored. This value is a percentage,
    //! e.g 0.06 -> 6%
    property real minThicknessPadding: 0
    //! this indicator specifies a minimum length padding in order to be drawn correctly.
    //! Values below the specified value are ignored. This value is a percentage,
    //! e.g 0.06 -> 6%
    property real minLengthPadding: 0

    //! this indicator specifies a different padding value for applets. This value is a percentage,
    //! -1, means disabled and the regular length padding is used
    //!  0.06 -> 6%
    //! [since v0.9.2]
    property real appletLengthPadding: -1

    //! this indicator specifies how much is going to be the items regular length padding
    //! as long as other more specialized paddings such as appletLengthPadding are not
    //! applied. This value is a percentage,
    //! e.g 0.06 -> 6%
    //! [since v0.10.0]
    property real lengthPadding: 0.08

    //! this indicator specifies how far would like to be from panel background edge
    //! it is totally panel background roundness related. By altering that value, indicators
    //! can get inside panel background big roundnesses. This value is a percentage,
    //! e.g 0.06 -> 6%
    //! [since v0.10.3]
    property real backgroundCornerMargin: 1.00

    //! svg image paths either from plasma theme or local files relevant to indicator "ui" directory
    //! in order to reduce resources usage
    property var svgImagePaths: []

    //! LEVEL.Requested API reference

    //! indicator requests different icon X offset from icon center
    //! [since v0.9.2]
    //property int level.requested.iconOffsetX: 0

    //! indicator requests different icon Y offset from icon center
    //! [since v0.9.2]
    //property int level.requested.iconOffsetY: 0

    //! indicator requests different icon tranform origin
    //! [since v0.10.4]
    //property int level.requested.iconTransformOrigin: Item.Center

    //! indicator requests different icon opacity
    //! [since v0.10.4]
    //property real level.requested.iconOpacity: 1.0

    //! indicator requests different icon rotation
    //! [since v0.10.4]
    //property real level.requested.iconRotation: 0

    //! indicator requests different icon scale
    //! [since v0.10.4]
    //property real level.requested.iconScale: 1.0

    //! indicator is playing a task launcher animation. It is used in combination with
    //! providesTaskLauncherAnimation
    //! [since v0.10.4]
    //property bool level.requested.isTaskLauncherAnimationRunning: false

    onLevelChanged: {
        if (level) {
            //! initialization on startup, this way when changing from indicator that changes them
            //! at an indicator that is not using them at all, their values are restored
            level.requested.iconOffsetX = 0;
            level.requested.iconOffsetY = 0;
            level.requested.iconTransformOrigin = Item.Center;
            level.requested.iconOpacity = 1.0;
            level.requested.iconRotation = 0;
            level.requested.iconScale = 1.0;
            level.requested.isTaskLauncherAnimationRunning = false;
        }
    }
}
