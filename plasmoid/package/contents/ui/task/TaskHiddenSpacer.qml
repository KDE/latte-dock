/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0


Item{
    id: hiddenSpacer
    //we add one missing pixel from calculations
    width: root.vertical ? wrapper.width : nHiddenSize
    height: root.vertical ? nHiddenSize : wrapper.height

    visible: (rightSpacer ? index === icList.count - 1 : index === 0)
             || (separatorSpace > 0) || mainItemContainer.inAttentionAnimation
             || mainItemContainer.inFastRestoreAnimation || mainItemContainer.inMimicParabolicAnimation

    property bool neighbourSeparator: false
    //in case there is a neighbour separator, lastValidIndex is used in order to protect from false
    //when the task is removed
    property int indexUsed: index === -1 ? lastValidIndex : index
    property int separatorSpace: ((parabolicManager.internalSeparatorPos !== -1
                                   && !root.internalSeparatorHidden
                                   && parabolicManager.internalSeparatorPos === (rightSpacer ? indexUsed + 1 : indexUsed-1))
                                  || neighbourSeparator) && !isSeparator && !showWindowAnimation.running ?
                                     (2+root.iconMargin/2) : 0

    property bool rightSpacer: false

    property real nHiddenSize: {
        if (!inAttentionAnimation && !inMimicParabolicAnimation && !inFastRestoreAnimation) {
            return (nScale > 0) ? (mainItemContainer.spacersMaxSize * nScale) + separatorSpace : separatorSpace;
        } else {
            return (nScale > 0) ? (root.iconSize * nScale) + separatorSpace : separatorSpace;
        }
    }

    property real nScale: 0

    function updateNeighbour() {
        //index===-1 indicates that this item is removed
        if (latteDock && index!==-1) {
            if (!rightSpacer) {
                hiddenSpacer.neighbourSeparator = (latteDock.parabolicManager.isSeparator(latteDock.latteAppletPos-1)
                        && (indexUsed===0 || (indexUsed===1 && root.internalSeparatorPos===0 && root.internalSeparatorHidden)))
                        || (parabolicManager.taskIsSeparator(itemIndex-1) && !isSeparator);
            } else {
                hiddenSpacer.neighbourSeparator = (latteDock.parabolicManager.isSeparator(latteDock.latteAppletPos+1)
                        && ( indexUsed===root.tasksCount-1 || (indexUsed===root.tasksCount-2 && indexUsed>=0
                                                               && root.internalSeparatorPos===root.tasksCount-1
                                                               && root.internalSeparatorHidden)))
                        || (parabolicManager.taskIsSeparator(itemIndex+1));
            }
        }
    }

    Connections{
        target: root
        onLatteDockChanged: hiddenSpacer.updateNeighbour();
        onInternalSeparatorHiddenChanged: hiddenSpacer.updateNeighbour();
        onSeparatorsUpdated: hiddenSpacer.updateNeighbour();
    }

    Connections{
        target: latteDock
        onSeparatorsUpdated: hiddenSpacer.updateNeighbour();
        onLatteAppletPosChanged: hiddenSpacer.updateNeighbour();
    }

    Connections{
        target: mainItemContainer
        onItemIndexChanged: hiddenSpacer.updateNeighbour();
    }

    Component.onCompleted: hiddenSpacer.updateNeighbour();

    /*Behavior on nScale {
        enabled: !root.globalDirectRender
        NumberAnimation { duration: 3 * mainItemContainer.animationTime }
    }

    Behavior on nScale {
        enabled: root.globalDirectRender
        NumberAnimation { duration: root.directRenderAnimationTime }
    }*/

    Behavior on separatorSpace {
        enabled: !root.globalDirectRender
        NumberAnimation { duration: 3 * mainItemContainer.animationTime }
    }

    Behavior on separatorSpace {
        enabled: root.globalDirectRender
        NumberAnimation { duration: root.directRenderAnimationTime }
    }

    /*Rectangle{
        width: !root.vertical ? parent.width : 1
        height: !root.vertical ? 1 : parent.height
        x: root.vertical ? parent.width /2 : 0
        y: !root.vertical ? parent.height /2 : 0
        border.width: 1
        border.color: "red"
        color: "transparent"
    }*/
}
