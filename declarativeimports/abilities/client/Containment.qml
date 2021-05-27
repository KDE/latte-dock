/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.Containment {
    property Item bridge: null

    isFirstAppletInContainment: ref.containment.isFirstAppletInContainment
    isLastAppletInContainment: ref.containment.isLastAppletInContainment

    alignment: ref.containment.alignment    

    readonly property AbilityDefinition.Containment local: AbilityDefinition.Containment {}

    Item {
        id: ref
        readonly property Item containment: bridge ? bridge.containment : local
    }
}
