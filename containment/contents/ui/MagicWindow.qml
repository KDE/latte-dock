import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.taskmanager 0.1 as TaskManager

import org.kde.nowdock 0.1 as NowDock

NowDock.PanelWindow{
    id: window

    property bool inStartup: root.inStartup
    property bool normalState : false  // this is being set from updateMaskArea

    property int animationSpeed: root.durationTime * 1.2 * units.longDuration
    property int length: root.isVertical ? screenGeometry.height : screenGeometry.width

    //it is used in order to not break the calculations for the thickness placement
    //especially in automatic icon sizes calculations
    property int iconMarginOriginal: 0.12*plasmoid.configuration.iconSize
    property int statesLineSizeOriginal: root.nowDock ? Math.ceil( plasmoid.configuration.iconSize/13 ) : 0

    property int thicknessAutoHidden: 8
    property int thicknessMid: root.statesLineSize + (1 + (0.65 * (root.zoomFactor-1)))*(root.iconSize+root.iconMargin) //needed in some animations
    property int thicknessNormal: root.statesLineSize + root.iconSize + root.iconMargin + 1
    property int thicknessZoom: root.statesLineSize + ((root.iconSize+root.iconMargin) * root.zoomFactor) + 2
    //it is used to keep thickness solid e.g. when iconSize changes from auto functions
    property int thicknessMidOriginal: statesLineSizeOriginal + (1 + (0.65 * (root.zoomFactor-1)))*(plasmoid.configuration.iconSize+iconMarginOriginal) //needed in some animations
    property int thicknessNormalOriginal: root.useThemePanel ? Math.max(thicknessNormalOriginalValue, root.realPanelSize) : thicknessNormalOriginalValue
    property int thicknessNormalOriginalValue: statesLineSizeOriginal + plasmoid.configuration.iconSize + iconMarginOriginal + 1
    property int thicknessZoomOriginal: statesLineSizeOriginal + ((plasmoid.configuration.iconSize+iconMarginOriginal) * root.zoomFactor) + 2


    childrenLength: root.isHorizontal ? mainLayout.width : mainLayout.height
    immutable: plasmoid.immutable
    location: plasmoid.location
    panelVisibility: plasmoid.configuration.panelVisibility

    //this is used especially in the wayland case or when the user might want
    //not to make the dock to go below windows but to hide, e.g. intelligent auto-hide
    //isDockWindowType: true


    width: root.isHorizontal ? length : thicknessZoomOriginal
    height: root.isHorizontal ? thicknessZoomOriginal : length

    // it is used in order to restore applets after right click menu
    onDisableHidingChanged: {
        if (!disableHiding) {
            checkListHovered.restart();
        }
    }

    onImmutableChanged: updateMaskArea();

    onInStartupChanged: {
        if (!inStartup) {
            delayAnimationTimer.start();
        }
    }

    onIsHoveredChanged: {
        if(isHovered) {
            //stop parent window timer for auto hiding
            if ((panelVisibility === NowDock.PanelWindow.AutoHide)|| isDockWindowType) {
                if(hideMagicWindowInAutoHide.forcedDisableHiding) {
                    hideMagicWindowInAutoHide.forcedDisableHiding = false;
                    window.disableHiding = false;
                }

                hideMagicWindowInAutoHide.stop();
            }

            if (delayerTimer.running) {
                delayerTimer.stop();
            }

            updateMaskArea();
        } else {
            // initialize the zoom
            delayerTimer.start();
        }
    }

    onMustBeRaised: {
        if ((panelVisibility === NowDock.PanelWindow.AutoHide) || isDockWindowType) {
            slidingAnimationAutoHiddenIn.init();
        } else {
            slidingAnimation.init(true,false);
        }
    }

    onMustBeRaisedImmediately: {
        slidingAnimation.init(true,true);
    }

    onMustBeLowered: {
        if ((panelVisibility === NowDock.PanelWindow.AutoHide) || isDockWindowType ) {
            slidingAnimationAutoHiddenOut.init();
        } else {
            slidingAnimation.init(false,false);
        }
    }

    onNormalStateChanged: {
        if(normalState && nowDock) {
            nowDock.publishTasksGeometries();
        }

        if (normalState) {
            root.updateAutomaticIconSize();
        }
    }

    onPanelVisibilityChanged: {
        if (panelVisibility !== NowDock.PanelWindow.AutoHide) {
            isAutoHidden = false;
        }
    }
    
    onWidthChanged: {
        if (plasmoid.immutable) {
            if (windowSystem.compositingActive) {
                magicWin.initialize();
            } else {
                magicWin.updateTransientThickness();
            }
            updateMaskArea();
        }
    }
    
    onHeightChanged: {
        if (plasmoid.immutable) {
            if (windowSystem.compositingActive) {
                magicWin.initialize();
            } else {
                magicWin.updateTransientThickness();
            }
            updateMaskArea();
        }
    }

    onVisibleChanged:{
        if (visible) {  //shrink the parent panel window
            initialize();
        }
    }

    function initializeSlidingInAnimation() {
        // Hide in Startup in order to show the contents with beautiful sliding animation
        var hiddenSpace;

        if ((location===PlasmaCore.Types.LeftEdge)||(location===PlasmaCore.Types.TopEdge)) {
            hiddenSpace = -thicknessNormal;
        } else {
            hiddenSpace = thicknessNormal;
        }

        if (root.isVertical) {
            layoutsContainer.x = hiddenSpace;
        } else {
            layoutsContainer.y = hiddenSpace;
        }

        layoutsContainer.opacity = 1;

        if (!inStartup) {initialize();
            delayAnimationTimer.start();
        }
    }

    function updateMaskArea() {
        if (!windowSystem.compositingActive) {
            return;
        }

        var localX = 0;
        var localY = 0;

        normalState = (root.nowDockHoveredIndex === -1) && (layoutsContainer.hoveredIndex === -1)
                && (root.appletsAnimations === 0)
                && (root.animationsNeedBothAxis === 0) && (root.animationsNeedLength === 0)
                && (!mainLayout.animatedLength)

        // debug maskArea criteria
        //console.log(root.nowDockHoveredIndex + ", " + layoutsContainer.hoveredIndex + ", "
        //         + root.appletsAnimations+ ", "
        //         + root.animationsNeedBothAxis + ", " + root.animationsNeedLength + ", " + root.animationsNeedThickness +", "
        //         + mainLayout.animatedLength);

        var tempLength = root.isHorizontal ? width : height;
        var tempThickness = root.isHorizontal ? height : width;

        var space = root.panelEdgeSpacing + 10;

        if (normalState) {
            //count panel length
            if(root.isHorizontal) {
                tempLength = plasmoid.configuration.panelPosition === NowDock.PanelWindow.Double ? layoutsContainer.width + 0.5*space : mainLayout.width + space;
            } else {
                tempLength = plasmoid.configuration.panelPosition === NowDock.PanelWindow.Double ? layoutsContainer.height + 0.5*space : mainLayout.height + space;
            }

            tempThickness = thicknessNormalOriginal;

            if (root.animationsNeedThickness > 0) {
                tempThickness = thicknessMidOriginal;
            }

            if (window.isAutoHidden && ((panelVisibility === NowDock.PanelWindow.AutoHide) || window.isDockWindowType)) {
                tempThickness = thicknessAutoHidden;
            }

            if (!immutable) {
                tempThickness = 2;
            }

            //configure x,y based on plasmoid position and root.panelAlignment(Alignment)
            if ((plasmoid.location === PlasmaCore.Types.BottomEdge) || (plasmoid.location === PlasmaCore.Types.TopEdge)) {
                if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    localY = window.height - tempThickness;
                } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                    localY = 0;
                }

                if (plasmoid.configuration.panelPosition === NowDock.PanelWindow.Double) {
                    localX = (window.width/2) - (layoutsContainer.width/2) - 0.25*space;
                } else if (root.panelAlignment === NowDock.PanelWindow.Left) {
                    localX = 0;
                } else if (root.panelAlignment === NowDock.PanelWindow.Center) {
                    localX = (window.width/2) - (mainLayout.width/2) - (space/2);
                } else if (root.panelAlignment === NowDock.PanelWindow.Right) {
                    localX = window.width - mainLayout.width - (space/2);
                }
            } else if ((plasmoid.location === PlasmaCore.Types.LeftEdge) || (plasmoid.location === PlasmaCore.Types.RightEdge)){
                if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    localX = 0;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    localX = window.width - tempThickness;
                }

                if (plasmoid.configuration.panelPosition === NowDock.PanelWindow.Double) {
                    localY = (window.height/2) - (layoutsContainer.height/2) - 0.25*space;
                } else if (root.panelAlignment === NowDock.PanelWindow.Top) {
                    localY = 0;
                } else if (root.panelAlignment === NowDock.PanelWindow.Center) {
                    localY = (window.height/2) - (mainLayout.height/2) - (space/2);
                } else if (root.panelAlignment === NowDock.PanelWindow.Bottom) {
                    localY = window.height - mainLayout.height - (space/2);
                }
            }
        } else {
            if(root.isHorizontal)
                tempLength = screenGeometry.width;
            else
                tempLength = screenGeometry.height;

            //grow only on length and not thickness
            if(mainLayout.animatedLength) {
                tempThickness = thicknessNormalOriginal;

                if (root.animationsNeedThickness > 0) {
                    tempThickness = thicknessMidOriginal;
                }

                //configure the x,y position based on thickness
                if(plasmoid.location === PlasmaCore.Types.RightEdge)
                    localX = window.width - tempThickness;
                else if(plasmoid.location === PlasmaCore.Types.BottomEdge)
                    localY = window.height - tempThickness;
            } else{
                //use all thickness space
                tempThickness = thicknessZoomOriginal;
            }
        }

        var maskLength = maskArea.width; //in Horizontal
        if (root.isVertical) {
            maskLength = maskArea.height;
        }

        var maskThickness = maskArea.height; //in Horizontal
        if (root.isVertical) {
            maskThickness = maskArea.width;
        }

        // console.log("Not updating mask...");
        if( maskArea.x !== localX || maskArea.y !== localY
                || maskLength !== tempLength || maskThickness !== tempThickness) {

            // console.log("Updating mask...");
            var newMaskArea = Qt.rect(-1,-1,0,0);
            newMaskArea.x = localX;
            newMaskArea.y = localY;

            if (isHorizontal) {
                newMaskArea.width = tempLength;
                newMaskArea.height = tempThickness;
            } else {
                newMaskArea.width = tempThickness;
                newMaskArea.height = tempLength;
            }

            maskArea = newMaskArea;
        }

    }

    function updateTransientThickness() {
        var thickness;

        if (root.isVertical) {
            thickness = root.width;
        } else {
            thickness = root.height;
        }

        var newThickness = statesLineSizeOriginal + plasmoid.configuration.iconSize + iconMarginOriginal;

        if (!windowSystem.compositingActive) {
            newThickness += iconMarginOriginal;
        }

        if (thickness<newThickness) {
            setTransientThickness(newThickness);
        }
    }

    Loader{
        anchors.fill: parent
        active: root.debugMode

        sourceComponent: Item{
            anchors.fill:parent

            Rectangle{
                id: windowBackground
                anchors.fill: parent
                border.color: "red"
                border.width: 1
                color: "transparent"
            }

            Rectangle{
                x: maskArea.x
                y: maskArea.y
                height: maskArea.height
                width: maskArea.width

                border.color: "green"
                border.width: 1
                color: "transparent"
            }
        }
    }

    /***Hiding/Showing Animations*****/

    SequentialAnimation{
        id: slidingAnimation

        property bool inHalf: false
        property bool raiseFlag: false
        property bool immediateShow: false

        SequentialAnimation{
            PropertyAnimation {
                target: layoutsContainer
                property: root.isVertical ? "x" : "y"
                to: ((location===PlasmaCore.Types.LeftEdge)||(location===PlasmaCore.Types.TopEdge)) ? -thicknessNormal : thicknessNormal
                duration: slidingAnimation.immediateShow ? 100 : window.animationSpeed
                easing.type: Easing.OutQuad
            }

            PropertyAnimation {
                target: slidingAnimation
                property: "inHalf"
                to: true
                duration: 100
            }

            PropertyAnimation {
                target: layoutsContainer
                property: root.isVertical ? "x" : "y"
                to: 0
                duration: window.animationSpeed
                easing.type: Easing.OutQuad
            }
        }

        onStopped: {
            inHalf = false;
            raiseFlag = false;
            immediateShow = false;

            if (!plasmoid.immutable) {
                updateTransientThickness();
            }
        }

        onInHalfChanged: {
            if (inHalf) {
                if (raiseFlag) {
                    window.showOnTop();
                } else {
                    window.showOnBottom();
                }
            }
        }

        function init(raise, immediate) {
            if(window.visible) {
                raiseFlag = raise;
                immediateShow = immediate;
                // if (!running) {
                start();
                // }
            }
        }
    }
    //////////////// Auto Hide Animations - Slide In - Out
    SequentialAnimation{
        id: slidingAnimationAutoHiddenOut

        PropertyAnimation {
            target: layoutsContainer
            property: root.isVertical ? "x" : "y"
            to: ((location===PlasmaCore.Types.LeftEdge)||(location===PlasmaCore.Types.TopEdge)) ? -thicknessNormal : thicknessNormal
            duration: window.animationSpeed
            easing.type: Easing.OutQuad
        }

        onStopped: {
            window.isAutoHidden = true;
            updateMaskArea();
        }

        function init() {
            start();
        }
    }

    SequentialAnimation{
        id: slidingAnimationAutoHiddenIn

        PropertyAnimation {
            target: layoutsContainer
            property: root.isVertical ? "x" : "y"
            to: 0
            duration: window.animationSpeed
            easing.type: Easing.OutQuad
        }

        onStopped: {
            if (!plasmoid.immutable) {
                updateTransientThickness();
            }
        }

        function init() {
            window.isAutoHidden = false;
            updateMaskArea();
            start();
        }
    }

    ///////////// External Connections //////
    TaskManager.ActivityInfo {
        onCurrentActivityChanged: {
            window.disableHiding = true;

            if (window.isAutoHidden) {
                window.mustBeRaised();
            }

            hideMagicWindowInAutoHide.forcedDisableHiding = true;
            hideMagicWindowInAutoHide.start();
        }
    }

    ////////////// Timers //////
    //Timer to delay onLeave event
    Timer {
        id: delayerTimer
        interval: 400
        onTriggered: {
            root.clearZoom();
            if (root.nowDock) {
                nowDock.clearZoom();
            }
        }
    }

    //Timer to delay onLeave event
    Timer {
        id: delayAnimationTimer
        interval: window.inStartup ? 1000 : 500
        onTriggered: {
            layoutsContainer.opacity = 1;
            if ((window.panelVisibility !== NowDock.PanelWindow.AutoHide) && !window.isDockWindowType) {
                slidingAnimation.init(true,false);
            } else {
                slidingAnimationAutoHiddenIn.init();
            }
        }
    }
}
