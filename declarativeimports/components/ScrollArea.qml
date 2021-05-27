/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
