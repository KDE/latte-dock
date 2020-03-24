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

import org.kde.latte 0.2 as Latte

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
        if(!taskIcon.toBeDestroyed && root.animationWindowRemovedFromGroup){
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

            Latte.IconItem{
                id: tempRemoveIcon
                anchors.rightMargin: root.position === PlasmaCore.Types.LeftPositioned ? root.thickMargin : 0
                anchors.leftMargin: root.position === PlasmaCore.Types.RightPositioned ? root.thickMargin : 0
                anchors.topMargin: root.position === PlasmaCore.Types.BottomPositioned ? root.thickMargin : 0
                anchors.bottomMargin: root.position === PlasmaCore.Types.TopPositioned ? root.thickMargin : 0

                anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                anchors.right: root.position === PlasmaCore.Types.LeftPositioned ? parent.right : undefined;
                anchors.left: root.position === PlasmaCore.Types.RightPositioned ? parent.left : undefined;
                anchors.top: root.position === PlasmaCore.Types.BottomPositioned ? parent.top : undefined;
                anchors.bottom: root.position === PlasmaCore.Types.TopPositioned ? parent.bottom : undefined;

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

                property int speed: 2*root.appliedDurationTime*root.longDuration
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

                if( (root.position === PlasmaCore.Types.BottomPositioned) ||
                        (root.position === PlasmaCore.Types.RightPositioned) ){
                    componentRemoveAnimation.toPoint = tempPoint + root.iconSize;
                }
                else{
                    componentRemoveAnimation.toPoint = tempPoint - root.iconSize;
                }

                visible = true;
                componentRemoveAnimation.start();
            }

        }
    }
}
