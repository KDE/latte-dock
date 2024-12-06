/*
    SPDX-FileCopyrightText: 2022 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.latte.core 0.2 as LatteCore

Item {
    id: edgeSpacer
    width: length
    height: length

    readonly property bool isParabolicEdgeSpacer: true
    readonly property bool isHidden: true

    readonly property bool isAutoFillApplet: false
    readonly property bool isInternalViewSplitter: false
    readonly property bool isPlaceHolder: false
    readonly property bool isTailSpacer: index < parent.beginIndex
    readonly property bool isHeadSpacer: index >= parent.beginIndex

    readonly property int animationTime: animations.speedFactor.normal * (1.2*animations.duration.small)
    readonly property int hiddenItemsCount: (parabolic.spread - 1)/2

    property int index: -1
    property real length: 0

    Behavior on length {
        id: animatedLengthBehavior
        enabled: !parabolic.directRenderingEnabled || restoreAnimation.running
        NumberAnimation {
            duration: 3 * edgeSpacer.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on length {
        enabled: !animatedLengthBehavior.enabled
        NumberAnimation { duration: 0 }
    }

    ParallelAnimation{
        id: restoreAnimation

        PropertyAnimation {
            target: edgeSpacer
            property: "length"
            to: 0
            duration: 4 * edgeSpacer.animationTime
            easing.type: Easing.InCubic
        }
    }

    function updateScale(istail, newScales) {
        var nextFactor = 0;
        for (var i=0; i<hiddenItemsCount; ++i) {
            if (i<newScales.length) {
                nextFactor += (newScales[i] - 1);
            }
        }

        length = nextFactor * metrics.totals.length;
    }

    function sltUpdateLowerItemScale(delegateIndex, newScales) {
        if (!isTailSpacer || delegateIndex !== index) {
            return;
        }

        if (myView.alignment === LatteCore.Types.Center || myView.alignment === LatteCore.Types.Justify) {
            updateScale(isTailSpacer, newScales);
        } else {
            length = 0;
        }

        //! clear side neighbours
        parabolic.sglUpdateLowerItemScale(index - 1, [1]);
    }

    function sltUpdateHigherItemScale(delegateIndex, newScales) {
        if (!isHeadSpacer || delegateIndex !== index) {
            return;
        }

        if (myView.alignment === LatteCore.Types.Center || myView.alignment === LatteCore.Types.Justify) {
            updateScale(isHeadSpacer, newScales);
        } else {
            length = 0;
        }

        //! clear side neighbours
        parabolic.sglUpdateHigherItemScale(index + 1, [1]);
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

    Loader{
        anchors.fill: parent
        active: debug.spacersEnabled

        sourceComponent: Rectangle{
            color: "#44ff0000"
            border.width: 1
            border.color: "red"
        }
    }
}
