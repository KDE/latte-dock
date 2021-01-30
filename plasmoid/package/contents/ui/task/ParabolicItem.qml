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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.latte.core 0.2 as LatteCore

Item{
    id: parabolicItem
    opacity: 0
    width: {
        if (!taskItem.visible)
            return 0;

        if (taskItem.isSeparator){
            if (!root.vertical)
                return 0;
            else
                return (taskItem.abilities.metrics.iconSize + root.widthMargins + taskItem.abilities.metrics.margin.screenEdge);
        }

        if (taskItem.isStartup && taskItem.abilities.animations.speedFactor.current !==0 ) {
            return root.vertical ? cleanScalingWidth + taskItem.abilities.metrics.margin.screenEdge : cleanScalingWidth;
        } else {
            return root.vertical ? showDelegateWidth + taskItem.abilities.metrics.margin.screenEdge : showDelegateWidth;
        }
    }

    height: {
        if (!taskItem.visible)
            return 0;

        if (taskItem.isSeparator){
            if (root.vertical)
                return 0;
            else
                return (taskItem.abilities.metrics.iconSize + root.heightMargins + taskItem.abilities.metrics.margin.screenEdge);
        }

        if (taskItem.isStartup && taskItem.abilities.animations.speedFactor.current !==0){
            return !root.vertical ? cleanScalingHeight + taskItem.abilities.metrics.margin.screenEdge : cleanScalingHeight;
        } else {
            return !root.vertical ? showDelegateHeight + taskItem.abilities.metrics.margin.screenEdge : showDelegateHeight;
        }
    }

    property bool isParabolicEventBlocked: false
    property bool isUpdatingOnlySpacers: false
    property bool isZoomed: false

    property int maxThickness: !root.vertical ? taskItem.abilities.parabolic.factor.zoom*(taskItem.abilities.metrics.iconSize+root.heightMargins)
                                              : taskItem.abilities.parabolic.factor.zoom*(taskItem.abilities.metrics.iconSize+root.widthMargins)

    property real showDelegateWidth: basicScalingWidth
    property real showDelegateHeight: basicScalingHeight

    //scales which are used mainly for activating InLauncher
    ////Scalers///////
    property bool inTempScaling: ((tempScaleWidth !== 1) || (tempScaleHeight !== 1) )

    property real zoom: 1.0
    property real tempScaleWidth: 1.0
    property real tempScaleHeight: 1.0

    property real scaleWidth: (inTempScaling == true) ? tempScaleWidth : zoom
    property real scaleHeight: (inTempScaling == true) ? tempScaleHeight : zoom

    property real cleanScalingWidth: (taskItem.abilities.metrics.iconSize + root.widthMargins) * zoom
    property real cleanScalingHeight: (taskItem.abilities.metrics.iconSize + root.heightMargins) * zoom

    property real basicScalingWidth : (inTempScaling == true) ? ((taskItem.abilities.metrics.iconSize + root.widthMargins) * scaleWidth) : cleanScalingWidth
    property real basicScalingHeight : (inTempScaling == true) ? ((taskItem.abilities.metrics.iconSize + root.heightMargins) * scaleHeight) : cleanScalingHeight

    property real regulatorWidth: taskItem.isSeparator ? width : basicScalingWidth;
    property real regulatorHeight: taskItem.isSeparator ? height : basicScalingHeight;

    property real visualScaledWidth: (taskItem.abilities.metrics.iconSize + root.internalWidthMargins) * zoom
    property real visualScaledHeight: (taskItem.abilities.metrics.iconSize + root.internalHeightMargins) * zoom
    /// end of Scalers///////

    property real center: !root.vertical ?
                             (width + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2 :
                             (height + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2

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
        enabled: !taskItem.abilities.parabolic.directRenderingEnabled || restoreAnimation.running
        NumberAnimation{
            duration: 3 * taskItem.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on zoom {
        enabled: !animatedBehavior.enabled
        NumberAnimation { duration: 0 }
    }

    Item{
        anchors.bottom: (root.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
        anchors.top: (root.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
        anchors.left: (root.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
        anchors.right: (root.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined
        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined

        anchors.bottomMargin: (root.location === PlasmaCore.Types.BottomEdge) ? taskItem.abilities.metrics.margin.screenEdge : 0
        anchors.topMargin: (root.location === PlasmaCore.Types.TopEdge) ? taskItem.abilities.metrics.margin.screenEdge : 0
        anchors.leftMargin: (root.location === PlasmaCore.Types.LeftEdge) ? taskItem.abilities.metrics.margin.screenEdge : 0
        anchors.rightMargin: (root.location === PlasmaCore.Types.RightEdge) ? taskItem.abilities.metrics.margin.screenEdge : 0

        anchors.horizontalCenterOffset: taskItem.iconOffsetX
        anchors.verticalCenterOffset: taskItem.iconOffsetY

        width: parabolicItem.regulatorWidth
        height: parabolicItem.regulatorHeight

        TitleTooltipParent{
            id: _titleTooltipVisualParent
            thickness: taskItem.abilities.parabolic.factor.zoom * taskItem.abilities.metrics.totals.thickness
        }

        //fix bug #478, when changing form factor sometimes the tasks are not positioned
        //correctly, in such case we make a fast reinitialization for the sizes
        Connections {
            target: plasmoid

            onFormFactorChanged:{
                taskItem.inAddRemoveAnimation = false;

                parabolicItem.zoom = 1.01;
                parabolicItem.tempScaleWidth = 1.01;
                parabolicItem.tempScaleHeight = 1.01;

                parabolicItem.zoom = 1;
                parabolicItem.tempScaleWidth = 1;
                parabolicItem.tempScaleHeight = 1;
            }
        }

        Item {
            id: _contentItemContainer
            anchors.centerIn: parent
            width: newTempSize
            height: width

            property int zoomedSize: taskItem.abilities.parabolic.factor.zoom * taskItem.abilities.metrics.iconSize

            property real basicScalingWidth : parabolicItem.inTempScaling ? (taskItem.abilities.metrics.iconSize * parabolicItem.scaleWidth) :
                                                                      taskItem.abilities.metrics.iconSize * parabolicItem.zoom
            property real basicScalingHeight : parabolicItem.inTempScaling ? (taskItem.abilities.metrics.iconSize * parabolicItem.scaleHeight) :
                                                                       taskItem.abilities.metrics.iconSize * parabolicItem.zoom

            property real newTempSize: {
                if (parabolicItem.opacity === 1 ) {
                    return Math.min(basicScalingWidth, basicScalingHeight);
                } else {
                    return Math.max(basicScalingWidth, basicScalingHeight);
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
            taskItem.abilities.animations.needBothAxis.removeEvent(bothAxisZoomEvent);
        }
    }

    onZoomChanged: {
        if ((zoom === taskItem.abilities.parabolic.factor.zoom) && !taskItem.abilities.parabolic.directRenderingEnabled) {
            taskItem.abilities.parabolic.setDirectRenderingEnabled(true);
        }

        if ((zoom > 1) && !isZoomed) {
            isZoomed = true;
            taskItem.abilities.animations.needBothAxis.addEvent(bothAxisZoomEvent);
        } else if ((zoom == 1) && isZoomed) {
            sendEndOfNeedBothAxisAnimation();
        }
    }

    Connections {
        target: taskItem
        onVisibleChanged: {
            if (!taskItem.visible) {
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
