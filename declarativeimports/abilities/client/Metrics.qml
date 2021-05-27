/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Metrics {
    property Item bridge: null
    property Item parabolic: null

    iconSize: ref.metrics.iconSize
    maxIconSize: ref.metrics.maxIconSize
    backgroundThickness: ref.metrics.backgroundThickness

    margin: ref.metrics.margin
    marginsArea: ref.metrics.marginsArea
    mask: ref.metrics.mask
    padding: ref.metrics.padding
    totals: ref.metrics.totals

    readonly property AbilityDefinition.Metrics local: AbilityDefinition.Metrics {
        mask.thickness.normalForItems: local.totals.thickness
        mask.thickness.zoomedForItems: parabolic.factor.zoom * local.totals.thickness

        mask.thickness.maxNormalForItems: local.mask.thickness.normalForItems
        mask.thickness.maxZoomedForItems: local.mask.thickness.zoomedForItems
    }

    Item {
        id: ref
        readonly property Item metrics: bridge ? bridge.metrics : local
    }
}
