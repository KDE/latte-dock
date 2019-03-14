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

import QtQuick 2.0
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Item{
    property Item rootItem: parent

    readonly property bool needsIconColors: true

    readonly property int thickness: plasmoid.formFactor === PlasmaCore.Types.Vertical ? width : height
    readonly property int freedThickness: (thickness - rectangleItem.size) / 2

    readonly property int shownWindows: rootItem.windowsCount - rootItem.windowsMinimizedCount
    readonly property int maxDrawnMinimizedWindows: shownWindows > 0 ? Math.min(rootItem.windowsMinimizedCount,2) : 3

    Item{
        id: rectangleItem
        width: rootItem.isTask ? Math.min(parent.width, parent.height) : parent.width
        height: rootItem.isTask ? width : parent.height
        anchors.centerIn: parent

        property bool isActive: rootItem.isActive || (rootItem.isWindow && rootItem.hasActive)
        readonly property int size: Math.min(parent.width, parent.height)

        Rectangle {
            id: unityRect
            anchors.fill: parent
            visible: rootItem.isActive || (rootItem.isWindow && rootItem.hasShown)

            radius: rootItem.currentIconSize / 12
            color: rootItem.backgroundColor
            clip: true
        }

        RadialGradient{
            id: glowGradient
            anchors.verticalCenter: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - unityRect.anchors.margins * 2 - 1
            height: (width * 0.85) - unityRect.anchors.margins * 2 - 1
            visible: false

            gradient: Gradient {
                GradientStop { position: 0.0;
                    color: {
                        if (rootItem.isMinimized) {
                            return "#aafcfcfc";
                        }

                        return rootItem.glowColor;
                    }
                }
                GradientStop { position: 0.6; color: "transparent" }
            }
        }

        Item {
            id: gradientMask
            anchors.fill: glowGradient

            Rectangle {
                anchors.top: gradientMask.verticalCenter
                anchors.topMargin: unityRect.anchors.margins
                width: glowGradient.width
                height: glowGradient.height / 2
                radius: unityRect.radius
            }

            visible: false
        }

        OpacityMask {
            anchors.fill: glowGradient
            source: glowGradient
            maskSource: gradientMask
            visible: unityRect.visible || borderRectangle.visible
        }

        Rectangle {
            id: borderRectangle
            anchors.fill: parent
            visible: (rootItem.isTask && rootItem.isWindow) || (rootItem.isApplet && rootItem.isActive)
            color: "transparent"
            border.width: 1
            border.color: "#303030"
            radius: unityRect.radius
            clip: true

            Rectangle {
                anchors.fill: parent
                anchors.margins: parent.border.width
                radius: unityRect.radius
                color: "transparent"
                border.width: 1
                border.color: "#25dedede"
            }
        }
    }

    Row {
        id: upperIndicators
        spacing: 2
        readonly property bool alwaysActive: true
        readonly property bool reversed: true

        Repeater {
            model: rootItem.isActive || rootItem.hasActive ? 1 : 0
            delegate: triangleComponent
        }
    }

    Grid {
        id: lowerIndicators
        rows: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? 1 : Math.min(3, rootItem.windowsCount)
        columns: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? Math.min(3, rootItem.windowsCount) : 1
        rowSpacing: 2
        columnSpacing: 2

        readonly property bool alwaysActive: false
        readonly property bool reversed: false

        Repeater {
            model: Math.min(3, rootItem.windowsCount)
            delegate: triangleComponent
        }
    }


    //! Triangle Indicator Component
    Component {
        id: triangleComponent
        Canvas {
            id: canvas
            width: rootItem.currentIconSize / 7
            height: width

            rotation: {
                if (!parent.reversed) {
                    if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                        return 0;
                    } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                        return 90;
                    } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                        return 180;
                    } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                        return 270;
                    }
                } else {
                    if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                        return 180;
                    } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                        return 270;
                    } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                        return 0;
                    } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                        return 90;
                    }
                }

                return 0;
            }

            property color drawColor:  theme.buttonFocusColor;
            property bool fillTriangle: {
                if (!parent.alwaysActive && rootItem.windowsMinimizedCount!==0
                        && ((index < maxDrawnMinimizedWindows)
                            || (rootItem.windowsCount === rootItem.windowsMinimizedCount))) {
                    return false;
                }

                return true;
            }

            onFillTriangleChanged: requestPaint();
            onDrawColorChanged: requestPaint();

            onPaint: {
                var ctx = getContext('2d');
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                ctx.fillStyle = fillTriangle ? drawColor : theme.backgroundColor;
                ctx.strokeStyle = drawColor;
                ctx.lineWidth = 2;

                ctx.beginPath();
                ctx.moveTo(0, canvas.height);
                ctx.lineTo(canvas.width/2, 0);
                ctx.lineTo(canvas.width, canvas.height);
                ctx.lineTo(0, canvas.height);
                ctx.closePath();

                ctx.fill();
                ctx.stroke();
            }
        }
    }

    //! States
    states: [
        State {
            name: "bottom"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

            AnchorChanges {
                target: lowerIndicators
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;
                    horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: upperIndicators
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined;
                    horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "top"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)

            AnchorChanges {
                target: lowerIndicators
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined;
                    horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: upperIndicators
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;
                    horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "left"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

            AnchorChanges {
                target: lowerIndicators
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: upperIndicators
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        State {
            name: "right"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)

            AnchorChanges {
                target: lowerIndicators
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: upperIndicators
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        }
    ]
}
