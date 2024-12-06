/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

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
                //! Don't animate above for length
                to: maxLength * multiplier
                duration: 700
                easing.type: Easing.Linear

                readonly property int multiplier: indicator.scaleFactor * 2
                readonly property int maxLength: Math.min(indicator.currentIconSize*10, Math.max(relevantItem.width, relevantItem.height))
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
        anchors.topMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? indicator.screenEdgeMargin : 0
        anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? indicator.screenEdgeMargin : 0
        anchors.leftMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? indicator.screenEdgeMargin : 0
        anchors.rightMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? indicator.screenEdgeMargin : 0

        visible: !indicator.isApplet && indicator.isGroup
        sourceComponent: Item{
            anchors.fill: parent

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

                svg: groupSvg
                elementId: elementForLocation(plasmoid.location)

                readonly property QtObject groupSvg: indicator.resources && indicator.resources.svgs.length > 0 ? indicator.resources.svgs[0] : null

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
