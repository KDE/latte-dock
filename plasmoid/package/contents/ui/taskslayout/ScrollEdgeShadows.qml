/*
 * Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: shadowsContainer
    opacity: 0.4

    readonly property int gradientLength: root.iconSize / 3
    readonly property int thickness: latteView ? latteView.realPanelSize : root.iconSize + root.thickMargins
    readonly property color appliedColor: root.appShadowColorSolid

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
            when: plasmoid.location === PlasmaCore.Types.BottomEdge

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
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin: 0;    anchors.bottomMargin:root.screenEdgeMargin;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "top"
            when: plasmoid.location === PlasmaCore.Types.TopEdge

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
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin: root.screenEdgeMargin;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "left"
            when: plasmoid.location === PlasmaCore.Types.LeftEdge

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
                anchors.leftMargin: root.screenEdgeMargin;    anchors.rightMargin:0;     anchors.topMargin: 0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        },
        State {
            name: "right"
            when: plasmoid.location === PlasmaCore.Types.RightEdge

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
                anchors.leftMargin: 0;    anchors.rightMargin:root.screenEdgeMargin;     anchors.topMargin: 0;    anchors.bottomMargin:0;
                anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
            }
        }
    ]
}
