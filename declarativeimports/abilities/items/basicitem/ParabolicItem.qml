/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

Item{
    id: parabolicItem
    width: abilityItem.isHorizontal ? length : thickness
    height: abilityItem.isHorizontal ? thickness : length

    readonly property real length: (!abilityItem.visible || abilityItem.isSeparator) ? 0 : basicScalingLength

    readonly property real thickness: (!abilityItem.visible ? 0 : (abilityItem.isSeparator ?
                                                                       abilityItem.abilities.metrics.mask.thickness.normalForItems :
                                                                       basicScalingThickness + abilityItem.abilities.metrics.margin.screenEdge))

    property bool isParabolicEventBlocked: false
    property bool isUpdatingOnlySpacers: false
    property bool isZoomed: false

    //scales which are used mainly for activating InLauncher
    ////Scalers///////
    property bool hasZoomPerAxis: ((zoomLength !== 1.0) || (zoomThickness !== 1.0) )

    property real zoom: 1.0
    property real zoomLength: 1.0
    property real zoomThickness: 1.0

    property real scaleLength: hasZoomPerAxis ? zoomLength : zoom
    property real scaleThickness: hasZoomPerAxis ? zoomThickness : zoom

    property real cleanScalingLength: abilityItem.abilities.metrics.totals.length * zoom
    property real cleanScalingThickness: abilityItem.abilities.metrics.totals.thickness * zoom

    property real basicScalingLength: hasZoomPerAxis ? abilityItem.abilities.metrics.totals.length * scaleLength : cleanScalingLength
    property real basicScalingThickness: hasZoomPerAxis ? abilityItem.abilities.metrics.totals.thickness * scaleThickness : cleanScalingThickness

    property real regulatorLength: abilityItem.isSeparator ? (abilityItem.isHorizontal ? width : height) : basicScalingLength
    property real regulatorThickness: abilityItem.isSeparator ? (abilityItem.isHorizontal ? height : width) : basicScalingThickness

    property real visualScaledLength: (abilityItem.abilities.metrics.iconSize + abilityItem.abilities.metrics.totals.lengthPaddings) * zoom
    property real visualScaledThickness: abilityItem.abilities.metrics.totals.thickness * zoom
    /// end of Scalers///////

    property real center: (abilityItem.isHorizontal ? abilityItem.width : abilityItem.height) / 2

    readonly property alias contentItemContainer: _contentItemContainer
    readonly property alias titleTooltipVisualParent: _titleTooltipVisualParent

    readonly property string bothAxisZoomEvent: parabolicItem + "_zoom"
    /* Rectangle{
            anchors.fill: parent
            border.width: 1
            border.color: "green"
            color: "transparent"
        }*/

    Behavior on zoom {
        id: animatedBehavior
        enabled: !abilityItem.abilities.parabolic.directRenderingEnabled || restoreAnimation.running
        NumberAnimation{
            duration: 3 * abilityItem.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on zoom {
        enabled: !animatedBehavior.enabled
        NumberAnimation { duration: 0 }
    }

    Item{
        anchors.bottom: (abilityItem.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
        anchors.top: (abilityItem.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
        anchors.left: (abilityItem.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
        anchors.right: (abilityItem.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

        anchors.horizontalCenter: abilityItem.isHorizontal ? parent.horizontalCenter : undefined
        anchors.verticalCenter: abilityItem.isHorizontal ? undefined : parent.verticalCenter

        anchors.bottomMargin: (abilityItem.location === PlasmaCore.Types.BottomEdge) ? abilityItem.abilities.metrics.margin.screenEdge : 0
        anchors.topMargin: (abilityItem.location === PlasmaCore.Types.TopEdge) ? abilityItem.abilities.metrics.margin.screenEdge : 0
        anchors.leftMargin: (abilityItem.location === PlasmaCore.Types.LeftEdge) ? abilityItem.abilities.metrics.margin.screenEdge : 0
        anchors.rightMargin: (abilityItem.location === PlasmaCore.Types.RightEdge) ? abilityItem.abilities.metrics.margin.screenEdge : 0

        width: abilityItem.isHorizontal ? parabolicItem.regulatorLength : parabolicItem.regulatorThickness
        height: abilityItem.isHorizontal ? parabolicItem.regulatorThickness : parabolicItem.regulatorLength

        TitleTooltipParent{
            id: _titleTooltipVisualParent
            thickness: abilityItem.abilities.parabolic.factor.zoom * abilityItem.abilities.metrics.totals.thickness
        }

        //fix bug #478, when changing form factor sometimes the tasks are not positioned
        //correctly, in such case we make a fast reinitialization for the sizes
        Connections {
            target: plasmoid
            onFormFactorChanged:{
                parabolicItem.zoom = 1.01;
                parabolicItem.zoomLength = 1.01;
                parabolicItem.zoomThickness = 1.01;

                parabolicItem.zoom = 1;
                parabolicItem.zoomLength = 1;
                parabolicItem.zoomThickness = 1;
            }
        }

        //! Shadows
        Loader{
            anchors.fill: _contentItemContainer
            active: abilityItem.abilities.myView.itemShadow.isEnabled
                    && !abilityItem.isSeparator
                    && abilityItem.abilities.environment.isGraphicsSystemAccelerated

            sourceComponent: DropShadow{
                anchors.fill: parent
                transformOrigin: abilityItem.iconTransformOrigin
                opacity: abilityItem.iconOpacity
                rotation: abilityItem.iconRotation
                scale: abilityItem.iconScale
                color: abilityItem.abilities.myView.itemShadow.shadowColor
                fast: true
                samples: 2 * radius
                source: _contentItemContainer
                radius: abilityItem.abilities.myView.itemShadow.size
                verticalOffset: 2
            }
        }

        //! Main contented item
        Item {
            id: _contentItemContainer
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: abilityItem.iconOffsetX
            anchors.verticalCenterOffset: abilityItem.iconOffsetY
            transformOrigin: abilityItem.iconTransformOrigin
            opacity: abilityItem.iconOpacity
            rotation: abilityItem.iconRotation
            scale: abilityItem.iconScale

            width: newTempSize
            height: width
            visible: !abilityItem.isSeparator

            property int zoomedSize: abilityItem.abilities.parabolic.factor.zoom * abilityItem.abilities.metrics.iconSize
            property real basicScalingLength: abilityItem.abilities.metrics.iconSize * (parabolicItem.hasZoomPerAxis ? parabolicItem.scaleLength : parabolicItem.zoom)
            property real basicScalingThickness: proposedItemThickness * (parabolicItem.hasZoomPerAxis ? parabolicItem.scaleThickness : parabolicItem.zoom)

            readonly property int proposedItemThickness: abilityItem.abilities.indexer.inMarginsArea ?
                                                             Math.max(16, abilityItem.abilities.metrics.marginsArea.iconSize) :
                                                             abilityItem.abilities.metrics.iconSize

            property real newTempSize: {
                if (parabolicItem.opacity === 1 ) {
                    return Math.min(basicScalingLength, basicScalingThickness);
                } else {
                    return Math.max(basicScalingLength, basicScalingThickness);
                }
            }

            readonly property Item contentItem: children.length > 0 ? children[0] : null

            Component.onCompleted: {
                if (abilityItem.contentItem && abilityItem.contentItem.parent !== _contentItemContainer) {
                    abilityItem.contentItem.parent = _contentItemContainer;
                }
            }

            Connections {
                target: abilityItem
                onContentItemChanged: {
                    if (abilityItem.contentItem && abilityItem.contentItem.parent !== _contentItemContainer) {
                        abilityItem.contentItem.parent = _contentItemContainer;
                    }
                }
            }
        } //_contentItemContainer

        //! Latte Side Painting-style if the user chose it
        Loader{
            anchors.fill: _contentItemContainer
            active: abilityItem.isMonochromaticForcedContentItem && abilityItem.monochromizedItem

            sourceComponent: ColorOverlay {
                anchors.fill: parent
                color: latteBridge ? latteBridge.palette.textColor : "transparent"
                source: abilityItem.monochromizedItem
            }
        }
        //! Latte Side Painting-style if the user chose it

        ShortcutBadge{
            id: shortcutBadge
            anchors.centerIn: parent
            width: abilityItem.abilities.metrics.iconSize * (parabolicItem.hasZoomPerAxis ? parabolicItem.scaleThickness : parabolicItem.zoom)
            height: width
        }
    }

    function sendEndOfNeedBothAxisAnimation(){
        if (isZoomed) {
            isZoomed = false;
            abilityItem.abilities.animations.needBothAxis.removeEvent(bothAxisZoomEvent);
        }
    }

    onZoomChanged: {
        if ((zoom === abilityItem.abilities.parabolic.factor.zoom) && !abilityItem.abilities.parabolic.directRenderingEnabled) {
            abilityItem.abilities.parabolic.setDirectRenderingEnabled(true);
        }

        if ((zoom > 1) && !isZoomed) {
            isZoomed = true;
            abilityItem.abilities.animations.needBothAxis.addEvent(bothAxisZoomEvent);
        } else if ((zoom == 1) && isZoomed) {
            sendEndOfNeedBothAxisAnimation();
        }
    }

    Connections {
        target: abilityItem
        onVisibleChanged: {
            if (!abilityItem.visible) {
                //! is mostly used when the user destroys tasks applet from the context menu and both
                //! axis animations should be released in that case
                parabolicItem.sendEndOfNeedBothAxisAnimation();
            }
        }
    }

    Component.onCompleted: {
        if (!LatteCore.WindowSystem.compositingActive) {
            opacity = 1;
        }
    }
}//parabolicItem
