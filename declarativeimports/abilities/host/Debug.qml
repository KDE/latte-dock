/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Debug {
    id: apis

    readonly property Item publicApi: Item {
        readonly property alias eventsSinkEnabled: apis.eventsSinkEnabled
        readonly property alias graphicsEnabled: apis.graphicsEnabled
        readonly property alias inputMaskEnabled: apis.inputMaskEnabled
        readonly property alias layouterEnabled: apis.layouterEnabled
        readonly property alias localGeometryEnabled: apis.localGeometryEnabled
        readonly property alias maskEnabled: apis.maskEnabled
        readonly property alias spacersEnabled: apis.spacersEnabled
        readonly property alias timersEnabled: apis.timersEnabled
        readonly property alias windowEnabled: apis.windowEnabled
        readonly property alias overloadedIconsEnabled: apis.overloadedIconsEnabled
    }
}
