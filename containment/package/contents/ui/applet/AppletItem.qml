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
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.latte 0.1 as Latte

import "../../code/AppletIdentifier.js" as AppletIndetifier

Item {
    id: container

    visible: false
    width: isInternalViewSplitter && !root.editMode ? 0 : (root.isHorizontal ? computeWidth : computeWidth + shownAppletMargin)
    height: isInternalViewSplitter && !root.editMode ? 0 : (root.isVertical ?  computeHeight : computeHeight + shownAppletMargin)

    property bool animationsEnabled: true
    property bool animationWasSent: false  //protection flag for animation broadcasting
    property bool canBeHovered: true
    property bool canShowAppletNumberBadge: !isSeparator && !isHidden && !isLattePlasmoid
                                            && !isSpacer && !isInternalViewSplitter
    property bool disableLatteParabolicIconHeuristics: applet && applet.disableLatteParabolicIcon !== undefined ?  applet.disableLatteParabolicIcon : false
    property bool inFillCalculations: false //temp record, is used in calculations for fillWidth,fillHeight applets
    property bool needsFillSpace: { //fill flag, it is used in calculations for fillWidth,fillHeight applets
        if (!applet || !applet.Layout ||  (applet && applet.pluginName === "org.kde.plasma.panelspacer"))
            return false;

        if (((root.isHorizontal && applet.Layout.fillWidth===true)
             || (root.isVertical && applet.Layout.fillHeight===true))
                && (applet.status !== PlasmaCore.Types.HiddenStatus))
            return true;
        else
            return false;
    }
    property bool showZoomed: false
    property bool lockZoom: false
    property bool isExpanded: applet && applet.status >= PlasmaCore.Types.NeedsAttentionStatus
                              && applet.status !== PlasmaCore.Types.HiddenStatus

    property bool isHidden: applet && applet.status === PlasmaCore.Types.HiddenStatus ? true : false
    property bool isInternalViewSplitter: (internalSplitterId > 0)
    property bool isLattePlasmoid: latteApplet !== null
    property bool isZoomed: false
    property bool isSeparator: applet && applet.pluginName === "audoban.applet.separator"
    property bool isSpacer: latteSpacer !== null
    property bool isSystray: applet && (applet.pluginName === "org.kde.plasma.systemtray" || applet.pluginName === "org.nomad.systemtray" )

    property bool firstChildOfStartLayout: (index === layoutsContainer.startLayout.beginIndex)
    property bool lastChildOfEndLayout: ((index === layoutsContainer.endLayout.beginIndex+layoutsContainer.endLayout.count-1)&&(layoutsContainer.endLayout.count>1))
    //applet is in starting edge
    /*property bool startEdge: index < layoutsContainer.endLayout.beginIndex ? (index === 0)&&(layoutsContainer.mainLayout.count > 1) :
                                                               (index === layoutsContainer.endLayout.beginIndex)&&(layoutsContainer.endLayout.count > 1)*/
    property bool startEdge: (index === layoutsContainer.startLayout.beginIndex) || (index === layoutsContainer.mainLayout.beginIndex) || (index === layoutsContainer.endLayout.beginIndex)
    //applet is in ending edge
    property bool endEdge: plasmoid.configuration.panelPosition !== Latte.Dock.Justify ? (index === layoutsContainer.mainLayout.beginIndex + layoutsContainer.mainLayout.count - 1)&&(layoutsContainer.mainLayout.count>1) :
                                                                                         (((index === layoutsContainer.startLayout.beginIndex+layoutsContainer.startLayout.count-2)&&(layoutsContainer.startLayout.count>2))
                                                                                          ||((index === layoutsContainer.mainLayout.beginIndex+layoutsContainer.mainLayout.count-2)&&(layoutsContainer.mainLayout.count>2))
                                                                                          ||((index === layoutsContainer.endLayout.beginIndex+layoutsContainer.endLayout.count-1)&&(layoutsContainer.endLayout.count>1)))
    property bool supportsIsInLatte: applet && applet.isInLatte !== undefined ? true : false

    property int animationTime: root.durationTime* (1.2 *units.shortDuration) // 70
    property int hoveredIndex: layoutsContainer.hoveredIndex
    property int index: -1
    property int appletMargin: (applet && (applet.pluginName === root.plasmoidName))
                               || isInternalViewSplitter
                               || root.reverseLinesPosition ? 0 : root.statesLineSize
    property int maxWidth: root.isHorizontal ? root.height : root.width
    property int maxHeight: root.isHorizontal ? root.height : root.width
    property int shownAppletMargin: isSystray ? 0 : appletMargin
    property int internalSplitterId: 0

    property int previousIndex: -1
    property int sizeForFill: -1 //it is used in calculations for fillWidth,fillHeight applets
    property int spacersMaxSize: Math.max(0,Math.ceil(0.5*root.iconSize) - root.iconMargin)
    property int status: applet ? applet.status : -1

    property real computeWidth: root.isVertical ? wrapper.width :
                                                  hiddenSpacerLeft.width+wrapper.width+hiddenSpacerRight.width

    property real computeHeight: root.isVertical ? hiddenSpacerLeft.height + wrapper.height + hiddenSpacerRight.height :
                                                   wrapper.height

    property string title: isInternalViewSplitter ? "Now Dock Splitter" : ""

    property Item applet: null
    property Item latteApplet: applet && (applet.pluginName === root.plasmoidName) ?
                                   (applet.children[0] ? applet.children[0] : null) : null
    property Item latteSpacer: applet && (applet.pluginName === "org.kde.latte.spacer") ?
                                   (applet.children[0] ? applet.children[0] : null) : null

    property Item appletWrapper: applet &&
                                 ((applet.pluginName === root.plasmoidName) ||
                                  isSystray) ? wrapper : wrapper.wrapperContainer
    property Item appletIconItem; //first applet's IconItem, to be activated onExit signal
    property Item appletImageItem;

    property Item tooltipVisualParent: titleTooltipParent

    //this is used for folderView and icon widgets to fake their visual
    property bool fakeIconItem: applet && appletIconItem //(applet.pluginName === "org.kde.plasma.folder" || applet.pluginName === "org.kde.plasma.icon")

    property alias containsMouse: appletMouseArea.containsMouse
    property alias pressed: appletMouseArea.pressed

    /*onComputeHeightChanged: {
        if(index==0)
            console.log(computeHeight);
    }*/

    //a timer that is used in  order to init the fake applets on startup
    Timer {
        id: fakeInitTimer
        interval: 4000
        onTriggered: {
            AppletIndetifier.reconsiderAppletIconItem();

            if (root.debugModeTimers) {
                console.log("containment timer: appletItem fakeInitTimer called...");
            }
        }
    }

    //set up the fake containers and properties for when a fakeIconItem must be presented to the user
    //because the plasma widgets specific implementation breaks the Latte experience
    onFakeIconItemChanged: {
        if (fakeIconItem) {
            applet.opacity = 0;

            if (applet.pluginName === "org.kde.plasma.folder") {
                applet.parent =  wrapper.fakeIconItemContainer;
                applet.anchors.fill = wrapper.fakeIconItemContainer;
            }

            wrapper.disableScaleWidth = false;
            wrapper.disableScaleHeight = false;

            wrapper.updateLayoutWidth();
            wrapper.updateLayoutHeight();
        }
    }

    onDisableLatteParabolicIconHeuristicsChanged: {
        if (disableLatteParabolicIconHeuristics && applet.opacity === 0) {
            applet.opacity = 1;

            wrapper.disableScaleWidth = false;
            wrapper.disableScaleHeight = false;

            wrapper.updateLayoutWidth();
            wrapper.updateLayoutHeight();
        }
    }

    /// BEGIN functions
    function activateAppletForNeutralAreas(mouse){
        //if the event is at the active indicator or spacers area then try to expand the applet,
        //unfortunately for other applets there is no other way to activate them yet
        //for example the icon-only applets
        var choords = mapToItem(container.appletWrapper, mouse.x, mouse.y);
        if (choords.x<0 || choords.y<0 || choords.x>=container.appletWrapper.width || choords.y>=container.appletWrapper.height) {
            dock.toggleAppletExpanded(applet.id);
        }
    }

    function checkIndex(){
        index = -1;

        for(var i=0; i<layoutsContainer.startLayout.count; ++i){
            if(layoutsContainer.startLayout.children[i] === container){
                index = layoutsContainer.startLayout.beginIndex + i;
                break;
            }
        }

        for(var i=0; i<layoutsContainer.mainLayout.count; ++i){
            if(layoutsContainer.mainLayout.children[i] === container){
                index = layoutsContainer.mainLayout.beginIndex + i;
                break;
            }
        }

        for(var i=0; i<layoutsContainer.endLayout.count; ++i){
            if(layoutsContainer.endLayout.children[i] === container){
                //create a very high index in order to not need to exchange hovering messages
                //between layoutsContainer.mainLayout and layoutsContainer.endLayout
                index = layoutsContainer.endLayout.beginIndex + i;
                break;
            }
        }


        if(container.latteApplet){
            if(index===layoutsContainer.startLayout.beginIndex || index===layoutsContainer.mainLayout.beginIndex || index===layoutsContainer.endLayout.beginIndex)
                latteApplet.disableLeftSpacer = false;
            else
                latteApplet.disableLeftSpacer = true;

            if( index === layoutsContainer.startLayout.beginIndex + layoutsContainer.startLayout.count - 1
                    || index===layoutsContainer.mainLayout.beginIndex + layoutsContainer.mainLayout.count - 1
                    || index === layoutsContainer.endLayout.beginIndex + layoutsContainer.endLayout.count - 1)
                latteApplet.disableRightSpacer = false;
            else
                latteApplet.disableRightSpacer = true;
        }
    }

    //this functions gets the signal from the plasmoid, it can be used for signal items
    //outside the LatteApplet Plasmoid
    //property int debCounter: 0;
    function clearZoom(){
        if (layoutsContainer.hoveredIndex === -1 && root.latteAppletHoveredIndex === -1) {
            restoreAnimation.start();
        }
    }

    function checkCanBeHovered(){
        if ( (((applet && (applet.Layout.minimumWidth > root.iconSize) && root.isHorizontal) ||
               (applet && (applet.Layout.minimumHeight > root.iconSize) && root.isVertical))
              && (applet && applet.pluginName !== "org.kde.plasma.panelspacer" && (applet.pluginName !== "org.kde.latte.spacer"))
              && !container.fakeIconItem)
                || (isSystray)
                || (container.needsFillSpace) ) {
            canBeHovered = false;
        }
        else{
            canBeHovered = true;
        }
    }

    //! pos in global root positioning
    function containsPos(pos) {
        var relPos = root.mapToItem(container,pos.x, pos.y);

        if (relPos.x>=0 && relPos.x<=width && relPos.y>=0 && relPos.y<=height)
            return true;

        return false;
    }

    function reconsiderAppletIconItem() {
        AppletIndetifier.reconsiderAppletIconItem();
    }

    ///END functions

    //BEGIN connections
    onAppletChanged: {
        if (!applet) {
            destroy();
        } else {
            AppletIndetifier.reconsiderAppletIconItem();
            fakeInitTimer.start();
        }
    }

    onHoveredIndexChanged:{
        if ( (Math.abs(hoveredIndex-index) > 1) && (hoveredIndex !== -1) ) {
            wrapper.zoomScale = 1;
        }

        if (Math.abs(hoveredIndex-index) >= 1) {
            hiddenSpacerLeft.nScale = 0;
            hiddenSpacerRight.nScale = 0;
        }
    }

    onIndexChanged: {
        if (container.latteApplet) {
            root.latteAppletPos = index;
        }

        if (isHidden) {
            parabolicManager.setHidden(previousIndex, index);
        }

        if (isSeparator) {
            parabolicManager.setSeparator(previousIndex, index);
        }

        if (index>-1) {
            previousIndex = index;
        }
    }

    onIsExpandedChanged: {
        if (isExpanded) {
            root.hideTooltipLabel();
        }
    }

    onIsHiddenChanged: {
        if (isHidden) {
            parabolicManager.setHidden(-1, index);
        } else {
            parabolicManager.setHidden(index, -1);
        }
    }

    onIsSeparatorChanged: {
        if (isSeparator) {
            parabolicManager.setSeparator(-1, index);
        } else {
            parabolicManager.setSeparator(index, -1);
        }

    }

    onLatteAppletChanged: {
        if(container.latteApplet){
            root.latteApplet = container.latteApplet;
            root.latteAppletContainer = container;
            root.latteAppletPos = index;
            container.latteApplet.latteDock = root;
            container.latteApplet.forceHidePanel = true;

            container.latteApplet.signalAnimationsNeedBothAxis.connect(slotAnimationsNeedBothAxis);
            container.latteApplet.signalAnimationsNeedLength.connect(slotAnimationsNeedLength);
            container.latteApplet.signalAnimationsNeedThickness.connect(slotAnimationsNeedThickness);
            container.latteApplet.signalActionsBlockHiding.connect(slotActionsBlockHiding);
            container.latteApplet.signalPreviewsShown.connect(slotPreviewsShown);
            container.latteApplet.clearZoomSignal.connect(titleTooltipDialog.hide);
        }
    }

    onLatteSpacerChanged: {
        if(container.latteSpacer){
            latteSpacer.latteDock = root;
            //container.lockZoom = true;
        }
    }

    onNeedsFillSpaceChanged: checkCanBeHovered();

    onShowZoomedChanged: {
        if(showZoomed){
            //var newZ = container.maxHeight / root.iconSize;
            //wrapper.zoomScale = newZ;
            wrapper.zoomScale = 1;
        }
        else{
            wrapper.zoomScale = 1;
        }
    }

    onSupportsIsInLatteChanged: {
        if (supportsIsInLatte) {
            applet.isInLatte = true;
        }
    }

    Component.onCompleted: {
        checkIndex();
        root.updateIndexes.connect(checkIndex);
        root.clearZoomSignal.connect(clearZoom);
    }

    Component.onDestruction: {
        if (isSeparator){
            parabolicManager.setSeparator(previousIndex, -1);
        }

        if (isHidden)
            parabolicManager.setHidden(previousIndex, -1);

        root.updateIndexes.disconnect(checkIndex);
        root.clearZoomSignal.disconnect(clearZoom);

        if (container.latteApplet) {
            container.latteApplet.signalAnimationsNeedBothAxis.disconnect(slotAnimationsNeedBothAxis);
            container.latteApplet.signalAnimationsNeedLength.disconnect(slotAnimationsNeedLength);
            container.latteApplet.signalAnimationsNeedThickness.disconnect(slotAnimationsNeedThickness);
            container.latteApplet.signalActionsBlockHiding.disconnect(slotActionsBlockHiding);
            container.latteApplet.signalPreviewsShown.disconnect(slotPreviewsShown);
            container.latteApplet.clearZoomSignal.disconnect(titleTooltipDialog.hide);
        }
    }

    Connections{
        target: root

        /* onGlobalDirectRenderChanged:{
            if (root.globalDirectRender && restoreAnimation.running) {
                // console.log("CLEAR APPLET SCALE !!!!");
                //restoreAnimation.stop();
                //wrapper.zoomScale = 1;
            }
        }*/

        onLatteAppletHoveredIndexChanged: {
            if ( (root.zoomFactor>1) && (root.latteAppletHoveredIndex >= 0) ){
                var distance = 2;
                //for Tasks plasmoid distance of 2 is not always safe there are
                //cases that needs to be 3, when an internal separator there is
                //between the hovered task and the current applet
                if (root.hasInternalSeparator) {
                    if (index < root.latteAppletPos) {
                        var firstTaskIndex = root.latteApplet.parabolicManager.availableHigherIndex(0);

                        distance = firstTaskIndex+2;
                    } else if (index > root.latteAppletPos) {
                        var lastTaskIndex = root.latteApplet.parabolicManager.availableLowerIndex(root.tasksCount-1);

                        distance = root.tasksCount-1-lastTaskIndex+2;
                    }
                }

                if(Math.abs(index-root.latteAppletPos+root.latteAppletHoveredIndex)>=Math.max(2,distance)) {
                    container.clearZoom();
                }
            }
        }

        onSignalActivateEntryAtIndex: {
            if (parabolicManager.pseudoIndexBelongsToLatteApplet(entryIndex) && container.isLattePlasmoid) {
                latteApplet.activateTaskAtIndex(entryIndex - latteApplet.tasksNumbersBase);
            } else if (entryIndex === parabolicManager.pseudoAppletIndex(container.index)) {
                dock.toggleAppletExpanded(applet.id);
            }
        }

        onSignalNewInstanceForEntryAtIndex: {
            if (parabolicManager.pseudoIndexBelongsToLatteApplet(entryIndex) && container.isLattePlasmoid) {
                latteApplet.newInstanceForTaskAtIndex(entryIndex - latteApplet.tasksNumbersBase);
            } else if (entryIndex === parabolicManager.pseudoAppletIndex(container.index)) {
                dock.toggleAppletExpanded(applet.id);
            }
        }
    }

    Connections{
        target: layoutsContainer
        onHoveredIndexChanged:{
            //for applets it is safe to consider that a distance of 2
            //is enough to clearZoom
            if ( (root.zoomFactor>1) && (layoutsContainer.hoveredIndex>=0)
                    && (Math.abs(index-layoutsContainer.hoveredIndex)>=2))
                container.clearZoom();

            if ((restoreAnimation.running) && (layoutsContainer.hoveredIndex !== -1)) {
                restoreAnimation.stop();
            }
        }
    }

    Connections{
        target: root
        onLatteAppletHoveredIndexChanged: {
            if ((restoreAnimation.running) && (root.latteAppletHoveredIndex !== -1)) {
                restoreAnimation.stop();
            }
        }
    }

    ///END connections

    /*  Rectangle{
        anchors.fill: parent
        color: "transparent"
        border.color: "green"
        border.width: 1
    }*/
    MouseArea{
        id: appletMouseAreaBottom
        anchors.fill: parent
        propagateComposedEvents: true
        visible: (!appletMouseArea.visible || !appletMouseArea.enabled) && !root.editMode

        property bool pressed: false

        onPressed: {
            container.activateAppletForNeutralAreas(mouse);
            pressed = true;
            mouse.accepted = false;
        }

        onReleased: {
            pressed = false;
            mouse.accepted = false;
        }
    }


    Flow{
        id: appletFlow
        width: container.computeWidth
        height: container.computeHeight

        anchors.rightMargin: (latteApplet || (showZoomed && root.editMode)) ||
                             (plasmoid.location !== PlasmaCore.Types.RightEdge) ? 0 : shownAppletMargin
        anchors.leftMargin: (latteApplet || (showZoomed && root.editMode)) ||
                            (plasmoid.location !== PlasmaCore.Types.LeftEdge) ? 0 : shownAppletMargin
        anchors.topMargin: (latteApplet || (showZoomed && root.editMode)) ||
                           (plasmoid.location !== PlasmaCore.Types.TopEdge)? 0 : shownAppletMargin
        anchors.bottomMargin: (latteApplet || (showZoomed && root.editMode)) ||
                              (plasmoid.location !== PlasmaCore.Types.BottomEdge) ? 0 : shownAppletMargin


        // a hidden spacer for the first element to add stability
        // IMPORTANT: hidden spacers must be tested on vertical !!!
        AppletHiddenSpacer{id: hiddenSpacerLeft}

        AppletItemWrapper{
            id: wrapper

            TitleTooltipParent{
                id: titleTooltipParent
            }
        }

        // a hidden spacer on the right for the last item to add stability
        AppletHiddenSpacer{id: hiddenSpacerRight; rightSpacer: true}

    }// Flow with hidden spacers inside

    //Busy Indicator
    PlasmaComponents.BusyIndicator {
        z: 1000
        visible: applet && applet.busy
        running: visible
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width
    }

    //! The Launchers Area Indicator
    Rectangle{
        anchors.fill: parent
        radius: root.iconSize/10

        property color tempColor: "#aa222222"
        color: tempColor
        border.width: 1
        border.color: "#ff656565"

        opacity: latteApplet && root.addLaunchersMessage ? 1 : 0

        Behavior on opacity{
            NumberAnimation { duration: 2*root.durationTime*container.animationTime }
        }

        PlasmaExtras.Heading {
            width: parent.width
            height: parent.height

            text: i18n("Launchers Area")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            fontSizeMode: Text.Fit
            color: colorizerLoader.themeBrightColor

            rotation: {
                if (root.isHorizontal)
                    return 0;
                else if (plasmoid.location === PlasmaCore.Types.LeftEdge)
                    return -90;
                else if (plasmoid.location === PlasmaCore.Types.RightEdge)
                    return 90;
            }
        }
    }


    MouseArea{
        id: appletMouseArea

        anchors.fill: parent
        enabled: !latteApplet && canBeHovered && !root.editMode && !lockZoom && root.zoomFactor>1
        hoverEnabled: !root.editMode && (!latteApplet) ? true : false
        propagateComposedEvents: true

        //! a way must be found in order for this be enabled
        //! only to support springloading for plasma 5.10
        //! also on this is based the tooltips behavior by enabling it
        //! plasma tooltips are disabled
        visible: !container.latteApplet && !lockZoom && canBeHovered && !(container.isSeparator && !root.editMode)  //&& (root.zoomFactor>1)

        property bool blockWheel: false
        property bool pressed: false

        onClicked: {
            pressed = false;
            mouse.accepted = false;
        }

        onEntered: {
            //AppletIndetifier.reconsiderAppletIconItem();
            if (containsMouse && !container.lockZoom && container.canBeHovered){
                root.stopCheckRestoreZoomTimer();
            }

            if (restoreAnimation.running) {
                restoreAnimation.stop();
            }

            root.showTooltipLabel(container, applet.title);

            //console.log("entered applet:" + layoutsContainer.hoveredIndex);


            if (layoutsContainer.hoveredIndex === -1 && root.latteAppletHoveredIndex===-1) {
                root.startDirectRenderDelayerDuringEntering();
            }

            if (!(root.dockIsHidden || root.inSlidingIn || root.inSlidingOut)){
                layoutsContainer.hoveredIndex = index;
            }

            if (lockZoom || !canBeHovered) {
                return;
            }

            if (root.isHalfShown || (root.latteApplet
                                     && (root.latteApplet.noTasksInAnimation>0 || root.latteApplet.contextMenu))) {
                return;
            }

            if (root.isHorizontal){
                layoutsContainer.currentSpot = mouseX;
                wrapper.calculateScales(mouseX);
            }
            else{
                layoutsContainer.currentSpot = mouseY;
                wrapper.calculateScales(mouseY);
            }
        }

        onExited:{
            if (appletIconItem && appletIconItem.visible)
                appletIconItem.active = false;

            root.hideTooltipLabel();

            if (root.zoomFactor>1){
                root.startCheckRestoreZoomTimer();
            }
        }

        onPositionChanged: {
            //  if(!pressed){
            if (lockZoom || !canBeHovered) {
                mouse.accepted = false;
                return;
            }

            if (root.isHalfShown || (root.latteApplet
                                     && (root.latteApplet.noTasksInAnimation>0 || root.latteApplet.contextMenu))) {
                return;
            }

            if (layoutsContainer.hoveredIndex === -1 && root.latteAppletHoveredIndex===-1) {
                root.startDirectRenderDelayerDuringEntering();
            }

            if (!(root.dockIsHidden || root.inSlidingIn || root.inSlidingOut)){
                layoutsContainer.hoveredIndex = index;
            }

            if (!root.globalDirectRender && !root.directRenderDelayerIsRunning) {
                root.setGlobalDirectRender(true);
            }

            if( ((wrapper.zoomScale == 1 || wrapper.zoomScale === root.zoomFactor) && !root.globalDirectRender) || root.globalDirectRender) {
                if (root.isHorizontal){
                    var step = Math.abs(layoutsContainer.currentSpot-mouse.x);
                    if (step >= root.animationStep){
                        layoutsContainer.currentSpot = mouse.x;

                        wrapper.calculateScales(mouse.x);
                    }
                }
                else{
                    var step = Math.abs(layoutsContainer.currentSpot-mouse.y);
                    if (step >= root.animationStep){
                        layoutsContainer.currentSpot = mouse.y;

                        wrapper.calculateScales(mouse.y);
                    }
                }
            }

            mouse.accepted = false;
        }

        onPressed: {
            container.activateAppletForNeutralAreas(mouse);

            pressed = true;
            //! this is needed for some applets is order to be activated/deactivated correctly
            //! such case is the "Application Menu". (bug #928)
            mouse.accepted = false;
        }

        onReleased: {
            pressed = false;
            mouse.accepted = false;
        }

        onWheel: {
            if (isSeparator || !root.mouseWheelActions || blockWheel
                    || (root.dockIsHidden || root.inSlidingIn || root.inSlidingOut)){
                return;
            }

            var angle = wheel.angleDelta.y / 8;

            blockWheel = true;
            scrollDelayer.start();

            if (angle > 12) {
                //positive direction
                if (!isExpanded) {
                    dock.toggleAppletExpanded(applet.id);
                }
            } else if (angle < -12) {
                //negative direction
                if (isExpanded) {
                    dock.toggleAppletExpanded(applet.id);
                }
            }
        }

        //! A timer is needed in order to handle also touchpads that probably
        //! send too many signals very fast. This way the signals per sec are limited.
        //! The user needs to have a steady normal scroll in order to not
        //! notice a annoying delay
        Timer{
            id: scrollDelayer

            interval: 700

            onTriggered: appletMouseArea.blockWheel = false;
        }
    }

    //BEGIN states
    states: [
        State {
            name: "left"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

            AnchorChanges {
                target: appletFlow
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;}
            }
        },
        State {
            name: "right"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)

            AnchorChanges {
                target: appletFlow
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;}
            }
        },
        State {
            name: "bottom"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

            AnchorChanges {
                target: appletFlow
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;}
            }
        },
        State {
            name: "top"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)

            AnchorChanges {
                target: appletFlow
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined;}
            }
        }
    ]
    //END states

    //BEGIN animations
    ///////Restore Zoom Animation/////
    ParallelAnimation{
        id: restoreAnimation

        PropertyAnimation {
            target: wrapper
            property: "zoomScale"
            to: 1
            duration: 4 * container.animationTime
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: hiddenSpacerLeft
            property: "nScale"
            to: 0
            duration: 4 * container.animationTime
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: hiddenSpacerRight
            property: "nScale"
            to: 0
            duration: 4 * container.animationTime
            easing.type: Easing.InCubic
        }
    }


    /////Clicked Animation/////
    SequentialAnimation{
        id: clickedAnimation
        alwaysRunToEnd: true
        running: (appletMouseArea.pressed || appletMouseAreaBottom.pressed) && (root.durationTime > 0)

        onStopped: {
            appletMouseArea.pressed = false;
            appletMouseAreaBottom.pressed = false;
        }

        ParallelAnimation{
            PropertyAnimation {
                target: wrapper.clickedEffect
                property: "brightness"
                to: -0.35
                duration: units.longDuration
                easing.type: Easing.OutQuad
            }
        }
        ParallelAnimation{
            PropertyAnimation {
                target: wrapper.clickedEffect
                property: "brightness"
                to: 0
                duration: units.longDuration
                easing.type: Easing.OutQuad
            }
        }
    }
    //END animations
}
