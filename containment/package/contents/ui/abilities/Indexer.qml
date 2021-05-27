/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
                return appletItem.applet ? appletItem.applet.id : -1;
            }
        }

        var mLayout = layouts.mainLayout;
        for (var i=0; i<mLayout.children.length; ++i){
            var appletItem = mLayout.children[i];

            if (visibleIndexBelongsAtApplet(appletItem, itemVisibleIndex)) {
                return appletItem.applet ? appletItem.applet.id : -1;
            }
        }

        var eLayout = layouts.endLayout;
        for (var i=0; i<eLayout.children.length; ++i){
            var appletItem = eLayout.children[i];

            if (visibleIndexBelongsAtApplet(appletItem, itemVisibleIndex)) {
                return appletItem.applet ? appletItem.applet.id : -1;
            }
        }

        return -1;
    }
}
