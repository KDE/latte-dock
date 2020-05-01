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
        if(!taskIcon.toBeDestroyed && taskItem.animations.windowRemovedFromGroupEnabled){
            removingAnimation.init();
        }
    }

    Component.onCompleted: {
        taskItem.groupWindowRemoved.connect(removeTask);
    }

    Component.onDestruction: {
        taskItem.groupWindowRemoved.disconnect(removeTask);
    }

    ///////////// Component for animating removing window from group

    Component {
        id: removeTaskComponent
        Item{
            id: removeTask
            width: taskIcon.width
            height: taskIcon.height

            visible: false

            LatteCore.IconItem{
                id: tempRemoveIcon
                anchors.rightMargin: root.location === PlasmaCore.Types.LeftEdge ? taskItem.metrics.margin.thickness : 0
                anchors.leftMargin: root.location === PlasmaCore.Types.RightEdge ? taskItem.metrics.margin.thickness : 0
                anchors.topMargin: root.location === PlasmaCore.Types.BottomEdge ? taskItem.metrics.margin.thickness : 0
                anchors.bottomMargin: root.location === PlasmaCore.Types.TopEdge ? taskItem.metrics.margin.thickness : 0

                anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                anchors.right: root.location === PlasmaCore.Types.LeftEdge ? parent.right : undefined;
                anchors.left: root.location === PlasmaCore.Types.RightEdge ? parent.left : undefined;
                anchors.top: root.location === PlasmaCore.Types.BottomEdge ? parent.top : undefined;
                anchors.bottom: root.location === PlasmaCore.Types.TopEdge ? parent.bottom : undefined;

                width: iconImageBuffer.width
                height: width
                visible: root.enableShadows ? false : true

                source: iconImageBuffer.lastValidSourceName
            }

            Loader{
                id: tempTaskShadow
                anchors.fill: tempRemoveIcon
                active: root.enableShadows && graphicsSystem.isAccelerated

                sourceComponent: DropShadow{
                    anchors.fill: parent
                    color: "#ff080808"
                    fast: true
                    samples: 2 * radius
                    source: tempRemoveIcon
                    radius: taskIcon.shadowSize
                    verticalOffset: 2
                }
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

                property int speed: 2 * taskItem.animations.speedFactor.normal * taskItem.animations.duration.large
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
                    componentRemoveAnimation.toPoint = tempPoint + taskItem.metrics.iconSize;
                }
                else{
                    componentRemoveAnimation.toPoint = tempPoint - taskItem.metrics.iconSize;
                }

                visible = true;
                componentRemoveAnimation.start();
            }

        }
    }
}
