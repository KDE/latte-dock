/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.components 1.0 as LatteComponents

LatteComponents.IndicatorItem {
    id: root

    needsMouseEventCoordinates: true
    providesClickedAnimation: clickedAnimationEnabled
    //providesHoveredAnimation: true
    providesFrontLayer: true
    svgImagePaths: ["widgets/tasks"]

    enabledForApplets: true
    /*enabledForApplets: configurationIsReady && indicator.configuration.clickedAnimationEnabled !== undefined ?
                           indicator.configuration.enabledForApplets : true*/
    lengthPadding: configurationIsReady && indicator.configuration.clickedAnimationEnabled !== undefined ?
                       indicator.configuration.lengthPadding : 0.08

    backgroundCornerMargin: configurationIsReady && indicator.configuration.backgroundCornerMargin !== undefined ?
                                indicator.configuration.backgroundCornerMargin : 1.00

    //! config options
    readonly property bool clickedAnimationEnabled: configurationIsReady
                                                    && indicator.configuration.clickedAnimationEnabled !== undefined
                                                    && indicator.configuration.clickedAnimationEnabled

    /*readonly property bool reversedEnabled: configurationIsReady
                                            && indicator.configuration.reversed !== undefined
                                            && indicator.configuration.reversed*/


    readonly property bool configurationIsReady: indicator && indicator.configuration

    //! Background Layer
    Loader{
        id: backLayer
        anchors.fill: parent
        anchors.topMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? indicator.screenEdgeMargin : 0
        anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? indicator.screenEdgeMargin : 0
        anchors.leftMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? indicator.screenEdgeMargin : 0
        anchors.rightMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? indicator.screenEdgeMargin : 0

        active: level.isBackground && indicator.isTask && !indicator.isEmptySpace
        sourceComponent: TaskBackLayer{}
    }

    //! Applet Background Layer
    Loader{
        id: appletBackLayer
        anchors.fill: parent
        anchors.topMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? indicator.screenEdgeMargin : 0
        anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? indicator.screenEdgeMargin : 0
        anchors.leftMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? indicator.screenEdgeMargin : 0
        anchors.rightMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? indicator.screenEdgeMargin : 0

        active: level.isBackground && indicator.isApplet && !indicator.isEmptySpace
        sourceComponent: AppletBackLayer{}
    }

    /* progress overlay for BackLayer*/
    /* it is not added in the BackLayer because the BackLayer is rotated in some cases*/
    Loader {
        anchors.fill: backLayer
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

    function taskPrefixHovered(prefix) {
        var effectivePrefix = taskPrefix(prefix);

        if ("" !== prefix)
            effectivePrefix = [
                ...taskPrefix(prefix + "-hover"),
                ...taskPrefix("hover"),
                ...effectivePrefix
            ];

        return effectivePrefix;
    }
}
