/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Animations {
    id: apis

    readonly property Item publicApi: Item {
        //! animations properties
        readonly property alias active: apis.active
        readonly property alias hasThicknessAnimation:  apis.hasThicknessAnimation

        readonly property alias duration: apis.duration
        readonly property alias speedFactor: apis.speedFactor

        //! animations tracking
        readonly property alias needBothAxis: apis.needBothAxis
        readonly property alias needLength: apis.needLength
        readonly property alias needThickness: apis.needThickness

        //! parabolic effect animations
        readonly property alias hoverPixelSensitivity: apis.hoverPixelSensitivity
    }
}
