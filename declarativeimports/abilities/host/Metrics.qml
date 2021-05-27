/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Metrics {
    id: apis

    readonly property Item publicApi: Item {
        readonly property alias iconSize: apis.iconSize
        readonly property alias maxIconSize: apis.maxIconSize
        readonly property alias backgroundThickness: apis.backgroundThickness

        readonly property alias margin: apis.margin
        readonly property alias marginsArea: apis.marginsArea
        readonly property alias mask: apis.mask
        readonly property alias padding: apis.padding
        readonly property alias totals: apis.totals
    }
}
