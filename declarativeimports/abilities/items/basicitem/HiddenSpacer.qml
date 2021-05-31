/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.latte.core 0.2 as LatteCore

Item{
    id: hiddenSpacer
    width: plasmoid.formFactor === PlasmaCore.Types.Vertical ? abilityItem.parabolicItem.width : nHiddenSize
    height: plasmoid.formFactor === PlasmaCore.Types.Vertical ? nHiddenSize : abilityItem.parabolicItem.height

    visible: (rightSpacer ? index === abilityItem.abilities.indexer.lastVisibleItemIndex : index === abilityItem.abilities.indexer.firstVisibleItemIndex)
             || (separatorSpace > 0)
             || abilityItem.isHiddenSpacerForcedShow

    property bool neighbourSeparator: rightSpacer ? abilityItem.headItemIsVisibleSeparator : abilityItem.tailItemIsVisibleSeparator
    property bool rightSpacer: false

    property int separatorSpace: neighbourSeparator && !abilityItem.isSeparator && abilityItem.abilities.parabolic.isEnabled
                                 && !(abilityItem.abilities.indexer.separators.length>0 && abilityItem.isSeparatorInRealLength) ?
                                     (LatteCore.Environment.separatorLength/2) : 0

    property real nScale: 0
    property real nHiddenSize: 0

    readonly property int maxSize: Math.max(0,Math.ceil(0.55*abilityItem.abilities.metrics.iconSize) - abilityItem.abilities.metrics.totals.lengthEdges)

    Binding{
        target: hiddenSpacer
        property: "nHiddenSize"
        when: !hiddenSizeDelayer.running && itemIndex > -1 //! helps to solve BUGLOCALREF: #1
        value: {
            if (abilityItem.isHidden) {
                return 0;
            } else if (!abilityItem.parabolicItem.isUpdatingOnlySpacers) {
                return (nScale > 0) ? (maxSize * nScale) + separatorSpace : separatorSpace;
            } else {
                return (nScale > 0) ? (abilityItem.abilities.metrics.iconSize * nScale) + separatorSpace : separatorSpace;
            }
        }
    }

    Connections{
        target: abilityItem
        onContainsMouseChanged: {
            if (!abilityItem.containsMouse && !abilityItem.isHiddenSpacerForcedShow) {
                hiddenSpacer.nScale = 0;
            }
        }
    }

    Behavior on nHiddenSize {
        id: animatedBehavior
        enabled: abilityItem.isHiddenSpacerAnimated || restoreAnimation.running
        NumberAnimation{ duration: 3 * abilityItem.animationTime }
    }

    Behavior on nHiddenSize {
        id: directBehavior
        enabled: !animatedBehavior.enabled
        NumberAnimation { duration: 0 }
    }

    Loader{
        active: abilityItem.abilities.debug.spacersEnabled

        sourceComponent: Rectangle{
            width: abilityItem.isHorizontal ? hiddenSpacer.width : 1
            height: abilityItem.isHorizontal ? 1 : hiddenSpacer.height
            x: abilityItem.isHorizontal ? 0 : hiddenSpacer.width/2
            y: abilityItem.isHorizontal ? hiddenSpacer.height/2 : 0

            border.width: 1
            border.color: "red"
            color: "transparent"
        }
    }

    //! Delayer
    onSeparatorSpaceChanged: {
        if (!hiddenSizeDelayer.running) {
            hiddenSizeDelayer.start();
        }
    }

    //! BUGLOCALREF: #1
    //! Timer that helps to avoid binding loops from head/tailItemIsVisibleSeparator by delaying to update the spacer size
    //! This solution needs confirmation. That specific binding loop is very nasty as it breaks the tasks model consistency
    //! somehow when switching between different activities and the model contains separators that become shown and hidden
    //! during the activity change
    Timer {
        id: hiddenSizeDelayer
        interval: 400
    }
}
