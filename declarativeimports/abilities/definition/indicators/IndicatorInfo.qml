/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Item{
    property bool enabledForApplets: true
    property bool needsIconColors: false
    property bool needsMouseEventCoordinates: false

    property bool providesFrontLayer: false
    property bool providesHoveredAnimation: false
    property bool providesClickedAnimation: false

    property int extraMaskThickness: 0

    property real minThicknessPadding: 0
    property real minLengthPadding: 0

    property real lengthPadding: 0.08
    property real appletLengthPadding: -1

    property variant svgPaths: []
}
