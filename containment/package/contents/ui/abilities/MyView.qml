/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore

import "./privates" as Ability

Ability.MyViewPrivate {
    view: latteView
    isReady: view && view.visibility && view.layout && view.effects && view.positioner
    groupId: view ? view.groupId : -1

    inNormalState: ((animations.needBothAxis.count === 0) && (animations.needLength.count === 0))
                                 || (isReady && view.visibility.isHidden && !view.visibility.containsMouse && animations.needThickness.count === 0)
    isHidden: isReady && view.visibility.isHidden
    isShownPartially:  isReady && (inSlidingIn || inSlidingOut)
    isShownFully: isReady && !isHidden && !inSlidingIn && !inSlidingOut

    isHidingBlocked: isHidingBlockedFromApplet || layouts.contextMenuIsShown

    inEditMode: root.editMode
    inConfigureAppletsMode: root.inConfigureAppletsMode

    inSlidingIn: visibilityManager.inSlidingIn
    inSlidingOut: visibilityManager.inSlidingOut
    inRelocationAnimation: isReady && view.positioner.inRelocationAnimation
    inRelocationHiding: visibilityManager.inRelocationHiding

    badgesIn3DStyle: universalSettings ? universalSettings.badges3DStyle : true

    alignment: plasmoid.configuration.alignment
    visibilityMode: isReady ? view.visibility.mode : LatteCore.Types.None

    backgroundOpacity: background.currentOpacity

    lastUsedActivity: isReady ? view.layout.lastUsedActivity : ""

    appletsLayoutGeometry: isReady ? view.effects.appletsLayoutGeometry : Qt.rect(0, 0, -1, -1)
    screenGeometry: isReady ? view.screenGeometry : plasmoid.screenGeometry

    containmentActions: isReady ? view.containmentActions : []

    itemShadow.isEnabled: plasmoid.configuration.appletShadowsEnabled
    itemShadow.size: itemShadow.isEnabled ? (0.5*metrics.iconSize) * (plasmoid.configuration.shadowSize/100) : 0
    itemShadow.shadowColor: "#" + myView.decimalToHex(myView.itemShadowOpacity) + myView.itemShadowCurrentColor
    itemShadow.shadowSolidColor: "#" + myView.itemShadowCurrentColor
}



