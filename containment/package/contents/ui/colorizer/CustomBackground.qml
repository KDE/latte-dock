/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Item{
    id: main
    clip: true

    property int roundness: 0
    property color backgroundColor
    property color borderColor: "transparent"
    property int borderWidth: 0

    property bool topBorder: false
    property bool leftBorder: false
    property bool bottomBorder: false
    property bool rightBorder: false

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


    readonly property bool drawWithoutRoundness: noOfBorders === 1

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

    Rectangle{
        id: painter
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

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
                } else if (noOfBorders === 2 || (noOfBorders === 3 && bothHorizontals)) {
                    return parent.width + Math.max(roundness, 2 * borderWidth);
                }
            }

            return parent.width;
        }


        height: {
            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                if (drawWithoutRoundness) {
                    return parent.height + 2 * borderWidth;
                } else if (noOfBorders === 2 || (noOfBorders === 3 && bothVerticals)) {
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
        color: parent.backgroundColor
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
                        if (allBorders || (noOfBorders === 3 && bothHorizontals)) {
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
                        if (allBorders || (noOfBorders === 3 && bothVerticals)) {
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
