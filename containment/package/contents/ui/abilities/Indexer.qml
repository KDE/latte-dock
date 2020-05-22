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

import "./privates" as Ability

Ability.IndexerPrivate {
    objectName: "IndexerAbilityHost"

    //! do not update during dragging/moving applets inConfigureAppletsMode
    updateIsBlocked: (root.dragOverlay && root.dragOverlay.pressed)
                     || layouter.appletsInParentChange

    function getClientBridge(index) {
        if (clientsBridges.length<=0) {
            return false;
        }

        var ibl = clientsBridges.length;

        for(var i=0; i<ibl; ++i) {
            if (clientsBridges[i].appletIndex === index) {
                return clientsBridges[i];
            }
        }

        return false;
    }

    function appletIdForVisibleIndex(itemVisibleIndex) {
        var sLayout = layouts.startLayout;
        for (var i=0; i<sLayout.children.length; ++i){
            var appletItem = sLayout.children[i];

            if (visibleIndexBelongsAtApplet(appletItem, itemVisibleIndex)) {
                return appletItem.index;
            }
        }

        var mLayout = layouts.mainLayout;
        for (var i=0; i<mLayout.children.length; ++i){
            var appletItem = sLayout.children[i];

            if (visibleIndexBelongsAtApplet(appletItem, itemVisibleIndex)) {
                return appletItem.index;
            }
        }

        var eLayout = layouts.endLayout;
        for (var i=0; i<eLayout.children.length; ++i){
            var appletItem = sLayout.children[i];

            if (visibleIndexBelongsAtApplet(appletItem, itemVisibleIndex)) {
                return appletItem.index;
            }
        }

        return -1;
    }
}
