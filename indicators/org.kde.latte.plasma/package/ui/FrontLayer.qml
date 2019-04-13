/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Item {
    anchors.fill: parent

    Item {
        id: relevantItem
        anchors.fill: parent
        clip: true

        Item{
            id: clickedCenter

            readonly property int center: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? parent.width/2 : parent.height/2

            states:[
                State {
                    name: "bottom"
                    when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

                    AnchorChanges {
                        target: clickedCenter
                        anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;
                            horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
                    }
                },
                State {
                    name: "left"
                    when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

                    AnchorChanges {
                        target: clickedCenter
                        anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;
                            horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
                    }
                },
                State {
                    name: "right"
                    when: (plasmoid.location === PlasmaCore.Types.RightEdge)

                    AnchorChanges {
                        target: clickedCenter
                        anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;
                            horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
                    }
                },
                State {
                    name: "top"
                    when: (plasmoid.location === PlasmaCore.Types.TopEdge)

                    AnchorChanges {
                        target: clickedCenter
                        anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined;
                            horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
                    }
                }
            ]
        }

        Rectangle {
            id: clickedRectangle
            anchors.centerIn: clickedCenter
            opacity: 0
            radius: width/2
            height: width

            color: theme.highlightColor
        }
    }
    
    SequentialAnimation {
        id: clickedAnimation
        ScriptAction{
            script: {
                clickedRectangle.width = 0;
                clickedRectangle.opacity = 0.75;
                clickedRectangle.anchors.rightMargin = 0;
                clickedRectangle.anchors.leftMargin = 0;
                clickedRectangle.anchors.topMargin = 0;
                clickedRectangle.anchors.bottomMargin = 0;
                clickedRectangle.anchors.horizontalCenterOffset = 0;
                clickedRectangle.anchors.verticalCenterOffset = 0;
            }
        }

        ParallelAnimation{
            PropertyAnimation {
                target: clickedRectangle
                property: "width"
                to: Math.max(relevantItem.width, relevantItem.height) * indicator.scaleFactor * 2
                duration: 700
                easing.type: Easing.Linear
            }
            PropertyAnimation {
                target: clickedRectangle
                property: "opacity"
                to: 0
                duration: 700
                easing.type: Easing.Linear
            }
        }
    }

    Connections {
        target: level
        enabled: root.clickedAnimationEnabled
        onMousePressed: {
            var fixedX = 0;
            var fixedY = 0;

            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                fixedX = x - clickedCenter.center;
            } else {
                if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                   fixedX = relevantItem.width - x;
                } else {
                   fixedX = x;
                }
            }

            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                fixedY = y - clickedCenter.center;
            } else {
                 if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    fixedY = relevantItem.height - y;
                 } else {
                    fixedY = y;
                 }
            }

            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                clickedCenter.anchors.horizontalCenterOffset = fixedX;
            } else {
                if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    clickedCenter.anchors.leftMargin = fixedX;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    clickedCenter.anchors.rightMargin = fixedX;
                }
            }

            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                clickedCenter.anchors.verticalCenterOffset = fixedY;
            } else {
                if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    clickedCenter.anchors.bottomMargin = fixedY;
                } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                    clickedCenter.anchors.topMargin = fixedY;
                }
            }




            clickedCenter.anchors.verticalCenterOffset = fixedY;

            clickedAnimation.start();
        }
    }
    

    Loader {
        anchors.fill: parent
        visible: !indicator.isApplet && indicator.isGroup
        sourceComponent: Item{
            anchors.fill: parent

            PlasmaCore.Svg {
                id: taskSvg
                imagePath: "widgets/tasks"
            }

            Item {
                id: iconBox
                anchors.centerIn: parent
                width: indicator.currentIconSize
                height: width
            }

            PlasmaCore.SvgItem {
                id: arrow

                implicitWidth: 0.25 * iconBox.width
                implicitHeight: implicitWidth

                svg: taskSvg
                elementId: elementForLocation(plasmoid.location)


                function elementForLocation(location) {
                    switch (location) {
                    case PlasmaCore.Types.LeftEdge:
                        return "group-expander-left";
                    case PlasmaCore.Types.TopEdge:
                        return "group-expander-top";
                    case PlasmaCore.Types.RightEdge:
                        return "group-expander-right";
                    case PlasmaCore.Types.BottomEdge:
                    default:
                        return "group-expander-bottom";
                    }
                }
            }

            states: [
                State {
                    name: "bottom"
                    when: plasmoid.location === PlasmaCore.Types.BottomEdge
                    AnchorChanges {
                        target: arrow
                        anchors.top: undefined; anchors.left: undefined; anchors.right: undefined; anchors.bottom: arrow.parent.bottom;
                        anchors.horizontalCenter: iconBox.horizontalCenter; anchors.verticalCenter: undefined;
                    }
                },
                State {
                    name: "top"
                    when: plasmoid.location === PlasmaCore.Types.TopEdge
                    AnchorChanges {
                        target: arrow
                        anchors.top: arrow.parent.top; anchors.left: undefined; anchors.right: undefined; anchors.bottom: undefined;
                        anchors.horizontalCenter: iconBox.horizontalCenter; anchors.verticalCenter: undefined;
                    }
                },
                State {
                    name: "left"
                    when: plasmoid.location === PlasmaCore.Types.LeftEdge
                    AnchorChanges {
                        target: arrow
                        anchors.top: undefined; anchors.left: arrow.parent.left; anchors.right: undefined; anchors.bottom: undefined;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: iconBox.verticalCenter;
                    }
                },
                State {
                    name: "right"
                    when: plasmoid.location === PlasmaCore.Types.RightEdge
                    AnchorChanges {
                        target: arrow
                        anchors.top: undefined; anchors.left: undefined; anchors.right: arrow.parent.right; anchors.bottom: undefined;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: iconBox.verticalCenter;
                    }
                }
            ]
        }
    }
}
