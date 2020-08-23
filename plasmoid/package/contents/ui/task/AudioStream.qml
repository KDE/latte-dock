/***************************************************************************
 *   Copyright (C) 2017 Kai Uwe Broulik <kde@privat.broulik.de>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

Item {
    id: background

    Item {
        id: subRectangle
        width: parent.width/ 2
        height: width

        states: [
            State {
                name: "default"
                when: (root.location !== PlasmaCore.Types.RightEdge)

                AnchorChanges {
                    target: subRectangle
                    anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;}
                }
            },
            State {
                name: "right"
                when: (root.location === PlasmaCore.Types.RightEdge)

                AnchorChanges {
                    target: subRectangle
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;}
                }
            }
        ]

        LatteComponents.BadgeText {
            anchors.centerIn: parent
            // To make the icon clearer on the small panel size, we need to enlarge it slightly when the compact badges option enabled.
            width: 0.8 * parent.width * (root.badgesCompactStyle ? 1.1 : 1)
            height: width
            minimumWidth: width
            maximumWidth: width

            fullCircle: true
            showNumber: false
            showText: true

            color: theme.backgroundColor
            borderColor: root.lightTextColor
            proportion: 0
            radiusPerCentage: 100

            style3d: root.badges3DStyle
            styleCompact: root.badgesCompactStyle

            LatteCore.IconItem{
                id: audioStreamIcon
                anchors.centerIn: parent
                // To make the icon clearer on the small panel size, we need to enlarge it slightly when the compact badges option enabled.
                width: 0.9*parent.width * (root.badgesCompactStyle ? 1.05 : 1)
                height: width
                colorGroup: PlasmaCore.Theme.ButtonColorGroup
                usesPlasmaTheme: true

                //opacity: taskItem.playingAudio && !taskItem.muted ? 1 : 0.85
                source: {
                    if (taskItem.volume <= 0 || taskItem.muted) {
                        return "audio-volume-muted";
                    } else if (taskItem.volume <= 25) {
                        return "audio-volume-low";
                    } else if (taskItem.volume <= 75) {
                        return "audio-volume-medium";
                    } else {
                        return "audio-volume-high" ;
                    }
                }

                MouseArea{
                    id: audioBadgeMouseArea
                    anchors.fill: parent
                    enabled: root.audioBadgeActionsEnabled

                    property bool wheelIsBlocked: false;

                    onClicked: {
                        taskItem.toggleMuted();
                    }

                    onWheel: {
                        if (wheelIsBlocked) {
                            return;
                        }

                        wheelIsBlocked = true;
                        scrollDelayer.start();

                        var angle = wheel.angleDelta.y / 8;

                        if (angle > 2) {
                            taskItem.increaseVolume();
                        } else if (angle < -2) {
                            taskItem.decreaseVolume();
                        }
                    }

                    //! A timer is needed in order to handle also touchpads that probably
                    //! send too many signals very fast. This way the signals per sec are limited.
                    //! The user needs to have a steady normal scroll in order to not
                    //! notice a annoying delay
                    Timer{
                        id: scrollDelayer

                        interval: 80

                        onTriggered: audioBadgeMouseArea.wheelIsBlocked = false;
                    }
                }
            }
        }
    }
}
