/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

BridgeItem {
    id: launchersBridge
    readonly property bool isConnected: host && client

    onClientChanged: {
        if (client) {
            if (host.appletIdStealingDroppedLaunchers !== launchersBridge.appletIndex) {
                client.disabledIsStealingDroppedLaunchers();
            }
        }
    }

    Connections {
        target: client
        onIsStealingDroppedLaunchersChanged: {
            if (isConnected && client.isStealingDroppedLaunchers) {
                host.currentAppletStealingDroppedLaunchers(appletIndex);
            }
        }
    }

    Connections {
        target: host
        onCurrentAppletStealingDroppedLaunchers: {
            if (appletIndex !== id && client) {
                client.disabledIsStealingDroppedLaunchers();
            }
        }
    }
}
