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

    readonly property int gradientLength: root.iconSize / 3

    LinearGradient {
        id: firstGradient
        width: !root.vertical ? gradientLength : scrollableList.thickness
        height: !root.vertical ? scrollableList.thickness : gradientLength

        start: Qt.point(0, 0)
        end: !root.vertical ? Qt.point(gradientLength, 0) : Qt.point(0, gradientLength)

        gradient: Gradient {
            GradientStop { position: 0.0; color: scrollableList.currentPos <= scrollableList.scrollFirstPos ? "white" : "transparent" }
            GradientStop { position: 1.0; color: "white" }
        }
    }

    Rectangle {
        id: centralRectangle
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: !root.vertical ? length : firstGradient.width
        height: !root.vertical ? firstGradient.height : length
        color: "white"

        property int length: !root.vertical ? parent.width - 2*gradientLength : parent.height - 2*gradientLength
    }

    LinearGradient {
        id: lastGradient
        anchors.top: parent.top
        anchors.right: parent.right
        width: firstGradient.width
        height: firstGradient.height

        start: firstGradient.start
        end: firstGradient.end

        gradient: Gradient {
            GradientStop { position: 0.0; color: "white" }
            GradientStop { position: 1.0; color: (scrollableList.currentPos >= scrollableList.scrollLastPos ? "white" : "transparent") }
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
                target: centralRectangle
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;
                    horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: lastGradient
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:undefined}
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
                target: centralRectangle
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined;
                    horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            AnchorChanges {
                target: lastGradient
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:undefined}
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
                target: centralRectangle
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: lastGradient
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:undefined}
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
                target: centralRectangle
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            AnchorChanges {
                target: lastGradient
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:undefined}
            }
        }
    ]
}
