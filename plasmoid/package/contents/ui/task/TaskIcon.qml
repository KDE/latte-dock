/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import org.kde.kirigami 2.0 as Kirigami

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

import "animations" as TaskAnimations

Item {
    id: taskIconContainer
    anchors.fill: parent
    property bool toBeDestroyed: false

    readonly property color backgroundColor: iconColorsLoader.active ? iconColorsLoader.item.backgroundColor : theme.backgroundColor
    readonly property color glowColor: iconColorsLoader.active ? iconColorsLoader.item.glowColor : theme.textColor

    readonly property bool smartLauncherEnabled: (taskItem.isStartup === false) //! it needs to be enabled independent of user-set option because it is used from indicators
    readonly property bool progressVisible: smartLauncherItem && smartLauncherItem.progressVisible
    readonly property real progress: smartLauncherItem && smartLauncherItem.progress ? smartLauncherItem.progress : 0
    readonly property QtObject smartLauncherItem: smartLauncherLoader.active ? smartLauncherLoader.item : null

    readonly property Item monochromizedItem: badgesLoader.active ? badgesLoader.item : taskIconItem

    Rectangle{
        id: draggedRectangle
        width: parent.width + 2
        height: parent.height + 2
        anchors.centerIn: taskIconContainer
        opacity: 0
        radius: 3
        anchors.margins: 5

        property color tempColor: theme.highlightColor
        color: tempColor
        border.width: 1
        border.color: theme.highlightColor

        onTempColorChanged: tempColor.a = 0.35;
    }

    Loader {
        id: smartLauncherLoader
        active: taskIconContainer.smartLauncherEnabled
        sourceComponent: TaskManagerApplet.SmartLauncherItem {
            //! It creates issues with Valgrind and needs to be completely removed in that case
            launcherUrl: taskItem.launcherUrlWithIcon
        }
    }

    //! Provide icon background and glow colors
    Loader {
        id: iconColorsLoader
        active: taskItem.abilities.indicators.info.needsIconColors
        visible: false

        sourceComponent: LatteCore.IconItem{
            width:64
            height:64
            source: taskIconItem.source
            providesColors: true
        }
    }

    Kirigami.Icon {
        id: taskIconItem
        anchors.fill: parent
        //roundToIconSize: false
        source: decoration
        visible: !badgesLoader.active

        readonly property real size: Math.min(width,height)

        ///states for launcher animation
        states: [
            State{
                name: "*"
                //! since qt 5.14 default state can not use "when" property
                //! it breaks restoring transitions otherwise

                AnchorChanges{
                    target: taskIconItem;
                    anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                    anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                    anchors.right: root.location === PlasmaCore.Types.RightEdge ? parent.right : undefined;
                    anchors.left: root.location === PlasmaCore.Types.LeftEdge ? parent.left : undefined;
                    anchors.top: root.location === PlasmaCore.Types.TopEdge ? parent.top : undefined;
                    anchors.bottom: root.location === PlasmaCore.Types.BottomEdge ? parent.bottom : undefined;
                }
            },

            State{
                name: "animating"
                when: !taskItem.inAddRemoveAnimation && (launcherAnimation.running || newWindowAnimation.running)

                AnchorChanges{
                    target: taskIconItem;
                    anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                    anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                    anchors.right: root.location === PlasmaCore.Types.LeftEdge ? parent.right : undefined;
                    anchors.left: root.location === PlasmaCore.Types.RightEdge ? parent.left : undefined;
                    anchors.top: root.location === PlasmaCore.Types.BottomEdge ? parent.top : undefined;
                    anchors.bottom: root.location === PlasmaCore.Types.TopEdge ? parent.bottom : undefined;
                }
            }
        ]

        ///transitions, basic for the anchor changes
        transitions: [
            Transition{
                from: "animating"
                to: "*"
                AnchorAnimation { duration: 1.5 * taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large }
            }
        ]
    }

    //! Combined Loader for Progress and Audio badges masks
    Loader{
        id: badgesLoader
        anchors.fill: taskIconContainer
        active: (activateProgress > 0) && taskItem.abilities.environment.isGraphicsSystemAccelerated
        asynchronous: true
        opacity: stateColorizer.opacity > 0 ? 0 : 1

        property real activateProgress: showInfo || showProgress || showAudio ? 1 : 0

        property bool showInfo: (root.showInfoBadge
                                 && taskIcon.smartLauncherItem
                                 && (taskIcon.smartLauncherItem.countVisible || taskItem.badgeIndicator > 0)
                                 && !showProgress)

        property bool showProgress: root.showProgressBadge
                                    && taskIcon.smartLauncherItem
                                    && taskIcon.smartLauncherItem.progressVisible

        property bool showAudio: (root.showAudioBadge && taskItem.hasAudioStream && taskItem.playingAudio)

        Behavior on activateProgress {
            NumberAnimation { duration: 2 * taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large }
        }

        sourceComponent: Item{
            ShaderEffect {
                id: iconOverlay
                enabled: false
                anchors.fill: parent
                property var source: ShaderEffectSource {
                    sourceItem: Kirigami.Icon{
                        width: taskIconItem.width
                        height: taskIconItem.height
                        smooth: taskIconItem.smooth
                        source: taskIconItem.source
                        //roundToIconSize: taskIconItem.roundToIconSize
                        active: taskIconItem.active

                        Loader{
                            anchors.fill: parent
                            active: plasmoid.configuration.forceMonochromaticIcons

                            sourceComponent: ColorOverlay {
                                anchors.fill: parent
                                color: latteBridge ? latteBridge.palette.textColor : "transparent"
                                source: taskIconItem
                            }
                        }
                    }
                }
                property var mask: ShaderEffectSource {
                    sourceItem: Item{
                        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft && !root.vertical
                        LayoutMirroring.childrenInherit: true

                        width: taskIconContainer.width
                        height: taskIconContainer.height

                        Rectangle{
                            id: maskRect
                            width: Math.max(badgeVisualsLoader.infoBadgeWidth, parent.width / 2)
                            height: parent.height / 2
                            radius: parent.height
                            visible: badgesLoader.showInfo || badgesLoader.showProgress

                            //! Removes any remainings from the icon around the roundness at the corner
                            Rectangle{
                                id: maskCorner
                                width: parent.width/2
                                height: parent.height/2
                            }

                            states: [
                                State {
                                    name: "default"
                                    when: (root.location !== PlasmaCore.Types.RightEdge)

                                    AnchorChanges {
                                        target: maskRect
                                        anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;}
                                    }
                                    AnchorChanges {
                                        target: maskCorner
                                        anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;}
                                    }
                                },
                                State {
                                    name: "right"
                                    when: (root.location === PlasmaCore.Types.RightEdge)

                                    AnchorChanges {
                                        target: maskRect
                                        anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;}
                                    }
                                    AnchorChanges {
                                        target: maskCorner
                                        anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;}
                                    }
                                }
                            ]
                        } // progressMask

                        Rectangle{
                            id: maskRect2
                            width: parent.width/2
                            height: width
                            radius: width
                            visible: badgesLoader.showAudio

                            Rectangle{
                                id: maskCorner2
                                width:parent.width/2
                                height:parent.height/2
                            }

                            states: [
                                State {
                                    name: "default"
                                    when: (root.location !== PlasmaCore.Types.RightEdge)

                                    AnchorChanges {
                                        target: maskRect2
                                        anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;}
                                    }
                                    AnchorChanges {
                                        target: maskCorner2
                                        anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined;}
                                    }
                                },
                                State {
                                    name: "right"
                                    when: (root.location === PlasmaCore.Types.RightEdge)

                                    AnchorChanges {
                                        target: maskRect2
                                        anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;}
                                    }
                                    AnchorChanges {
                                        target: maskCorner2
                                        anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right;}
                                    }
                                }
                            ]
                        } // audio mask
                    }
                    hideSource: true
                    live: true
                } //end of mask

                supportsAtlasTextures: true

                fragmentShader: "
        varying highp vec2 qt_TexCoord0;
        uniform highp float qt_Opacity;
        uniform lowp sampler2D source;
        uniform lowp sampler2D mask;
        void main() {
            gl_FragColor = texture2D(source, qt_TexCoord0.st) * (1.0 - (texture2D(mask, qt_TexCoord0.st).a)) * qt_Opacity;
        }
    "
            } //end of sourceComponent
        }
    }
    ////!

    //! START: Badges Visuals
    //! the badges visual get out from iconGraphic in order to be able to draw shadows that
    //! extend beyond the iconGraphic boundaries
    Loader {
        id: badgeVisualsLoader
        anchors.fill: taskIconContainer
        active: (badgesLoader.activateProgress > 0)

        readonly property int infoBadgeWidth: active ? publishedInfoBadgeWidth : 0
        property int publishedInfoBadgeWidth: 0

        sourceComponent: Item {
            ProgressOverlay{
                id: infoBadge
                anchors.right: parent.right
                anchors.top: parent.top
                width: Math.max(parent.width, contentWidth)
                height: parent.height

                opacity: badgesLoader.activateProgress
                visible: badgesLoader.showInfo || badgesLoader.showProgress
            }

            AudioStream{
                id: audioStreamBadge
                anchors.fill: parent
                opacity: badgesLoader.activateProgress
                visible: badgesLoader.showAudio
            }

            Binding {
                target: badgeVisualsLoader
                property: "publishedInfoBadgeWidth"
                value: infoBadge.contentWidth
            }
        }
    }

    //! GREY-ing the information badges when the task is dragged
    //! moved out of badgeVisualsLoader in order to avoid crashes
    //! when the latte view is removed
    Loader {
        anchors.fill: parent
        active: badgeVisualsLoader.active
                && taskItem.abilities.environment.isGraphicsSystemAccelerated
        sourceComponent: Colorize{
            source: badgeVisualsLoader.item

            //! HACK TO AVOID PIXELIZATION
            //! WORKAROUND: When Effects are enabled e.g. BrightnessContrast, Colorize etc.
            //! the icon appears pixelated. It is even most notable when parabolic.factor.zoom === 1
            //! I don't know enabling cached=true helps, but it does.
            //! In Question?
            //cached: true

            opacity: stateColorizer.opacity
            hue: stateColorizer.hue
            saturation: stateColorizer.saturation
            lightness: stateColorizer.lightness
        }
    }
    //! END: Badges Visuals

    //! Effects
    Colorize{
        id: stateColorizer
        anchors.fill: parent
        source: badgesLoader.active ? badgesLoader : taskIconItem

        opacity:0

        hue:0
        saturation:0
        lightness:0
    }

    BrightnessContrast{
        id:hoveredImage
        anchors.fill: parent

        //! HACK TO AVOID PIXELIZATION
        //! WORKAROUND: When Effects are enabled e.g. BrightnessContrast, Colorize etc.
        //! the icon appears pixelated. It is even most notable when parabolic.factor.zoom === 1
        //! I don't know enabling cached=true helps, but it does.
        //! In Question?
        //cached: true

        source: badgesLoader.active ? badgesLoader : taskIconItem

        opacity: taskItem.containsMouse && !clickedAnimation.running && !taskItem.abilities.indicators.info.providesHoveredAnimation ? 1 : 0
        brightness: 0.30
        contrast: 0.1

        Behavior on opacity {
            NumberAnimation { duration: taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large }
        }
    }

    BrightnessContrast {
        id: brightnessTaskEffect
        anchors.fill: parent

        //! HACK TO AVOID PIXELIZATION
        //! WORKAROUND: When Effects are enabled e.g. BrightnessContrast, Colorize etc.
        //! the icon appears pixelated. It is even most notable when parabolic.factor.zoom === 1
        //! I don't know enabling cached=true helps, but it does.
        //! In Question?
        //cached: true

        source: badgesLoader.active ? badgesLoader : taskIconItem

        visible: clickedAnimation.running
    }
    //! Effects

    Loader {
        id: dropFilesVisual
        active: applyOpacity>0

        width: !root.vertical ? length : thickness
        height: !root.vertical ? thickness : length
        anchors.centerIn: parent

        readonly property int length: taskItem.abilities.metrics.totals.length
        readonly property int thickness: taskItem.abilities.metrics.totals.thickness

        readonly property real applyOpacity: (mouseHandler.isDroppingSeparator || mouseHandler.isDroppingFiles)
                                             && (root.dragSource === null)
                                             && (mouseHandler.hoveredItem === taskItem) ? 0.7 : 0

        sourceComponent: LatteComponents.AddItem {
            anchors.fill: parent
            backgroundOpacity: dropFilesVisual.applyOpacity
        }
    }

    //! Animations
    TaskAnimations.ClickedAnimation { id: clickedAnimation }

    TaskAnimations.LauncherAnimation { id:launcherAnimation }

    TaskAnimations.NewWindowAnimation { id: newWindowAnimation }

    TaskAnimations.RemoveWindowFromGroupAnimation { id: removingAnimation }
    //! Animations

    Component.onDestruction: {
        taskIcon.toBeDestroyed = true;

        if(removingAnimation.removingItem)
            removingAnimation.removingItem.destroy();
    }

    //////////// States ////////////////////
    states: [
        State{
            name: "*"
            //! since qt 5.14 default state can not use "when" property
            //! it breaks restoring transitions otherwise
        },

        State{
            name: "isDragged"
            when: taskItem.isDragged
        }
    ]

    //////////// Transitions //////////////

    readonly property string draggingNeedThicknessEvent: taskIcon + "_dragging"

    transitions: [
        Transition{
            id: isDraggedTransition
            to: "isDragged"
            property int speed: taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large

            SequentialAnimation{
                ScriptAction{
                    script: {
                        taskItem.abilities.animations.needThickness.addEvent(draggingNeedThicknessEvent);
                        taskItem.inBlockingAnimation = true;
                        taskItem.abilities.parabolic.setDirectRenderingEnabled(false);
                    }
                }

                PropertyAnimation {
                    target: taskItem.parabolicItem
                    property: "zoom"
                    to: 1
                    duration: taskItem.abilities.parabolic.factor.zoom === 1 ? 0 : (isDraggedTransition.speed*1.2)
                    easing.type: Easing.OutQuad
                }

                ParallelAnimation{
                    PropertyAnimation {
                        target: draggedRectangle
                        property: "opacity"
                        to: 1
                        duration: isDraggedTransition.speed
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: taskIconItem
                        property: "opacity"
                        to: 0
                        duration: isDraggedTransition.speed
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: stateColorizer
                        property: "opacity"
                        to: taskItem.isSeparator ? 0 : 1
                        duration: isDraggedTransition.speed
                        easing.type: Easing.OutQuad
                    }
                }

                ScriptAction{
                    script: {
                        taskItem.abilities.animations.needThickness.removeEvent(draggingNeedThicknessEvent);
                    }
                }
            }

            onRunningChanged: {
                if(running){
                    taskItem.animationStarted();
                } else {
                    taskItem.abilities.animations.needThickness.removeEvent(draggingNeedThicknessEvent);
                }
            }
        },
        Transition{
            id: defaultTransition
            from: "isDragged"
            to: "*"
            property int speed: taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large

            SequentialAnimation{
                ScriptAction{
                    script: {
                        taskItem.abilities.parabolic.setDirectRenderingEnabled(false);
                    }
                }

                ParallelAnimation{
                    PropertyAnimation {
                        target: draggedRectangle
                        property: "opacity"
                        to: 0
                        duration: defaultTransition.speed
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: taskIconItem
                        property: "opacity"
                        to: 1
                        duration: defaultTransition.speed
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: stateColorizer
                        property: "opacity"
                        to: 0
                        duration: isDraggedTransition.speed
                        easing.type: Easing.OutQuad
                    }
                }

                ScriptAction{
                    script: {
                        taskItem.inBlockingAnimation = false;
                    }
                }
            }

            onRunningChanged: {
                if(!running){
                    taskItem.animationEnded();
                }
            }
        }
    ]
}
