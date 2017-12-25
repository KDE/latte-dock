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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components

import QtGraphicalEffects 1.0

Item{
    //   property string color
    id: glowItem

    property bool roundCorners: true
    property bool showAttention: false

    property bool showGlow: false
    property int animation: Math.max(1.65*3*units.longDuration,root.durationTime*3*units.longDuration)

    property color attentionColor: colorScopePalette.negativeTextColor // "#ffff1717"
    property color basicColor: "blue"

    property color animationColor
    property color currentColor: glowItem.showAttention ? animationColor : basicColor

    readonly property real glowOpacity: root.glowOpacity

    Grid{
        id: mainGlow
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        opacity: glowOpacity
        visible: glowItem.showGlow

        rows: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? 1 : 0
        columns: plasmoid.formFactor === PlasmaCore.Types.Vertical ? 1 : 0

        property int halfCorner: 3*glowFrame.size
        property int fullCorner: 6*glowFrame.size

        Item {
            id: firstGlowCorner
            width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? mainGlow.halfCorner : mainGlow.fullCorner
            height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? mainGlow.fullCorner : mainGlow.halfCorner
            clip: true

            Item {
                id: firstGlowCornerFull
                width: mainGlow.fullCorner
                height: mainGlow.fullCorner

                RadialGradient {
                    anchors.fill: parent
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 0.07; color: "transparent" }
                        GradientStop { position: 0.125; color: glowItem.currentColor }
                        GradientStop { position: 0.4; color: "transparent" }
                        GradientStop { position: 1; color: "transparent" }
                    }
                }
            }
        }

        Item {
            id:mainGlowPart
            width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? glowItem.width - glowFrame.size : mainGlow.fullCorner
            height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? mainGlow.fullCorner : glowItem.height - glowFrame.size

            LinearGradient {
                anchors.fill: parent
                start: {
                    if (plasmoid.location === PlasmaCore.Types.BottomEdge)
                        return Qt.point(0, 0);
                    else if (plasmoid.location === PlasmaCore.Types.TopEdge)
                        return Qt.point(0, mainGlow.fullCorner);
                    else if (plasmoid.location === PlasmaCore.Types.LeftEdge)
                        return Qt.point(mainGlow.fullCorner, 0);
                    else if (plasmoid.location === PlasmaCore.Types.RightEdge)
                        return Qt.point(0, 0);

                    return Qt.point(mainGlow.fullCorner, 0);
                }
                end: {
                    if (plasmoid.location === PlasmaCore.Types.BottomEdge)
                        return Qt.point(0, mainGlow.fullCorner);
                    else if (plasmoid.location === PlasmaCore.Types.TopEdge)
                        return Qt.point(0, 0);
                    else if (plasmoid.location === PlasmaCore.Types.LeftEdge)
                        return Qt.point(0,0);
                    else if (plasmoid.location === PlasmaCore.Types.RightEdge)
                        return Qt.point(mainGlow.fullCorner, 0);

                    return Qt.point(0,0);
                }

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 0.08; color: "transparent" }
                    GradientStop { position: 0.37; color: glowItem.currentColor }
                    GradientStop { position: 0.49; color: "transparent" }
                }
            }
        }

        Item {
            id:lastGlowCorner
            width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? mainGlow.halfCorner : mainGlow.fullCorner
            height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? mainGlow.fullCorner : mainGlow.halfCorner
            clip: true

            Item {
                id: lastGlowCornerFull
                anchors.right: parent.right
                width: mainGlow.fullCorner
                height: mainGlow.fullCorner

                RadialGradient {
                    anchors.fill: parent
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 0.07; color: "transparent" }
                        GradientStop { position: 0.125; color: glowItem.currentColor }
                        GradientStop { position: 0.4; color: "transparent" }
                        GradientStop { position: 1; color: "transparent" }
                    }
                }

                states: [
                    State{
                        name: "*"
                        when:  plasmoid.formFactor === PlasmaCore.Types.Horizontal

                        AnchorChanges{
                            target:lastGlowCornerFull;
                            anchors{ bottom: undefined; right:parent.right;}
                        }
                    },
                    State{
                        name: "vertical"
                        when:  plasmoid.formFactor === PlasmaCore.Types.Vertical

                        AnchorChanges{
                            target:lastGlowCornerFull;
                            anchors{ bottom: parent.bottom; right:undefined;}
                        }
                    }
                ]
            }
        }
    }

    Item{
        id:mainElement
        anchors.fill: parent

        Rectangle {
            id: smallCircle
            anchors.centerIn: parent
            anchors.fill: parent

            color: glowItem.basicColor
            radius: glowItem.roundCorners ? Math.min(width,height) / 2 : 0
            visible: !glowItem.showAttention
        }

        Loader{
            anchors.centerIn: parent
            anchors.fill: parent

            active: glowItem.showAttention

            sourceComponent:Rectangle {
                id: smallCircleInAttention

                color: glowItem.animationColor
                radius: smallCircle.radius

                SequentialAnimation{
                    running: glowItem.showAttention
                    loops: Animation.Infinite
                    alwaysRunToEnd: true

                    PropertyAnimation {
                        target: glowItem
                        property: "animationColor"
                        to: glowItem.attentionColor
                        duration: glowItem.animation
                        easing.type: Easing.InOutQuad
                    }

                    PropertyAnimation {
                        target: glowItem
                        property: "animationColor"
                        to: glowItem.basicColor
                        duration: glowItem.animation
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        Rectangle {
            visible: glowItem.showGlow && root.glow3D
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            anchors.horizontalCenterOffset: {
                if (plasmoid.formFactor === PlasmaCore.Types.Horizontal)
                    return 0;
                else if (plasmoid.location === PlasmaCore.Types.LeftEdge)
                    return -glowItem.width / 7;
                else if (plasmoid.location === PlasmaCore.Types.RightEdge)
                    return glowItem.width / 7;
            }
            anchors.verticalCenterOffset: {
                if (plasmoid.formFactor === PlasmaCore.Types.Vertical)
                    return 0;
                else if (plasmoid.location === PlasmaCore.Types.BottomEdge)
                    return glowItem.height / 7;
                else if (plasmoid.location === PlasmaCore.Types.TopEdge)
                    return -glowItem.height / 7;
            }

            width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? Math.max(mainGlowPart.width, shadow) : shadow
            height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? shadow : Math.max(mainGlowPart.height, shadow)
            radius: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? height/2 : width/2

            property int shadow: glowFrame.size / 3

            color: root.appShadowColor
            opacity: 0.2
        }
    }
}
