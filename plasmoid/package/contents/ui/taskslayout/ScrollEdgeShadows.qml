/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: shadowsContainer
    opacity: 0.4

    readonly property int gradientLength: appletAbilities.metrics.iconSize / 3
    readonly property int thickness: appletAbilities.metrics.backgroundThickness
    readonly property color appliedColor: appletAbilities.myView.itemShadow.shadowSolidColor

    property Item flickable

    LinearGradient {
        id: firstGradient
        width: !root.vertical ? gradientLength : shadowsContainer.thickness
        height: !root.vertical ? shadowsContainer.thickness : gradientLength

        start: Qt.point(0, 0)
        end: !root.vertical ? Qt.point(width, 0) : Qt.point(0,height)

        gradient: Gradient {
            GradientStop { position: 0.0; color: (scrollableList.currentPos > scrollableList.scrollFirstPos ? appliedColor : "transparent") }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    LinearGradient {
        id: lastGradient
        width: firstGradient.width
        height: firstGradient.height
        start: firstGradient.start
        end: firstGradient.end

        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: (scrollableList.currentPos < scrollableList.scrollLastPos ? appliedColor : "transparent") }
        }
    }

    states: [
        State {
            name: "bottom"
            when: root.location === PlasmaCore.Types.BottomEdge

            AnchorChanges {
                target: firstGradient
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: lastGradient
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsContainer
                anchors{ top:undefined; bottom:flickable.bottom; left:undefined; right:undefined;
                    horizontalCenter:flickable.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges {
                target: shadowsContainer
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin: 0;    anchors.bottomMargin: appletAbilities.metrics.margin.screenEdge;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "top"
            when: root.location === PlasmaCore.Types.TopEdge

            AnchorChanges {
                target: firstGradient
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: lastGradient
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsContainer
                anchors{ top:flickable.top; bottom:undefined; left:undefined; right:undefined;
                    horizontalCenter:flickable.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges {
                target: shadowsContainer
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin: appletAbilities.metrics.margin.screenEdge;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "left"
            when: root.location === PlasmaCore.Types.LeftEdge

            AnchorChanges {
                target: firstGradient
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: lastGradient
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsContainer
                anchors{ top:undefined; bottom:undefined; left:flickable.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:flickable.verticalCenter}
            }
            PropertyChanges {
                target: shadowsContainer
                anchors.leftMargin: appletAbilities.metrics.margin.screenEdge;    anchors.rightMargin:0;     anchors.topMargin: 0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "right"
            when: root.location === PlasmaCore.Types.RightEdge

            AnchorChanges {
                target: firstGradient
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: lastGradient
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
            AnchorChanges {
                target: shadowsContainer
                anchors{ top:undefined; bottom:undefined; left:undefined; right:flickable.right;
                    horizontalCenter:undefined; verticalCenter:flickable.verticalCenter}
            }
            PropertyChanges {
                target: shadowsContainer
                anchors.leftMargin: 0;    anchors.rightMargin: appletAbilities.metrics.margin.screenEdge;     anchors.topMargin: 0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        }
    ]
}
