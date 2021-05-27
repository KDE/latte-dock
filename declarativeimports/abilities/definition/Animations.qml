/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import "./animations" as AnimationsTypes

Item {
    property bool active: false
    readonly property bool hasThicknessAnimation: (needBothAxis.count>0) || (needThickness.count>0)
    //! mouse sensitivity in pixels for parabolic effect hover animation
    property int hoverPixelSensitivity: 1

    //! animations properties
    property AnimationsTypes.Duration duration: AnimationsTypes.Duration {
        large: 500
        proposed: 1000
        small: 250
    }

    property AnimationsTypes.SpeedFactor speedFactor: AnimationsTypes.SpeedFactor {
        id: _speedFactor
        normal: 1.0
        current: 1.0
    }

    property AnimationsTypes.Requirements requirements: AnimationsTypes.Requirements {
        zoomFactor: 1.0
    }

    //! animations tracking
    property AnimationsTypes.Tracker needBothAxis: AnimationsTypes.Tracker{
        count: 0
        events: []

        //function addEvent(event);
        //function removeEvent(event);
    }
    property AnimationsTypes.Tracker needLength: AnimationsTypes.Tracker{
        count: 0
        events: []

        //function addEvent(event);
        //function removeEvent(event);
    }
    property AnimationsTypes.Tracker needThickness: AnimationsTypes.Tracker{
        count: 0
        events: []

        //function addEvent(event);
        //function removeEvent(event);
    }
}
