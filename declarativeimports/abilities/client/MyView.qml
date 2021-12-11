/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.MyView {
    id: _myView
    property Item bridge: null
    readonly property bool isBridgeActive: bridge !== null

    isReady: ref.myView.isReady
    groupId: ref.myView.groupId

    inNormalState: ref.myView.inNormalState

    isHidden: ref.myView.isHidden
    isShownPartially: ref.myView.isShownPartially
    isShownFully: ref.myView.isShownFully
    isHidingBlocked: ref.myView.isHidingBlocked

    inEditMode: ref.myView.inEditMode
    inConfigureAppletsMode: ref.myView.inConfigureAppletsMode

    inSlidingIn: ref.myView.inSlidingIn
    inSlidingOut: ref.myView.inSlidingOut
    inRelocationAnimation: ref.myView.inRelocationAnimation
    inRelocationHiding: ref.myView.inRelocationHiding

    badgesIn3DStyle: ref.myView.badgesIn3DStyle

    alignment: ref.myView.alignment
    visibilityMode: ref.myView.visibilityMode

    backgroundOpacity: ref.myView.backgroundOpacity

    lastUsedActivity: ref.myView.lastUsedActivity

    screenGeometry: ref.myView.screenGeometry

    containmentActions: ref.myView.containmentActions

    itemShadow: ref.myView.itemShadow

    palette: bridge && bridge.applyPalette ? bridge.palette : theme

    readonly property AbilityDefinition.MyView local: AbilityDefinition.MyView {
        isShownFully: true
        inEditMode: plasmoid.userConfiguring
        inConfigureAppletsMode: plasmoid.userConfiguring
    }

    Item {
        id: ref
        readonly property Item myView: bridge && bridge.myView ? bridge.myView.host : local
    }

    //! Bridge - Client assignment
    onIsBridgeActiveChanged: {
        if (isBridgeActive) {
            bridge.myView.client = _myView;
        }
    }

    Component.onCompleted: {
        if (isBridgeActive) {
            bridge.myView.client = _myView;
        }
    }

    Component.onDestruction: {
        if (isBridgeActive) {
            bridge.myView.client = null;
        }
    }

    function inCurrentLayout() {
        return bridge && ref.myView.isReady ? ref.myView.inCurrentLayout() : true;
    }

    function action(name) {
        return bridge && ref.myView.isReady ? ref.myView.action(name) : null;
    }
}
