/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

///////Restore Zoom Animation/////
SequentialAnimation{
    id: restoreAnimation

    PropertyAnimation {
        target: abilityItem.parabolicItem
        property: "zoom"
        to: 1
        duration: 3 * abilityItem.animationTime
        easing.type: Easing.InCubic
    }
}
