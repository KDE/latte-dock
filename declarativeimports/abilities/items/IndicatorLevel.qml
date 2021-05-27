/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore

import "./indicators" as IndicatorItem

Loader {
    id: indicatorLevelLoader
    active: level.isDrawn
            && level.indicator
            && level.indicator.host.isEnabled
            && level.indicator.configuration
            && (level.isBackground || (level.isForeground && level.indicator.host.info.providesFrontLayer))

    sourceComponent: level.indicator.host ? level.indicator.host.indicatorComponent : null

    readonly property IndicatorItem.LevelOptions level: IndicatorItem.LevelOptions{}
}

