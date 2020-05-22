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

import QtQuick 2.0

Item {
    id: shortcutsBridge
    property int appletIndex: -1

    property Item host: null
    property Item client: null

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
