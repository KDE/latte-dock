/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.8
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte.core 0.2 as LatteCore

import org.kde.latte.abilities.items 0.1 as AbilityItems

import "./basicitem" as BasicItemParts

Item{
    id: abilityItem
    signal mousePressed(int x, int y, int button);
    signal mouseReleased(int x, int y, int button);
    signal shortcutRequestedActivate();
    signal shortcutRequestedNewInstance();
    signal taskLauncherActivated();
    signal taskGroupedWindowAdded();
    signal taskGroupedWindowRemoved();

    anchors.bottom: (parent && abilityItem.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (parent && abilityItem.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (parent && abilityItem.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (parent && abilityItem.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    width: {
        if (!visible)
            return 0;

        if (isSeparator) {
            if (abilityItem.isVertical) {
                return abilityItem.abilities.metrics.totals.thickness + abilityItem.abilities.metrics.margin.screenEdge;
            } else {
                if (isSeparatorInRealLength) {
                    return LatteCore.Environment.separatorLength+2*abilityItem.abilities.metrics.margin.length;
                }
            }

            return 0;
        }

        if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            return _parabolicItem.width;
        } else {
            return hiddenSpacerLeft.width + _parabolicItem.width + hiddenSpacerRight.width;
        }
    }

    height: {
        if (!visible)
            return 0;

        if (isSeparator) {
            if (abilityItem.isHorizontal) {
                return abilityItem.abilities.metrics.totals.thickness + abilityItem.abilities.metrics.margin.screenEdge;
            } else {
                if (isSeparatorInRealLength) {
                    return LatteCore.Environment.separatorLength+2*abilityItem.abilities.metrics.margin.length;
                }
            }

            return 0;
        }

        if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            return hiddenSpacerLeft.height + abilityItem.parabolicItem.height + hiddenSpacerRight.height;
        } else {
            return _parabolicItem.height;
        }
    }

    property bool isMonochromaticForcedContentItem: false

    property bool isHiddenSpacerAnimated: true
    property bool isHiddenSpacerForcedShow: false

    property bool isHidden: false
    property bool isSeparator: false
    property bool isSeparatorInRealLength: false
    property bool isSeparatorHidden: false
    readonly property bool isSeparatorVisible: isSeparator && !isSeparatorHidden && !tailItemIsSeparator
    property bool preserveIndicatorInInitialPosition: false

    property bool containsMouse: false

    property string thinTooltipText: ""

    property Item abilities: null
    property Item contentItem: null
    property Item monochromizedItem: null

    readonly property bool isHorizontal: !isVertical
    readonly property bool isVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property bool parabolicAreaContainsMouse: parabolicEventsAreaLoader.active && parabolicEventsAreaLoader.item.containsMouse
    readonly property int parabolicAreaLastMousePos: {
        if (parabolicEventsAreaLoader.active) {
            return Math.round(plasmoid.formFactor === PlasmaCore.Types.Horizontal ? parabolicEventsAreaLoader.item.lastMouseX : parabolicEventsAreaLoader.item.lastMouseY);
        }

        return 0;
    }

    readonly property int location: {
        if (plasmoid.location === PlasmaCore.Types.LeftEdge
                || plasmoid.location === PlasmaCore.Types.RightEdge
                || plasmoid.location === PlasmaCore.Types.TopEdge) {
            return plasmoid.location;
        }

        return PlasmaCore.Types.BottomEdge;
    }

    readonly property bool isFirstItemInContainer: abilityItem.abilities.containment.isFirstAppletInContainment && (index === abilityItem.abilities.indexer.firstVisibleItemIndex)
    readonly property bool isLastItemInContainer: abilityItem.abilities.containment.isLastAppletInContainment && (index === abilityItem.abilities.indexer.lastVisibleItemIndex)

    readonly property int itemIndex: index
    readonly property int animationTime: (abilityItem.abilities.animations.active ? abilityItem.abilities.animations.speedFactor.current : 2) * (1.2 * abilityItem.abilities.animations.duration.small)
    property int iconAnimatedOffsetX: 0
    property int iconAnimatedOffsetY: 0
    readonly property int iconOffsetX: iconAnimatedOffsetX + indicatorBackLayer.level.requested.iconOffsetX
    readonly property int iconOffsetY: iconAnimatedOffsetY + indicatorBackLayer.level.requested.iconOffsetY
    readonly property int iconTransformOrigin: indicatorBackLayer.level.requested.iconTransformOrigin
    readonly property real iconOpacity: indicatorBackLayer.level.requested.iconOpacity
    readonly property real iconRotation: indicatorBackLayer.level.requested.iconRotation
    readonly property real iconScale: indicatorBackLayer.level.requested.iconScale

    readonly property bool isIndicatorTaskLauncherAnimationRunning: (indicatorBackLayer.level.requested.isTaskLauncherAnimationRunning
                                                                     || indicatorFrontLayer.level.requested.isTaskLauncherAnimationRunning)

    readonly property alias indicator: abilityIndicatorObj
    readonly property alias parabolicItem: _parabolicItem
    readonly property alias restoreAnimation: _restoreAnimation
    readonly property alias tooltipVisualParent: _parabolicItem.titleTooltipVisualParent

    Component.onCompleted: {
        abilityItem.abilities.parabolic.sglClearZoom.connect(slotClearZoom);
    }

    Component.onDestruction: {
        abilityItem.abilities.parabolic.sglClearZoom.disconnect(slotClearZoom);
    }

    Connections {
        target: abilityItem.abilities.shortcuts
        onSglActivateEntryAtIndex: {
            if (!abilityItem.abilities.shortcuts.isEnabled) {
                return;
            }

            var shortcutIndex = abilityItem.abilities.shortcuts.shortcutIndex(abilityItem.itemIndex);

            if (shortcutIndex === entryIndex) {
                abilityItem.shortcutRequestedActivate();
            }
        }

        onSglNewInstanceForEntryAtIndex: {
            if (!abilityItem.abilities.shortcuts.isEnabled) {
                return;
            }

            var shortcutIndex = abilityItem.abilities.shortcuts.shortcutIndex(taskItem.itemIndex);

            if (shortcutIndex === entryIndex) {
                abilityItem.shortcutRequestedNewInstance();
            }
        }
    }

    //! separators flags
    readonly property bool tailItemIsSeparator: {
        if (isSeparator || index < 0 ) {
            return false;
        }

        var tail = index - 1;

        while(tail>=0
              && abilityItem.abilities.indexer.hidden.indexOf(tail)>=0 /*ignore hidden items but not hidden separators*/
              && abilityItem.abilities.indexer.separators.indexOf(tail)<0) {
            tail = tail - 1;
        }

        var hasTailItemSeparator = abilityItem.abilities.indexer.separators.indexOf(tail)>=0;

        if (!hasTailItemSeparator && itemIndex === abilityItem.abilities.indexer.firstVisibleItemIndex){
            return abilityItem.abilities.indexer.tailAppletIsSeparator;
        }

        return hasTailItemSeparator;
    }

    readonly property bool tailItemIsVisibleSeparator: {
        if (isSeparator || index < 0 || !tailItemIsSeparator ) {
            return false;
        }

        var tail = index - 1;

        while(tail>=0 && abilityItem.abilities.indexer.hidden.indexOf(tail)>=0) {
            tail = tail - 1;
        }

        var hasTailItemSeparator = abilityItem.abilities.indexer.separators.indexOf(tail)>=0 && abilityItem.abilities.indexer.hidden.indexOf(tail)<0;

        if (!hasTailItemSeparator && itemIndex === abilityItem.abilities.indexer.firstVisibleItemIndex){
            return abilityItem.abilities.indexer.tailAppletIsSeparator;
        }

        return hasTailItemSeparator;
    }

    readonly property bool headItemIsSeparator: {
        if (isSeparator || index < 0 ) {
            return false;
        }

        var head = index + 1;

        while(head>=0
              && abilityItem.abilities.indexer.hidden.indexOf(head)>=0 /*ignore hidden items but not hidden separators*/
              && abilityItem.abilities.indexer.separators.indexOf(head)<0) {
            head = head + 1;
        }

        var hasHeadItemSeparator = abilityItem.abilities.indexer.separators.indexOf(head)>=0;

        if (!hasHeadItemSeparator && itemIndex === abilityItem.abilities.indexer.lastVisibleItemIndex){
            return abilityItem.abilities.indexer.headAppletIsSeparator;
        }

        return hasHeadItemSeparator;
    }

    readonly property bool headItemIsVisibleSeparator: {
        if (isSeparator || index < 0 || !headItemIsSeparator) {
            return false;
        }

        var head = index + 1;

        while(head>=0 && abilityItem.abilities.indexer.hidden.indexOf(head)>=0) {
            head = head + 1;
        }

        var hasHeadItemSeparator = abilityItem.abilities.indexer.separators.indexOf(head)>=0 && abilityItem.abilities.indexer.hidden.indexOf(head)<0;

        if (!hasHeadItemSeparator && itemIndex === abilityItem.abilities.indexer.lastVisibleItemIndex){
            return abilityItem.abilities.indexer.headAppletIsSeparator;
        }

        return hasHeadItemSeparator;
    }



    Behavior on opacity {
        NumberAnimation { duration: abilityItem.abilities.animations.speedFactor.current * abilityItem.abilities.animations.duration.large }
    }

    Loader{
        anchors.fill: parent
        active: abilityItem.abilities.debug.graphicsEnabled

        sourceComponent: Rectangle{
            anchors.fill: parent
            color: "transparent"
            border.color:  "blue"
            border.width: 1
        }
    }

    BasicItemParts.SeparatorItem {}

    Flow{
        width: parent.width
        height: parent.height
        z:10

        BasicItemParts.HiddenSpacer{ id:hiddenSpacerLeft}

        Item{
            width: _parabolicItem.width
            height: _parabolicItem.height

            AbilityItems.IndicatorObject {
                id: abilityIndicatorObj
                animations: abilityItem.abilities.animations
                metrics: abilityItem.abilities.metrics
                host: abilityItem.abilities.indicators

                iconBackgroundColor: "transparent"
                iconGlowColor: "transparent"
            }

            //! Indicator Back Layer
            BasicItemParts.IndicatorLevel{
                id: indicatorBackLayer
                level.isBackground: true
                level.indicator: abilityIndicatorObj

                Loader{
                    anchors.fill: parent
                    active: abilityItem.abilities.debug.graphicsEnabled
                    sourceComponent: Rectangle{
                        color: "transparent"
                        border.width: 1
                        border.color: "purple"
                        opacity: 0.4
                    }
                }
            }

            BasicItemParts.ParabolicItem{id: _parabolicItem}

            //! Indicator Front Layer
            BasicItemParts.IndicatorLevel{
                id: indicatorFrontLayer
                level.isForeground: true
                level.indicator: abilityIndicatorObj
            }
        }

        BasicItemParts.HiddenSpacer{ id:hiddenSpacerRight; rightSpacer: true }
    }// Flow with hidden spacers inside

    Loader {
        id: parabolicEventsAreaLoader
        active: isParabolicEnabled || isThinTooltipEnabled
        width: abilityItem.isHorizontal ? abilityItem.width : abilityItem.abilities.metrics.mask.thickness.zoomedForItems
        height: abilityItem.isHorizontal ? abilityItem.abilities.metrics.mask.thickness.zoomedForItems : abilityItem.height
        z:10000       
        sourceComponent: BasicItemParts.ParabolicEventsArea{}

        readonly property bool isParabolicEnabled: abilityItem.abilities.parabolic.isEnabled
        readonly property bool isThinTooltipEnabled: abilityItem.abilities.thinTooltip.isEnabled && abilityItem.thinTooltipText !== ""

        states:[
            State{
                name: "top"
                when: abilityItem.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: parabolicEventsAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "left"
                when: abilityItem.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: parabolicEventsAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "right"
                when: abilityItem.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: parabolicEventsAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "bottom"
                when: abilityItem.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: parabolicEventsAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
            }
        ]
    }

    BasicItemParts.RestoreAnimation{id: _restoreAnimation}

    function slotClearZoom(){
        restoreAnimation.start();
    }

    //BEGIN states
    states: [
        State {
            name: "grid"
            when: abilityItem.parent && abilities.isLayoutGridContainer

            AnchorChanges {
                target: abilityItem
                anchors{ top:undefined; bottom:undefined; left:undefined; right:undefined;}
            }
        },
        State {
            name: "left"
            when: abilityItem.parent && !abilities.isLayoutGridContainer && (abilities.location === PlasmaCore.Types.LeftEdge)

            AnchorChanges {
                target: abilityItem
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;}
            }
        },
        State {
            name: "right"
            when: abilityItem.parent && !abilities.isLayoutGridContainer && (abilities.location === PlasmaCore.Types.RightEdge)

            AnchorChanges {
                target: abilityItem
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;}
            }
        },
        State {
            name: "bottom"
            when: abilityItem.parent && !abilities.isLayoutGridContainer && (abilities.location === PlasmaCore.Types.BottomEdge)

            AnchorChanges {
                target: abilityItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;}
            }
        },
        State {
            name: "top"
            when: abilityItem.parent && !abilities.isLayoutGridContainer && (abilities.location === PlasmaCore.Types.TopEdge)

            AnchorChanges {
                target: abilityItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined;}
            }
        }
    ]
    //END states
}
