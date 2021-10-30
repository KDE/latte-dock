/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.kirigami 2.0 as Kirigami

import org.kde.latte.core 0.2 as LatteCore

/////Removing a Window from a group////
Item{
    id:removingAnimation

    function init(){
        var relavantPoint = root.mapFromItem(taskIcon,0,0);

        var removingItem = removeTaskComponent.createObject(root);
        removingItem.x = relavantPoint.x;
        removingItem.y = relavantPoint.y;

        removingItem.start();
    }

    function removeTask(){
        if (taskItem.abilities.indicators.info.providesGroupedWindowRemovedAnimation) {
            return;
        }

        if(!taskIconContainer.toBeDestroyed && root.windowRemovedFromGroupEnabled){
            removingAnimation.init();
        }
    }

    Component.onCompleted: {
        taskItem.taskGroupedWindowRemoved.connect(removeTask);
    }

    Component.onDestruction: {
        taskItem.taskGroupedWindowRemoved.disconnect(removeTask);
    }

    ///////////// Component for animating removing window from group

    Component {
        id: removeTaskComponent
        Item{
            id: removeTask
            width: taskIcon.width
            height: taskIcon.height
            visible: false

            //! Shadow
            Loader{
                id: tempTaskShadow
                anchors.fill: tempRemoveIcon
                active: taskItem.abilities.myView.itemShadow.isEnabled
                        && taskItem.abilities.environment.isGraphicsSystemAccelerated

                sourceComponent: DropShadow{
                    anchors.fill: parent
                    color: "#ff080808"
                    fast: true
                    samples: 2 * radius
                    source: tempRemoveIcon
                    radius: taskItem.abilities.myView.itemShadow.size
                    verticalOffset: 2
                }
            }

            Kirigami.Icon{
                id: tempRemoveIcon
                anchors.rightMargin: root.location === PlasmaCore.Types.LeftEdge ? taskItem.abilities.metrics.margin.thickness : 0
                anchors.leftMargin: root.location === PlasmaCore.Types.RightEdge ? taskItem.abilities.metrics.margin.thickness : 0
                anchors.topMargin: root.location === PlasmaCore.Types.BottomEdge ? taskItem.abilities.metrics.margin.thickness : 0
                anchors.bottomMargin: root.location === PlasmaCore.Types.TopEdge ? taskItem.abilities.metrics.margin.thickness : 0

                anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                anchors.right: root.location === PlasmaCore.Types.LeftEdge ? parent.right : undefined;
                anchors.left: root.location === PlasmaCore.Types.RightEdge ? parent.left : undefined;
                anchors.top: root.location === PlasmaCore.Types.BottomEdge ? parent.top : undefined;
                anchors.bottom: root.location === PlasmaCore.Types.TopEdge ? parent.bottom : undefined;

                width: taskIconItem.width
                height: width

                source: taskIconItem.source
            }



            Colorize{
                source: tempRemoveIcon
                anchors.fill: tempRemoveIcon

                hue: 0
                saturation: 0
                lightness: 0
            }

            ParallelAnimation{
                id: componentRemoveAnimation

                property int speed: 2 * taskItem.abilities.animations.speedFactor.normal * taskItem.abilities.animations.duration.large
                property Item removingItem: parent
                property int toPoint: 0

                PropertyAnimation {
                    target: removeTask
                    property: "opacity"
                    to: 0
                    duration: componentRemoveAnimation.speed
                    easing.type: Easing.InQuad
                }

                PropertyAnimation {
                    target: removeTask
                    property: (icList.orientation == Qt.Horizontal) ? "y" : "x"
                    to: componentRemoveAnimation.toPoint
                    duration: componentRemoveAnimation.speed
                    easing.type: Easing.InQuad
                }

                onStopped: {
                    removeTask.destroy();
                }
            }

            function start(){
                var tempPoint = 0;

                if(icList.orientation == Qt.Horizontal)
                    tempPoint = y;
                else
                    tempPoint = x;

                if( (root.location === PlasmaCore.Types.BottomEdge) ||
                        (root.location === PlasmaCore.Types.RightEdge) ){
                    componentRemoveAnimation.toPoint = tempPoint + taskItem.abilities.metrics.iconSize;
                }
                else{
                    componentRemoveAnimation.toPoint = tempPoint - taskItem.abilities.metrics.iconSize;
                }

                visible = true;
                componentRemoveAnimation.start();
            }

        }
    }
}
