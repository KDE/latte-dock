/*
    SPDX-FileCopyrightText: 2022 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.latte.core 0.2 as LatteCore

AppletsContainer {
    id: parabolicContainer

    property real parabolicOffset: root.myView.alignment === LatteCore.Types.Center ? tailOffsetLength - headOffsetLength : 0

    property real tailFactor: 0
    property real headFactor: 0

    readonly property int hiddenItemsCount: (parabolic.spread - 1)/2
    readonly property int animationTime: animations.speedFactor.normal * (1.2*animations.duration.small)
    readonly property real normalHiddenLength: hiddenItemsCount * metrics.totals.length
    readonly property real tailOffsetLength: (tailFactor * normalHiddenLength) / 2
    readonly property real headOffsetLength: (headFactor * normalHiddenLength) / 2

    Behavior on tailFactor {
        id: animatedTailFactorBehavior
        enabled: !parabolic.directRenderingEnabled || restoreAnimation.running
        NumberAnimation {
            duration: 3 * parabolicContainer.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on tailFactor {
        enabled: !animatedTailFactorBehavior.enabled
        NumberAnimation { duration: 0 }
    }

    Behavior on headFactor {
        id: animatedHeadFactorBehavior
        enabled: !parabolic.directRenderingEnabled || restoreAnimation.running
        NumberAnimation {
            duration: 3 * parabolicContainer.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on headFactor {
        enabled: !animatedHeadFactorBehavior.enabled
        NumberAnimation { duration: 0 }
    }

    ParallelAnimation{
        id: restoreAnimation

        PropertyAnimation {
            target: parabolicContainer
            property: "tailFactor"
            to: 0
            duration: 3 * parabolicContainer.animationTime
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: parabolicContainer
            property: "headFactor"
            to: 0
            duration: 3 * parabolicContainer.animationTime
            easing.type: Easing.InCubic
        }
    }

    function updateFactor(istail, newScales) {
        var nextFactor = 0;

        for (var i=0; i<hiddenItemsCount; ++i) {
            if (i<newScales.length) {
                nextFactor += ((newScales[i] - 1) / hiddenItemsCount);
            }
        }

        if (istail) {
            tailFactor = nextFactor;
        } else {
            headFactor = nextFactor;
        }
    }

    function sltUpdateLowerItemScale(delegateIndex, newScales) {
        if (delegateIndex !== beginIndex-1) {
            return;
        }

        var istail = true;
        updateFactor(true, newScales);
    }

    function sltUpdateHigherItemScale(delegateIndex, newScales) {
        if (delegateIndex !== layouter.mainLayout.lastVisibleIndex + 1) {
            return;
        }

        var ishead = false;
        updateFactor(ishead, newScales);
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
