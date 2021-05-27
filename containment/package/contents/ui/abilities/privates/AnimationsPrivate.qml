/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.abilities.host 0.1 as AbilityHost

AbilityHost.Animations {
    id: animationsPrivate
    property Item layouts: null
    property Item metrics: null
    property QtObject settings: null

    property bool updateIsBlocked: false

    Binding{
        target: animationsPrivate.requirements
        property: "zoomFactor"
        when: !updateIsBlocked
        value: {
            var zoom = 1.0;

            var sLayout = layouts.startLayout;
            for (var i=0; i<sLayout.children.length; ++i){
                var appletItem = sLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.animationsAreSupported) {
                    zoom = Math.max(zoom, appletItem.communicator.bridge.animations.client.requirements.zoomFactor);
                }
            }

            var mLayout = layouts.mainLayout;
            for (var i=0; i<mLayout.children.length; ++i){
                var appletItem = mLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.animationsAreSupported) {
                    zoom = Math.max(zoom, appletItem.communicator.bridge.animations.client.requirements.zoomFactor);
                }
            }

            var eLayout = layouts.endLayout;
            for (var i=0; i<eLayout.children.length; ++i){
                var appletItem = eLayout.children[i];
                if (appletItem
                        && appletItem.index>=0
                        && appletItem.communicator
                        && appletItem.communicator.animationsAreSupported) {
                    zoom = Math.max(zoom, appletItem.communicator.bridge.animations.client.requirements.zoomFactor);
                }
            }

            return zoom;
        }
    }

}
