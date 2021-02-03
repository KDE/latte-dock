/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    restoreZoomIsBlocked: restoreZoomIsBlockedFromApplet || (view && view.contextMenuIsShown)

    currentParabolicItem: view ? view.parabolic.currentItem : null
}
