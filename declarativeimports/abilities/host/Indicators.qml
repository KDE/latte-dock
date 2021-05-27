/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Indicators {
    id: apis

    readonly property Item publicApi: Item {
        readonly property alias isEnabled: apis.isEnabled

        readonly property alias padding: apis.padding

        readonly property alias type: apis.type

        readonly property alias configuration: apis.configuration
        readonly property alias resources: apis.resources

        readonly property alias indicatorComponent: apis.indicatorComponent

        readonly property alias info: apis.info
    }
}
