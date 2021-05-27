/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import "./paraboliceffect" as ParabolicEffectTypes

Item {
    property bool isEnabled: false
    property bool restoreZoomIsBlocked: false

    property ParabolicEffectTypes.Factor factor: ParabolicEffectTypes.Factor{
        zoom: 1.6
        maxZoom: 1.6
    }

    readonly property ParabolicEffectTypes.PrivateProperties _privates: ParabolicEffectTypes.PrivateProperties {
        directRenderingEnabled: false
    }

    property Item currentParabolicItem: null

    signal sglClearZoom();
    signal sglUpdateLowerItemScale(int delegateIndex, real newScale, real step);
    signal sglUpdateHigherItemScale(int delegateIndex, real newScale, real step);
}
