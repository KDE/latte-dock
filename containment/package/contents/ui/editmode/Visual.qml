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

import org.kde.latte 0.2 as Latte

import "../applet" as Applet

Item{
    id: editVisual
    width: root.isHorizontal ? (latteView ? latteView.width : root.width) :
                               visibilityManager.thicknessEditMode
    height: root.isVertical ? (latteView ? latteView.height : root.height) :
                              visibilityManager.thicknessEditMode

    readonly property int settingsThickness: settingsOverlay.thickness

    property int speed: Latte.WindowSystem.compositingActive ? root.durationTime*2.8*units.longDuration : 10
    property int thickness: visibilityManager.thicknessEditMode + root.editShadow
    property int rootThickness: visibilityManager.thicknessZoomOriginal + root.editShadow //- visibilityManager.thicknessEditMode
    property int editLength: root.isHorizontal ? (root.behaveAsPlasmaPanel ? root.width - root.maxIconSize/4 : root.width)://root.maxLength) :
                                                 (root.behaveAsPlasmaPanel ? root.height - root.maxIconSize/4 : root.height)

    property bool animationSent: false
    property bool farEdge: (plasmoid.location===PlasmaCore.Types.BottomEdge) || (plasmoid.location===PlasmaCore.Types.RightEdge)
    property bool editAnimationEnded: false
    property bool plasmaEditMode: plasmoid.userConfiguring
    property bool inEditMode: false

    property rect efGeometry

    property string layoutColor: latteView && latteView.managedLayout ? latteView.managedLayout.color : "blue"

    readonly property real appliedOpacity: imageTiler.opacity
    readonly property real maxOpacity: root.inConfigureAppletsMode ? 1 : plasmoid.configuration.editBackgroundOpacity

    Latte.ExternalShadow{
        id: editExternalShadow
        width: root.isHorizontal ? imageTiler.width : root.editShadow
        height: root.isHorizontal ? root.editShadow : imageTiler.height
        visible: !editTransition.running && root.editMode && Latte.WindowSystem.compositingActive

        shadowSize: root.editShadow
        shadowOpacity: Math.max(0.35, imageTiler.opacity)
        shadowDirection: plasmoid.location

        states: [
            ///topShadow
            State {
                name: "topShadow"
                when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

                AnchorChanges {
                    target: editExternalShadow
                    anchors{ top:undefined; bottom:imageTiler.top; left:undefined; right:undefined;
                        horizontalCenter:imageTiler.horizontalCenter; verticalCenter:undefined}
                }
            },
            ///bottomShadow
            State {
                name: "bottomShadow"
                when: (plasmoid.location === PlasmaCore.Types.TopEdge)

                AnchorChanges {
                    target: editExternalShadow
                    anchors{ top:imageTiler.bottom; bottom:undefined; left:undefined; right:undefined;
                        horizontalCenter:imageTiler.horizontalCenter; verticalCenter:undefined}
                }
            },
            ///leftShadow
            State {
                name: "leftShadow"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)

                AnchorChanges {
                    target: editExternalShadow
                    anchors{ top:undefined; bottom:undefined; left:undefined; right:imageTiler.left;
                        horizontalCenter:undefined; verticalCenter:imageTiler.verticalCenter}
                }
            },
            ///rightShadow
            State {
                name: "rightShadow"
                when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

                AnchorChanges {
                    target: editExternalShadow
                    anchors{ top:undefined; bottom:undefined; left:imageTiler.right; right:undefined;
                        horizontalCenter:undefined; verticalCenter:imageTiler.verticalCenter}
                }
            }
        ]
    }

    Image{
        id: imageTiler
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        opacity: 0

        fillMode: Image.Tile
        source: hasBackground ? latteView.managedLayout.background : "../../icons/"+editVisual.layoutColor+"print.jpg"

        readonly property bool hasBackground: (latteView && latteView.managedLayout && latteView.managedLayout.background.startsWith("/")) ?
                                                  true : false

        Connections {
            target: editVisual

            onMaxOpacityChanged: {
                if (editVisual.editAnimationEnded) {
                    imageTiler.opacity = editVisual.maxOpacity;
                }
            }
        }

        Behavior on opacity {
            enabled: editVisual.editAnimationEnded
            NumberAnimation {
                duration: 0.8 * root.animationTime
                easing.type: Easing.OutCubic
            }
        }
    }

    MouseArea {
        id: editBackMouseArea
        anchors.fill: imageTiler
        visible: editModeVisual.editAnimationEnded && !root.inConfigureAppletsMode
        hoverEnabled: true

        property bool wheelIsBlocked: false;
        readonly property double opacityStep: 0.1
        readonly property string tooltip: i18nc("opacity for background under edit mode, %0% is opacity percentage",
                                                "You can use mouse wheel to change background opacity of %0%").arg(Math.round(plasmoid.configuration.editBackgroundOpacity * 100))

        onWheel: {
            if (wheelIsBlocked) {
                return;
            }

            wheelIsBlocked = true;
            scrollDelayer.start();

            var angle = wheel.angleDelta.y / 8;

            if (angle > 10) {
                plasmoid.configuration.editBackgroundOpacity = Math.min(100, plasmoid.configuration.editBackgroundOpacity + opacityStep)
            } else if (angle < -10) {
                plasmoid.configuration.editBackgroundOpacity = Math.max(0, plasmoid.configuration.editBackgroundOpacity - opacityStep)
            }
        }

        //! A timer is needed in order to handle also touchpads that probably
        //! send too many signals very fast. This way the signals per sec are limited.
        //! The user needs to have a steady normal scroll in order to not
        //! notice a annoying delay
        Timer{
            id: scrollDelayer

            interval: 80
            onTriggered: editBackMouseArea.wheelIsBlocked = false;
        }
    }


    //! Settings Overlay
    SettingsOverlay {
        id: settingsOverlay
        anchors.fill: parent
        visible: root.editMode
    }

    Applet.TitleTooltipParent {
        id: titleTooltipParent

        minimumThickness: visibilityManager.thicknessEditMode
        maximumThickness: root.inConfigureAppletsMode ? visibilityManager.thicknessEditMode : 9999
    }

    Connections{
        target: root
        onThemeColorsChanged: imageTiler.opacity = editVisual.maxOpacity
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
       if (Latte.WindowSystem.compositingActive ||
               !latteView || state !== "edit" || !editAnimationEnded) {
            return;
       }

        var rootGeometry = mapToItem(root, 0, 0);

        efGeometry.x = rootGeometry.x;
        efGeometry.y = rootGeometry.y;
        efGeometry.width = width;
        efGeometry.height = height;

        latteView.effects.rect = efGeometry;
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
                        imageTiler.opacity = 0
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
                        target: imageTiler
                        property: "opacity"
                        to: editVisual.maxOpacity
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
                        latteView.effects.rect = Qt.rect(-1, -1, 0, 0);
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
                        target: imageTiler
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
