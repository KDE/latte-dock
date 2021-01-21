/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.core 0.2 as LatteCore

import org.kde.latte.abilities.host 0.1 as AbilityHost

AbilityHost.MyView {
    isReady: latteView

    inNormalState: ((animations.needBothAxis.count === 0) && (animations.needLength.count === 0))
                                 || (latteView && latteView.visibility.isHidden && !latteView.visibility.containsMouse && animations.needThickness.count === 0)
    isHidden: isReady && latteView.visibility.isHidden
    isShownPartially:  isReady && (inSlidingIn || inSlidingOut)
    isShownFully: isReady && !isHidden && !inSlidingIn && !inSlidingOut

    inSlidingIn: visibilityManager.inSlidingIn
    inSlidingOut: visibilityManager.inSlidingOut
    inRelocationAnimation: latteView && latteView.positioner && latteView.positioner.inRelocationAnimation
    inRelocationHiding: visibilityManager.inRelocationHiding

    alignment: plasmoid.configuration.alignment
    visibilityMode: latteView ? latteView.visibility.mode : LatteCore.Types.None

    containmentActions: isReady ? latteView.containmentActions() : []
}



