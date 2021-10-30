/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Item {
    id: level
    signal mousePressed(int x, int y, int button);
    signal mouseReleased(int x, int y, int button);
    signal taskLauncherActivated();
    signal taskGroupedWindowAdded();
    signal taskGroupedWindowRemoved();

    property bool isBackground: true
    property bool isForeground: false

    property bool isDrawn: false

    readonly property Item requested: Item{
        property int iconOffsetX: 0
        property int iconOffsetY: 0
        property int iconTransformOrigin: Item.Center
        property real iconOpacity: 1.0
        property real iconRotation: 0
        property real iconScale: 1.0
        property bool isTaskLauncherAnimationRunning: false
    }

    property Item indicator: null

    onIsBackgroundChanged: {
        isForeground = !isBackground;
    }

    onIsForegroundChanged: {
        isBackground = !isForeground;
    }
}
