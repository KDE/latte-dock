/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item{
    id: main
    clip: !kirigamiRect.active

    property int borderWidth: 0
    property int roundness: 0
    property int shadowSize: 0

    property real backgroundOpacity: 1.0

    property color backgroundColor: "black"
    property color borderColor: "transparent"
    property color shadowColor: "black"

    property bool topBorder: false
    property bool leftBorder: false
    property bool bottomBorder: false
    property bool rightBorder: false

    property bool onlyHorizontalBorders: !leftBorder && !rightBorder
    property bool onlyVerticalBorders: !topBorder && !bottomBorder

    property int noOfBorders: {
        if (allBorders) {
            return 4;
        }

        var i = 0;

        if (topBorder) {
            i = i + 1;
        }
        if (leftBorder) {
            i = i + 1;
        }
        if (rightBorder) {
            i = i + 1;
        }
        if (bottomBorder) {
            i = i + 1;
        }

        return i;
    }

    readonly property bool allBorders: (latteView && latteView.effects && (latteView.effects.enabledBorders === PlasmaCore.FrameSvg.AllBorders))

    readonly property bool bothVerticals: (leftBorder && rightBorder)
    readonly property bool bothHorizontals: (bottomBorder && topBorder)

    readonly property bool drawWithoutRoundness: onlyHorizontalBorders || onlyVerticalBorders

    readonly property Item painterRectangle: painter

    Binding{
        target: main
        property: "topBorder"
        when: latteView
        value: {
            return (latteView && latteView.effects && ((latteView.effects.enabledBorders & PlasmaCore.FrameSvg.TopBorder)) > 0) || allBorders;
        }
    }

    Binding{
        target: main
        property: "leftBorder"
        when: latteView
        value: {
            return (latteView && latteView.effects && ((latteView.effects.enabledBorders & PlasmaCore.FrameSvg.LeftBorder)) > 0) || allBorders;
        }
    }

    Binding{
        target: main
        property: "bottomBorder"
        when: latteView
        value: {
            return (latteView && latteView.effects && ((latteView.effects.enabledBorders & PlasmaCore.FrameSvg.BottomBorder)) > 0) || allBorders;
        }
    }

    Binding{
        target: main
        property: "rightBorder"
        when: latteView
        value: {
            return (latteView && latteView.effects && ((latteView.effects.enabledBorders & PlasmaCore.FrameSvg.RightBorder)) > 0) || allBorders;
        }
    }

    Loader {
        id: kirigamiRect
        anchors.fill: painter
        active: root.kirigamiLibraryIsFound && main.shadowSize>0
        //! this "source" approach is needed in order for KF5<=5.68 to load Latte correctly with no
        //! qml breakage because Kirigami2.ShadowedRectangle is not present
        source: root.kirigamiLibraryIsFound ? "KirigamiShadowedRectangle.qml" : "NormalRectangle.qml"
    }

    Rectangle{
        id: painter
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: backgroundOpacity

        width: {
            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                if (drawWithoutRoundness) {
                    return parent.width + 2*borderWidth;
                } else if (noOfBorders === 2 || (noOfBorders === 3 && bothHorizontals)) {
                    return parent.width + Math.max(roundness, 2*borderWidth);
                } else if (noOfBorders === 3 && bothVerticals) {
                    return parent.width;
                }
            } else if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                if (drawWithoutRoundness) {
                    return parent.width + 2 * borderWidth;
                } else if ((noOfBorders === 2 && !bothVerticals) || (noOfBorders === 3 && bothHorizontals)) {
                    return parent.width + Math.max(roundness, 2 * borderWidth);
                }
            }

            return parent.width;
        }


        height: {
            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                if (drawWithoutRoundness) {
                    return parent.height + 2 * borderWidth;
                } else if ((noOfBorders === 2 && !bothHorizontals) || (noOfBorders === 3 && bothVerticals)) {
                    return parent.height + Math.max(roundness,2 * borderWidth);
                }
            } else if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                if (drawWithoutRoundness) {
                    return parent.height + 2*borderWidth;
                } else if (noOfBorders === 2 || (noOfBorders === 3 && bothVerticals)) {
                    return parent.height + Math.max(roundness, 2*borderWidth);
                } else if (noOfBorders === 3 && bothHorizontals) {
                    return parent.height;
                }
            }

            return parent.height;
        }

        radius: drawWithoutRoundness ? 0 : roundness
        color: main.backgroundColor
        border.width: main.borderWidth
        border.color: main.borderColor

        readonly property int centerStep: {
            if (allBorders) {
                return 0;
            }

            return Math.max(Math.ceil((drawWithoutRoundness ? 0 : roundness) / 2) , borderWidth)
        }

        states: [
            State {
                name: "horizontal"
                when: (plasmoid.formFactor === PlasmaCore.Types.Horizontal)

                PropertyChanges{
                    target: painter
                    anchors.horizontalCenterOffset: {
                        if (drawWithoutRoundness || (noOfBorders === 3 && bothVerticals)) {
                            return 0;
                        } else if (noOfBorders === 2 || (noOfBorders === 3 && bothHorizontals)) {
                            if (leftBorder) {
                                return centerStep;
                            } else if (rightBorder) {
                                return -centerStep;
                            }
                        }

                        return 0;
                    }
                    anchors.verticalCenterOffset: {
                        if (allBorders || (noOfBorders>=2 && bothHorizontals)) {
                            return 0;
                        }

                        //bottom edge and top edge
                        return plasmoid.location === PlasmaCore.Types.TopEdge ? -centerStep : centerStep;
                    }
                }
            },
            State {
                name: "vertical"
                when: (plasmoid.formFactor === PlasmaCore.Types.Vertical)

                PropertyChanges{
                    target: painter
                    anchors.verticalCenterOffset: {
                        if (drawWithoutRoundness || (noOfBorders === 3 && bothHorizontals)) {
                            return 0;
                        } else if (noOfBorders === 2 || (noOfBorders === 3 && bothVerticals)) {
                            if (bottomBorder) {
                                return -centerStep;
                            } else if (topBorder) {
                                return centerStep;
                            }
                        }

                        return 0;
                    }
                    anchors.horizontalCenterOffset: {
                        if (allBorders || (noOfBorders>=2 && bothVerticals)) {
                            return 0;
                        }

                        //left edge and right edge
                        return plasmoid.location === PlasmaCore.Types.LeftEdge ? -centerStep : centerStep;
                    }
                }
            }
        ]
    }

}
