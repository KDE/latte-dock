/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

import "./privates" as Ability

Ability.ParabolicEffectPrivate {
    isEnabled: factor.zoom>1 && !root.inConfigureAppletsMode
    factor.zoom: {
        var storedZoom = LatteCore.WindowSystem.compositingActive && animations.active ? ( 1 + (plasmoid.configuration.zoomLevel / 20) ) : 1;
        var maxSize = storedZoom * metrics.iconSize;
        //! round to nearest odd number
        maxSize = 2 * Math.round(Math.round(maxSize) / 2);

        //! this way we make sure that the iconSize at the maximum of its parabolic effect is an integer
        return (maxSize/metrics.iconSize);
    }
    factor.maxZoom: Math.max(factor.zoom, animations.requirements.zoomFactor)
    factor.marginThicknessZoomInPercentage: settings ? settings.thicknessMarginInfluence : 1.0 //100%
    restoreZoomIsBlocked: restoreZoomIsBlockedFromApplet || layouts.contextMenuIsShown
    spread: settings ? settings.parabolicSpread : 3

    currentParabolicItem: view ? view.parabolic.currentItem : null
}
