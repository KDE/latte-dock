/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
                    if (appletItem.isExpanded) {
                        return true;
                    }

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
