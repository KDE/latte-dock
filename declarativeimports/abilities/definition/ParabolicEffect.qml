/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import "./paraboliceffect" as ParabolicEffectTypes

Item {
    property bool isEnabled: false
    property bool restoreZoomIsBlocked: false

    property int spread: 3

    property ParabolicEffectTypes.Factor factor: ParabolicEffectTypes.Factor{
        zoom: 1.6
        maxZoom: 1.6
    }

    readonly property ParabolicEffectTypes.PrivateProperties _privates: ParabolicEffectTypes.PrivateProperties {
        directRenderingEnabled: false
    }

    property Item currentParabolicItem: null

    signal sglClearZoom();
    signal sglUpdateLowerItemScale(int delegateIndex, variant newScales);
    signal sglUpdateHigherItemScale(int delegateIndex, variant newScales);

    readonly property int _spreadSteps: (spread - 1) / 2

    function applyParabolicEffect(itemIndex, itemMousePosition, itemLength) {
        var percentage = Math.max(0, Math.min(1, itemMousePosition / itemLength));

        //! left scales
        var leftScales = [];
        for (var i=_spreadSteps; i>=1; --i) {
            leftScales.push(linearEffect(1-percentage, i, _spreadSteps));
        }
        leftScales.push(1); //! clearing

        //! right scales
        var rightScales = [];
        for (var j=_spreadSteps; j>=1; --j) {
            rightScales.push(linearEffect(percentage, j, _spreadSteps));
        }
        rightScales.push(1); //! clearing

        var reversed = Qt.application.layoutDirection === Qt.RightToLeft && (plasmoid.formFactor === PlasmaCore.Types.Horizontal);

        if (reversed) {
            var temp = leftScales;
            leftScales = rightScales;
            rightScales = temp;
        }

        sglUpdateHigherItemScale(itemIndex+1, rightScales);
        sglUpdateLowerItemScale(itemIndex-1, leftScales);

        return {leftScale:leftScales[0], rightScale:rightScales[0]};
    }

    function linearEffect(mousePosPercentage, partIndex, partsCount) {
        var part = 1/partsCount;
        var min = (partIndex-1) * part;
        var max = partIndex * part;
        var x = min + (max-min) * mousePosPercentage;
        var yl = (factor.zoom - 1) * x;
        return 1+yl;
    }
}
