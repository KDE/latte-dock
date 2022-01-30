/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Item {
    property real zoom: 1.6
    property real maxZoom: 1.6
    readonly property real marginThicknessZoom: 1.0 + ((zoom-1.0) * marginThicknessZoomInPercentage)

    property real marginThicknessZoomInPercentage: 1.0 //100&
}
