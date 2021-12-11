/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.MyView {
    id: apis

    property QtObject view: null

    function inCurrentLayout() {
        return view && view.layout && view.layout.isCurrent();
    }

    function action(name) {
        return view ? view.action(name) : null;
    }

    readonly property Item publicApi: Item {
        readonly property alias isReady: apis.isReady
        readonly property alias groupId: apis.groupId

        readonly property alias inNormalState: apis.inNormalState

        readonly property alias isHidden: apis.isHidden
        readonly property alias isShownPartially: apis.isShownPartially
        readonly property alias isShownFully: apis.isShownFully
        readonly property alias isHidingBlocked: apis.isHidingBlocked

        readonly property alias inEditMode: apis.inEditMode
        readonly property alias inConfigureAppletsMode: apis.inConfigureAppletsMode

        readonly property alias inSlidingIn: apis.inSlidingIn
        readonly property alias inSlidingOut: apis.inSlidingOut
        readonly property alias inRelocationAnimation: apis.inRelocationAnimation
        readonly property alias inRelocationHiding: apis.inRelocationHiding

        readonly property alias badgesIn3DStyle: apis.badgesIn3DStyle

        readonly property alias alignment: apis.alignment
        readonly property alias visibilityMode: apis.visibilityMode

        readonly property alias backgroundOpacity: apis.backgroundOpacity

        readonly property alias lastUsedActivity: apis.lastUsedActivity

        readonly property alias appletsLayoutGeometry: apis.appletsLayoutGeometry
        readonly property alias screenGeometry: apis.screenGeometry

        readonly property alias containmentActions: apis.containmentActions

        readonly property alias itemShadow: apis.itemShadow

        readonly property alias palette: apis.palette

        function action(name) {
            return apis.action(name);
        }

        function inCurrentLayout() {
            return apis.inCurrentLayout();
        }
    }
}
