/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.latte.core 0.2 as LatteCore

Item{
    id: hiddenSpacer

    //we add one missing pixel from calculations
    width: root.isHorizontal ? nHiddenSize : wrapper.width
    height: root.isHorizontal ? wrapper.height : nHiddenSize

    ///check also if this is the first/last plasmoid in anylayout
    visible: (isRightSpacer ? appletItem.lastAppletInContainer : appletItem.firstAppletInContainer) || separatorSpace>0

    property bool hasNeighbourSeparator: isRightSpacer ? appletItem.headAppletIsSeparator : appletItem.tailAppletIsSeparator

    property int separatorSpace: hasNeighbourSeparator && appletItem.parabolic.isEnabled ? LatteCore.Environment.separatorLength / 2 : 0

    property real nHiddenSize: {
        if (isSeparator || !communicator.requires.lengthMarginsEnabled) {
            return 0;
        }

        return (nScale > 0) ? (appletItem.spacersMaxSize * nScale) + separatorSpace : separatorSpace
    }

    property bool isRightSpacer: false

    property real nScale: 0

    Behavior on nHiddenSize {
        id: animatedBehavior
        enabled: !appletItem.parabolic.directRenderingEnabled || restoreAnimation.running
        NumberAnimation { duration: 3 * appletItem.animationTime }
    }

    Behavior on nHiddenSize {
        id: directBehavior
        enabled: !animatedBehavior.enabled
        NumberAnimation { duration: 0 }
    }

    Connections{
        target: appletItem
        onContainsMouseChanged: {
            if (!appletItem.containsMouse) {
                hiddenSpacer.nScale = 0;
            }
        }
    }

    Loader{
        active: appletItem.debug.spacersEnabled

        sourceComponent: Rectangle{
            width: !root.isVertical ? hiddenSpacer.width : 1
            height: !root.isVertical ? 1 : hiddenSpacer.height
            x: root.isVertical ? hiddenSpacer.width/2 : 0
            y: !root.isVertical ? hiddenSpacer.height/2 : 0

            border.width: 1
            border.color: "red"
            color: "transparent"
        }
    }
}
