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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

import "animations" as TaskAnimations

//I am using  KQuickControlAddons.QIconItem even though onExit it triggers the following error
//QObject::~QObject: Timers cannot be stopped from another thread
//but it increases performance almost to double during animation

Item{
    id: taskIcon

   // width: wrapper.regulatorWidth
   // height: wrapper.regulatorHeight

    //big interval to show shadows only after all the crappy adds and removes of tasks
    //have happened
    property bool firstDrawed: true
    property bool toBeDestroyed: false

    // three intervals in order to create the necessary buffers from the
    // PlasmaCore.IconItem, one big interval for the first creation of the
    // plasmoid, a second one for the first creation of a task and a small one
    // for simple updates.
    // This is done before especially on initialization stage some visuals
    // are not ready and empty buffers are created

    //property int firstDrawedInterval: root.initializationStep ? 2000 : 1000
    // property int shadowInterval: firstDrawed ? firstDrawedInterval : 250
    property int shadowInterval: firstDrawed ? 1000 : 250

    property int shadowSize : root.appShadowSize

    readonly property bool smartLauncherEnabled: ((taskItem.isStartup === false) && (root.showInfoBadge || root.showProgressBadge))
    readonly property variant iconDecoration: decoration

    readonly property color backgroundColor: iconImageBuffer.backgroundColor
    readonly property color glowColor: iconImageBuffer.glowColor
    readonly property bool progressVisible: smartLauncherItem && smartLauncherItem.progressVisible
    readonly property real progress: smartLauncherItem && smartLauncherItem.progress ? smartLauncherItem.progress : 0

    property QtObject buffers: null
    property QtObject smartLauncherItem: null

    property Item visualIconItem: iconImageBuffer
    property Item titleTooltipVisualParent: titleTooltipParent
    property Item previewsTootipVisualParent: previewsTooltipParent

    /* Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "green"
        color: "transparent"
    } */

    onSmartLauncherEnabledChanged: {
        if (smartLauncherEnabled && !smartLauncherItem) {
            var smartLauncher = Qt.createQmlObject(
                        " import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet; TaskManagerApplet.SmartLauncherItem { }",
                        taskIcon);

            smartLauncher.launcherUrl = Qt.binding(function() { return taskItem.launcherUrlWithIcon; });

            smartLauncherItem = smartLauncher;
        } else if (!smartLauncherEnabled && smartLauncherItem) {
            smartLauncherItem.destroy();
            smartLauncherItem = null;
        }
    }

    Rectangle{
        id: draggedRectangle
        width: taskItem.isSeparator ? parent.width + 1 : iconImageBuffer.width+1
        height: taskItem.isSeparator ? parent.height + 1 : iconImageBuffer.height+1
        anchors.centerIn: iconGraphic
        opacity: 0
        radius: 3
        anchors.margins: 5

        property color tempColor: theme.highlightColor
        color: tempColor
        border.width: 1
        border.color: theme.highlightColor

        onTempColorChanged: tempColor.a = 0.35;
    }

    TitleTooltipParent{
        id: titleTooltipParent
        thickness: root.zoomFactor * (root.iconSize + root.thickMargins)
    }

    TitleTooltipParent{
        id: previewsTooltipParent
        thickness: root.zoomFactor * (root.iconSize + root.thickMargins) + 1
    }

    //!

    // KQuickControlAddons.QIconItem{
    Item{
        id: iconGraphic
        width: parent.width
        height: parent.height

        //fix bug #478, when changing form factor sometimes the tasks are not positioned
        //correctly, in such case we make a fast reinitialization for the sizes
        Connections {
            target: plasmoid

            onFormFactorChanged:{
                taskItem.inAddRemoveAnimation = false;

                wrapper.mScale = 1.01;
                wrapper.tempScaleWidth = 1.01;
                wrapper.tempScaleHeight = 1.01;

                wrapper.mScale = 1;
                wrapper.tempScaleWidth = 1;
                wrapper.tempScaleHeight = 1;
            }
        }

        Latte.IconItem{
            id: iconImageBuffer

            anchors.centerIn: parent

            width: Math.round(newTempSize)
            height: Math.round(width)
            source: decoration
            smooth: root.zoomFactor === 1 ? true : false
            providesColors: indicators ? indicators.info.needsIconColors : false

            opacity: root.enableShadows
                     && taskWithShadow.active
                     && graphicsSystem.isAccelerated ? 0 : 1
            visible: !taskItem.isSeparator && (!badgesLoader.active || !graphicsSystem.isAccelerated)

            onValidChanged: {
                if (!valid && (source === decoration || source === "unknown")) {
                    source = "application-x-executable";
                }
            }

            //! try to show the correct icon when a window is removed... libtaskmanager when a window is removed
            //! sends an unknown pixmap as icon
            Connections {
                target: taskItem
                onInRemoveStageChanged: {
                    if (taskItem.inRemoveStage && iconImageBuffer.lastValidSourceName !== "") {
                        iconImageBuffer.source = iconImageBuffer.lastValidSourceName;
                    }
                }
            }

            property int zoomedSize: root.zoomFactor * root.iconSize

            property real basicScalingWidth : wrapper.inTempScaling ? (root.iconSize * wrapper.scaleWidth) :
                                                                      root.iconSize * wrapper.mScale
            property real basicScalingHeight : wrapper.inTempScaling ? (root.iconSize * wrapper.scaleHeight) :
                                                                       root.iconSize * wrapper.mScale

            property real newTempSize: {
                if (wrapper.opacity == 1)
                    return Math.min(basicScalingWidth, basicScalingHeight)
                else
                    return Math.max(basicScalingWidth, basicScalingHeight)
            }

            //! Latte Side Painting-style if the user chose it
            Loader{
                anchors.fill: iconImageBuffer
                active: plasmoid.configuration.forceMonochromaticIcons

                sourceComponent: ColorOverlay {
                    anchors.fill: parent
                    color: latteBridge ? latteBridge.palette.textColor : "transparent"
                    source: iconImageBuffer
                }
            }
            //! Latte Side Painting-style if the user chose it

            ///states for launcher animation
            states: [
                State{
                    name: "*"
                    //! since qt 5.14 default state can not use "when" property
                    //! it breaks restoring transitions otherwise

                    AnchorChanges{
                        target:iconImageBuffer;
                        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                        anchors.right: root.position === PlasmaCore.Types.RightPositioned ? parent.right : undefined;
                        anchors.left: root.position === PlasmaCore.Types.LeftPositioned ? parent.left : undefined;
                        anchors.top: root.position === PlasmaCore.Types.TopPositioned ? parent.top : undefined;
                        anchors.bottom: root.position === PlasmaCore.Types.BottomPositioned ? parent.bottom : undefined;
                    }
                },

                State{
                    name: "inAddRemoveAnimation"
                    when:  taskItem.inAddRemoveAnimation

                    AnchorChanges{
                        target:iconImageBuffer;
                        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                        anchors.right: root.position === PlasmaCore.Types.LeftPositioned ? parent.right : undefined;
                        anchors.left: root.position === PlasmaCore.Types.RightPositioned ? parent.left : undefined;
                        anchors.top: root.position === PlasmaCore.Types.BottomPositioned ? parent.top : undefined;
                        anchors.bottom: root.position === PlasmaCore.Types.TopPositioned ? parent.bottom : undefined;
                    }
                },

                State{
                    name: "animating"
                    when: !taskItem.inAddRemoveAnimation && (launcherAnimation.running || newWindowAnimation.running || fastRestoreAnimation.running)

                    AnchorChanges{
                        target:iconImageBuffer;
                        anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                        anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                        anchors.right: root.position === PlasmaCore.Types.LeftPositioned ? parent.right : undefined;
                        anchors.left: root.position === PlasmaCore.Types.RightPositioned ? parent.left : undefined;
                        anchors.top: root.position === PlasmaCore.Types.BottomPositioned ? parent.top : undefined;
                        anchors.bottom: root.position === PlasmaCore.Types.TopPositioned ? parent.bottom : undefined;
                    }
                }
            ]

            ///transitions, basic for the anchor changes
            transitions: [
                Transition{
                    from: "animating"
                    to: "*"
                    enabled: !fastRestoreAnimation.running && !taskItem.inMimicParabolicAnimation

                    AnchorAnimation { duration: 1.5*root.durationTime*units.longDuration }
                }
            ]
        } //IconImageBuffer

        //! Shadows
        Loader{
            id: taskWithShadow
            anchors.fill: iconImageBuffer
            active: root.enableShadows && !taskItem.isSeparator && graphicsSystem.isAccelerated

            sourceComponent: DropShadow{
                anchors.fill: parent
                color: root.appShadowColor
                fast: true
                samples: 2 * radius
                source: badgesLoader.active ? badgesLoader.item : iconImageBuffer
                radius: root.appShadowSize
                verticalOffset: 2
            }
        }
        //! Shadows

        //! Combined Loader for Progress and Audio badges masks
        Loader{
            id: badgesLoader
            anchors.fill: iconImageBuffer
            active: (activateProgress > 0) && graphicsSystem.isAccelerated
            asynchronous: true
            opacity: stateColorizer.opacity > 0 ? 0 : 1

            property real activateProgress: showInfo || showProgress || showAudio ? 1 : 0

            property bool showInfo: (root.showInfoBadge && taskIcon.smartLauncherItem && !taskItem.isSeparator
                                     && (taskIcon.smartLauncherItem.countVisible || taskItem.badgeIndicator > 0) && !taskIcon.smartLauncherItem.progressVisible)

            property bool showProgress: root.showProgressBadge && taskIcon.smartLauncherItem && !taskItem.isSeparator
                                        && taskIcon.smartLauncherItem.progressVisible

            property bool showAudio: (root.showAudioBadge && taskItem.hasAudioStream && taskItem.playingAudio && !taskItem.isSeparator) && !shortcutBadge.active

            Behavior on activateProgress {
                NumberAnimation { duration: root.durationTime*2*units.longDuration }
            }

            sourceComponent: Item{
                ShaderEffect {
                    id: iconOverlay
                    enabled: false
                    anchors.fill: parent
                    property var source: ShaderEffectSource {
                        sourceItem: Latte.IconItem{
                            width: iconImageBuffer.width
                            height: iconImageBuffer.height
                            source: iconImageBuffer.source

                            Loader{
                                anchors.fill: parent
                                active: plasmoid.configuration.forceMonochromaticIcons

                                sourceComponent: ColorOverlay {
                                    anchors.fill: parent
                                    color: latteBridge ? latteBridge.palette.textColor : "transparent"
                                    source: iconImageBuffer
                                }
                            }
                        }
                    }
                    property var mask: ShaderEffectSource {
                        sourceItem: Item{
                            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft && !root.vertical
                            LayoutMirroring.childrenInherit: true

                            width: iconImageBuffer.width
                            height: iconImageBuffer.height

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
                                        when: (plasmoid.location !== PlasmaCore.Types.RightEdge)

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
                                        when: (plasmoid.location === PlasmaCore.Types.RightEdge)

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
                                        when: (plasmoid.location !== PlasmaCore.Types.RightEdge)

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
                                        when: (plasmoid.location === PlasmaCore.Types.RightEdge)

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
            anchors.fill: iconImageBuffer
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

                    layer.enabled: root.enableShadows && graphicsSystem.isAccelerated
                    layer.effect: DropShadow {
                        color: root.appShadowColor
                        fast: true
                        samples: 2 * radius
                        source: infoBadge
                        radius: root.appShadowSize
                        verticalOffset: 2
                    }
                }

                AudioStream{
                    id: audioStreamBadge
                    anchors.fill: parent
                    opacity: badgesLoader.activateProgress
                    visible: badgesLoader.showAudio

                    layer.enabled: root.enableShadows && graphicsSystem.isAccelerated
                    layer.effect: DropShadow {
                        color: root.appShadowColor
                        fast: true
                        samples: 2 * radius
                        source: audioStreamBadge
                        radius: root.appShadowSize
                        verticalOffset: 2
                    }
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
            width: iconImageBuffer.width
            height: iconImageBuffer.height
            anchors.centerIn: iconImageBuffer

            active: badgeVisualsLoader.active && graphicsSystem.isAccelerated
            sourceComponent: Colorize{
                source: badgeVisualsLoader.item

                //! HACK TO AVOID PIXELIZATION
                //! WORKAROUND: When Effects are enabled e.g. BrightnessContrast, Colorize etc.
                //! the icon appears pixelated. It is even most notable when zoomFactor === 1
                //! I don't know enabling cached=true helps, but it does.
                cached: true

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
            anchors.centerIn: iconImageBuffer
            width: iconImageBuffer.width
            height: iconImageBuffer.height

            //! HACK TO AVOID PIXELIZATION
            //! WORKAROUND: When Effects are enabled e.g. BrightnessContrast, Colorize etc.
            //! the icon appears pixelated. It is even most notable when zoomFactor === 1
            //! I don't know enabling cached=true helps, but it does.
            cached: true

            source: badgesLoader.active ? badgesLoader : iconImageBuffer
            visible: !isSeparator

            opacity:0

            hue:0
            saturation:0
            lightness:0
        }

        BrightnessContrast{
            id:hoveredImage
            anchors.centerIn: iconImageBuffer
            width: iconImageBuffer.width
            height: iconImageBuffer.height

            //! HACK TO AVOID PIXELIZATION
            //! WORKAROUND: When Effects are enabled e.g. BrightnessContrast, Colorize etc.
            //! the icon appears pixelated. It is even most notable when zoomFactor === 1
            //! I don't know enabling cached=true helps, but it does.
            cached: true

            source: badgesLoader.active ? badgesLoader : iconImageBuffer
            visible: !isSeparator

            opacity: taskItem.containsMouse && !clickedAnimation.running && !indicators.info.providesHoveredAnimation ? 1 : 0
            brightness: 0.30
            contrast: 0.1

            Behavior on opacity {
                NumberAnimation { duration: root.durationTime*units.longDuration }
            }
        }

        BrightnessContrast {
            id: brightnessTaskEffect
            anchors.centerIn: iconImageBuffer
            width: iconImageBuffer.width
            height: iconImageBuffer.height

            //! HACK TO AVOID PIXELIZATION
            //! WORKAROUND: When Effects are enabled e.g. BrightnessContrast, Colorize etc.
            //! the icon appears pixelated. It is even most notable when zoomFactor === 1
            //! I don't know enabling cached=true helps, but it does.
            cached: true

            source: badgesLoader.active ? badgesLoader : iconImageBuffer

            visible: clickedAnimation.running && !isSeparator
        }
        //! Effects

        ShortcutBadge{
            id: shortcutBadge
        }
    }

    Loader {
        id: dropFilesVisual
        active: applyOpacity>0

        width: !root.vertical ? length : thickness
        height: !root.vertical ? thickness : length
        anchors.centerIn: parent

        readonly property int length: root.iconSize + root.lengthMargins
        readonly property int thickness: root.iconSize + root.thickMargins

        readonly property real applyOpacity: root.dropNewLauncher && !mouseHandler.onlyLaunchers
                                             && (root.dragSource == null) && (mouseHandler.hoveredItem === taskItem) ? 0.7 : 0

        sourceComponent: LatteComponents.AddItem {
            anchors.fill: parent
            backgroundOpacity: dropFilesVisual.applyOpacity
        }
    }

    Component.onDestruction: {
        taskIcon.toBeDestroyed = true;

        if(removingAnimation.removingItem)
            removingAnimation.removingItem.destroy();
    }

    Connections{
        target: taskItem

        onInAttentionChanged:{
            if (!taskItem.inAttention && newWindowAnimation.running && taskItem.inAttentionAnimation) {
                newWindowAnimation.pause();
                fastRestoreAnimation.start();
            }
        }
    }

    ///// Animations /////

    TaskAnimations.ClickedAnimation { id: clickedAnimation }

    TaskAnimations.LauncherAnimation { id:launcherAnimation }

    TaskAnimations.NewWindowAnimation { id: newWindowAnimation }

    TaskAnimations.RemoveWindowFromGroupAnimation { id: removingAnimation }

    TaskAnimations.FastRestoreAnimation { id: fastRestoreAnimation }

    //////////// States ////////////////////
    states: [
        State{
            name: "*"
            //! since qt 5.14 default state can not use "when" property
            //! it breaks restoring transitions otherwise
        },

        State{
            name: "isDragged"
            when: ( (taskItem.isDragged) && (!root.inConfigureAppletsMode) )
        }
    ]

    //////////// Transitions //////////////

    transitions: [
        Transition{
            id: isDraggedTransition
            to: "isDragged"
            property int speed: root.durationTime*units.longDuration

            SequentialAnimation{
                ScriptAction{
                    script: {
                        icList.directRender = false;
                        if(latteView) {
                            latteView.globalDirectRender=false;
                        }

                        taskItem.inBlockingAnimation = true;
                        root.clearZoom();
                    }
                }

                PropertyAnimation {
                    target: wrapper
                    property: "mScale"
                    to: 1 + ((root.zoomFactor - 1) / 3)
                    duration: isDraggedTransition.speed / 2
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
                        target: iconImageBuffer
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
            }

            onRunningChanged: {
                if(running){
                    taskItem.animationStarted();
                    //root.animations++;

                    parabolicManager.clearTasksGreaterThan(index);
                    parabolicManager.clearTasksLowerThan(index);

                    if (latteView){
                        latteView.parabolicManager.clearAppletsGreaterThan(latteView.latteAppletPos);
                        latteView.parabolicManager.clearAppletsLowerThan(latteView.latteAppletPos);
                    }
                }
            }
        },
        Transition{
            id: defaultTransition
            from: "isDragged"
            to: "*"
            property int speed: root.durationTime*units.longDuration

            SequentialAnimation{
                ScriptAction{
                    script: {
                        icList.directRender = false;
                        if(latteView) {
                            latteView.globalDirectRender=false;
                        }
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
                        target: iconImageBuffer
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

                /*  PropertyAnimation {
                    target: wrapper
                    property: "mScale"
                    to: 1;
                    duration: isDraggedTransition.speed
                    easing.type: Easing.OutQuad
                }*/

                ScriptAction{
                    script: {
                        taskItem.inBlockingAnimation = false;
                    }
                }
            }

            onRunningChanged: {
                if(!running){
                    var halfZoom = 1 + ((root.zoomFactor - 1) / 2);

                    wrapper.calculateScales((root.iconSize+root.thickMargins)/2);

                    taskItem.animationEnded();
                    //   root.animations--;
                }
            }
        }
    ]

}// Icon Item
