/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.8

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Environment {
    property Item bridge: null

    isGraphicsSystemAccelerated: ref.environment.isGraphicsSystemAccelerated

    readonly property AbilityDefinition.Environment local: AbilityDefinition.Environment {
        isGraphicsSystemAccelerated: (GraphicsInfo.api !== GraphicsInfo.Software) && (GraphicsInfo.api !== GraphicsInfo.Unknown)
    }

    Item {
        id: ref
        readonly property Item environment: bridge ? bridge.environment : local
    }
}
