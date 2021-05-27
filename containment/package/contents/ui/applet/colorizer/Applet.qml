/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    ColorOverlay {
        id: colorizer
        anchors.fill: parent
        color: colorizerManager.applyColor
        source: wrapper
    }

    ///Shadow in applets
    Loader{
        id: colorizedAppletShadow
        anchors.fill: colorizer

        active: appletItem.environment.isGraphicsSystemAccelerated
                && plasmoid.configuration.appletShadowsEnabled
                && (appletColorizer.opacity>0)

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: appletItem.myView.itemShadow.shadowColor
            fast: true
            samples: 2 * radius
            source: colorizer
            radius: shadowSize
            verticalOffset: forcedShadow ? 0 : 2

            readonly property int shadowSize : appletItem.myView.itemShadow.size

            readonly property bool forcedShadow: root.forceTransparentPanel
                                                 && plasmoid.configuration.appletShadowsEnabled
                                                 && !appletItem.communicator.indexerIsSupported ? true : false
        }
    }
}
