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
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.1 as Latte

Item{
    id: editVisual
    width: root.isHorizontal ? (dock ? dock.width : root.width) :
                               visibilityManager.thicknessNormalOriginal + theme.defaultFont.pixelSize
    height: root.isVertical ? (dock ? dock.height : root.height) :
                              visibilityManager.thicknessNormalOriginal + theme.defaultFont.pixelSize

    opacity: 0

    property int speed: Latte.WindowSystem.compositingActive ? root.durationTime*2.8*units.longDuration : 10
    property int thickness: visibilityManager.thicknessNormalOriginalValue + root.editShadow
    property int rootThickness: visibilityManager.thicknessZoomOriginal + root.editShadow - theme.defaultFont.pixelSize
    property int editLength: root.isHorizontal ? (root.behaveAsPlasmaPanel ? root.width - root.maxIconSize/4 : root.width)://root.maxLength) :
                                                 (root.behaveAsPlasmaPanel ? root.height - root.maxIconSize/4 : root.height)

    property real editStateOpacity: 0.95 //root.behaveAsPlasmaPanel  ? 0.5 : 0.95// root.blurEnabled ? 0.8 : 0.9

    property bool animationSent: false
    property bool farEdge: (plasmoid.location===PlasmaCore.Types.BottomEdge) || (plasmoid.location===PlasmaCore.Types.RightEdge)
    property bool editAnimationEnded: false
    property bool plasmaEditMode: plasmoid.userConfiguring
    property bool inEditMode: false

    property rect efGeometry

    property string layoutColor: root.dockManagedLayout ? root.dockManagedLayout.color : "blue"


    Item{
        id: shadow
        width: root.isHorizontal ? imageTiler.width + 2*root.editShadow : root.editShadow
        height: root.isHorizontal ? root.editShadow : imageTiler.height + 2*root.editShadow

        clip: true
        visible: !editTransition.running

        EditShadow{
            id: editShadow
        }

        states: [
            ///topShadow
            State {
                name: "topShadow"
                when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

                AnchorChanges {
                    target: shadow
                    anchors{ top:undefined; bottom:imageTiler.top; left:undefined; right:undefined;
                        horizontalCenter:imageTiler.horizontalCenter; verticalCenter:undefined}
                }
                AnchorChanges {
                    target: editShadow
                    anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;
                        horizontalCenter:parent.horizontalCenter; verticalCenter:parent.undefined}
                }
                PropertyChanges{
                    target: editShadow
                    anchors{ leftMargin: 0; rightMargin:0; topMargin:root.editShadow; bottomMargin:0}
                }
            },
            ///bottomShadow
            State {
                name: "bottomShadow"
                when: (plasmoid.location === PlasmaCore.Types.TopEdge)

                AnchorChanges {
                    target: shadow
                    anchors{ top:imageTiler.bottom; bottom:undefined; left:undefined; right:undefined;
                        horizontalCenter:imageTiler.horizontalCenter; verticalCenter:undefined}
                }
                AnchorChanges {
                    target: editShadow
                    anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;
                        horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: editShadow
                    anchors{ leftMargin: 0; rightMargin:0; topMargin:0; bottomMargin:root.editShadow}
                }
            },
            ///leftShadow
            State {
                name: "leftShadow"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)

                AnchorChanges {
                    target: shadow
                    anchors{ top:undefined; bottom:undefined; left:undefined; right:imageTiler.left;
                        horizontalCenter:undefined; verticalCenter:imageTiler.verticalCenter}
                }
                AnchorChanges {
                    target: editShadow
                    anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;
                        horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: editShadow
                    anchors{ leftMargin: root.editShadow; rightMargin:0; topMargin:0; bottomMargin:0}
                }
            },
            ///rightShadow
            State {
                name: "rightShadow"
                when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

                AnchorChanges {
                    target: shadow
                    anchors{ top:undefined; bottom:undefined; left:imageTiler.right; right:undefined;
                        horizontalCenter:undefined; verticalCenter:imageTiler.verticalCenter}
                }
                AnchorChanges {
                    target: editShadow
                    anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;
                        horizontalCenter:undefined; verticalCenter:undefined}
                }
                PropertyChanges{
                    target: editShadow
                    anchors{ leftMargin: 0; rightMargin:root.editShadow; topMargin:0; bottomMargin:0}
                }
            }
        ]
    }

    Image{
        id: imageTiler
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        opacity: editVisual.editStateOpacity

        fillMode: Image.Tile
        source: hasBackground ? dock.managedLayout.background : "../icons/"+editVisual.layoutColor+"print.jpg"

        readonly property bool hasBackground: (dock && dock.managedLayout && dock.managedLayout.background.startsWith("/")) ?
                                                  true : false
    }

    Connections{
        target: plasmoid
        onLocationChanged: initializeEditPosition();
    }

    onRootThicknessChanged: {
        initializeEditPosition();
    }

    onThicknessChanged: {
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
        /*if (root.isHorizontal) {
            initializeEditPosition();
        }*/

        updateEffectsArea();
    }

    onHeightChanged: {
       /* if (root.isVertical) {
            initializeEditPosition();
        }*/

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
            x = 0;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            x = rootThickness;
            y = 0;
        } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            x = -editVisual.thickness;
            y = 0;
        } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            y = -editVisual.thickness;
            x = 0;
        }
    }

    function initializeEditPosition() {
        if (root.editMode) {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge){
                x = 0;
                y = 0;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                x = 0;
                y = 0;
            } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                x = 0;
                y = rootThickness - thickness;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                x = rootThickness - thickness;
                y = 0;
            }
        }
    }

    //////////// States ////////////////////

    states: [
        State{
            name: "*"
            when:  !plasmaEditMode
        },

        State{
            name: "edit"
            when: plasmaEditMode
        }
    ]

    transitions: [
        Transition{
            id: editTransition
            from: "*"
            to: "edit"

            SequentialAnimation{
                id:normalAnimationTransition
                ScriptAction{
                    script:{
                        editVisual.inEditMode = true;
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
                        to: 1
                        duration: editVisual.speed / 2
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: editVisual
                        property: root.isHorizontal ? "y" : "x"
                        to: editVisual.farEdge ? editVisual.rootThickness - editVisual.thickness : 0
                        duration: editVisual.speed
                        easing.type: Easing.OutQuad
                    }
                }

                ScriptAction{
                    script:{
                        editVisual.editAnimationEnded = true;
                        updateEffectsArea();
                        updateAutomaticIconSize();
                        visibilityManager.updateMaskArea();
                    }
                }
            }
        },
        Transition{
            from: "edit"
            to: "*"
            SequentialAnimation{
                ScriptAction{
                    script: {
                        //! remove kwin effects when starting the animation
                        dock.effectsArea = Qt.rect(-1, -1, 0, 0);
                    }
                }

                ParallelAnimation{
                    PropertyAnimation {
                        target: editVisual
                        property: root.isHorizontal ? "y" : "x"
                        to: editVisual.farEdge ? editVisual.rootThickness : -editVisual.thickness
                        duration: editVisual.speed
                        easing.type: Easing.InQuad
                    }
                    PropertyAnimation {
                        target: editVisual
                        property: "opacity"
                        to: 0
                        duration: editVisual.speed
                        easing.type: Easing.InQuad
                    }
                }

                ScriptAction{
                    script:{
                        editVisual.inEditMode = false;
                        editVisual.editAnimationEnded = false;
                        if (editVisual.animationSent) {
                            root.slotAnimationsNeedLength(-1);
                            editVisual.animationSent = false;
                        }

                        if (visibilityManager.inTempHiding) {
                            visibilityManager.sendHideDockDuringLocationChangeFinished();
                        }
                    }
                }
            }

        }
    ]
}
