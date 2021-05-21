/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

BridgeItem {
    id: parabolicBridge

    function clientRequestUpdateLowerItemScale(newScale, step) {
        host.sglUpdateLowerItemScale(appletIndex-1, newScale, step);
    }

    function clientRequestUpdateHigherItemScale(newScale, step) {
        host.sglUpdateHigherItemScale(appletIndex+1, newScale, step);
    }

  /*Be Careful, needs to be considered how to not create
    endless recursion because each one calls the other.
    If client in inside a container and as such is using
    a parabolic host then the parabolic host clearZoom signal
    should be called when needed.

    Connections {
        target: client ? client : null
        onSglClearZoom: {
            if (parabolicBridge.host) {
                parabolicBridge.host.sglClearZoom();
            }
        }
    }*/

    Connections {
        target: host ? host : null
        onSglClearZoom: {
            if (parabolicBridge.client) {
                parabolicBridge.client.sglClearZoom();
            }
        }
    }
}
