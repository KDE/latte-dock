/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import QtGraphicalEffects 1.0

import org.kde.latte 0.1 as Latte

Image{
    id: editVisual
    width: root.isHorizontal ? root.maxLength : visibilityManager.thicknessNormalOriginalValue
    height: root.isVertical ? root.maxLength : visibilityManager.thicknessNormalOriginalValue

    fillMode: Image.Tile
    source: "../icons/blueprint.jpg"
    opacity: 0

    property int speed: root.durationTime*4*units.longDuration
    property int thickness: visibilityManager.thicknessNormalOriginalValue + shadowSize
    property int rootThickness: visibilityManager.thicknessZoomOriginal
    property int shadowSize : Math.ceil(root.iconSize / 5)

    property bool animationSent: false
    property bool farEdge: (plasmoid.location===PlasmaCore.Types.BottomEdge) || (plasmoid.location===PlasmaCore.Types.RightEdge)
    property bool editAnimationEnded: false

    layer.enabled: true
    layer.effect: DropShadow {
        radius: editVisual.shadowSize
        samples: 2 * radius
        color: "#ee080808"
    }


    /*Behavior on width {
        NumberAnimation { duration: 300 }
        enabled: root.isHorizontal
    }

    Behavior on height {
        NumberAnimation { duration: 300 }
        enabled: root.isVertical
    }*/

    Connections{
        target: plasmoid
        onLocationChanged: initializeEditPosition();
    }

    Connections{
        target: root
        onIconSizeChanged: initializeEditPosition();
        onPanelAlignmentChanged: initializeEditPosition();
    }

    onRootThicknessChanged: {
        initializeEditPosition();
    }

    onEditAnimationEndedChanged: {
        if (editAnimationEnded) {
            dock.shadow = shadowSize;
        } else {
            dock.shadow = 0;
        }
    }

    onWidthChanged: {
        if (root.isHorizontal) {
            initializeEditPosition();
        }
    }

    onHeightChanged: {
        if (root.isVertical) {
            initializeEditPosition();
        }
    }

    function initializeNormalPosition() {
        if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
            y = rootThickness;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            x = rootThickness;
        } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            x = -editVisual.thickness;
        } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            y = -editVisual.thickness;
        }

        if (root.isHorizontal) {
            if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                x = root.width/2 - root.maxLength/2;
            } else if (root.panelAlignment === Latte.Dock.Left) {
                x = 0;
            } else if (root.panelAlignment === Latte.Dock.Center) {
                x = root.width/2 - root.maxLength/2;
            } else if (root.panelAlignment === Latte.Dock.Right) {
                x = root.width - root.maxLength;
            }
        } else if (root.isVertical) {
            if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                y = root.height/2 - root.maxLength/2;
            } else if (root.panelAlignment === Latte.Dock.Top) {
                y = 0;
            } else if (root.panelAlignment === Latte.Dock.Center) {
                y = root.height/2 - root.maxLength/2;
            } else if (root.panelAlignment === Latte.Dock.Bottom) {
                y = root.height - root.maxLength;
            }
        }
    }

    function initializeEditPosition() {
        if (root.editMode) {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge){
                x = 0;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                y = 0;
            } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                y = rootThickness - thickness + shadowSize;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                x = rootThickness - thickness + shadowSize;
            }

            if (root.isHorizontal) {
                if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                    x = root.width/2 - editVisual.width/2;
                } else if (root.panelAlignment === Latte.Dock.Left) {
                    x = 0;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    x = root.width/2 - editVisual.width/2;
                } else if (root.panelAlignment === Latte.Dock.Right) {
                    x = root.width - editVisual.width;
                }
            } else if (root.isVertical) {
                if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                    y = root.height/2 - editVisual.height/2;
                } else if (root.panelAlignment === Latte.Dock.Top) {
                    y = 0;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    y = root.height/2 - editVisual.height/2;
                } else if (root.panelAlignment === Latte.Dock.Bottom) {
                    y = root.height - editVisual.height;
                }
            }
        }
    }

    //////////// States ////////////////////

    states: [
        State{
            name: "*"
            when:  !root.editMode
        },

        State{
            name: "edit"
            when: root.editMode
        }
    ]

    transitions: [
        Transition{
            id: editTransition
            from: "*"
            to: "edit"

            SequentialAnimation{
                ScriptAction{
                    script:{
                        editVisual.opacity = 0

                        initializeNormalPosition();

                        if(!animationSent) {
                            animationSent = true;
                            root.slotAnimationsNeedLength(1);
                        }
                    }
                }

                ParallelAnimation{
                    PropertyAnimation {
                        target: editVisual
                        property: "opacity"
                        to: 0.6
                        duration: editVisual.speed / 2
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: editVisual
                        property: root.isHorizontal ? "y" : "x"
                        to: editVisual.farEdge ? editVisual.rootThickness - editVisual.thickness + editVisual.shadowSize : 0
                        duration: editVisual.speed
                        easing.type: Easing.OutBounce


                    }
                }

                ScriptAction{
                    script:{
                        editVisual.editAnimationEnded = true;
                        visibilityManager.updateMaskArea();
                    }
                }
            }
        },
        Transition{
            from: "edit"
            to: "*"
            SequentialAnimation{
                ParallelAnimation{
                    PropertyAnimation {
                        target: editVisual
                        property: root.isHorizontal ? "y" : "x"
                        to: editVisual.farEdge ? editVisual.rootThickness : -editVisual.thickness
                        duration: editVisual.speed
                        easing.type: Easing.OutBounce
                    }
                    PropertyAnimation {
                        target: editVisual
                        property: "opacity"
                        to: 0
                        duration: editVisual.speed
                        easing.type: Easing.OutQuad
                    }
                }

                ScriptAction{
                    script:{
                        editVisual.editAnimationEnded = false;
                        if (editVisual.animationSent) {
                            root.slotAnimationsNeedLength(-1);
                            editVisual.animationSent = false;
                        }
                    }
                }
            }

        }
    ]
}
