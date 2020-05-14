/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import "./privates" as Ability

Ability.LayouterPrivate {
    id: _layouter

    property bool appletsInParentChange: false

    readonly property bool inNormalFillCalculationsState: animations.needBothAxis.count === 0
                                                          && animations.needThickness.count === 0
                                                          && ((animations.needLength.count === 0)
                                                              || (animations.needLength.count===1 && editModeVisual.inEditMode))
                                                          && (!dragOverlay || (dragOverlay && !dragOverlay.pressed)) /*do not update during moving/dragging applets*/
                                                          && !appletsInParentChange

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

    //! This timer is needed in order to reduce the calls to heavy cpu function
    //! updateSizeForAppletsInFill()
    Timer{
        id: updateSizeForAppletsInFillTimer
        interval: 75
        onTriggered: _layouter._updateSizeForAppletsInFill();
    }
}
