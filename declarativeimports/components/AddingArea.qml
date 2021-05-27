/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import "code/ColorizerTools.js" as ColorizerTools

Rectangle{
    id: addingArea
    color: Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b, backgroundOpacity)
    border.width: 1
    border.color: outlineColor

    property bool drawAddCross: true

    property int iconSize: 64

    readonly property color outlineColorBase: theme.backgroundColor
    readonly property real outlineColorBaseBrightness: ColorizerTools.colorBrightness(outlineColorBase)
    readonly property color outlineColor: {
        if (outlineColorBaseBrightness > 127.5) {
            return Qt.darker(outlineColorBase, 1.5);
        } else {
            return Qt.lighter(outlineColorBase, 2.2);
        }
    }

    property real backgroundOpacity: 0.75
    property real duration: 2
    property string title: ""

    Behavior on opacity{
        NumberAnimation { duration: 2*addingArea.duration*appletItem.animationTime }
    }


    Label {
        id: heading
        text: title       
        color: theme.textColor
        font.bold: true

        rotation: {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                return 90;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                return -90;
            }

            return 0;
        }

        transformOrigin: {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                return Item.TopLeft;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                return Item.TopRight;
            }

            return Item.Center;
        }

        readonly property int lengthEdge: addingArea.radius + units.smallSpacing

        layer.enabled: true
        layer.effect: DropShadow {
            radius: 4
            fast: true
            samples: 2 * radius
            color: "#020202"
        }

        states: [
            ///Bottom Edge
            State {
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges {
                    target: heading
                    anchors{ top:parent.top; bottom:undefined; left:parent.right; right:undefined;}
                }

                PropertyChanges {
                    target: heading
                    anchors{ topMargin: heading.lengthEdge; bottomMargin:0; leftMargin:-units.smallSpacing; rightMargin:-0;}
                }
            },
            State {
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges {
                    target: heading
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.left;}
                }

                PropertyChanges {
                    target: heading
                    anchors{ topMargin:heading.lengthEdge; bottomMargin:0; leftMargin: 0; rightMargin:-units.smallSpacing;}
                }
            },
            State {
                name: "top"
                when:  plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges {
                    target: heading
                    anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined;}
                }

                PropertyChanges {
                    target: heading
                    anchors{ topMargin:0; bottomMargin:units.smallSpacing; leftMargin: heading.lengthEdge; rightMargin:0;}
                }
            },
            State {
                name: "bottom"
                when: plasmoid.location !== PlasmaCore.Types.TopEdge
                      && plasmoid.location !== PlasmaCore.Types.LeftEdge
                      && plasmoid.location !== PlasmaCore.Types.RightEdge

                AnchorChanges {
                    target: heading
                    anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;}
                }

                PropertyChanges {
                    target: heading
                    anchors{ topMargin:units.smallSpacing; bottomMargin:0; leftMargin: heading.lengthEdge; rightMargin:0;}
                }
            }
        ]
    }

    AddItem {
        anchors.centerIn: parent
        visible: drawAddCross
        width: thickness
        height: thickness

        readonly property int thickness: Math.min(addingArea.iconSize,
                                                  plasmoid.formFactor === PlasmaCore.Types.Horizontal ? (parent.height - freeSpace):(parent.width - freeSpace))

        readonly property int freeSpace: Math.max(16, (heading.implicitHeight + units.smallSpacing*2))
    }
}
