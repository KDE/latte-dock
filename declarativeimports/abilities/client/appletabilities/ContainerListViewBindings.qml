/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

Item {
    Binding {
        target: _abilityContainer.layout.parent
        property: "width"
        value: plasmoid.formFactor !== PlasmaCore.Types.Vertical ? _abilityContainer.layout.parent.contentWidth :
                                                                   _abilityContainer.thickness
    }

    Binding {
        target: _abilityContainer.layout.parent
        property: "height"
        value: plasmoid.formFactor !== PlasmaCore.Types.Vertical ? _abilityContainer.thickness :
                                                                   _abilityContainer.layout.parent.contentHeight
    }

    Binding {
        target: _abilityContainer.layout.parent
        property: "boundsBehavior"
        value: Flickable.StopAtBounds
    }

    Binding {
        target: _abilityContainer.layout.parent
        property: "orientation"
        value: plasmoid.formFactor === PlasmaCore.Types.Vertical ? Qt.Vertical : Qt.Horizontal
    }
}
