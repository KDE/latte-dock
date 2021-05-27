/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import "./privates" as Ability

Ability.LayouterPrivate {
    id: _layouter

    property bool appletsInParentChange: false

    readonly property bool inNormalFillCalculationsState: animations.needBothAxis.count === 0
                                                          && animations.needThickness.count === 0
                                                          && (animations.needLength.count === 0 || root.dragInfo.entered) /*update when the user drags a plasmoid on the view*/
                                                          && !appletsInParentChange
                                                        //  && (!dragOverlay || (dragOverlay && !dragOverlay.pressed)) /*do not update during moving/dragging applets*/

    readonly property bool maxMetricsInHigherPriority: root.minLength === root.maxLength

    function updateSizeForAppletsInFill() {
        if (!updateSizeForAppletsInFillTimer.running) {
            _layouter._updateSizeForAppletsInFill();
            updateSizeForAppletsInFillTimer.start();
        }
    }

    onInNormalFillCalculationsStateChanged: {
        if (inNormalFillCalculationsState) {
            _layouter.updateSizeForAppletsInFill();
        }
    }

    onMaxLengthChanged: {
        _layouter.updateSizeForAppletsInFill();
    }

    Connections {
        target: layouts
        onContentsLengthChanged: _layouter.updateSizeForAppletsInFill();
    }

    Connections {
        target: metrics
        onIconSizeChanged: _layouter.updateSizeForAppletsInFill();
    }

    Connections {
        target: plasmoid
        onFormFactorChanged: _layouter.updateSizeForAppletsInFill();
    }

    Connections {
        target: visibilityManager
        onInNormalStateChanged: {
            if (visibilityManager.inNormalState) {
                _layouter.updateSizeForAppletsInFill();
            }
        }
    }

    //! This timer is needed in order to reduce the calls to heavy cpu function
    //! updateSizeForAppletsInFill()
    Timer{
        id: updateSizeForAppletsInFillTimer
        interval: 75
        onTriggered: _layouter._updateSizeForAppletsInFill();
    }
}
