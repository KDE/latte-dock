/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Debug {
    property Item bridge: null

    eventsSinkEnabled: ref.debug.eventsSinkEnabled
    graphicsEnabled: ref.debug.graphicsEnabled
    inputMaskEnabled: ref.debug.inputMaskEnabled
    layouterEnabled: ref.debug.layouterEnabled
    localGeometryEnabled: ref.debug.localGeometryEnabled
    maskEnabled: ref.debug.maskEnabled
    overloadedIconsEnabled: ref.debug.overloadedIconsEnabled
    spacersEnabled: ref.debug.spacersEnabled
    timersEnabled: ref.debug.timersEnabled
    windowEnabled: ref.debug.windowEnabled

    readonly property AbilityDefinition.Debug local: AbilityDefinition.Debug {}

    Item {
        id: ref
        readonly property Item debug: bridge ? bridge.debug : local
    }
}
