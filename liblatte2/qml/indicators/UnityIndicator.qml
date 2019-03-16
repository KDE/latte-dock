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

import "../code/ColorizerTools.js" as ColorizerTools

Item{
    id: indicatorItem
    readonly property Item parentItem: parent.manager
    readonly property Item rootItem: parent

    readonly property bool needsIconColors: true
    readonly property bool providesFrontLayer: true

    readonly property int thickness: plasmoid.formFactor === PlasmaCore.Types.Vertical ? width : height

    readonly property int shownWindows: parentItem.windowsCount - parentItem.windowsMinimizedCount
    readonly property int maxDrawnMinimizedWindows: shownWindows > 0 ? Math.min(parentItem.windowsMinimizedCount,2) : 3

    readonly property real backColorBrightness: ColorizerTools.colorBrightness(theme.backgroundColor)
    readonly property color backgroundColor: backColorBrightness < 127 ? theme.backgroundColor : theme.textColor

    //! Background Layer
    Loader{
        id: backLayer
        anchors.fill: parent
        active: rootItem.isBackLayer

        sourceComponent: Item{
            Item{
                id: rectangleItem
                width: parentItem.isTask ? Math.min(parent.width, parent.height) : parent.width
                height: parentItem.isTask ? width : parent.height
                anchors.centerIn: parent

                property bool isActive: parentItem.isActive || (parentItem.isWindow && parentItem.hasActive)
                readonly property int size: Math.min(parent.width, parent.height)

                Rectangle {
                    id: unityRect
                    anchors.fill: parent
                    visible: parentItem.isActive || (parentItem.isWindow && parentItem.hasShown)

                    radius: parentItem.currentIconSize / 12
                    color: parentItem.backgroundColor
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
                                if (parentItem.isMinimized) {
                                    return "#aafcfcfc";
                                }

                                return parentItem.glowColor;
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
                    visible: (parentItem.isTask && parentItem.isWindow) || (parentItem.isApplet && parentItem.isActive)
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
        }
    }


    //! Foreground Layer to draw Triangles
    Loader{
        id: frontLayer
        anchors.fill: parent
        active: !rootItem.isBackLayer

        sourceComponent: Item {
            anchors.fill: parent

            Row {
                id: upperIndicators
                spacing: 2
                readonly property bool alwaysActive: true
                readonly property bool reversed: true

                Repeater {
                    model: parentItem.isTask && (parentItem.isActive || parentItem.hasActive) ? 1 : 0
                    delegate: triangleComponent
                }
            }

            Grid {
                id: lowerIndicators
                rows: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? 1 : Math.min(3, parentItem.windowsCount)
                columns: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? Math.min(3, parentItem.windowsCount) : 1
                rowSpacing: 2
                columnSpacing: 2

                readonly property bool alwaysActive: false
                readonly property bool reversed: false

                Repeater {
                    model: Math.min(3, parentItem.windowsCount)
                    delegate: triangleComponent
                }
            }


            //! Triangle Indicator Component
            Component {
                id: triangleComponent
                Canvas {
                    id: canvas
                    width: parentItem.currentIconSize / 6
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
                        if (!parent.alwaysActive && parentItem.windowsMinimizedCount!==0
                                && ((index < maxDrawnMinimizedWindows)
                                    || (parentItem.windowsCount === parentItem.windowsMinimizedCount))) {
                            return false;
                        }

                        return true;
                    }

                    readonly property int lineWidth: 2

                    onFillTriangleChanged: requestPaint();
                    onDrawColorChanged: requestPaint();

                    onPaint: {
                        var ctx = getContext('2d');
                        ctx.clearRect(0, 0, canvas.width, canvas.height);
                        ctx.strokeStyle = indicatorItem.backgroundColor;
                        ctx.lineWidth = lineWidth;

                        ctx.beginPath();
                        ctx.moveTo(0, canvas.height);
                        ctx.lineTo(canvas.width/2, 0);
                        ctx.lineTo(canvas.width, canvas.height);
                        ctx.lineTo(0, canvas.height);
                        ctx.closePath();
                        ctx.stroke();

                        ctx.strokeStyle = drawColor;
                        ctx.fillStyle = fillTriangle ? drawColor : indicatorItem.backgroundColor;

                        ctx.beginPath();
                        ctx.moveTo(lineWidth, canvas.height - lineWidth);
                        ctx.lineTo(canvas.width/2, lineWidth);
                        ctx.lineTo(canvas.width - lineWidth, canvas.height - lineWidth);
                        ctx.lineTo(lineWidth, canvas.height - lineWidth);
                        ctx.closePath();
                        ctx.stroke();
                        ctx.fill();
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
    }
}
