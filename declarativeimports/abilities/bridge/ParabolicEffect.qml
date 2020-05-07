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
    id: parabolicBridge
    property int appletIndex: -1

    property Item host: null
    property Item client: null

    function clientRequestUpdateLowerItemScale(newScale, step) {
        host.sglUpdateLowerItemScale(appletIndex-1, newScale, step);
    }

    function clientRequestUpdateHigherItemScale(newScale, step) {
        host.sglUpdateHigherItemScale(appletIndex+1, newScale, step);
    }

  /*Be Careful, needs to be considered how to not create
    endless recursion because each one calls the other.
    If applet in inside a container and as such is using
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
