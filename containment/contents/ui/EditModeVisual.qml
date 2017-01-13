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

Image{
    id: editVisual
    width: root.isHorizontal ? parent.width : visibilityManager.thicknessNormal
    height: root.isVertical ? parent.height : visibilityManager.thicknessNormal

    fillMode: Image.Tile
    source: "../icons/blueprint.jpg"
    opacity: 0

    property int speed: root.durationTime*4*units.longDuration
    property int thickness: visibilityManager.thicknessNormal + shadowSize
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

    Connections{
        target: plasmoid
        onLocationChanged: initializeEditPosition();
    }

    Connections{
        target: root
        onIconSizeChanged: initializeEditPosition();
    }

    onEditAnimationEndedChanged: {
        if (editAnimationEnded) {
            dock.shadow = shadowSize;
        } else {
            dock.shadow = 0;
        }
    }

    function initializeNormalPosition() {
        if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
            x = 0;
            y = rootThickness;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            x = rootThickness;
            y = 0;
        } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            x = -editVisual.thickness;
            y = 0;
        } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            x = 0;
            y = -editVisual.thickness;
        }
    }

    function initializeEditPosition() {
        if (root.editMode) {
            if ((plasmoid.location === PlasmaCore.Types.LeftEdge) || (plasmoid.location === PlasmaCore.Types.TopEdge)){
                x = 0;
                y = 0;
            } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                x = 0;
                y = rootThickness - thickness + shadowSize;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                x = rootThickness - thickness + shadowSize;
                y = 0;
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
