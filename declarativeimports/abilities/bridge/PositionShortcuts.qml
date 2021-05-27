/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

BridgeItem {
    id: shortcutsBridge
    readonly property bool isConnected: host && client

    onIsConnectedChanged: {
        if (isConnected) {
            host.sglActivateEntryAtIndex.connect(client.sglActivateEntryAtIndex);
            host.sglNewInstanceForEntryAtIndex.connect(client.sglNewInstanceForEntryAtIndex);
        } else {
            host.sglActivateEntryAtIndex.disconnect(client.sglActivateEntryAtIndex);
            host.sglNewInstanceForEntryAtIndex.disconnect(client.sglNewInstanceForEntryAtIndex);
        }
    }

    onClientChanged: {
        if (client) {
            if (host.appletIdStealingPositionShortcuts !== shortcutsBridge.appletIndex) {
                client.disabledIsStealingGlobalPositionShortcuts();
            }
        }
    }
    Connections {
        target: client
        onIsStealingGlobalPositionShortcutsChanged: {
            if (isConnected && client.isStealingGlobalPositionShortcuts) {
                host.currentAppletStealingPositionShortcuts(appletIndex);
            }
        }
    }

    Connections {
        target: host
        onCurrentAppletStealingPositionShortcuts: {
            if (appletIndex !== id && client) {
                client.disabledIsStealingGlobalPositionShortcuts();
            }
        }
    }

    Component.onDestruction: {
        if (isConnected) {
            host.sglActivateEntryAtIndex.disconnect(client.sglActivateEntryAtIndex);
            host.sglNewInstanceForEntryAtIndex.disconnect(client.sglNewInstanceForEntryAtIndex);
        }
    }
}
