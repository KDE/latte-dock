/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore

import org.kde.latte.abilities.host 0.1 as AbilityHost

import org.kde.latte.private.containment 0.1 as LatteContainment

AbilityHost.MyView {
    id: _myView
    property Item layouts: null

    readonly property int itemShadowOpacity: (plasmoid.configuration.shadowOpacity/100) * 255
    readonly property int itemShadowMaxSize: itemShadow.isEnabled ? (0.5*metrics.maxIconSize) * (plasmoid.configuration.shadowSize/100) : 0

    readonly property real backgroundStoredOpacity: background.themeExtendedBackground && plasmoid.configuration.panelTransparency === -1 ?
                                                      background.themeExtendedBackground.maxOpacity : plasmoid.configuration.panelTransparency / 100

    readonly property string itemShadowCurrentColor: {
        if (plasmoid.configuration.shadowColorType === LatteContainment.Types.ThemeColorShadow) {
            var strC = String(theme.textColor);
            return strC.indexOf("#") === 0 ? strC.substr(1) : strC;
        } else if (plasmoid.configuration.shadowColorType === LatteContainment.Types.UserColorShadow) {
            return plasmoid.configuration.shadowColor;
        }

        return "080808"; // default
    }

    property bool isHidingBlockedFromApplet: false

    //! do not update during dragging/moving applets inConfigureAppletsMode
    readonly property bool isBindingUpdateEnabled: !(root.dragOverlay && root.dragOverlay.pressed)

    readonly property string hidingBlockedStr: "_myViewHost"

    onIsHidingBlockedChanged: {
        if (isHidingBlocked) {
            view.visibility.addBlockHidingEvent(_myView + hidingBlockedStr);
        } else {
            view.visibility.removeBlockHidingEvent(_myView + hidingBlockedStr);
        }
    }

    Binding{
        target: _myView
        property: "isHidingBlockedFromApplet"
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
                            && appletItem.communicator.myViewIsSupported
                            && appletItem.communicator.bridge.myView.client.local.isHidingBlocked) {
                        return true;
                    }
                }
            }

            return false;
        }
    }

    function decimalToHex(d, padding) {
        var hex = Number(d).toString(16);
        padding = typeof (padding) === "undefined" || padding === null ? padding = 2 : padding;

        while (hex.length < padding) {
            hex = "0" + hex;
        }

        return hex;
    }
}

