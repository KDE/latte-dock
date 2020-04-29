/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.1

MouseArea{
    id: __scrollArea

    property bool wheelIsBlocked: false
    property bool delayIsEnabled: true
    property int delay: 75

    signal scrolledUp(QtObject wheel);
    signal scrolledDown(QtObject wheel);

    onWheel: {
        if (delayIsEnabled && wheelIsBlocked){
            return;
        } else {
            wheelIsBlocked = true;
            __scrollDelayer.start();
        }

        var angle = wheel.angleDelta.y / 8;

        if (angle > 12) {
            //positive direction
            __scrollArea.scrolledUp(wheel);
        } else if (angle < -12) {
            //negative direction
            __scrollArea.scrolledDown(wheel);
        }
    }

    //! A timer is needed in order to handle also touchpads that probably
    //! send too many signals very fast. This way the signals per sec are limited.
    //! The user needs to have a steady normal scroll in order to not
    //! notice a annoying delay
    Timer{
        id: __scrollDelayer
        interval: __scrollArea.delay
        onTriggered: __scrollArea.wheelIsBlocked = false;
    }
}
