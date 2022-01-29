/*
    SPDX-FileCopyrightText: 2022 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.latte.core 0.2 as LatteCore

AppletsContainer {
    id: parabolicContainer

    property real parabolicOffset: root.myView.alignment === LatteCore.Types.Center ? (tailOffsetLength - headOffsetLength) / 2 : 0
    property real tailOffsetLength: 0
    property real headOffsetLength: 0

    readonly property int hiddenItemsCount: (parabolic.spread - 1)/2
    readonly property int animationTime: animations.speedFactor.normal * (1.2*animations.duration.small)

    Behavior on parabolicOffset {
        id: animatedParabolicOffsetBehavior
        enabled: !parabolic.directRenderingEnabled || restoreAnimation.running
        NumberAnimation {
            duration: 3 * parabolicContainer.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on parabolicOffset {
        enabled: !animatedParabolicOffsetBehavior.enabled
        NumberAnimation { duration: 0 }
    }

    ParallelAnimation{
        id: restoreAnimation

        PropertyAnimation {
            target: parabolicContainer
            property: "tailOffsetLength"
            to: 0
            duration: 4 * parabolicContainer.animationTime
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: parabolicContainer
            property: "headOffsetLength"
            to: 0
            duration: 4 * parabolicContainer.animationTime
            easing.type: Easing.InCubic
        }
    }

    function updateLength(istail, newScales) {
        var nextFactor = 0;

        var nextLength = 0;
        for (var i=0; i<hiddenItemsCount; ++i) {
            if (i<newScales.length) {
                nextLength += (newScales[i] - 1)  * metrics.totals.length;
            }
        }

        if (istail) {
            tailOffsetLength = nextLength + 1; //1px. added because it was missing from calculations for some reason
        } else {
            headOffsetLength = nextLength + 1; //1px. added because it was missing from calculations for some reason
        }
    }

    function sltUpdateLowerItemScale(delegateIndex, newScales) {
        if (delegateIndex !== beginIndex-1) {
            return;
        }

        var istail = true;
        updateLength(true, newScales);
    }

    function sltUpdateHigherItemScale(delegateIndex, newScales) {
        if (delegateIndex !== layouter.mainLayout.lastVisibleIndex + 1) {
            return;
        }

        var ishead = false;
        updateLength(ishead, newScales);
    }

    function sltClearZoom(){
        restoreAnimation.start();
    }

    Component.onCompleted: {
        parabolic.sglClearZoom.connect(sltClearZoom);
        parabolic.sglUpdateLowerItemScale.connect(sltUpdateLowerItemScale);
        parabolic.sglUpdateHigherItemScale.connect(sltUpdateHigherItemScale);
    }

    Component.onDestruction: {
        parabolic.sglClearZoom.disconnect(sltClearZoom);
        parabolic.sglUpdateLowerItemScale.disconnect(sltUpdateLowerItemScale);
        parabolic.sglUpdateHigherItemScale.disconnect(sltUpdateHigherItemScale);
    }

}
