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

import org.kde.latte 0.1 as Latte

Item{
    id: hiddenSpacer
    //we add one missing pixel from calculations
    width: root.vertical ? wrapper.width : nHiddenSize
    height: root.vertical ? nHiddenSize : wrapper.height

    visible: (rightSpacer ? index === parabolicManager.lastRealTaskIndex : index === parabolicManager.firstRealTaskIndex)
             || (separatorSpace > 0) || mainItemContainer.inAttentionAnimation
             || mainItemContainer.inFastRestoreAnimation || mainItemContainer.inMimicParabolicAnimation

    property bool neighbourSeparator: false
    //in case there is a neighbour separator, lastValidIndex is used in order to protect from false
    //when the task is removed
    property int indexUsed: index === -1 ? lastValidIndex : index
    property int separatorSpace: neighbourSeparator && !isSeparator
                                 && !(parabolicManager.hasInternalSeparator && root.dragSource) ?
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
        if (root.editMode) {
            neighbourSeparator = false;
        } else if (latteDock && index!==-1) {
            if (!rightSpacer) {
                neighbourSeparator = (parabolicManager.taskIsSeparator(itemIndex-1) && !isSeparator && itemIndex!==parabolicManager.firstRealTaskIndex)
                        || (latteDock.parabolicManager.isSeparator(latteDock.latteAppletPos-1) && parabolicManager.firstRealTaskIndex === itemIndex);
            } else {
                neighbourSeparator = (parabolicManager.taskIsSeparator(itemIndex+1) && !isSeparator && itemIndex!==parabolicManager.lastRealTaskIndex)
                        || (latteDock.parabolicManager.isSeparator(latteDock.latteAppletPos+1) && parabolicManager.lastRealTaskIndex === itemIndex );
            }

            /* if (launcherUrl.indexOf("kwrite") > -1 || launcherUrl.indexOf("dolphin") > -1 ) {
                var spacerName = "left";
                if (rightSpacer)
                    spacerName = "right";

                console.log(launcherUrl +":" + itemIndex +"," + spacerName + " _-_- " +neighbourSeparator);
            }*/
        }
    }

    Connections{
        target: root
        onEditModeChanged: hiddenSpacer.updateNeighbour();
        onLatteDockChanged: hiddenSpacer.updateNeighbour();
        // onInternalSeparatorHiddenChanged: hiddenSpacer.updateNeighbour();
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


    Behavior on separatorSpace {
        enabled: mainItemContainer.inFastRestoreAnimation || showWindowAnimation.running
        NumberAnimation{ duration: 3 * mainItemContainer.animationTime }
    }

    Behavior on separatorSpace {
        enabled: !mainItemContainer.inFastRestoreAnimation && !showWindowAnimation.running && !restoreAnimation.running
        NumberAnimation { duration: root.directRenderAnimationTime }
    }

    Loader{
        active: latteDock && latteDock.debugModeSpacers

        sourceComponent: Rectangle{
            width: !root.vertical ? hiddenSpacer.width : 1
            height: !root.vertical ? 1 : hiddenSpacer.height
            x: root.vertical ? hiddenSpacer.width/2 : 0
            y: !root.vertical ? hiddenSpacer.height/2 : 0

            border.width: 1
            border.color: "red"
            color: "transparent"
        }
    }
}
