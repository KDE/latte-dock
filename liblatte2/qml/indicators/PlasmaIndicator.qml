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

Item {
    readonly property Item options: parent.manager
    readonly property Item rootItem: parent

    readonly property bool providesFrontLayer: true

    //! Background Layer
    Loader{
        id: backLayer
        anchors.fill: parent
        active: rootItem.isBackLayer

        sourceComponent: PlasmaCore.FrameSvgItem {
            id: frame
            property string basePrefix: "normal"

            imagePath: options.explicit.usePlasmaTabsStyle ? "widgets/tabbar" : "widgets/tasks"

            prefix: {
                if (options.explicit.usePlasmaTabsStyle) {
                    if (!options.isActive) {
                        return "";
                    }

                    if ((plasmoid.location === PlasmaCore.Types.LeftEdge && !options.common.reversedEnabled)
                            || (plasmoid.location === PlasmaCore.Types.RightEdge && options.common.reversedEnabled)) {
                        return "west-active-tab";
                    }

                    if ((plasmoid.location === PlasmaCore.Types.TopEdge && !options.common.reversedEnabled)
                            || (plasmoid.location === PlasmaCore.Types.BottomEdge && options.common.reversedEnabled)) {
                        return "north-active-tab";
                    }

                    if ((plasmoid.location === PlasmaCore.Types.RightEdge && !options.common.reversedEnabled)
                            || (plasmoid.location === PlasmaCore.Types.LeftEdge && options.common.reversedEnabled)) {
                        return "east-active-tab";
                    }

                    if ((plasmoid.location === PlasmaCore.Types.BottomEdge && !options.common.reversedEnabled)
                            || (plasmoid.location === PlasmaCore.Types.TopEdge && options.common.reversedEnabled)) {
                        return "south-active-tab";
                    }

                    return  "south-active-tab";
                } else {
                    return taskPrefix(basePrefix);
                }
            }

            function taskPrefix(prefix) {
                var effectivePrefix;

                if ((plasmoid.location === PlasmaCore.Types.LeftEdge && !options.common.reversedEnabled)
                        || (plasmoid.location === PlasmaCore.Types.RightEdge && options.common.reversedEnabled)) {
                    effectivePrefix = "west-" + prefix;
                }

                if ((plasmoid.location === PlasmaCore.Types.TopEdge && !options.common.reversedEnabled)
                        || (plasmoid.location === PlasmaCore.Types.BottomEdge && options.common.reversedEnabled)) {
                    effectivePrefix = "north-" + prefix;
                }

                if ((plasmoid.location === PlasmaCore.Types.RightEdge && !options.common.reversedEnabled)
                        || (plasmoid.location === PlasmaCore.Types.LeftEdge && options.common.reversedEnabled)) {
                    effectivePrefix = "east-" + prefix;
                }

                if ((plasmoid.location === PlasmaCore.Types.BottomEdge && !options.common.reversedEnabled)
                        || (plasmoid.location === PlasmaCore.Types.TopEdge && options.common.reversedEnabled)) {
                    effectivePrefix = "south-" + prefix;
                }

                return [effectivePrefix, prefix];
            }

            states: [
                State {
                    name: "launcher"
                    when: options.isLauncher || (options.isApplet && !options.isActive)

                    PropertyChanges {
                        target: frame
                        basePrefix: ""
                    }
                },
                State {
                    name: "hovered"
                    when: options.isHovered && frame.hasElementPrefix("hover")

                    PropertyChanges {
                        target: frame
                        basePrefix: "hover"
                    }
                },
                State {
                    name: "attention"
                    when: options.inAttention

                    PropertyChanges {
                        target: frame
                        basePrefix: "attention"
                    }
                },
                State {
                    name: "minimized"
                    when: options.isMinimized

                    PropertyChanges {
                        target: frame
                        basePrefix: "minimized"
                    }
                },
                State {
                    name: "active"
                    when: options.isActive

                    PropertyChanges {
                        target: frame
                        basePrefix: "focus"
                    }
                }
            ]
        }
    }

    //! Foreground Layer to draw arrows
    Loader{
        id: frontLayer
        anchors.fill: parent
        active: !rootItem.isBackLayer && !options.isApplet && options.isGroup

        sourceComponent: Item {
            anchors.fill: parent

            PlasmaCore.Svg {
                id: taskSvg
                imagePath: "widgets/tasks"
            }

            Item {
                id: iconBox
                anchors.centerIn: parent
                width: options.currentIconSize
                height: width
            }

            PlasmaCore.SvgItem {
                id: arrow

                implicitWidth: 0.25 * iconBox.width
                implicitHeight: implicitWidth

                svg: taskSvg
                elementId: elementForLocation(plasmoid.location)

                function elementForLocation(location) {
                    switch (location) {
                    case PlasmaCore.Types.LeftEdge:
                        return "group-expander-left";
                    case PlasmaCore.Types.TopEdge:
                        return "group-expander-top";
                    case PlasmaCore.Types.RightEdge:
                        return "group-expander-right";
                    case PlasmaCore.Types.BottomEdge:
                    default:
                        return "group-expander-bottom";
                    }
                }
            }

            states: [
                State {
                    name: "bottom"
                    when: plasmoid.location == PlasmaCore.Types.BottomEdge
                    AnchorChanges {
                        target: arrow
                        anchors.top: undefined; anchors.left: undefined; anchors.right: undefined; anchors.bottom: arrow.parent.bottom;
                        anchors.horizontalCenter: iconBox.horizontalCenter; anchors.verticalCenter: undefined;
                    }
                },
                State {
                    name: "top"
                    when: plasmoid.location == PlasmaCore.Types.TopEdge
                    AnchorChanges {
                        target: arrow
                        anchors.top: arrow.parent.top; anchors.left: undefined; anchors.right: undefined; anchors.bottom: undefined;
                        anchors.horizontalCenter: iconBox.horizontalCenter; anchors.verticalCenter: undefined;
                    }
                },
                State {
                    name: "left"
                    when: plasmoid.location == PlasmaCore.Types.LeftEdge
                    AnchorChanges {
                        target: arrow
                        anchors.top: undefined; anchors.left: arrow.parent.left; anchors.right: undefined; anchors.bottom: undefined;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: iconBox.verticalCenter;
                    }
                },
                State {
                    name: "right"
                    when: plasmoid.location == PlasmaCore.Types.RightEdge
                    AnchorChanges {
                        target: arrow
                        anchors.top: undefined; anchors.left: undefined; anchors.right: arrow.parent.right; anchors.bottom: undefined;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: iconBox.verticalCenter;
                    }
                }
            ]
        }
    }

}
