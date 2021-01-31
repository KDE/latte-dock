/*
*  Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

    anchors.bottom: (parent && plasmoid.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (parent && plasmoid.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (parent && plasmoid.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (parent && plasmoid.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    width: {
        if (!visible)
            return 0;

        if (isSeparator) {
            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                return abilityItem.abilities.metrics.totals.thickness + abilityItem.abilities.metrics.margin.screenEdge;
            } else {
                if (isSeparatorInRealLength || !abilityItem.abilities.parabolic.isEnabled) {
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
            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                return abilityItem.abilities.metrics.totals.thickness + abilityItem.abilities.metrics.margin.screenEdge;
            } else {
                if (isSeparatorInRealLength || !abilityItem.abilities.parabolic.isEnabled) {
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

    property bool isHiddenSpacerAnimated: true
    property bool isHiddenSpacerVisible: false
    property bool isHidden: false
    property bool isSeparator: false
    property bool isSeparatorInRealLength: false

    property bool containsMouse: false

    property string thinTooltipText: ""

    property Item abilities: null
    property Item contentItem: null

    readonly property bool isHorizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    readonly property bool isVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property bool parabolicAreaContainsMouse: parabolicEventsAreaLoader.active && parabolicEventsAreaLoader.item.containsMouse

    readonly property int itemIndex: index
    readonly property int animationTime: (abilityItem.abilities.animations.active ? abilityItem.abilities.animations.speedFactor.current : 2) * (1.2 * abilityItem.abilities.animations.duration.small)
    readonly property int iconOffsetX: indicatorBackLayer.level.requested.iconOffsetX
    readonly property int iconOffsetY: indicatorBackLayer.level.requested.iconOffsetY

    readonly property alias indicator: abilityIndicatorObj
    readonly property alias parabolicItem: _parabolicItem
    readonly property alias tooltipVisualParent: _parabolicItem.titleTooltipVisualParent

    Component.onCompleted: {
        if (contentItem) {
            contentItem.parent = _parabolicItem.contentItemContainer;
        }
    }

    onContentItemChanged: {
        if (contentItem) {
            contentItem.parent = _parabolicItem.contentItemContainer;
        }
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

        while(tail>=0 && abilityItem.abilities.indexer.hidden.indexOf(tail)>=0) {
            tail = tail - 1;
        }

        var hasTailItemSeparator = abilityItem.abilities.indexer.separators.indexOf(tail)>=0;

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

        while(head>=0 && abilityItem.abilities.indexer.hidden.indexOf(head)>=0) {
            head = head + 1;
        }

        var hasHeadItemSeparator = abilityItem.abilities.indexer.separators.indexOf(head)>=0;

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

    Item{
        id:separatorItem
        anchors.bottom: (plasmoid.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
        anchors.top: (plasmoid.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
        anchors.left: (plasmoid.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
        anchors.right: (plasmoid.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

        anchors.horizontalCenter: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? parent.horizontalCenter : undefined
        anchors.verticalCenter: plasmoid.formFactor === PlasmaCore.Types.Vertical ? parent.verticalCenter : undefined

        anchors.bottomMargin: (plasmoid.location === PlasmaCore.Types.BottomEdge) ? margin : 0
        anchors.topMargin: (plasmoid.location === PlasmaCore.Types.TopEdge) ? margin : 0
        anchors.leftMargin: (plasmoid.location === PlasmaCore.Types.LeftEdge) ? margin : 0
        anchors.rightMargin: (plasmoid.location === PlasmaCore.Types.RightEdge) ? margin : 0

        opacity: (separatorShadow.active) || forceHiddenState ? 0 : 0.4
        visible: abilityItem.isSeparator

        width: plasmoid.formFactor === PlasmaCore.Types.Vertical ?
                   abilityItem.abilities.metrics.iconSize :
                   (isSeparatorInRealLength ? LatteCore.Environment.separatorLength+abilityItem.abilities.metrics.totals.lengthEdges: 1)
        height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ?
                    abilityItem.abilities.metrics.iconSize :
                    (isSeparatorInRealLength  ? LatteCore.Environment.separatorLength+abilityItem.abilities.metrics.totals.lengthEdges: 1)

        property bool forceHiddenState: false

        readonly property int margin: abilityItem.abilities.metrics.margin.screenEdge + abilityItem.abilities.metrics.margin.thickness

        Behavior on opacity {
            NumberAnimation { duration: abilityItem.abilities.animations.speedFactor.current * abilityItem.abilities.animations.duration.large }
        }

        Rectangle {
            anchors.centerIn: parent
            width: plasmoid.formFactor === PlasmaCore.Types.Vertical ? abilityItem.abilities.metrics.iconSize - 4  : 1
            height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? abilityItem.abilities.metrics.iconSize - 4 : 1
            color: abilityItem.abilities.myView.palette.textColor
        }
    }

    ///Shadow in separator
    Loader{
        id: separatorShadow
        anchors.fill: separatorItem
        active: abilityItem.abilities.myView.itemShadow.isEnabled && isSeparator && graphicsSystem.isAccelerated
        opacity: separatorItem.forceHiddenState ? 0 : 0.4

        Behavior on opacity {
            NumberAnimation { duration: abilityItem.abilities.animations.speedFactor.current * abilityItem.abilities.animations.duration.large }
        }

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: abilityItem.abilities.myView.itemShadow.shadowColor
            fast: true
            samples: 2 * radius
            source: separatorItem
            radius: abilityItem.abilities.myView.itemShadow.size
            verticalOffset: 2
        }
    }

    Flow{
        width: parent.width
        height: parent.height

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
        width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? abilityItem.width : abilityItem.abilities.metrics.mask.thickness.zoomedForItems
        height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? abilityItem.abilities.metrics.mask.thickness.zoomedForItems : abilityItem.height
        z:10000       
        sourceComponent: BasicItemParts.ParabolicEventsArea{}

        readonly property bool isParabolicEnabled: abilityItem.abilities.parabolic.isEnabled
        readonly property bool isThinTooltipEnabled: abilityItem.abilities.thinTooltip.isEnabled && abilityItem.thinTooltipText !== ""

        states:[
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: parabolicEventsAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: parabolicEventsAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: parabolicEventsAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: parabolicEventsAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
            }
        ]
    }
}
