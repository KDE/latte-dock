/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.containment 0.1 as LatteContainment

Item{
    id: manager
    anchors.fill: parent

    property QtObject window

    property bool isFloatingInClientSide: !root.behaveAsPlasmaPanel
                                          && screenEdgeMarginEnabled
                                          && !root.floatingInternalGapIsForced
                                          && !inSlidingIn
                                          && !inSlidingOut

    property int animationSpeed: LatteCore.WindowSystem.compositingActive ?
                                     (root.editMode ? 400 : animations.speedFactor.current * 1.62 * animations.duration.large) : 0

    property bool inClientSideScreenEdgeSliding: root.behaveAsDockWithMask && hideThickScreenGap
    property bool inNormalState: ((animations.needBothAxis.count === 0) && (animations.needLength.count === 0))
                                 || (latteView && latteView.visibility.isHidden && !latteView.visibility.containsMouse && animations.needThickness.count === 0)
    property bool inRelocationAnimation: latteView && latteView.positioner && latteView.positioner.inRelocationAnimation

    property bool inSlidingIn: false //necessary because of its init structure
    property alias inSlidingOut: slidingAnimationAutoHiddenOut.running
    property bool inRelocationHiding: false

    readonly property bool isSinkedEventEnabled: !(parabolic.isEnabled && (animations.needBothAxis.count>0 || animations.needLength.count>0))
                                                 && myView.isShownFully

    property int length: root.isVertical ?  Screen.height : Screen.width   //screenGeometry.height : screenGeometry.width

    property int slidingOutToPos: {
        if (root.behaveAsPlasmaPanel) {
            var edgeMargin = screenEdgeMarginEnabled ? plasmoid.configuration.screenEdgeMargin : 0
            var thickmarg = latteView.visibility.isSidebar ? 0 : 1;

            return root.isHorizontal ? root.height + edgeMargin - thickmarg : root.width + edgeMargin - thickmarg;
        } else {
            var topOrLeftEdge = ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge));
            return (topOrLeftEdge ? -metrics.mask.thickness.normal : metrics.mask.thickness.normal);
        }
    }

    //! when Latte behaves as Plasma panel
    property int thicknessAsPanel: metrics.totals.thickness

    property Item layouts: null

    property bool updateIsEnabled: autosize.inCalculatedIconSize && !inSlidingIn && !inSlidingOut && !inRelocationHiding

    Connections{
        target: background.totals
        onVisualLengthChanged: updateMaskArea();
        onVisualThicknessChanged: updateMaskArea();
    }

    Connections{
        target: background.shadows
        onHeadThicknessChanged: updateMaskArea();
    }

    Connections{
        target: latteView ? latteView : null
        onXChanged: updateMaskArea();
        onYChanged: updateMaskArea()
        onWidthChanged: updateMaskArea();
        onHeightChanged: updateMaskArea();
    }

    Connections{
        target: animations.needBothAxis
        onCountChanged: updateMaskArea();
    }

    Connections{
        target: animations.needLength
        onCountChanged: updateMaskArea();
    }

    Connections{
        target: animations.needThickness
        onCountChanged: updateMaskArea();
    }

    Connections{
        target: layoutsManager
        onCurrentLayoutIsSwitching: {
            if (LatteCore.WindowSystem.compositingActive && latteView && latteView.layout && latteView.layout.name === layoutName) {
                parabolic.sglClearZoom();
            }
        }
    }

    Connections {
        target: metrics.mask.thickness
        onMaxZoomedChanged: updateMaskArea()
    }

    Connections {
        target: root.myView
        onInRelocationAnimationChanged: {
            if (!root.myView.inRelocationAnimation) {
                manager.updateMaskArea();
            }
        }
    }

    Connections {
        target: latteView ? latteView.effects : null
        onRectChanged: manager.updateMaskArea()
    }

    Connections{
        target: themeExtended ? themeExtended : null
        onThemeChanged: latteView.effects.forceMaskRedraw();
    }

    Connections {
        target: LatteCore.WindowSystem
        onCompositingActiveChanged: {
            manager.updateMaskArea();
        }
    }

    onIsFloatingInClientSideChanged: updateMaskArea();

    onInNormalStateChanged: {
        if (inNormalState) {
            updateMaskArea();
        }
    }

    onInSlidingInChanged: {
        if (latteView && !inSlidingIn && latteView.positioner.inRelocationShowing) {
            latteView.positioner.inRelocationShowing = false;
        }
    }

    onUpdateIsEnabledChanged: {
        if (updateIsEnabled) {
            updateMaskArea();
        }
    }

    function slotContainsMouseChanged() {
        if(latteView.visibility.containsMouse && latteView.visibility.mode !== LatteCore.Types.SidebarOnDemand) {
            updateMaskArea();

            if (slidingAnimationAutoHiddenOut.running && !inRelocationHiding) {
                slotMustBeShown();
            }
        }
    }

    function slotMustBeShown() {
        if (root.inStartup) {
            slidingAnimationAutoHiddenIn.init();
            return;
        }

        //! WindowsCanCover case
        if (latteView && latteView.visibility.mode === LatteCore.Types.WindowsCanCover) {
            latteView.visibility.setViewOnFrontLayer();
        }

        if (!latteView.visibility.isHidden && latteView.positioner.inSlideAnimation) {
            // Do not update when Positioner mid-slide animation takes place, for example:
            // 1. Latte panel is hiding its floating gap for maximized window
            // 2. the user clicks on an applet popup.
            // 3. Applet popups showing/hiding are triggering hidingIsBlockedChanged() signals.
            // 4. hidingIsBlockedChanged() signals create mustBeShown events when visibility::hidingIsBlocked() is not enabled.
            return;
        }

        //! Normal Dodge/AutoHide case
        if (!slidingAnimationAutoHiddenIn.running
                && !inRelocationHiding
                && (latteView.visibility.isHidden || slidingAnimationAutoHiddenOut.running /*it is not already shown or is trying to hide*/)){
            slidingAnimationAutoHiddenIn.init();
        }
    }

    function slotMustBeHide() {
        if (root.inStartup) {
            slidingAnimationAutoHiddenOut.init();
            return;
        }

        if (inSlidingIn && !inRelocationHiding) {
            /*consider hiding after sliding in has finished*/
            return;
        }

        if (latteView && latteView.visibility.mode === LatteCore.Types.WindowsCanCover) {
            latteView.visibility.setViewOnBackLayer();
            return;
        }

        //! Normal Dodge/AutoHide case
        if (!slidingAnimationAutoHiddenOut.running
                && !latteView.visibility.blockHiding
                && (!latteView.visibility.containsMouse || latteView.visibility.mode === LatteCore.Types.SidebarOnDemand /*for SidebarOnDemand mouse should be ignored on hiding*/)
                && (!latteView.visibility.isHidden || slidingAnimationAutoHiddenIn.running /*it is not already hidden or is trying to show*/)) {
            slidingAnimationAutoHiddenOut.init();
        }
    }

    //! functions used for sliding out/in during location/screen changes
    function slotHideDockDuringLocationChange() {
        inRelocationHiding = true;

        if(!slidingAnimationAutoHiddenOut.running) {
            slidingAnimationAutoHiddenOut.init();
        }
    }

    function slotShowDockAfterLocationChange() {
        slidingAnimationAutoHiddenIn.init();
    }

    function sendHideDockDuringLocationChangeFinished(){
        latteView.positioner.hidingForRelocationFinished();
    }

    function sendSlidingOutAnimationEnded() {
        latteView.visibility.hide();
        latteView.visibility.isHidden = true;

        if (debug.maskEnabled) {
            console.log("hiding animation ended...");
        }

        sendHideDockDuringLocationChangeFinished();
    }

    ///test maskArea
    function updateMaskArea() {
        if (!latteView || !root.viewIsAvailable) {
            return;
        }

        var localX = 0;
        var localY = 0;

        // debug maskArea criteria
        if (debug.maskEnabled) {
            console.log(animations.needBothAxis.count + ", " + animations.needLength.count + ", " +
                        animations.needThickness.count + ", " + latteView.visibility.isHidden);
        }

        //console.log("reached updating geometry ::: "+dock.maskArea);


        if (!latteView.visibility.isHidden && updateIsEnabled && inNormalState) {
            //! Important: Local Geometry must not be updated when view ISHIDDEN
            //! because it breaks Dodge(s) modes in such case

            var localGeometry = Qt.rect(0, 0, root.width, root.height);

            //the shadows size must be removed from the maskArea
            //before updating the localDockGeometry
            if (!latteView.behaveAsPlasmaPanel) {
                var cleanThickness = metrics.totals.thickness;
                var edgeMargin = metrics.mask.screenEdge;

                if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                    localGeometry.x = latteView.effects.rect.x; // from effects area
                    localGeometry.width = latteView.effects.rect.width; // from effects area

                    localGeometry.y = edgeMargin;
                    localGeometry.height = cleanThickness;
                } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                    localGeometry.x = latteView.effects.rect.x; // from effects area
                    localGeometry.width = latteView.effects.rect.width; // from effects area

                    localGeometry.y = root.height - cleanThickness - edgeMargin;
                    localGeometry.height = cleanThickness;
                } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    localGeometry.y = latteView.effects.rect.y; // from effects area
                    localGeometry.height = latteView.effects.rect.height; // from effects area

                    localGeometry.x = edgeMargin;
                    localGeometry.width = cleanThickness;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                    localGeometry.y = latteView.effects.rect.y; // from effects area
                    localGeometry.height = latteView.effects.rect.height; // from effects area

                    localGeometry.x = root.width - cleanThickness - edgeMargin;
                    localGeometry.width = cleanThickness;
                }

                //set the boundaries for latteView local geometry
                //qBound = qMax(min, qMin(value, max)).

                localGeometry.x = Math.max(0, Math.min(localGeometry.x, latteView.width));
                localGeometry.y = Math.max(0, Math.min(localGeometry.y, latteView.height));
                localGeometry.width = Math.min(localGeometry.width, latteView.width);
                localGeometry.height = Math.min(localGeometry.height, latteView.height);
            }

            //console.log("update geometry ::: "+localGeometry);
            latteView.localGeometry = localGeometry;
        }

        //! Input Mask
        if (updateIsEnabled) {
            updateInputGeometry();
        }
    }

    function updateInputGeometry() {
        // VisibilityManager.qml tries to workaround faulty onEntered() signals from ParabolicMouseArea
        // by specifying inputThickness when ParabolicEffect is applied. (inputThickness->animated scenario)
        var animated = (animations.needBothAxis.count>0);

        if (!LatteCore.WindowSystem.compositingActive || latteView.behaveAsPlasmaPanel) {
            //! clear input mask
            latteView.effects.inputMask = Qt.rect(0, 0, -1, -1);
        } else {
            var floatingInternalGapAcceptsInput = behaveAsDockWithMask && floatingInternalGapIsForced;
            var inputThickness;

            if (latteView.visibility.isHidden) {
                inputThickness = metrics.mask.thickness.hidden;
            } else if (root.hasFloatingGapInputEventsDisabled) {
                inputThickness = animated ? metrics.mask.thickness.zoomedForItems - metrics.margins.screenEdge : metrics.totals.thickness;
            } else {
                inputThickness = animated ? metrics.mask.thickness.zoomedForItems : metrics.mask.screenEdge + metrics.totals.thickness;
            }

            var subtractedScreenEdge = root.hasFloatingGapInputEventsDisabled && !latteView.visibility.isHidden ? metrics.mask.screenEdge : 0;

            var inputGeometry = Qt.rect(0, 0, root.width, root.height);

            //!use view.localGeometry for length properties
            if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                if (!animated) {
                    inputGeometry.x = latteView.localGeometry.x;
                    inputGeometry.width = latteView.localGeometry.width;
                }

                inputGeometry.y = subtractedScreenEdge;
                inputGeometry.height = inputThickness;
            } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                if (!animated) {
                    inputGeometry.x = latteView.localGeometry.x;
                    inputGeometry.width = latteView.localGeometry.width;
                }

                inputGeometry.y = root.height - inputThickness - subtractedScreenEdge;
                inputGeometry.height = inputThickness;
            } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                if (!animated) {
                    inputGeometry.y = latteView.localGeometry.y;
                    inputGeometry.height = latteView.localGeometry.height;
                }

                inputGeometry.x = subtractedScreenEdge;
                inputGeometry.width = inputThickness;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                if (!animated) {
                    inputGeometry.y = latteView.localGeometry.y;
                    inputGeometry.height = latteView.localGeometry.height;
                }

                inputGeometry.x = root.width - inputThickness - subtractedScreenEdge;
                inputGeometry.width = inputThickness;
            }

            //set the boundaries for latteView local geometry
            //qBound = qMax(min, qMin(value, max)).

            inputGeometry.x = Math.max(0, Math.min(inputGeometry.x, latteView.width));
            inputGeometry.y = Math.max(0, Math.min(inputGeometry.y, latteView.height));
            inputGeometry.width = Math.min(inputGeometry.width, latteView.width);
            inputGeometry.height = Math.min(inputGeometry.height, latteView.height);

            if (latteView.visibility.isSidebar && latteView.visibility.isHidden) {
                //! this way we make sure than no input is accepted anywhere
                inputGeometry = Qt.rect(-1, -1, 1, 1);
            }

            latteView.effects.inputMask = inputGeometry;
        }
    }

    Loader{
        anchors.fill: parent
        active: debug.graphicsEnabled

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
                x: latteView ? latteView.effects.mask.x : -1
                y: latteView ? latteView.effects.mask.y : -1
                height: latteView ? latteView.effects.mask.height : 0
                width: latteView ? latteView.effects.mask.width : 0

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
            target: !root.behaveAsPlasmaPanel ? layoutsContainer : latteView.positioner
            property: !root.behaveAsPlasmaPanel ? (root.isVertical ? "x" : "y") : "slideOffset"
            to: {
                if (root.behaveAsPlasmaPanel) {
                    return slidingOutToPos;
                }

                if (LatteCore.WindowSystem.compositingActive) {
                    return slidingOutToPos;
                } else {
                    if ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) {
                        return slidingOutToPos + 1;
                    } else {
                        return slidingOutToPos - 1;
                    }
                }
            }
            duration: manager.animationSpeed
            easing.type: Easing.InQuad
        }

        ScriptAction{
            script: {
                latteView.visibility.isHidden = true;

                if (root.behaveAsPlasmaPanel && latteView.positioner.slideOffset !== 0) {
                    //! hide real panels when they slide-out
                    latteView.visibility.hide();
                }
            }
        }

        onStarted: {
            if (debug.maskEnabled) {
                console.log("hiding animation started...");
            }
        }

        onStopped: {
            //! Trying to move the ending part of the signals at the end of editing animation
            if (!manager.inRelocationHiding) {
                manager.updateMaskArea();
            } else {
                if (!root.editMode) {
                    manager.sendSlidingOutAnimationEnded();
                }
            }

            latteView.visibility.slideOutFinished();
            manager.updateInputGeometry();

            if (root.inStartup) {
                //! when view is first created slide-outs when that animation ends then
                //! it flags that startup has ended and first slide-in can be started
                //! this is important because if it is removed then some views
                //! wont slide-in after startup.
                root.inStartup = false;
            }
        }

        function init() {
            if (manager.inRelocationAnimation || root.inStartup/*used from recreating views*/ || !latteView.visibility.blockHiding) {
                start();
            }
        }
    }

    SequentialAnimation{
        id: slidingAnimationAutoHiddenIn

        PauseAnimation{
            duration: manager.inRelocationHiding && animations.active ? 500 : 0
        }

        PropertyAnimation {
            target: !root.behaveAsPlasmaPanel ? layoutsContainer : latteView.positioner
            property: !root.behaveAsPlasmaPanel ? (root.isVertical ? "x" : "y") : "slideOffset"
            to: 0
            duration: manager.animationSpeed
            easing.type: Easing.OutQuad
        }

        ScriptAction{
            script: {
                // deprecated
                // root.inStartup = false;
            }
        }

        onStarted: {
            latteView.visibility.show();
            manager.updateInputGeometry();

            if (debug.maskEnabled) {
                console.log("showing animation started...");
            }
        }

        onStopped: {
            inSlidingIn = false;

            if (manager.inRelocationHiding) {
                manager.inRelocationHiding = false;
                autosize.updateIconSize();
            }

            manager.inRelocationHiding = false;
            autosize.updateIconSize();

            if (debug.maskEnabled) {
                console.log("showing animation ended...");
            }

            latteView.visibility.slideInFinished();

            //! this is needed in order to update dock absolute geometry correctly in the end AND
            //! when a floating dock is sliding-in through masking techniques
            updateMaskArea();
        }

        function init() {
            if (!root.viewIsAvailable) {
                return;
            }

            inSlidingIn = true;

            if (slidingAnimationAutoHiddenOut.running) {
                slidingAnimationAutoHiddenOut.stop();
            }

            latteView.visibility.isHidden = false;
            updateMaskArea();

            start();
        }
    }

    //! Slides Animations for FLOATING+BEHAVEASPLASMAPANEL when
    //! HIDETHICKSCREENCAP dynamically is enabled/disabled
    SequentialAnimation{
        id: slidingInRealFloating

        PropertyAnimation {
            target: latteView ? latteView.positioner : null
            property: "slideOffset"
            to: 0
            duration: manager.animationSpeed
            easing.type: Easing.OutQuad
        }

        ScriptAction{
            script: {
                latteView.positioner.inSlideAnimation = false;
            }
        }

        onStopped: latteView.positioner.inSlideAnimation = false;

    }

    SequentialAnimation{
        id: slidingOutRealFloating

        ScriptAction{
            script: {
                latteView.positioner.inSlideAnimation = true;
            }
        }

        PropertyAnimation {
            target: latteView ? latteView.positioner : null
            property: "slideOffset"
            to: plasmoid.configuration.screenEdgeMargin
            duration: manager.animationSpeed
            easing.type: Easing.InQuad
        }
    }

    Connections {
        target: root
        onHideThickScreenGapChanged: {
            if (!latteView || !root.viewIsAvailable) {
                return;
            }

            if (root.behaveAsPlasmaPanel && !latteView.visibility.isHidden && !inSlidingIn && !inSlidingOut && !inStartup) {
                slideInOutRealFloating();
            }
        }

        onInStartupChanged: {
            //! used for positioning properly real floating panels when there is a maximized window
            if (root.hideThickScreenGap && !inStartup && latteView.positioner.slideOffset===0) {
                if (root.behaveAsPlasmaPanel && !latteView.visibility.isHidden) {
                    slideInOutRealFloating();
                }
            }
        }

        function slideInOutRealFloating() {
            if (root.hideThickScreenGap) {
                slidingInRealFloating.stop();
                slidingOutRealFloating.start();
            } else {
                slidingOutRealFloating.stop();
                slidingInRealFloating.start();
            }
        }
    }


}
