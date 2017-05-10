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
    width: root.isHorizontal ? editLength : visibilityManager.thicknessNormalOriginal
    height: root.isVertical ? editLength : visibilityManager.thicknessNormalOriginal

    fillMode: Image.Tile
    source: !dock || dock.session===Latte.Dock.DefaultSession ? "../icons/blueprint.jpg":"../icons/purpleprint.jpg"
    opacity: 0

    property int speed: root.durationTime*4*units.longDuration
    property int thickness: visibilityManager.thicknessNormalOriginal + root.editShadow
    property int rootThickness: visibilityManager.thicknessZoomOriginal + root.editShadow
    property int editLength: root.isHorizontal ? (root.behaveAsPlasmaPanel ? root.width - root.maxIconSize/4 : root.maxLength) :
                                               (root.behaveAsPlasmaPanel ? root.height - root.maxIconSize/4 : root.maxLength)

    property bool animationSent: false
    property bool farEdge: (plasmoid.location===PlasmaCore.Types.BottomEdge) || (plasmoid.location===PlasmaCore.Types.RightEdge)
    property bool editAnimationEnded: false

    property rect efGeometry

    layer.enabled: true
    layer.effect: DropShadow {
        radius: root.editShadow
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
        onOffsetChanged: initializeEditPosition();
    }

    onRootThicknessChanged: {
        initializeEditPosition();
    }

    onEditAnimationEndedChanged: {
        if (!dock) {
            return;
        }

        if (editAnimationEnded && !root.behaveAsPlasmaPanel) {
            dock.shadow = root.editShadow;
        } else {
            dock.shadow = root.panelShadow;
        }
    }

    onXChanged: updateEffectsArea();
    onYChanged: updateEffectsArea();

    onWidthChanged: {
        if (root.isHorizontal) {
            initializeEditPosition();
        }

        updateEffectsArea();
    }

    onHeightChanged: {
        if (root.isVertical) {
            initializeEditPosition();
        }

        updateEffectsArea();
    }

    function updateEffectsArea(){
        if (!dock || state !== "edit" || !editAnimationEnded)
            return;

        var rootGeometry = mapToItem(root, 0, 0);

        efGeometry.x = rootGeometry.x;
        efGeometry.y = rootGeometry.y;
        efGeometry.width = width;
        efGeometry.height = height;

        dock.effectsArea = efGeometry;
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
                x = root.width/2 - editVisual.editLength/2 + root.offset;
            } else if (root.panelAlignment === Latte.Dock.Left) {
                x = root.offset;
            } else if (root.panelAlignment === Latte.Dock.Center) {
                x = root.width/2 - editVisual.editLength/2 + root.offset;
            } else if (root.panelAlignment === Latte.Dock.Right) {
                x = root.width - editVisual.editLength - root.offset;
            }
        } else if (root.isVertical) {
            if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                y = root.height/2 - editVisual.editLength/2 + root.offset;
            } else if (root.panelAlignment === Latte.Dock.Top) {
                y = root.offset;
            } else if (root.panelAlignment === Latte.Dock.Center) {
                y = root.height/2 - editVisual.editLength/2 + root.offset;
            } else if (root.panelAlignment === Latte.Dock.Bottom) {
                y = root.height - editVisual.editLength - root.offset;
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
                y = rootThickness - thickness;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                x = rootThickness - thickness;
            }

            if (root.isHorizontal) {
                if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                    x = root.width/2 - editVisual.width/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Left) {
                    x = root.offset;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    x = root.width/2 - editVisual.width/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Right) {
                    x = root.width - editVisual.width - root.offset;
                }
            } else if (root.isVertical) {
                if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                    y = root.height/2 - editVisual.height/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Top) {
                    y = root.offset;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    y = root.height/2 - editVisual.height/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Bottom) {
                    y = root.height - editVisual.height - root.offset;
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
                        editVisual.editAnimationEnded = false;

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
                        to: root.behaveAsPlasmaPanel && dock.session===Latte.Dock.DefaultSession  ? 0.3 : 0.6
                        duration: editVisual.speed / 2
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: editVisual
                        property: root.isHorizontal ? "y" : "x"
                        to: editVisual.farEdge ? editVisual.rootThickness - editVisual.thickness : 0
                        duration: editVisual.speed
                        easing.type: Easing.OutBounce


                    }
                }

                ScriptAction{
                    script:{
                        editVisual.editAnimationEnded = true;
                        updateEffectsArea();
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
