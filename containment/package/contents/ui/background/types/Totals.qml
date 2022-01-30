/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item{
    id: totalsItem
    property int visualThickness: 0
    property int visualLength: 0
    property int visualMaxThickness: 0

    readonly property int shadowsLength: {
        if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
            return shadows.left+shadows.right;
        } else {
            return shadows.top+shadows.bottom;
        }
    }

    readonly property int shadowsThickness: {
        if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
            return shadows.top+shadows.bottom;
        } else {
            return shadows.left+shadows.right;
        }
    }

    readonly property int paddingsLength: {
        if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
            return paddings.left+paddings.right;
        } else {
            return paddings.top+paddings.bottom;
        }
    }

    /*updated through binding in order to not change its value when hiding screen gaps for maximized windows*/
    property int minThickness: 0

    Binding {
        target: totalsItem
        property: "minThickness"
        when: !(hideThickScreenGap || hideLengthScreenGaps)
        value: (paddings.headThickness + paddings.tailThickness)
    }
}
