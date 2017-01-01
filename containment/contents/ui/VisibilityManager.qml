import QtQuick 2.1
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import org.kde.taskmanager 0.1 as TaskManager

import org.kde.latte 0.1 as Latte

Item{
    id: manager

    anchors.fill: parent

    property QtObject window

    property bool inStartup: root.inStartup
    property bool normalState : false  // this is being set from updateMaskArea

    property int animationSpeed: root.durationTime * 1.2 * units.longDuration
    property int length: root.isVertical ?  Screen.height : Screen.width   //screenGeometry.height : screenGeometry.width

    //it is used in order to not break the calculations for the thickness placement
    //especially in automatic icon sizes calculations
    property int iconMarginOriginal: 0.12*plasmoid.configuration.iconSize
    property int statesLineSizeOriginal: root.nowDock ? Math.ceil( plasmoid.configuration.iconSize/13 ) : 0

    property int thicknessAutoHidden: 2
    property int thicknessMid: root.statesLineSize + (1 + (0.65 * (root.zoomFactor-1)))*(root.iconSize+root.iconMargin) //needed in some animations
    property int thicknessNormal: root.statesLineSize + root.iconSize + root.iconMargin + 1
    property int thicknessZoom: root.statesLineSize + ((root.iconSize+root.iconMargin) * root.zoomFactor) + 2
    //it is used to keep thickness solid e.g. when iconSize changes from auto functions
    property int thicknessMidOriginal: statesLineSizeOriginal + (1 + (0.65 * (root.zoomFactor-1)))*(plasmoid.configuration.iconSize+iconMarginOriginal) //needed in some animations
    property int thicknessNormalOriginal: root.useThemePanel ? Math.max(thicknessNormalOriginalValue, root.realPanelSize) : thicknessNormalOriginalValue
    property int thicknessNormalOriginalValue: statesLineSizeOriginal + plasmoid.configuration.iconSize + iconMarginOriginal + 1
    property int thicknessZoomOriginal: statesLineSizeOriginal + ((plasmoid.configuration.iconSize+iconMarginOriginal) * root.zoomFactor) + 2

    Binding{
        target: dock ? dock : manager
        property:"maxThickness"
        when: dock
        value: thicknessZoomOriginal
    }

    onInStartupChanged: {
        if (!inStartup) {
            delayAnimationTimer.start();
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

    onThicknessZoomOriginalChanged: updateMaskArea();


    function slotContainsMouseChanged() {
        if(dock.visibility.containsMouse) {
            if (delayerTimer.running) {
                delayerTimer.stop();
            }

            updateMaskArea();
        } else {
            // initialize the zoom
            delayerTimer.start();
        }
    }

    function slotMustBeShown() {
      //  console.log("show...");
        slidingAnimationAutoHiddenIn.init();
    }

    function slotMustBeHide() {
       // console.log("hide....");
        slidingAnimationAutoHiddenOut.init();
    }

    function slotModeChanged() {
        if (dock.visibility.mode !== Latte.Dock.AutoHide) {
            dock.visibility.isHidden = false;
        }
    }

    ///test maskArea
    function updateMaskArea() {
        if (!windowSystem.compositingActive || !dock) {
            return;
        }

        var localX = 0;
        var localY = 0;

        normalState = (root.nowDockHoveredIndex === -1) && (layoutsContainer.hoveredIndex === -1)
                && (root.appletsAnimations === 0)
                && (root.animationsNeedBothAxis === 0) && (root.animationsNeedLength === 0)
                && (!mainLayout.animatedLength)

        // debug maskArea criteria
        console.log(root.nowDockHoveredIndex + ", " + layoutsContainer.hoveredIndex + ", "
                 + root.appletsAnimations+ ", "
                 + root.animationsNeedBothAxis + ", " + root.animationsNeedLength + ", " + root.animationsNeedThickness +", "
                 + mainLayout.animatedLength);

        var tempLength = root.isHorizontal ? width : height;
        var tempThickness = root.isHorizontal ? height : width;

        var space = root.panelEdgeSpacing + 10;

        if (normalState && !root.editMode) {
            //console.log("entered normal state...");
            //count panel length
            if(root.isHorizontal) {
                tempLength = plasmoid.configuration.panelPosition === Latte.Dock.Double ? layoutsContainer.width + 0.5*space : mainLayout.width + space;
            } else {
                tempLength = plasmoid.configuration.panelPosition === Latte.Dock.Double ? layoutsContainer.height + 0.5*space : mainLayout.height + space;
            }

            tempThickness = thicknessNormalOriginal;

            if (root.animationsNeedThickness > 0) {
                tempThickness = thicknessMidOriginal;
            }

            if (dock.visibility.isHidden) {
                tempThickness = thicknessAutoHidden;
            }

            //configure x,y based on plasmoid position and root.panelAlignment(Alignment)
            if ((plasmoid.location === PlasmaCore.Types.BottomEdge) || (plasmoid.location === PlasmaCore.Types.TopEdge)) {
                if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    localY = dock.height - tempThickness;
                } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                    localY = 0;
                }

                if (plasmoid.configuration.panelPosition === Latte.Dock.Double) {
                    localX = (dock.width/2) - (layoutsContainer.width/2) - 0.25*space;
                } else if (root.panelAlignment === Latte.Dock.Left) {
                    localX = 0;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    localX = (dock.width/2) - (mainLayout.width/2) - (space/2);
                } else if (root.panelAlignment === Latte.Dock.Right) {
                    localX = dock.width - mainLayout.width - (space/2);
                }
            } else if ((plasmoid.location === PlasmaCore.Types.LeftEdge) || (plasmoid.location === PlasmaCore.Types.RightEdge)){
                if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    localX = 0;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    localX = dock.width - tempThickness;
                }

                if (plasmoid.configuration.panelPosition === Latte.Dock.Double) {
                    localY = (dock.height/2) - (layoutsContainer.height/2) - 0.25*space;
                } else if (root.panelAlignment === Latte.Dock.Top) {
                    localY = 0;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    localY = (dock.height/2) - (mainLayout.height/2) - (space/2);
                } else if (root.panelAlignment === Latte.Dock.Bottom) {
                    localY = dock.height - mainLayout.height - (space/2);
                }
            }
        } else {
            if(root.isHorizontal)
                tempLength = Screen.width; //screenGeometry.width;
            else
                tempLength = Screen.height; //screenGeometry.height;

            //grow only on length and not thickness
            if(mainLayout.animatedLength || root.editMode
                    || (root.animationsNeedLength>0 && root.animationsNeedBothAxis === 0)) {

                //this is used to fix a bug with shadow showing when the animation of edit mode
                //is triggered
                var editModeThickness = editModeVisual.editAnimationEnded ? thicknessNormalOriginal + editModeVisual.shadowSize :
                                                                            thicknessNormalOriginal

                tempThickness = root.editMode ? editModeThickness : thicknessNormalOriginal;

                if (root.animationsNeedThickness > 0) {
                    tempThickness = thicknessMidOriginal;
                }

                //configure the x,y position based on thickness
                if(plasmoid.location === PlasmaCore.Types.RightEdge)
                    localX = dock.width - tempThickness;
                else if(plasmoid.location === PlasmaCore.Types.BottomEdge)
                    localY = dock.height - tempThickness;
            } else{
                //use all thickness space
                tempThickness = thicknessZoomOriginal;
            }
        }
        var maskArea = dock.maskArea;

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

            dock.maskArea = newMaskArea;

            if(normalState && !dock.visibility.isHidden){
                dock.setLocalDockGeometry(newMaskArea);
            }
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
                x: dock ? dock.maskArea.x : -1
                y: dock ? dock.maskArea.y : -1
                height: dock ? dock.maskArea.height : 0
                width: dock ? dock.maskArea.width : 0

                border.color: "green"
                border.width: 1
                color: "transparent"
            }
        }
    }

    /***Hiding/Showing Animations*****/

    //////////////// Animations - Slide In - Out
    SequentialAnimation{
        id: slidingAnimationAutoHiddenOut

        PropertyAnimation {
            target: layoutsContainer
            property: root.isVertical ? "x" : "y"
            to: ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) ? -thicknessNormal : thicknessNormal
            duration: manager.animationSpeed
            easing.type: Easing.OutQuad
        }

        onStopped: {
            dock.visibility.isHidden = true;
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
            duration: manager.animationSpeed
            easing.type: Easing.OutQuad
        }

        onStopped: {
        }

        function init() {
            dock.visibility.isHidden = false;
            updateMaskArea();
            start();
        }
    }

    ///////////// External Connections //////
    TaskManager.ActivityInfo {
        onCurrentActivityChanged: {
            dock.visibility.disableHiding = true;

            if (dock.visibility.isHidden) {
                dock.visibility.mustBeShown();
            }
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
        interval: manager.inStartup ? 1000 : 500
        onTriggered: {
            layoutsContainer.opacity = 1;
            if (dock.visibility.mode !== Latte.Dock.AutoHide) {
                slidingAnimationAutoHiddenIn.init();
            }
        }
    }

}
