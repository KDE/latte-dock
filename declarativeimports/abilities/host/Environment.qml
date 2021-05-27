/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.8

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Environment {
    id: apis

    isGraphicsSystemAccelerated: (GraphicsInfo.api !== GraphicsInfo.Software) && (GraphicsInfo.api !== GraphicsInfo.Unknown)

    readonly property Item publicApi: Item {
        readonly property alias isGraphicsSystemAccelerated: apis.isGraphicsSystemAccelerated
    }
}
