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

import "./privates" as Ability

Ability.MyViewPrivate {
    view: latteView

    isReady: latteView

    inNormalState: ((animations.needBothAxis.count === 0) && (animations.needLength.count === 0))
                                 || (view && view.visibility.isHidden && !view.visibility.containsMouse && animations.needThickness.count === 0)
    isHidden: isReady && view.visibility.isHidden
    isShownPartially:  isReady && (inSlidingIn || inSlidingOut)
    isShownFully: isReady && !isHidden && !inSlidingIn && !inSlidingOut

    isHidingBlocked: isHidingBlockedFromApplet || (view && view.contextMenuIsShown)

    inEditMode: root.editMode
    inConfigureAppletsMode: root.inConfigureAppletsMode

    inSlidingIn: visibilityManager.inSlidingIn
    inSlidingOut: visibilityManager.inSlidingOut
    inRelocationAnimation: view && view.positioner && view.positioner.inRelocationAnimation
    inRelocationHiding: visibilityManager.inRelocationHiding

    badgesIn3DStyle: universalSettings ? universalSettings.badges3DStyle : true

    alignment: plasmoid.configuration.alignment
    visibilityMode: view ? view.visibility.mode : LatteCore.Types.None

    backgroundOpacity: background.currentOpacity

    lastUsedActivity: view && view.layout ? view.layout.lastUsedActivity : ""

    screenGeometry: view ? view.screenGeometry : plasmoid.screenGeometry

    containmentActions: isReady ? view.containmentActions : []

    itemShadow.isEnabled: plasmoid.configuration.appletShadowsEnabled
    itemShadow.size: itemShadow.isEnabled ? (0.5*metrics.iconSize) * (plasmoid.configuration.shadowSize/100) : 0
    itemShadow.shadowColor: "#" + myView.decimalToHex(myView.itemShadowOpacity) + myView.itemShadowCurrentColor
    itemShadow.shadowSolidColor: "#" + myView.itemShadowCurrentColor
}



