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

import org.kde.latte.abilities.host 0.1 as AbilityHost

AbilityHost.ThinTooltip {
    id: _tooltip
    property Item layouts: null
    property QtObject view: null

    property bool showIsBlockedFromApplet: false

    //! do not update during dragging/moving applets inConfigureAppletsMode
    readonly property bool isBindingUpdateEnabled: !(root.dragOverlay && root.dragOverlay.pressed)

    Binding{
        target: _tooltip
        property: "showIsBlockedFromApplet"
        when: isBindingUpdateEnabled
        value: {
            var grid;

            for (var l=0; l<=2; ++l) {
                if (l===0) {
                    grid = layouts.startLayout;
                } else if (l===1) {
                    grid = layouts.mainLayout;
                } else if (l===2) {
                    grid = layouts.endLayout;
                }

                for (var i=0; i<grid.children.length; ++i){
                    var appletItem = grid.children[i];
                    if (appletItem
                            && appletItem.communicator
                            && appletItem.communicator.thinTooltipIsSupported
                            && appletItem.communicator.bridge.thinTooltip.client.local.showIsBlocked) {
                        return true;
                    }
                }
            }

            return false;
        }
    }

}
