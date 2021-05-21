/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import "./indicators" as IndicatorTypes

Item{
    property bool isEnabled: false

    readonly property real padding: Math.max(info.minLengthPadding, info.lengthPadding)

    property string type: "org.kde.latte.default"

    property QtObject configuration: null
    property QtObject resources: null   

    property Component indicatorComponent

    property IndicatorTypes.IndicatorInfo info: IndicatorTypes.IndicatorInfo{
        enabledForApplets: true
        needsIconColors: false
        needsMouseEventCoordinates: false

        providesFrontLayer: false
        providesHoveredAnimation: false
        providesClickedAnimation: false

        extraMaskThickness: 0

        minThicknessPadding: 0
        minLengthPadding: 0

        lengthPadding: 0.08
        appletLengthPadding: -1

        svgPaths: []
    }
}

