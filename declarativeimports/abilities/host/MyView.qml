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

        readonly property alias inNormalState: apis.inNormalState

        readonly property alias isHidden: apis.isHidden
        readonly property alias isShownPartially: apis.isShownPartially
        readonly property alias isShownFully: apis.isShownFully
        readonly property alias isHidingBlocked: apis.isHidingBlocked

        readonly property alias inSlidingIn: apis.inSlidingIn
        readonly property alias inSlidingOut: apis.inSlidingOut
        readonly property alias inRelocationAnimation: apis.inRelocationAnimation
        readonly property alias inRelocationHiding: apis.inRelocationHiding

        readonly property alias badgesIn3DStyle: apis.badgesIn3DStyle

        readonly property alias alignment: apis.alignment
        readonly property alias visibilityMode: apis.visibilityMode

        readonly property alias backgroundOpacity: apis.backgroundOpacity

        readonly property alias lastUsedActivity: apis.lastUsedActivity

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
