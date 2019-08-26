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

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

LatteComponents.IndicatorItem {
    id: root

    needsMouseEventCoordinates: true
    providesClickedAnimation: clickedAnimationEnabled
    //providesHoveredAnimation: true
    providesFrontLayer: true
    svgImagePaths: ["widgets/tasks"]

    //! config options
    readonly property bool clickedAnimationEnabled: indicator && indicator.configuration
                                                    && indicator.configuration.clickedAnimationEnabled !== undefined
                                                    && indicator.configuration.clickedAnimationEnabled

    readonly property bool reversedEnabled: indicator && indicator.configuration
                                            && indicator.configuration.reversed !== undefined
                                            && indicator.configuration.reversed


    //! Background Layer
    Loader{
        id: backLayer
        anchors.fill: parent
        active: level.isBackground

        sourceComponent: BackLayer{}
    }

    /* progress overlay for BackLayer*/
    /* it is not added in the BackLayer because the BackLayer is rotated in some cases*/
    Loader {
        anchors.fill: parent
        asynchronous: true
        active: level.isBackground && indicator.progressVisible
        sourceComponent: Item {
            id: background

            Item {
                anchors {
                    top: parent.top
                    left: parent.left
                    bottom: parent.bottom
                }

                width: parent.width * (Math.min(indicator.progress, 100) / 100)
                clip: true

                PlasmaCore.FrameSvgItem {
                    id: progressFrame
                    width: background.width
                    height: background.height

                    imagePath: "widgets/tasks"
                    prefix: root.taskPrefix("progress").concat(root.taskPrefix("hover"))
                }
            }
        }
    }

    //! Foreground Layer to draw arrows
    Loader{
        id: frontLayer
        anchors.fill: parent
        active: (level.isForeground && !indicator.isApplet && indicator.isGroup) || providesClickedAnimation

        sourceComponent: FrontLayer{}
    }


    function taskPrefix(prefix) {
        var effectivePrefix;

        if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            effectivePrefix = "west-" + prefix;
        }

        if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            effectivePrefix = "north-" + prefix;
        }

        if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            effectivePrefix = "east-" + prefix;
        }

        if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
            effectivePrefix = "south-" + prefix;
        }

        return [effectivePrefix, prefix];
    }
}
