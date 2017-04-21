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

    property int shadowSize : Math.ceil(root.iconSize / 10)

    readonly property bool smartLauncherEnabled: ((mainItemContainer.isStartup === false) && (root.smartLaunchersEnabled))
    readonly property variant iconDecoration: decoration
    property QtObject buffers: null
    property QtObject smartLauncherItem: null

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

            smartLauncher.launcherUrl = Qt.binding(function() { return model.LauncherUrlWithoutIcon; });

            smartLauncherItem = smartLauncher;
        } else if (!smartLauncherEnabled && smartLauncherItem) {
            smartLauncherItem.destroy();
            smartLauncherItem = null;
        }
    }

    Rectangle{
        id: draggedRectangle
        width: iconImageBuffer.width+1
        height: iconImageBuffer.height+1
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

    /* Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "red"
        color: "transparent"
    } */

    // KQuickControlAddons.QIconItem{
    Item{
        id: iconGraphic
        //width: iconImageBuffer.width
        //height: iconImageBuffer.height
        width: parent.width
        height: parent.height

        Latte.IconItem{
            id: iconImageBuffer

            anchors.rightMargin: root.position === PlasmaCore.Types.RightPositioned
                                 || (root.position === PlasmaCore.Types.LeftPositioned
                                     && mainItemContainer.inAddRemoveAnimation)
                                 ? root.thickMarginBase : 0
            anchors.leftMargin: root.position === PlasmaCore.Types.LeftPositioned
                                || (root.position === PlasmaCore.Types.RightPositioned
                                    && mainItemContainer.inAddRemoveAnimation)
                                ? root.thickMarginBase : 0
            anchors.topMargin: root.position === PlasmaCore.Types.TopPositioned
                               || (root.position === PlasmaCore.Types.BottomPositioned
                                   && mainItemContainer.inAddRemoveAnimation)
                               ? root.thickMarginBase : 0
            anchors.bottomMargin: root.position === PlasmaCore.Types.BottomPositioned
                                  || (root.position === PlasmaCore.Types.TopPositioned
                                      && mainItemContainer.inAddRemoveAnimation)
                                  ? root.thickMarginBase : 0

            width: Math.round(newTempSize) //+ 2*centralItem.shadowSize
            height: Math.round(width)
            //icon: decoration
            source: decoration

            //visible: !root.enableShadows

            onValidChanged: {
                if (!valid && (source === decoration || source === "unknown")) {
                    source = "application-x-executable";
                }
            }

            property int zoomedSize: root.zoomFactor * root.iconSize

            property real basicScalingWidth : wrapper.inTempScaling ? (root.iconSize * wrapper.scaleWidth) :
                                                                      root.iconSize * wrapper.mScale
            property real basicScalingHeight : wrapper.inTempScaling ? (root.iconSize * wrapper.scaleHeight) :
                                                                       root.iconSize * wrapper.mScale

            property real newTempSize: (wrapper.opacity == 1) ? Math.min(basicScalingWidth, basicScalingHeight) :
                                                                Math.max(basicScalingWidth, basicScalingHeight)


            ///states for launcher animation
            states: [

                State{
                    name: "*"
                    when:  !launcherAnimation.running && !newWindowAnimation.running && !mainItemContainer.inAddRemoveAnimation

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
                    when: (launcherAnimation.running || newWindowAnimation.running) && !mainItemContainer.inAddRemoveAnimation

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

                    AnchorAnimation { duration: 1.5*root.durationTime*units.longDuration }
                }
            ]
        }

        //////
        Loader{
            id: progressLoader
            anchors.fill: parent
            active: (centralItem.smartLauncherEnabled && centralItem.smartLauncherItem
                     && (centralItem.smartLauncherItem.progressVisible || mainItemContainer.badgeIndicator > 0))
            asynchronous: true

            sourceComponent: Item{
                ShaderEffect {
                    id: iconOverlay
                    enabled: false
                    anchors.fill: parent
                    property var source: ShaderEffectSource {
                        sourceItem: iconImageBuffer
                        hideSource: true
                    }
                    property var mask: ShaderEffectSource {
                        sourceItem: Item{
                            width: iconImageBuffer.width
                            height: iconImageBuffer.height
                            Rectangle{
                                id: maskRect
                                width: parent.width/2
                                height: width
                                radius: width

                                Rectangle{
                                    id: maskCorner
                                    width:parent.width/2
                                    height:parent.height/2
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

                                Connections{
                                    target: plasmoid
                                    onLocationChanged: iconOverlay.mask.scheduleUpdate();
                                }
                            }
                            //badgeMask
                        }
                        hideSource: true
                        live: mainItemContainer.badgeIndicator > 0 ? true : false
                    }

                    //  onWidthChanged: mask.scheduleUpdate();
                    //  onHeightChanged: mask.scheduleUpdate();

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
                }

                TaskProgressOverlay{
                    anchors.fill:parent
                }
            }
        }

        //  Loader {
        //   anchors.fill: parent
        ////  asynchronous: true
        // source: "TaskProgressOverlay.qml"
        //  active: true
        //active: (centralItem.smartLauncherEnabled && centralItem.smartLauncherItem
        //        && centralItem.smartLauncherItem.progressVisible)
        //}
    }

    ///Shadow in tasks
    Loader{
        id: taskWithShadow
        anchors.fill: iconGraphic
        active: root.enableShadows

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: "#ff080808"
            samples: 2 * radius
            source: iconGraphic
            radius: centralItem.shadowSize
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
        opacity: mainItemContainer.containsMouse ? 1 : 0
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
        source: progressLoader.active ? progressLoader : iconImageBuffer
        anchors.fill: iconGraphic
        //visible: false
        opacity:0

        hue:0
        saturation:0
        lightness:0
    }

    ///////Activate animation/////
    SequentialAnimation{
        id: clickedAnimation

        property bool pressed: mainItemContainer.pressed
        property int speed: root.durationTime*units.longDuration

        ParallelAnimation{
            PropertyAnimation {
                target: brightnessTaskEffect
                property: "brightness"
                to: -0.5
                duration: clickedAnimation.speed
                easing.type: Easing.OutQuad
            }
            PropertyAnimation {
                target: wrapper
                property: "mScale"
                to: root.taskInAnimation ? 0.9 : wrapper.mScale - (root.zoomFactor - 1) / 2
                duration: clickedAnimation.speed
                easing.type: Easing.OutQuad
            }
        }

        ParallelAnimation{
            PropertyAnimation {
                target: brightnessTaskEffect
                property: "brightness"
                to: 0
                duration: clickedAnimation.speed
                easing.type: Easing.OutQuad
            }
            PropertyAnimation {
                target: wrapper
                property: "mScale"
                to: root.taskInAnimation ? 1 : root.zoomFactor
                duration: clickedAnimation.speed
                easing.type: Easing.OutQuad
            }
        }


        onPressedChanged: {
            if( (pressed)&&
                    ((mainItemContainer.lastButtonClicked == Qt.LeftButton)||(mainItemContainer.lastButtonClicked == Qt.MidButton)) ){
                mainItemContainer.animationStarted();
                start();
            }
        }

        onStopped: {
            if( !mainItemContainer.isDragged){
                mainItemContainer.animationEnded();
                checkListHovered.startDuration(6*units.longDuration);
            }
        }
    }


    Component.onCompleted: {
    }

    Component.onDestruction: {
        centralItem.toBeDestroyed = true;

        if(removingAnimation.removingItem)
            removingAnimation.removingItem.destroy();

        gc();
    }
    ////end of activate animation////

    ////bouncing task, e.g. on launcher activating and when a new window is
    ////added in a group task
    SequentialAnimation{
        id:launcherAnimation

        property bool launchedAlready: false
        property int speed: root.durationTime * 0.8 * units.longDuration

        SequentialAnimation{
            ScriptAction {
                script: mainItemContainer.launcherAction();
            }

            //Ghost animation that acts as a delayer
            PropertyAnimation {
                target: wrapper
                property: "opacity"
                to: 1
                duration:  50
                easing.type: Easing.InQuad
            }
            //end of ghost animation

            ParallelAnimation{
                PropertyAnimation {
                    target: wrapper
                    property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                    to: root.zoomFactor
                    duration: launcherAnimation.speed
                    easing.type: Easing.OutQuad
                }

                PropertyAnimation {
                    target: wrapper
                    property: (icList.orientation == Qt.Horizontal) ? "tempScaleWidth" : "tempScaleHeight"
                    to: 1
                    duration: launcherAnimation.speed
                    easing.type: Easing.OutQuad
                }
            }

            PropertyAnimation {
                target: wrapper
                property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                to: 1
                duration: 3*root.durationTime*launcherAnimation.speed
                easing.type: Easing.OutBounce
            }

            ParallelAnimation{
                PropertyAnimation {
                    target: wrapper
                    property: (icList.orientation == Qt.Vertical) ? "tempScaleHeight" : "tempScaleWidth"
                    to: 1
                    duration: root.durationTime*launcherAnimation.speed
                    easing.type: Easing.OutBounce
                }

                PropertyAnimation {
                    target: wrapper
                    property: "mScale"
                    to: 1
                    duration: root.durationTime*launcherAnimation.speed
                    easing.type: Easing.OutQuad
                }
            }
        }

        onStopped: {
            clearAnimationsSignals();

            mainItemContainer.setBlockingAnimation(false);
            mainItemContainer.animationEnded();
        }

        function clearAnimationsSignals() {
            if ( launchedAlready && root.noTasksInAnimation>0 ) {
                root.noTasksInAnimation--;
            }

            if ( launchedAlready ) {
                root.signalAnimationsNeedThickness(-1);
            }

            launchedAlready = false;
        }

        function init(){
            //console.log ("Nooo 1 : "+root.noTasksInAnimation);
            if(!launchedAlready) {
                launchedAlready = true;
                root.signalAnimationsNeedThickness(1);
                root.globalDirectRender=false;
                icList.directRender=false;
                root.updateScale(index+1, 1, 0);
                root.updateScale(index-1, 1, 0);
                root.noTasksInAnimation++;
                mainItemContainer.setBlockingAnimation(true);

                //trying to fix the ListView nasty behavior
                //during the removal the anchoring for ListView children changes a lot
                var previousTask = icList.chiltAtIndex(mainItemContainer.lastValidIndex-1);
                if (previousTask !== undefined){
                    if (root.vertical) {
                        mainItemContainer.anchors.top = previousTask.bottom;
                    } else {
                        mainItemContainer.anchors.left = previousTask.right;
                    }
                }
            }

            wrapper.tempScaleWidth = wrapper.mScale;
            wrapper.tempScaleHeight = wrapper.mScale;
            icList.hoveredIndex = -1;
        }

        function bounceLauncher(){
            if(root.zoomFactor > 1){
                mainItemContainer.animationStarted();
                init();
                start();
            }
            else{
                stopped();
                mainItemContainer.launcherAction();
            }
        }


        Component.onCompleted: {
            wrapper.runLauncherAnimation.connect(bounceLauncher);
        }

        Component.onDestruction: {
            clearAnimationsSignals();
        }
    }
    /////////////////// end of launcher animation


    ////////////////// new window and needs attention animation
    //stop the bouncing animation for attention needed when the plasmoid
    //does not need any more attention
    Connections{
        target: plasmoid
        onStatusChanged:{
            if ( (plasmoid.status === PlasmaCore.Types.PassiveStatus)
                    && newWindowAnimation.running && (newWindowAnimation.loops > 2) ) {
                newWindowAnimation.clear();
            }
        }
    }

    Connections{
        target: mainItemContainer
        onContainsMouseChanged: {
            if (mainItemContainer.containsMouse && newWindowAnimation.running) {
                newWindowAnimation.stop();
            }
        }
    }

    SequentialAnimation{
        id:newWindowAnimation

        property int speed: root.durationTime*units.longDuration
        property bool isDemandingAttention: (IsDemandingAttention === true)
        property bool entered: mainItemContainer.mouseEntered
        property bool needsThicknessSent: false //flag to check if the signal for thickness was sent

        SequentialAnimation{
            ParallelAnimation{
                PropertyAnimation {
                    target: wrapper
                    property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                    to: 1 + (0.6 * (root.zoomFactor-1))
                    duration: newWindowAnimation.speed
                    easing.type: Easing.OutQuad
                }

                PropertyAnimation {
                    target: wrapper
                    property: (icList.orientation == Qt.Horizontal) ? "tempScaleWidth" : "tempScaleHeight"
                    to: 1
                    duration: newWindowAnimation.speed
                    easing.type: Easing.OutQuad
                }
            }

            PropertyAnimation {
                target: wrapper
                property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                to: 1
                duration: 3*root.durationTime*newWindowAnimation.speed
                easing.type: Easing.OutBounce
            }
        }

        function clear(){
            loops = 1;
            newWindowAnimation.stop();
            //  iconImageBuffer.anchors.centerIn = iconImageBuffer.parent;

            wrapper.tempScaleWidth = 1;
            wrapper.tempScaleHeight = 1;
        }

        onStopped: {
            sendEndOfNeedThicknessAnimation();
            clear();
        }

        onIsDemandingAttentionChanged: {
            if( (!isDemandingAttention)&&(running)){
                clear();
                //  wrapper.animationEnded();
            }
            else if(isDemandingAttention){
                bounceNewWindow();
            }
        }

        function sendEndOfNeedThicknessAnimation(){
            if (needsThicknessSent) {
                needsThicknessSent = false;
                root.signalAnimationsNeedThickness(-1);
            }
        }

        function init(){
            wrapper.tempScaleWidth = wrapper.mScale;
            wrapper.tempScaleHeight = wrapper.mScale;

            if(!isDemandingAttention)
                loops = 2;
            else
                loops = 20;

            if (!needsThicknessSent) {
                needsThicknessSent = true;
                root.signalAnimationsNeedThickness(1);
            }

            // icList.hoveredIndex = -1;
        }

        function bounceNewWindow(){
            if (isDemandingAttention && !root.dockIsHidden && (root.zoomFactor > 1)){
                newWindowAnimation.init();
                start();
            }
        }

        Component.onCompleted: {
            mainItemContainer.groupWindowAdded.connect(bounceNewWindow);
        }

        Component.onDestruction: {
            sendEndOfNeedThicknessAnimation();
        }
    }

    /////Removing a Window from a group////

    Item{
        id:removingAnimation

        function init(){
            var relavantPoint = root.mapFromItem(centralItem,0,0);

            var removingItem = removeTaskComponent.createObject(root);
            removingItem.x = relavantPoint.x;
            removingItem.y = relavantPoint.y;

            removingItem.start();
        }

        function removeTask(){
            if(!centralItem.toBeDestroyed && plasmoid.configuration.showShadows && Latte.WindowSystem.compositingActive){
                removingAnimation.init();
            }
        }

        Component.onCompleted: {
            mainItemContainer.groupWindowRemoved.connect(removeTask);
        }

        ///////////// Component for animating removing window from group

        Component {
            id: removeTaskComponent
            Item{
                id: removeTask
                width: centralItem.width
                height: centralItem.height

                visible: false

                Latte.IconItem{
                    id: tempRemoveIcon
                    anchors.rightMargin: root.position === PlasmaCore.Types.LeftPositioned ? root.thickMarginBase : 0
                    anchors.leftMargin: root.position === PlasmaCore.Types.RightPositioned ? root.thickMarginBase : 0
                    anchors.topMargin: root.position === PlasmaCore.Types.BottomPositioned ? root.thickMarginBase : 0
                    anchors.bottomMargin: root.position === PlasmaCore.Types.TopPositioned ? root.thickMarginBase : 0

                    anchors.horizontalCenter: !root.vertical ? parent.horizontalCenter : undefined;
                    anchors.verticalCenter: root.vertical ? parent.verticalCenter : undefined;
                    anchors.right: root.position === PlasmaCore.Types.LeftPositioned ? parent.right : undefined;
                    anchors.left: root.position === PlasmaCore.Types.RightPositioned ? parent.left : undefined;
                    anchors.top: root.position === PlasmaCore.Types.BottomPositioned ? parent.top : undefined;
                    anchors.bottom: root.position === PlasmaCore.Types.TopPositioned ? parent.bottom : undefined;

                    width: iconImageBuffer.width
                    height: width
                    visible: root.enableShadows ? false : true

                    source: iconImageBuffer.lastValidSourceName
                }

                Loader{
                    id: tempTaskShadow
                    anchors.fill: tempRemoveIcon
                    active: root.enableShadows

                    sourceComponent: DropShadow{
                        anchors.fill: parent
                        color: "#ff080808"
                        samples: 2 * radius
                        source: tempRemoveIcon
                        radius: centralItem.shadowSize
                        verticalOffset: 2
                    }
                }

                Colorize{
                    source: tempRemoveIcon
                    anchors.fill: tempRemoveIcon

                    hue: 0
                    saturation: 0
                    lightness: 0
                }

                ParallelAnimation{
                    id: componentRemoveAnimation

                    property int speed: 2*root.durationTime*units.longDuration
                    property Item removingItem: parent
                    property int toPoint: 0

                    PropertyAnimation {
                        target: removeTask
                        property: "opacity"
                        to: 0
                        duration: componentRemoveAnimation.speed
                        easing.type: Easing.InQuad
                    }

                    PropertyAnimation {
                        target: removeTask
                        property: (icList.orientation == Qt.Horizontal) ? "y" : "x"
                        to: componentRemoveAnimation.toPoint
                        duration: componentRemoveAnimation.speed
                        easing.type: Easing.InQuad
                    }

                    onStopped: {
                        removeTask.destroy();
                        gc();
                    }
                }

                function start(){
                    var tempPoint = 0;

                    if(icList.orientation == Qt.Horizontal)
                        tempPoint = y;
                    else
                        tempPoint = x;

                    if( (root.position === PlasmaCore.Types.BottomPositioned) ||
                            (root.position === PlasmaCore.Types.RightPositioned) ){
                        componentRemoveAnimation.toPoint = tempPoint + root.iconSize;
                    }
                    else{
                        componentRemoveAnimation.toPoint = tempPoint - root.iconSize;
                    }

                    visible = true;
                    componentRemoveAnimation.start();
                }

            }
        }
    }

    //////////// States ////////////////////
    states: [
        State{
            name: "*"
            when:  !mainItemContainer.isDragged
        },

        State{
            name: "isDragged"
            when: ( (mainItemContainer.isDragged) && (!root.editMode) )

            //    PropertyChanges { target: clickedAnimation; running:false }
            PropertyChanges { target: wrapper; mScale:1 + ((root.zoomFactor - 1) / 2)}
        }
    ]

    //////////// Transitions //////////////

    transitions: [
        Transition{
            id: isDraggedTransition
            to: "isDragged"
            property int speed: root.durationTime*units.longDuration

            SequentialAnimation{
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
                        to: 1
                        duration: isDraggedTransition.speed
                        easing.type: Easing.OutQuad
                    }
                }
            }

            onRunningChanged: {
                if(running){
                    mainItemContainer.animationStarted();
                    //root.animations++;
                    root.updateScale(index-1, 1, 0);
                    root.updateScale(index+1, 1, 0);
                }
            }
        },
        Transition{
            id: defaultTransition
            from: "isDragged"
            to: "*"
            property int speed: root.durationTime*units.longDuration

            SequentialAnimation{
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

                PropertyAnimation {
                    target: wrapper
                    property: "mScale"
                    to: 1;
                    duration: isDraggedTransition.speed
                    easing.type: Easing.OutQuad
                }

            }

            onRunningChanged: {
                if(!running){
                    var halfZoom = 1 + ((root.zoomFactor - 1) / 2);
                    root.updateScale(index-1, halfZoom, 0);
                    root.updateScale(index+1, halfZoom, 0);

                    mainItemContainer.animationEnded();
                    //   root.animations--;
                }
            }
        }
    ]

}// Icon Item
