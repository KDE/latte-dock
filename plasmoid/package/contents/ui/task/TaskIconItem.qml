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
import org.kde.latte 0.1 as Latte

import "animations" as TaskAnimations


//I am using  KQuickControlAddons.QIconItem even though onExit it triggers the following error
//QObject::~QObject: Timers cannot be stopped from another thread
//but it increases performance almost to double during animation

Item{
    id: centralItem

    width: wrapper.regulatorWidth
    height: wrapper.regulatorHeight

    //big interval to show shadows only after all the crappy adds and removes of tasks
    //have happened
    property bool firstDrawed: true
    property bool toBeDestroyed: false

    // three intervals in order to create the necessarty buffers from the
    // PlasmaCore.IconItem, one big interval for the first creation of the
    // plasmoid, a second one for the first creation of a task and a small one
    // for simple updates.
    // This is done before especially on initialization stage some visuals
    // are not ready and empty buffers are created

    //property int firstDrawedInterval: root.initializationStep ? 2000 : 1000
    // property int shadowInterval: firstDrawed ? firstDrawedInterval : 250
    property int shadowInterval: firstDrawed ? 1000 : 250

    property int shadowSize : root.appShadowSize

    readonly property bool smartLauncherEnabled: ((mainItemContainer.isStartup === false) && (root.smartLaunchersEnabled))
    readonly property variant iconDecoration: decoration
    property QtObject buffers: null
    property QtObject smartLauncherItem: null

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
                        centralItem);

            smartLauncher.launcherUrl = Qt.binding(function() { return mainItemContainer.launcherUrlWithIcon; });

            smartLauncherItem = smartLauncher;
        } else if (!smartLauncherEnabled && smartLauncherItem) {
            smartLauncherItem.destroy();
            smartLauncherItem = null;
        }
    }

    Rectangle{
        id: draggedRectangle
        width: mainItemContainer.isSeparator ? parent.width + 1 : iconImageBuffer.width+1
        height: mainItemContainer.isSeparator ? parent.height + 1 : iconImageBuffer.height+1
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
        thickness: (root.zoomFactor * root.realSize) + root.statesLineSize
    }

    TitleTooltipParent{
        id: previewsTooltipParent
        thickness: (root.zoomFactor * (root.thickMarginBase + root.iconSize)) + root.statesLineSize + 1
    }

    // KQuickControlAddons.QIconItem{
    Item{
        id: iconGraphic
        //width: iconImageBuffer.width
        //height: iconImageBuffer.height
        width: parent.width
        height: parent.height

        opacity: root.enableShadows ? 0 : 1

        //fix bug #478, when changing form factor sometimes the tasks are not positioned
        //correctly, in such case we make a fast reinitialization for the sizes
        Connections {
            target: plasmoid

            onFormFactorChanged:{
                mainItemContainer.inAddRemoveAnimation = false;

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

            anchors.rightMargin:{
                if (root.position === PlasmaCore.Types.RightPositioned)
                    return root.thickMarginBase;
                else if (root.position === PlasmaCore.Types.LeftPositioned)
                    return wrapper.mScale * root.thickMarginHigh;
                else
                    return 0;
            }
            anchors.leftMargin: {
                if (root.position === PlasmaCore.Types.LeftPositioned)
                    return root.thickMarginBase;
                else if (root.position === PlasmaCore.Types.RightPositioned)
                    return wrapper.mScale * root.thickMarginHigh;
                else
                    return 0;
            }
            anchors.topMargin: {
                if (root.position === PlasmaCore.Types.TopPositioned)
                    return root.thickMarginBase;
                else if (root.position === PlasmaCore.Types.BottomPositioned)
                    return wrapper.mScale * root.thickMarginHigh;
                else
                    return 0;
            }
            anchors.bottomMargin:{
                if (root.position === PlasmaCore.Types.BottomPositioned)
                    return root.thickMarginBase;
                else if (root.position === PlasmaCore.Types.TopPositioned)
                    return wrapper.mScale * root.thickMarginHigh;
                else
                    return 0;
            }

            width: Math.round(newTempSize) //+ 2*centralItem.shadowSize
            height: Math.round(width)
            source: decoration

            visible: !mainItemContainer.isSeparator && !badgesLoader.active
            //visible: !root.enableShadows

            onValidChanged: {
                if (!valid && (source === decoration || source === "unknown")) {
                    source = "application-x-executable";
                }
            }

            //! try to show the correct icon when a window is removed... libtaskmanager when a window is removed
            //! sends an unknow pixmap as icon
            Connections {
                target: mainItemContainer
                onInRemoveStageChanged: {
                    if (mainItemContainer.inRemoveStage && iconImageBuffer.lastValidSourceName !== "") {
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

            ///states for launcher animation
            states: [
                State{
                    name: "*"
                    when:  !launcherAnimation.running && !newWindowAnimation.running && !mainItemContainer.inAddRemoveAnimation && !fastRestoreAnimation.running

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
                    when:  mainItemContainer.inAddRemoveAnimation

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
                    when: (launcherAnimation.running || newWindowAnimation.running || fastRestoreAnimation.running) && !mainItemContainer.inAddRemoveAnimation

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
                    enabled: !fastRestoreAnimation.running && !mainItemContainer.inMimicParabolicAnimation

                    AnchorAnimation { duration: 1.5*root.durationTime*units.longDuration }
                }
            ]
        }

        ////! Combined Loader for Progress and Audio badges
        Loader{
            id: badgesLoader
            anchors.fill: iconImageBuffer
            active: opacityN > 0
            asynchronous: true

            property real opacityN: showProgress || showAudio ? 1 : 0

            property bool showProgress: (centralItem.smartLauncherEnabled && centralItem.smartLauncherItem && !mainItemContainer.isSeparator
                                         && (centralItem.smartLauncherItem.countVisible || centralItem.smartLauncherItem.progressVisible
                                             || mainItemContainer.badgeIndicator > 0))

            property bool showAudio: mainItemContainer.hasAudioStream && mainItemContainer.playingAudio &&
                                     !mainItemContainer.isSeparator

            Behavior on opacityN {
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
                                width: Math.max(infoBadge.contentWidth, parent.width / 2)
                                height: parent.height / 2
                                radius: parent.height
                                visible: badgesLoader.showProgress

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

                            Connections{
                                target: plasmoid
                                onLocationChanged: iconOverlay.mask.scheduleUpdate();
                            }

                            Connections{
                                target:badgesLoader
                                onShowProgressChanged: iconOverlay.mask.scheduleUpdate();
                                onShowAudioChanged: iconOverlay.mask.scheduleUpdate();
                            }

                        }
                        hideSource: true
                        live: mainItemContainer.badgeIndicator > 0 ? true : false
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

                TaskProgressOverlay{
                    id: infoBadge
                    anchors.fill:parent
                    opacity: badgesLoader.opacityN
                    visible: badgesLoader.showProgress
                }

                AudioStream{
                    id: audioStreamBadge
                    anchors.fill:parent
                    opacity: badgesLoader.opacityN
                    visible: badgesLoader.showAudio
                }
            }
        }
        ////!


        /// START Task Number
        Loader{
            id: taskNumberLoader
            anchors.fill: iconImageBuffer
            active: opacityN>0 && !launcherAnimation.running
            asynchronous: true

            property int fixedIndex:-1

            onActiveChanged: {
                if (active) {
                    fixedIndex = parabolicManager.pseudoTaskIndex(index+1);
                }
            }

            Component.onCompleted: fixedIndex = parabolicManager.pseudoTaskIndex(index+1);

            property real opacityN: root.showTasksNumbers && !mainItemContainer.isSeparator && fixedIndex<20 ? 1 : 0

            Behavior on opacityN {
                NumberAnimation { duration: root.durationTime*2*units.longDuration }
            }

            sourceComponent: Item{
                Loader{
                    anchors.fill: taskNumber
                    active: root.enableShadows

                    sourceComponent: DropShadow{
                        color: root.appShadowColor
                        fast: true
                        samples: 2 * radius
                        source: taskNumber
                        radius: root.appShadowSize/2
                        verticalOffset: 2
                    }
                }

                Latte.BadgeText {
                    id: taskNumber
                    anchors.centerIn: parent
                    //opacity: taskNumberLoader.opacityN && !root.enableShadows ? 1 : 0

                    minimumWidth: 0.4 * root.iconSize
                    height: width
                    numberValue: taskNumberLoader.fixedIndex < 10 ? taskNumberLoader.fixedIndex : 0
                    textValue: (keysArrayIndex>=0 && keysArrayIndex<10) ? keysAboveTen[keysArrayIndex] : ''

                    showNumber: taskNumberLoader.fixedIndex < 10
                    showText: taskNumberLoader.fixedIndex>=10 && taskNumberLoader.fixedIndex<20

                    proportion: 0
                    radiusPerCentage: 50

                    property int keysArrayIndex: taskNumberLoader.fixedIndex-10;
                    property var keysAboveTen: ['0', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.']
                }
            }
        }
        //END of task number

        //showTasksNumbers
    }

    ///Shadow in tasks
    Loader{
        id: taskWithShadow
        anchors.fill: iconGraphic
        active: root.enableShadows

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: root.appShadowColor
            fast: true
            samples: 2 * radius
            source: iconGraphic
            radius: root.appShadowSize
            verticalOffset: 2
        }
    }

    VisualAddItem{
        id: dropFilesVisual
        anchors.fill: iconGraphic

        visible: opacity == 0 ? false : true
        opacity: root.dropNewLauncher && !mouseHandler.onlyLaunchers
                 && (root.dragSource == null) && (mouseHandler.hoveredItem === mainItemContainer) ? 1 : 0
    }

    BrightnessContrast{
        id:hoveredImage
        opacity: mainItemContainer.containsMouse && !clickedAnimation.running ? 1 : 0
        anchors.fill: iconGraphic

        brightness: 0.30
        contrast: 0.1
        source: iconGraphic

        Behavior on opacity {
            NumberAnimation { duration: root.durationTime*units.longDuration }
        }
    }

    BrightnessContrast {
        id: brightnessTaskEffect
        anchors.fill: iconGraphic
        source: iconGraphic

        visible: clickedAnimation.running
    }

    Colorize{
        id: stateColorizer
        source: badgesLoader.active ? badgesLoader : iconImageBuffer
        anchors.centerIn: iconGraphic
        width: source.width
        height: source.height

        //visible: false
        opacity:0

        hue:0
        saturation:0
        lightness:0
    }

    Component.onCompleted: {
    }

    Component.onDestruction: {
        centralItem.toBeDestroyed = true;

        if(removingAnimation.removingItem)
            removingAnimation.removingItem.destroy();
    }

    Connections{
        target: mainItemContainer

        onShowAttentionChanged:{
            if (!mainItemContainer.showAttention && newWindowAnimation.running && mainItemContainer.inAttentionAnimation) {
                newWindowAnimation.pause();
                fastRestoreAnimation.start();
            }
        }
    }

    ///// Animations /////

    TaskAnimations.TaskClickedAnimation { id: clickedAnimation }

    TaskAnimations.TaskLauncherAnimation { id:launcherAnimation }

    TaskAnimations.TaskNewWindowAnimation { id: newWindowAnimation }

    TaskAnimations.TaskRemoveWindowFromGroupAnimation { id: removingAnimation }

    TaskAnimations.TaskFastRestoreAnimation { id: fastRestoreAnimation }

    //////////// States ////////////////////
    states: [
        State{
            name: "*"
            when:  !mainItemContainer.isDragged
        },

        State{
            name: "isDragged"
            when: ( (mainItemContainer.isDragged) && (!root.editMode) )
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
                        if(latteDock) {
                            latteDock.globalDirectRender=false;
                        }

                        mainItemContainer.inBlockingAnimation = true;
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
                        to: mainItemContainer.isSeparator ? 0 : 1
                        duration: isDraggedTransition.speed
                        easing.type: Easing.OutQuad
                    }
                }
            }

            onRunningChanged: {
                if(running){
                    mainItemContainer.animationStarted();
                    //root.animations++;

                    parabolicManager.clearTasksGreaterThan(index);
                    parabolicManager.clearTasksLowerThan(index);

                    if (latteDock){
                        latteDock.parabolicManager.clearAppletsGreaterThan(latteDock.latteAppletPos);
                        latteDock.parabolicManager.clearAppletsLowerThan(latteDock.latteAppletPos);
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
                        if(latteDock) {
                            latteDock.globalDirectRender=false;
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
                        mainItemContainer.inBlockingAnimation = false;
                    }
                }
            }

            onRunningChanged: {
                if(!running){
                    var halfZoom = 1 + ((root.zoomFactor - 1) / 2);

                    wrapper.calculateScales((root.iconSize+root.iconMargin)/2);

                    mainItemContainer.animationEnded();
                    //   root.animations--;
                }
            }
        }
    ]

}// Icon Item
