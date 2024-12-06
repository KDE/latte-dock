/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.latte.core 0.2 as LatteCore

Item {
    property Item destination: null

    readonly property int headThickness: destination ? destination.headThicknessMargin : appletItem.metrics.margin.headThickness
    readonly property int tailThickness: destination ? destination.tailThicknessMargin : appletItem.metrics.margin.tailThickness
    readonly property int thickness: headThickness + tailThickness + (root.isHorizontal ? destination.height : destination.width)
    readonly property int lengthPadding: {
        if ( (root.myView.alignment === LatteCore.Types.Justify && appletItem.firstChildOfStartLayout)
                || (root.myView.alignment === LatteCore.Types.Justify && appletItem.lastChildOfEndLayout)
                || (root.myView.alignment !== LatteCore.Types.Justify && appletItem.firstChildOfMainLayout)
                || (root.myView.alignment !== LatteCore.Types.Justify && appletItem.lastChildOfMainLayout)) {
            //! Fitts Law on corners
            return destination ? destination.zoomScaleThickness * appletItem.lengthAppletFullMargin : appletItem.lengthAppletFullMargin;
        }

        return destination ? destination.zoomScaleThickness * appletItem.lengthAppletPadding : appletItem.lengthAppletPadding;
    }

    readonly property bool active: parent ? parent.active : false

    Loader{
        anchors.fill: parent
        active: appletItem.debug.eventsSinkEnabled && active

        sourceComponent: Rectangle {
            anchors.fill: parent
            color: "yellow"
            opacity: 0.2
            visible: root.latteView && root.latteView.sink.destinationItem === destination
        }
    }

    Item {
        id: originParentItem
        anchors.fill: parent

        //! NOTICE: Do not add any more child items. These child items are used from SinkedEvents handler in order
        //! to identify the areas from which mouse events should be sunk into the destination item

        EventsSinkOriginArea {
            id: topArea
            width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? destination.width + 2 * lengthPadding : thickness
            height: {
                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    //! Fitt;s Law consider the spacer also from parabolic effect
                    return appletItem.firstAppletInContainer ? lengthPadding + hiddenSpacerLeft.height + 1 : lengthPadding;
                } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                    return tailThickness;
                } else {
                    return headThickness;
                }
            }

            states:[
                State{
                    name: "horizontal"
                    when: plasmoid.formFactor === PlasmaCore.Types.Horizontal

                    AnchorChanges{
                        target: topArea;
                        anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.top;
                    }
                },
                State{
                    name: "vertical"
                    when: plasmoid.formFactor === PlasmaCore.Types.Vertical

                    AnchorChanges{
                        target: topArea;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: leftArea.left; anchors.top: undefined; anchors.bottom: parent.top;
                    }
                }
            ]
        }

        EventsSinkOriginArea {
            id: bottomArea
            width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? destination.width + 2 * lengthPadding : thickness
            height: {
                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    //! Fitt;s Law consider the spacer also from parabolic effect
                    return appletItem.lastAppletInContainer ? lengthPadding + hiddenSpacerRight.height + 1 : lengthPadding;
                } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    return tailThickness;
                } else {
                    return headThickness;
                }
            }

            states:[
                State{
                    name: "horizontal"
                    when: plasmoid.formFactor === PlasmaCore.Types.Horizontal

                    AnchorChanges{
                        target: bottomArea;
                        anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: undefined; anchors.top: parent.bottom; anchors.bottom: undefined;
                    }
                },
                State{
                    name: "vertical"
                    when: plasmoid.formFactor === PlasmaCore.Types.Vertical

                    AnchorChanges{
                        target: bottomArea;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: leftArea.left; anchors.top: parent.bottom; anchors.bottom: undefined;
                    }
                }
            ]
        }

        EventsSinkOriginArea {
            id: leftArea
            height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? thickness : destination.height + 2 * lengthPadding
            width: {
                if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                    //! Fitt;s Law consider the spacer also from parabolic effect
                    return appletItem.firstAppletInContainer ? lengthPadding + hiddenSpacerLeft.width + 1 : lengthPadding;
                } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    return tailThickness;
                } else {
                    return headThickness;
                }
            }

            states:[
                State{
                    name: "horizontal"
                    when: plasmoid.formFactor === PlasmaCore.Types.Horizontal

                    AnchorChanges{
                        target: leftArea;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: undefined;
                        anchors.right: parent.left; anchors.left: undefined; anchors.top: undefined; anchors.bottom: bottomArea.bottom;
                    }
                },
                State{
                    name: "vertical"
                    when: plasmoid.formFactor === PlasmaCore.Types.Vertical

                    AnchorChanges{
                        target: leftArea;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                        anchors.right: parent.left; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                    }
                }
            ]
        }

        EventsSinkOriginArea {
            id: rightArea
            height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? thickness : destination.height + 2 * lengthPadding
            width: {
                if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                    //! Fitt;s Law consider the spacer also from parabolic effect
                    return appletItem.lastAppletInContainer ? lengthPadding + hiddenSpacerRight.width + 1 : lengthPadding;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    return tailThickness;
                } else {
                    return headThickness;
                }
            }

            states:[
                State{
                    name: "horizontal"
                    when: plasmoid.formFactor === PlasmaCore.Types.Horizontal

                    AnchorChanges{
                        target: rightArea;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: parent.right; anchors.top: undefined; anchors.bottom: bottomArea.bottom;
                    }
                },
                State{
                    name: "vertical"
                    when: plasmoid.formFactor === PlasmaCore.Types.Vertical

                    AnchorChanges{
                        target: rightArea;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                        anchors.right: undefined; anchors.left: parent.right; anchors.top: undefined; anchors.bottom: undefined;
                    }
                }
            ]
        }
    }
}
