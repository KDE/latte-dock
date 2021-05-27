/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Item {
    property int count: 0
    property var events: []

    function addEvent(event) {
        if (events.indexOf(event) < 0) {
            events.push(event);
            count = count + 1;
        }
    }

    function removeEvent(event) {
        var pos = events.indexOf(event);
        if (pos >= 0) {
            events.splice(pos, 1);
            count = count - 1;
        }
    }
}
