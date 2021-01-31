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
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

Item{
    id: parabolicItem
    opacity: 0
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

    property real center: ((abilityItem.isHorizontal ? width : height) + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2

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
        anchors.bottom: (plasmoid.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
        anchors.top: (plasmoid.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
        anchors.left: (plasmoid.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
        anchors.right: (plasmoid.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

        anchors.horizontalCenter: abilityItem.isHorizontal ? parent.horizontalCenter : undefined
        anchors.verticalCenter: abilityItem.isHorizontal ? undefined : parent.verticalCenter

        anchors.bottomMargin: (plasmoid.location === PlasmaCore.Types.BottomEdge) ? abilityItem.abilities.metrics.margin.screenEdge : 0
        anchors.topMargin: (plasmoid.location === PlasmaCore.Types.TopEdge) ? abilityItem.abilities.metrics.margin.screenEdge : 0
        anchors.leftMargin: (plasmoid.location === PlasmaCore.Types.LeftEdge) ? abilityItem.abilities.metrics.margin.screenEdge : 0
        anchors.rightMargin: (plasmoid.location === PlasmaCore.Types.RightEdge) ? abilityItem.abilities.metrics.margin.screenEdge : 0

        anchors.horizontalCenterOffset: abilityItem.iconOffsetX
        anchors.verticalCenterOffset: abilityItem.iconOffsetY

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

        Item {
            id: _contentItemContainer
            anchors.centerIn: parent
            width: newTempSize
            height: width

            property int zoomedSize: abilityItem.abilities.parabolic.factor.zoom * abilityItem.abilities.metrics.iconSize
            property real basicScalingLength: abilityItem.abilities.metrics.iconSize * (parabolicItem.hasZoomPerAxis ? parabolicItem.scaleLength : parabolicItem.zoom)
            property real basicScalingThickness: abilityItem.abilities.metrics.iconSize * (parabolicItem.hasZoomPerAxis ? parabolicItem.scaleThickness : parabolicItem.zoom)

            property real newTempSize: {
                if (parabolicItem.opacity === 1 ) {
                    return Math.min(basicScalingLength, basicScalingThickness);
                } else {
                    return Math.max(basicScalingLength, basicScalingThickness);
                }
            }

            readonly property Item contentItem: children.length > 0 ? children[0] : null


            //! Latte Side Painting-style if the user chose it
            Loader{
                anchors.fill: _contentItemContainer
                active: plasmoid.configuration.forceMonochromaticIcons

                sourceComponent: ColorOverlay {
                    anchors.fill: parent
                    color: latteBridge ? latteBridge.palette.textColor : "transparent"
                    source: _contentItemContainer.contentItem
                }
            }
            //! Latte Side Painting-style if the user chose it
        } //_contentItemContainer

        ShortcutBadge{
            id: shortcutBadge
            anchors.fill: _contentItemContainer
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
