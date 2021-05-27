/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Animations {
    id: _animations
    property Item bridge: null
    readonly property bool bridgeIsActive: bridge !== null

    active: ref.animations.active
    readonly property bool hasThicknessAnimation: ref.animations.hasThicknessAnimation //redefined to make it readonly and switchable

    //! animations tracking
    needBothAxis: ref.animations.needBothAxis
    needLength: ref.animations.needLength
    needThickness: ref.animations.needThickness

    //! animations properties
    duration: ref.animations.duration
    speedFactor: ref.animations.speedFactor

    //! parabolic effect animations
    hoverPixelSensitivity: ref.animations.hoverPixelSensitivity

    //! requirements
    requirements: local.requirements

    readonly property AbilityDefinition.Animations local: AbilityDefinition.Animations{
        active: local.speedFactor.current !== 0
        hoverPixelSensitivity: 1
        speedFactor.normal: active ? speedFactor.current : 1.0

        duration.small: LatteCore.Environment.shortDuration
        duration.large: LatteCore.Environment.longDuration
    }

    Item {
        id: ref
        readonly property Item animations: bridge ? bridge.animations.host : local
    }

    //! Bridge - Client assignment
    onBridgeIsActiveChanged: {
        if (bridgeIsActive) {
            bridge.animations.client = _animations;
        }
    }

    Component.onCompleted: {
        if (bridgeIsActive) {
            bridge.animations.client = _animations;
        }
    }

    Component.onDestruction: {
        if (bridgeIsActive) {
            bridge.animations.client = null;
        }
    }
}
