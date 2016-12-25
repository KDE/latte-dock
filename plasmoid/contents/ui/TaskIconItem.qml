/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

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

    //property int firstDrawedInterval: panel.initializationStep ? 2000 : 1000
    // property int shadowInterval: firstDrawed ? firstDrawedInterval : 250
    property int shadowInterval: firstDrawed ? 1000 : 250

    property int shadowSize : Math.ceil(panel.iconSize / 10)

    readonly property bool smartLauncherEnabled: ((mainItemContainer.isStartup === false) && (plasmoid.configuration.smartLaunchersEnabled))
    readonly property variant iconDecoration: decoration
    property QtObject buffers: null
    property QtObject smartLauncherItem: null

    /* Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "green"
        color: "transparent"
    } */

    Connections{
        target: panel
        onZoomFactorChanged: updateImages()
        onIconSizeChanged: updateImages()
        onEnableShadowsChanged: updateImages()
    }

    onIconDecorationChanged: {
        updateImages();
    }

    Rectangle{
        id: draggedRectangle
        width: iconImageBuffer.width+1
        height: iconImageBuffer.height+1
        anchors.centerIn: iconImageBuffer
        opacity: 0
        radius: 3
        anchors.margins: 5

        property color tempColor: theme.highlightColor
        color: tempColor
        border.width: 1
        border.color: theme.highlightColor

        onTempColorChanged: tempColor.a = 0.35;
    }

    //temporary buffers containing the normal Image icon and the zoomed Image icon
    // Image{id:zoomedImage; visible:false}
    //  Image{id:normalImage; visible:false}
    Image{
        id:shadowedImage
        anchors.centerIn:iconImageBuffer
        // anchors.horizontalCenter: iconImageBuffer.horizontalCenter
        //anchors.verticalCenter: iconImageBuffer.verticalCenter

        width:iconImageBuffer.width+2*shadowSize
        height:iconImageBuffer.height+2*shadowSize

        //visible: plasmoid.configuration.showShadows
        visible: false

        states: State {
            name: "reparented"
            ParentChange { target: shadowedImage; parent: panel; }
        }

        //Corize to use when a window is removed....
        /*    Colorize{
            id: removeImageColorizer
            source: parent
            anchors.fill: parent

            enabled: false
            visible: false

            hue: 0
            saturation: 0
            lightness: 0
        }*/
    }

    /* Rectangle{
        anchors.fill: iconImageBuffer
        border.width: 1
        border.color: "red"
        color: "transparent"
    }*/

    KQuickControlAddons.QIconItem{
        id: iconImageBuffer

        //    anchors.centerIn: parent

        width: Math.round(newTempSize) //+ 2*centralItem.shadowSize
        height: Math.round(width)
        icon: decoration

        property int zoomedSize: panel.zoomFactor * panel.iconSize

        property real basicScalingWidth : wrapper.inTempScaling ? (panel.iconSize * wrapper.scaleWidth) :
                                                                  panel.iconSize * wrapper.scale
        property real basicScalingHeight : wrapper.inTempScaling ? (panel.iconSize * wrapper.scaleHeight) :
                                                                   panel.iconSize * wrapper.scale

        property real newTempSize: (wrapper.opacity == 1) ? Math.min(basicScalingWidth, basicScalingHeight) :
                                                            Math.max(basicScalingWidth, basicScalingHeight)

        ///states for launcher animation
        states: [
            State{
                name: "*"
                when:  !launcherAnimation.running && !newWindowAnimation.running

                AnchorChanges{
                    target:iconImageBuffer;
                    anchors.horizontalCenter: parent.horizontalCenter;
                    anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined;
                    anchors.left: undefined;
                    anchors.top: undefined;
                    anchors.bottom: undefined;
                }
            },

            State{
                name: "animating"
                when: launcherAnimation.running || newWindowAnimation.running

                AnchorChanges{
                    target:iconImageBuffer;
                    anchors.horizontalCenter: undefined;
                    anchors.verticalCenter: undefined;
                    anchors.right: panel.position === PlasmaCore.Types.LeftPositioned ? parent.right : undefined;
                    anchors.left: panel.position === PlasmaCore.Types.RightPositioned ? parent.left : undefined;
                    anchors.top: panel.position === PlasmaCore.Types.BottomPositioned ? parent.top : undefined;
                    anchors.bottom: panel.position === PlasmaCore.Types.TopPositioned ? parent.bottom : undefined;
                }
            }
        ]

        ///transitions, basic for the anchor changes
        transitions: [
            Transition{
                from: "animating"
                to: "*"

                AnchorAnimation { duration: 1.5*plasmoid.configuration.durationTime*units.longDuration }
            }
        ]
    }

    ///Shadow in tasks
    Loader{
        anchors.fill: iconImageBuffer
        active: plasmoid.configuration.showShadows

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: "#ff080808"
            samples: 2 * radius
            source: iconImageBuffer
            radius: centralItem.shadowSize
            verticalOffset: 2
        }
    }


    VisualAddItem{
        id: dropFilesVisual
        anchors.fill: iconImageBuffer

        visible: opacity == 0 ? false : true
        opacity: panel.dropNewLauncher && !mouseHandler.onlyLaunchers
                 && (panel.dragSource == null) && (mouseHandler.hoveredItem === mainItemContainer) ? 1 : 0
    }

    BrightnessContrast{
        id:hoveredImage
        opacity: mainItemContainer.containsMouse ? 1 : 0
        anchors.fill: iconImageBuffer

        brightness: 0.25
        source: iconImageBuffer

        Behavior on opacity {
            NumberAnimation { duration: plasmoid.configuration.durationTime*units.longDuration }
        }
    }

    BrightnessContrast {
        id: brightnessTaskEffect
        anchors.fill: iconImageBuffer
        source: iconImageBuffer

        visible: clickedAnimation.running
    }

    Colorize{
        id: stateColorizer
        source: iconImageBuffer
        anchors.fill: iconImageBuffer
        //visible: false
        opacity:0

        hue:0
        saturation:0
        lightness:0
    }

    //Something to show until the buffers are updated

    //KQuickControlAddons.QIconItem{
    /*
    PlasmaCore.IconItem{
        id: iconImageBackground

        //property real relatedSize: panel.iconSize  *  ( (doubleSize  - 7) / doubleSize );
        //    width: (visible) ? relatedSize * wrapper.scale : panel.iconSize
        width: (visible) ? panel.iconSize * wrapper.scale : panel.iconSize
        height: width
        anchors.centerIn: parent

        //        state: wrapper.containsMouse ? KQuickControlAddons.QIconItem.ActiveState : KQuickControlAddons.QIconItem.DefaultState
        //       icon: decoration
        active: wrapper.containsMouse
        enabled: true
        source: decoration
        usesPlasmaTheme: false

        visible: ((iconImageBuffer.opacity == 1) && (panel.enableShadows)) ? false : true

        Component{
            id:hideBackTimer

            Timer{
                id:hideBackgroundTimer
                repeat:false
                interval: centralItem.shadowInterval

                onTriggered: {
                    // iconImageBackground.visible = false;
                    iconImageBuffer.opacity = 1;
                    hideBackgroundTimer.destroy();
                    //   iconImageBuffer.visible = false;
                }

                Component.onCompleted: hideBackgroundTimer.start();
            }
        }
    }*/

    Loader{
        id:defaultWithShadow
        //sourceComponent: imageBufferingComponent
        sourceComponent: TaskIconBuffers{}
        active: mainItemContainer.isStartup ? false : true
    }

    Loader {
        anchors.fill: iconImageBuffer
        asynchronous: true
        source: "TaskProgressOverlay.qml"
        active: (centralItem.smartLauncherEnabled && centralItem.smartLauncherItem
                 && centralItem.smartLauncherItem.progressVisible)
    }

    ///////Activate animation/////
    SequentialAnimation{
        id: clickedAnimation

        property bool pressed: mainItemContainer.pressed
        property int speed: plasmoid.configuration.durationTime*units.longDuration

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
                property: "scale"
                to: panel.taskInAnimation ? 0.9 : wrapper.scale - (panel.zoomFactor - 1) / 2
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
                property: "scale"
                to: panel.taskInAnimation ? 1 : panel.zoomFactor
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
        if (smartLauncherEnabled && !smartLauncherItem) {
            var smartLauncher = Qt.createQmlObject(
                        " import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet; TaskManagerApplet.SmartLauncherItem { }",
                        centralItem);

            smartLauncher.launcherUrl = Qt.binding(function() { return model.LauncherUrlWithoutIcon; });

            smartLauncherItem = smartLauncher;
        }
    }

    Component.onDestruction: {
        centralItem.toBeDestroyed = true;

        if(shadowedImage && shadowedImage.source)
            shadowedImage.source.destroy();

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
        property int speed: plasmoid.configuration.durationTime * 0.8 * units.longDuration

        SequentialAnimation{
            ParallelAnimation{
                PropertyAnimation {
                    target: wrapper
                    property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                    to: panel.zoomFactor
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
                duration: 3*plasmoid.configuration.durationTime*launcherAnimation.speed
                easing.type: Easing.OutBounce
            }

            ParallelAnimation{
                PropertyAnimation {
                    target: wrapper
                    property: (icList.orientation == Qt.Vertical) ? "tempScaleHeight" : "tempScaleWidth"
                    to: 1
                    duration: plasmoid.configuration.durationTime*launcherAnimation.speed
                    easing.type: Easing.OutBounce
                }

                PropertyAnimation {
                    target: wrapper
                    property: "scale"
                    to: 1
                    duration: plasmoid.configuration.durationTime*launcherAnimation.speed
                    easing.type: Easing.OutQuad
                }
            }
        }


        onStopped: {
            //wrapper.scale = 1;
            /*   if ( panel.noTasksInAnimation>0 ) {
                panel.noTasksInAnimation--;
            }
            if ( panel.animations>0 ) {
                panel.animations--;
            }*/
            //console.log ("Nooo 2: "+panel.noTasksInAnimation + " - "+panel.animations);
            clearAnimationsSignals();

            mainItemContainer.setBlockingAnimation(false);
            mainItemContainer.animationEnded();
            mainItemContainer.launcherAction();
        }

        function clearAnimationsSignals() {
            if ( launchedAlready && panel.noTasksInAnimation>0 ) {
                panel.noTasksInAnimation--;
            }

            if ( launchedAlready && panel.animationsNeedThickness>0 ) {
                panel.setAnimationsNeedThickness( panel.animationsNeedThickness-1 );
            }

            launchedAlready = false;
        }

        function init(){
            //console.log ("Nooo 1 : "+panel.noTasksInAnimation);
            if(!launchedAlready) {
                launchedAlready = true;
                panel.setAnimationsNeedThickness( panel.animationsNeedThickness+1 );
                panel.noTasksInAnimation++;
                mainItemContainer.setBlockingAnimation(true);
            }

            wrapper.tempScaleWidth = wrapper.scale;
            wrapper.tempScaleHeight = wrapper.scale;

            icList.hoveredIndex = -1;
        }

        function bounceLauncher(){
            if(panel.zoomFactor > 1){
                mainItemContainer.animationStarted();
                init();
                start();
            }
            else
                stopped();
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
    SequentialAnimation{
        id:newWindowAnimation

        property int speed: plasmoid.configuration.durationTime*units.longDuration
        property bool isDemandingAttention: (IsDemandingAttention === true) ? true : false
        property bool entered: mainItemContainer.mouseEntered
        property bool needsThicknessSent: false //flag to check if the signal for thickness was sent

        SequentialAnimation{
            ParallelAnimation{
                PropertyAnimation {
                    target: wrapper
                    property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
                    to: 1 + (0.6 * (panel.zoomFactor-1))
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
                duration: 3*plasmoid.configuration.durationTime*newWindowAnimation.speed
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
                if (panel.animationsNeedThickness > 0) {
                    panel.setAnimationsNeedThickness( panel.animationsNeedThickness-1 );
                }
            }
        }

        function init(){
            wrapper.tempScaleWidth = wrapper.scale;
            wrapper.tempScaleHeight = wrapper.scale;

            if(!isDemandingAttention)
                loops = 2;
            else
                loops = 20;

            if (!needsThicknessSent) {
                needsThicknessSent = true;
                panel.setAnimationsNeedThickness( panel.animationsNeedThickness+1 );
            }

            // icList.hoveredIndex = -1;
        }

        function bounceNewWindow(){
            newWindowAnimation.init();
            start();
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
            var relavantPoint
            if(plasmoid.configuration.showShadows)
                relavantPoint = panel.mapFromItem(shadowedImage,0,0);
            else
                relavantPoint = panel.mapFromItem(iconImageBuffer,0,0);


            var removingItem = removeTaskComponent.createObject(panel);
            removingItem.x = relavantPoint.x;
            removingItem.y = relavantPoint.y;

            removingItem.start();
        }

        function removeTask(){
            if(centralItem.firstDrawed && !centralItem.toBeDestroyed
                    && mainItemContainer.buffersAreReady && plasmoid.configuration.showShadows
                    && windowSystem.compositingActive){
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
                width: plasmoid.configuration.showShadows ? shadowedImage.width : iconImageBuffer.width
                height: plasmoid.configuration.showShadows ? shadowedImage.height : iconImageBuffer.height
                //parent: panel

                visible: false

                Image {
                    id: tempRemoveIcon
                    source: plasmoid.configuration.showShadows ? shadowedImage.source : iconImageBuffer.source
                    anchors.fill: parent
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

                    property int speed: 2*plasmoid.configuration.durationTime*units.longDuration
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

                    if( (panel.position === PlasmaCore.Types.BottomPositioned) ||
                            (panel.position === PlasmaCore.Types.RightPositioned) ){
                        componentRemoveAnimation.toPoint = tempPoint + panel.iconSize;
                    }
                    else{
                        componentRemoveAnimation.toPoint = tempPoint - panel.iconSize;
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
            when: ( (mainItemContainer.isDragged) && (plasmoid.immutable) )

            //    PropertyChanges { target: clickedAnimation; running:false }
            PropertyChanges { target: wrapper; scale:1 + ((panel.zoomFactor - 1) / 2)}
        }
    ]

    //////////// Transitions //////////////

    transitions: [
        Transition{
            id: isDraggedTransition
            to: "isDragged"
            property int speed: plasmoid.configuration.durationTime*units.longDuration

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
                    //panel.animations++;
                    panel.updateScale(index-1, 1, 0);
                    panel.updateScale(index+1, 1, 0);
                }
            }
        },
        Transition{
            id: defaultTransition
            from: "isDragged"
            to: "*"
            property int speed: plasmoid.configuration.durationTime*units.longDuration

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
                    property: "scale"
                    to: 1;
                    duration: isDraggedTransition.speed
                    easing.type: Easing.OutQuad
                }

            }

            onRunningChanged: {
                if(!running){
                    var halfZoom = 1 + ((panel.zoomFactor - 1) / 2);
                    panel.updateScale(index-1, halfZoom, 0);
                    panel.updateScale(index+1, halfZoom, 0);

                    mainItemContainer.animationEnded();
                    //   panel.animations--;
                }
            }
        }
    ]


    //////////////////////////

    function updateImages(){
        if(panel){
            if(defaultWithShadow.item){
                defaultWithShadow.item.updateImage();
            }
        }
    }


}// Icon Item
