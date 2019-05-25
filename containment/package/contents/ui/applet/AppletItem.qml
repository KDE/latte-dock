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

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.latte 0.2 as Latte

import "colorizer" as Colorizer
import "communicator" as Communicator
import "indicator" as Indicator

Item {
    id: appletItem

    visible: false
    width: isInternalViewSplitter && !root.inConfigureAppletsMode ? 0 : computeWidth
    height: isInternalViewSplitter && !root.inConfigureAppletsMode ? 0 : computeHeight

    signal mousePressed(int x, int y, int button);
    signal mouseReleased(int x, int y, int button);

    property bool animationsEnabled: true
    property bool animationWasSent: false  //protection flag for animation broadcasting
    property bool canBeHovered: true
    property bool canShowAppletNumberBadge: !isSeparator && !isHidden && !isLattePlasmoid
                                            && !isSpacer && !isInternalViewSplitter
    property bool inFillCalculations: false //temp record, is used in calculations for fillWidth,fillHeight applets
    property bool needsFillSpace: { //fill flag, it is used in calculations for fillWidth,fillHeight applets
        if (!applet || !applet.Layout)
            return false;

        if (((root.isHorizontal && applet.Layout.fillWidth===true)
             || (root.isVertical && applet.Layout.fillHeight===true))
                && (applet.status !== PlasmaCore.Types.HiddenStatus))
            return true;
        else
            return false;
    }

    property bool userBlocksColorizing: false
    property bool appletBlocksColorizing: !communicator.latteSideColoringEnabled
    property bool appletBlocksParabolicEffect: communicator.parabolicEffectLocked
    property bool lockZoom: false

    property bool isActive: (isExpanded
                             && applet.pluginName !== root.plasmoidName
                             && applet.pluginName !== "org.kde.activeWindowControl"
                             && applet.pluginName !== "org.kde.plasma.appmenu")

    property bool isExpanded: applet && applet.status >= PlasmaCore.Types.NeedsAttentionStatus
                              && applet.status !== PlasmaCore.Types.HiddenStatus

    property bool isHidden: applet && applet.status === PlasmaCore.Types.HiddenStatus ? true : false
    property bool isInternalViewSplitter: (internalSplitterId > 0)
    property bool isLattePlasmoid: latteApplet !== null
    property bool isZoomed: false
    property bool isSeparator: applet && (applet.pluginName === "audoban.applet.separator"
                                          || applet.pluginName === "org.kde.latte.separator")
    property bool isSpacer: applet && (applet.pluginName === "org.kde.latte.spacer")
    property bool isSystray: applet && (applet.pluginName === "org.kde.plasma.systemtray" || applet.pluginName === "org.nomad.systemtray" )

    property bool firstChildOfStartLayout: (index === layoutsContainer.startLayout.beginIndex)
    property bool lastChildOfEndLayout: ((index === layoutsContainer.endLayout.beginIndex+layoutsContainer.endLayout.count-1)&&(layoutsContainer.endLayout.count>1))

    readonly property bool atScreenEdge: {
        if (root.isHorizontal) {
            if (firstChildOfStartLayout) {
                return latteView && latteView.x === latteView.screenGeometry.x;
            } else if (lastChildOfEndLayout) {
                return latteView && ((latteView.x + latteView.width) === (latteView.screenGeometry.x + latteView.screenGeometry.width));
            }
        } else {
            if (firstChildOfStartLayout) {
                return latteView && latteView.y === latteView.screenGeometry.y;
            } else if (lastChildOfEndLayout) {
                return latteView && ((latteView.y + latteView.height) === (latteView.screenGeometry.y + latteView.screenGeometry.height));
            }
        }

        return false;
    }

    //applet is in starting edge
    /*property bool startEdge: index < layoutsContainer.endLayout.beginIndex ? (index === 0)&&(layoutsContainer.mainLayout.count > 1) :
                                                               (index === layoutsContainer.endLayout.beginIndex)&&(layoutsContainer.endLayout.count > 1)*/
    property bool startEdge: (index === layoutsContainer.startLayout.beginIndex) || (index === layoutsContainer.mainLayout.beginIndex) || (index === layoutsContainer.endLayout.beginIndex)
    //applet is in ending edge
    property bool endEdge: plasmoid.configuration.panelPosition !== Latte.Types.Justify ? (index === layoutsContainer.mainLayout.beginIndex + layoutsContainer.mainLayout.count - 1)&&(layoutsContainer.mainLayout.count>1) :
                                                                                          (((index === layoutsContainer.startLayout.beginIndex+layoutsContainer.startLayout.count-2)&&(layoutsContainer.startLayout.count>2))
                                                                                           ||((index === layoutsContainer.mainLayout.beginIndex+layoutsContainer.mainLayout.count-2)&&(layoutsContainer.mainLayout.count>2))
                                                                                           ||((index === layoutsContainer.endLayout.beginIndex+layoutsContainer.endLayout.count-1)&&(layoutsContainer.endLayout.count>1)))

    readonly property bool acceptMouseEvents: applet && !isLattePlasmoid && !originalAppletBehavior && !appletItem.isSeparator && !communicator.parabolicEffectLocked
    readonly property bool originalAppletBehavior: root.zoomFactor === 1 || (root.zoomFactor>1 && canBeHovered && lockZoom)
    readonly property bool isSquare: communicator.overlayLatteIconIsActive

    property int animationTime: appliedDurationTime * (1.2 *units.shortDuration)
    property int hoveredIndex: layoutsContainer.hoveredIndex
    property int index: -1
    property int maxWidth: root.isHorizontal ? root.height : root.width
    property int maxHeight: root.isHorizontal ? root.height : root.width
    property int internalSplitterId: 0

    property int previousIndex: -1
    property int sizeForFill: -1 //it is used in calculations for fillWidth,fillHeight applets
    property int spacersMaxSize: Math.max(0,Math.ceil(0.55 * root.iconSize) - root.lengthMargins)
    property int status: applet ? applet.status : -1

    property real computeWidth: root.isVertical ? wrapper.width :
                                                  hiddenSpacerLeft.width+wrapper.width+hiddenSpacerRight.width

    property real computeHeight: root.isVertical ? hiddenSpacerLeft.height + wrapper.height + hiddenSpacerRight.height :
                                                   wrapper.height

    property string title: isInternalViewSplitter ? "Now Dock Splitter" : ""

    property Item applet: null
    property Item latteApplet: applet && (applet.pluginName === root.plasmoidName) ?
                                   (applet.children[0] ? applet.children[0] : null) : null
    property Item latteStyleApplet: applet && ((applet.pluginName === "org.kde.latte.spacer") || (applet.pluginName === "org.kde.latte.separator")) ?
                                        (applet.children[0] ? applet.children[0] : null) : null

    property Item appletWrapper: applet &&
                                 ((applet.pluginName === root.plasmoidName) ||
                                  isSystray) ? wrapper : wrapper.wrapperContainer

    property Item tooltipVisualParent: titleTooltipParent

    property Item communicatorAlias: communicator
    property Item wrapperAlias: wrapper

    property bool containsMouse: appletMouseArea.containsMouse /*|| appletMouseAreaBottom.containsMouse*/
    property bool pressed: viewSignalsConnector.pressed || clickedAnimation.running


    //// BEGIN :: Animate Applet when a new applet is dragged in the view

    //when the applet moves caused by its resize, don't animate.
    //this is completely heuristic, but looks way less "jumpy"
    property bool movingForResize: false
    property int oldX: x
    property int oldY: y

    onXChanged: {
        if (movingForResize || !root.dragInfo.entered) {
            movingForResize = false;
            return;
        }

        if (!root.isVertical) {
            translation.x = oldX - x;
            translation.y = 0;
        } else {
            translation.y = oldY - y;
            translation.x = 0;
        }

        translAnim.running = true

        if (!root.isVertical) {
            oldX = x;
            oldY = 0;
        } else {
            oldY = y;
            oldX = 0;
        }
    }

    onYChanged: {
        if (movingForResize || !root.dragInfo.entered) {
            movingForResize = false;
            return;
        }

        if (!root.isVertical) {
            translation.x = oldX - x;
            translation.y = 0;
        } else {
            translation.y = oldY - y;
            translation.x = 0;
        }

        translAnim.running = true;

        if (!root.isVertical) {
            oldX = x;
            oldY = 0;
        } else {
            oldY = y;
            oldX = 0;
        }
    }

    transform: Translate {
        id: translation
    }

    NumberAnimation {
        id: translAnim
        duration: units.longDuration
        easing.type: Easing.InOutQuad
        target: translation
        properties: "x,y"
        to: 0
    }

    //// END :: Animate Applet when a new applet is dragged in the view

    /// BEGIN functions
    function activateAppletForNeutralAreas(mouse){
        //if the event is at the active indicator or spacers area then try to expand the applet,
        //unfortunately for other applets there is no other way to activate them yet
        //for example the icon-only applets
        var choords = mapToItem(appletItem.appletWrapper, mouse.x, mouse.y);

        var wrapperContainsMouse = choords.x>=0 && choords.y>=0 && choords.x<appletItem.appletWrapper.width && choords.y<appletItem.appletWrapper.height;
        var appletItemContainsMouse = mouse.x>=0 && mouse.y>=0 && mouse.x<width && mouse.y<height;

        //console.log(" APPLET :: " + mouse.x +  " _ " + mouse.y);
        //console.log(" WRAPPER :: " + choords.x + " _ " + choords.y);

        if (appletItemContainsMouse && !wrapperContainsMouse) {
            //console.log("PASSED");
            latteView.toggleAppletExpanded(applet.id);
        } else {
            //console.log("REJECTED");
        }
    }

    function checkIndex(){
        index = -1;

        for(var i=0; i<layoutsContainer.startLayout.count; ++i){
            var child = layoutsContainer.startLayout.children[i];
            if (child === appletItem){
                index = layoutsContainer.startLayout.beginIndex + i;
                break;
            }
        }

        for(var i=0; i<layoutsContainer.mainLayout.count; ++i){
            var child = layoutsContainer.mainLayout.children[i];
            if (child === appletItem){
                index = layoutsContainer.mainLayout.beginIndex + i;
                break;
            }
        }

        for(var i=0; i<layoutsContainer.endLayout.count; ++i){
            var child = layoutsContainer.endLayout.children[i];
            if (child === appletItem){
                //create a very high index in order to not need to exchange hovering messages
                //between layoutsContainer.mainLayout and layoutsContainer.endLayout
                index = layoutsContainer.endLayout.beginIndex + i;
                break;
            }
        }

        if(appletItem.latteApplet){
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
        canBeHoveredTimer.start();
    }

    //! Reduce calculations and give the time to applet to adjust to prevent binding loops
    Timer{
        id: canBeHoveredTimer
        interval: 100
        onTriggered: {
            if (wrapper.zoomScale !== 1) {
                return;
            }

            var maxSize = root.iconSize + root.thickMargins;
            var maxForMinimumSize = root.iconSize + root.thickMargins;

            if ( (((applet && root.isHorizontal && (applet.width > maxSize || applet.Layout.minimumWidth > maxForMinimumSize))
                   || (applet && root.isVertical && (applet.height > maxSize || applet.Layout.minimumHeight > maxForMinimumSize)))
                  && !appletItem.isSpacer
                  && !communicator.canShowOverlaiedLatteIcon)
                    || isSystray
                    || appletItem.needsFillSpace) {
                appletItem.canBeHovered = false;
            } else {
                appletItem.canBeHovered = true;
            }
        }
    }

    function slotDestroyInternalViewSplitters() {
        if (isInternalViewSplitter) {
            destroy();
        }
    }

    //! pos in global root positioning
    function containsPos(pos) {
        var relPos = root.mapToItem(appletItem,pos.x, pos.y);

        if (relPos.x>=0 && relPos.x<=width && relPos.y>=0 && relPos.y<=height)
            return true;

        return false;
    }
    ///END functions

    //BEGIN connections
    onAppletChanged: {
        if (!applet) {
            destroy();
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
        if (appletItem.latteApplet) {
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
        if(appletItem.latteApplet){
            root.latteApplet = appletItem.latteApplet;
            root.latteAppletContainer = appletItem;
            root.latteAppletPos = index;
            appletItem.latteApplet.latteView = root;
            appletItem.latteApplet.forceHidePanel = true;

            appletItem.latteApplet.signalAnimationsNeedBothAxis.connect(slotAnimationsNeedBothAxis);
            appletItem.latteApplet.signalAnimationsNeedLength.connect(slotAnimationsNeedLength);
            appletItem.latteApplet.signalAnimationsNeedThickness.connect(slotAnimationsNeedThickness);
            appletItem.latteApplet.signalActionsBlockHiding.connect(slotActionsBlockHiding);
            appletItem.latteApplet.signalPreviewsShown.connect(slotPreviewsShown);
            appletItem.latteApplet.clearZoomSignal.connect(titleTooltipDialog.hide);
        }
    }

    onNeedsFillSpaceChanged: checkCanBeHovered();

    Component.onCompleted: {
        checkIndex();
        root.updateIndexes.connect(checkIndex);
        root.clearZoomSignal.connect(clearZoom);
        root.destroyInternalViewSplitters.connect(slotDestroyInternalViewSplitters);
    }

    Component.onDestruction: {
        if (isSeparator){
            parabolicManager.setSeparator(previousIndex, -1);
        }

        if (isHidden)
            parabolicManager.setHidden(previousIndex, -1);

        if(root.latteAppletPos>=0 && root.latteAppletPos === index){
            root.latteApplet = null;
            root.latteAppletContainer = null;
            root.latteAppletPos = -1;
        }

        root.updateIndexes.disconnect(checkIndex);
        root.clearZoomSignal.disconnect(clearZoom);
        root.destroyInternalViewSplitters.disconnect(slotDestroyInternalViewSplitters);

        if (appletItem.latteApplet) {
            appletItem.latteApplet.signalAnimationsNeedBothAxis.disconnect(slotAnimationsNeedBothAxis);
            appletItem.latteApplet.signalAnimationsNeedLength.disconnect(slotAnimationsNeedLength);
            appletItem.latteApplet.signalAnimationsNeedThickness.disconnect(slotAnimationsNeedThickness);
            appletItem.latteApplet.signalActionsBlockHiding.disconnect(slotActionsBlockHiding);
            appletItem.latteApplet.signalPreviewsShown.disconnect(slotPreviewsShown);
            appletItem.latteApplet.clearZoomSignal.disconnect(titleTooltipDialog.hide);
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
                    appletItem.clearZoom();
                }
            }
        }

        onSignalActivateEntryAtIndex: {
            if (parabolicManager.pseudoIndexBelongsToLatteApplet(entryIndex) && appletItem.isLattePlasmoid) {
                latteApplet.activateTaskAtIndex(entryIndex - latteApplet.tasksBaseIndex);
            } else if (root.unifiedGlobalShortcuts && (entryIndex === parabolicManager.pseudoAppletIndex(appletItem.index))) {
                latteView.toggleAppletExpanded(applet.id);
            }
        }

        onSignalNewInstanceForEntryAtIndex: {
            if (parabolicManager.pseudoIndexBelongsToLatteApplet(entryIndex) && appletItem.isLattePlasmoid) {
                latteApplet.newInstanceForTaskAtIndex(entryIndex - latteApplet.tasksBaseIndex);
            } else if (root.unifiedGlobalShortcuts && (entryIndex === parabolicManager.pseudoAppletIndex(appletItem.index))) {
                latteView.toggleAppletExpanded(applet.id);
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
                appletItem.clearZoom();

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

    Connections {
        id: viewSignalsConnector
        target: root.latteView ? root.latteView : null
        enabled: !appletItem.isLattePlasmoid && !appletItem.isSeparator && !appletItem.isSpacer && !appletItem.isHidden

        property bool pressed: false

        onMousePressed: {
            if (appletItem.containsPos(pos)) {
                viewSignalsConnector.pressed = true;
                var local = appletItem.mapFromItem(root, pos.x, pos.y);

                appletItem.mousePressed(local.x, local.y, button);
                appletItem.activateAppletForNeutralAreas(local);
            }
        }

        onMouseReleased: {
            if (appletItem.containsPos(pos)) {
                viewSignalsConnector.pressed = false;
                var local = appletItem.mapFromItem(root, pos.x, pos.y);
                appletItem.mouseReleased(local.x, local.y, button);
            }
        }
    }

    ///END connections

    //! It is used for any communication needed with the underlying applet
    Communicator.Engine{
        id: communicator

        //set up the overlayed appletItems and properties for when a overlaiedIconItem must be presented to the user
        //because the plasma widgets specific implementation breaks the Latte experience
        onOverlayLatteIconIsActiveChanged:{
            if (!overlayLatteIconIsActive && applet.opacity===0) {
                applet.opacity = 1;

                wrapper.disableScaleWidth = false;
                wrapper.disableScaleHeight = false;

                wrapper.updateLayoutWidth();
                wrapper.updateLayoutHeight();
            } else if (overlayLatteIconIsActive && applet.opacity>0) {
                applet.opacity = 0;

                if (applet.pluginName === "org.kde.plasma.folder") {
                    applet.parent =  wrapper.containerForOverlayIcon;
                    applet.anchors.fill = wrapper.containerForOverlayIcon;
                }

                wrapper.disableScaleWidth = false;
                wrapper.disableScaleHeight = false;

                wrapper.updateLayoutWidth();
                wrapper.updateLayoutHeight();
            }
        }
    }

    /*  Rectangle{
        anchors.fill: parent
        color: "transparent"
        border.color: "green"
        border.width: 1
    }*/


  /* DEPRECATED in favor of VIEW::MouseSignalsTracking
    MouseArea{
        id: appletMouseAreaBottom
        anchors.fill: parent
        propagateComposedEvents: true
        visible: (!appletMouseArea.visible || !appletMouseArea.enabled) && !root.editMode && !originalAppletBehavior

        onPressed: {
            appletItem.activateAppletForNeutralAreas(mouse);
            mouse.accepted = false;
        }

        onReleased: {
            mouse.accepted = false;
        }
    }*/

    //! Main Applet Shown Area
    Flow{
        id: appletFlow
        width: appletItem.computeWidth
        height: appletItem.computeHeight

        // a hidden spacer for the first element to add stability
        // IMPORTANT: hidden spacers must be tested on vertical !!!
        HiddenSpacer{id: hiddenSpacerLeft}

        Item {
            width: wrapper.width
            height: wrapper.height

            Indicator.Bridge{
                id: indicatorBridge
            }

            //! Indicator Back Layer
            Indicator.Loader{
                id: indicatorBackLayer
                level: Indicator.LevelOptions {
                    isBackground: true
                    bridge: indicatorBridge
                }
            }

            ItemWrapper{
                id: wrapper

                TitleTooltipParent{
                    id: titleTooltipParent
                }
            }

            //! The Applet Colorizer
            Colorizer.Applet {
                id: appletColorizer
                anchors.fill: parent
                opacity: mustBeShown ? 1 : 0

                readonly property bool mustBeShown: colorizerManager.mustBeShown
                                                    && !appletItem.userBlocksColorizing
                                                    && !appletItem.appletBlocksColorizing
                                                    && !appletItem.isInternalViewSplitter

                Behavior on opacity {
                    NumberAnimation {
                        duration: 1.2 * root.animationTime
                        easing.type: Easing.OutCubic
                    }
                }
            }

            //! Indicator Front Layer
            Indicator.Loader{
                id: indicatorFrontLayer
                level: Indicator.LevelOptions {
                    isForeground: true
                    bridge: indicatorBridge
                }
            }

            //! Applet Shortcut Visual Badge
            Item {
                anchors.centerIn: parent
                width: {
                    if (root.isHorizontal) {
                        return rectangled ? root.iconSize * wrapper.zoomScale : wrapper.width
                    } else {
                        return root.iconSize * wrapper.zoomScale
                    }
                }
                height: {
                    if (root.isHorizontal) {
                        return root.iconSize * wrapper.zoomScale
                    } else {
                        return rectangled ? root.iconSize * wrapper.zoomScale : wrapper.height
                    }
                }

                property bool rectangled: canBeHovered && !lockZoom

                ShortcutBadge{
                    anchors.fill: parent
                }
            }
        }

        // a hidden spacer on the right for the last item to add stability
        HiddenSpacer{id: hiddenSpacerRight; rightSpacer: true}
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
            NumberAnimation { duration: 2*root.durationTime*appletItem.animationTime }
        }

        PlasmaExtras.Heading {
            width: parent.width
            height: parent.height

            text: i18n("Tasks Area")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            fontSizeMode: Text.Fit
            color: colorizerManager.textColor

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
        enabled: visible
        hoverEnabled: latteApplet ? false : true
        propagateComposedEvents: true

        //! a way must be found in order for this be enabled
        //! only to support springloading for plasma 5.10
        //! also on this is based the tooltips behavior by enabling it
        //! plasma tooltips are disabled
        visible: acceptMouseEvents

        property bool blockWheel: false

        onEntered: {
            if (containsMouse && !originalAppletBehavior && !communicator.parabolicEffectLocked && appletItem.canBeHovered){
                root.stopCheckRestoreZoomTimer();
            }

            if (restoreAnimation.running) {
                restoreAnimation.stop();
            }

            if (!(isSeparator || isSpacer)) {
                root.showTooltipLabel(appletItem, applet.title);
            }

            //console.log("entered applet:" + layoutsContainer.hoveredIndex);


            if (layoutsContainer.hoveredIndex === -1 && root.latteAppletHoveredIndex===-1) {
                root.startDirectRenderDelayerDuringEntering();
            }

            if (!(root.latteViewIsHidden || root.inSlidingIn || root.inSlidingOut)){
                layoutsContainer.hoveredIndex = index;
            }

            if (originalAppletBehavior || communicator.parabolicEffectLocked || !canBeHovered) {
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
            if (communicator.appletIconItemIsShown()) {
                communicator.setAppletIconItemActive(false);
            }

            root.hideTooltipLabel();

            if (root.zoomFactor>1){
                root.startCheckRestoreZoomTimer();
            }
        }

        onPositionChanged: {
            if (originalAppletBehavior || !canBeHovered) {
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

            if (!(root.latteViewIsHidden || root.inSlidingIn || root.inSlidingOut)){
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

        onWheel: {
            if (isSeparator || !root.mouseWheelActions || blockWheel
                    || (root.latteViewIsHidden || root.inSlidingIn || root.inSlidingOut)){
                wheel.accepted = false;
                return;
            }

            var angle = wheel.angleDelta.y / 8;

            blockWheel = true;
            scrollDelayer.start();

            if (angle > 12) {
                //positive direction
                if (!isExpanded) {
                    latteView.toggleAppletExpanded(applet.id);
                }
            } else if (angle < -12) {
                //negative direction
                if (isExpanded) {
                    latteView.toggleAppletExpanded(applet.id);
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
            duration: 4 * appletItem.animationTime
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: hiddenSpacerLeft
            property: "nScale"
            to: 0
            duration: 4 * appletItem.animationTime
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: hiddenSpacerRight
            property: "nScale"
            to: 0
            duration: 4 * appletItem.animationTime
            easing.type: Easing.InCubic
        }
    }


    /////Clicked Animation/////
    SequentialAnimation{
        id: clickedAnimation
        alwaysRunToEnd: true
        running: appletItem.isSquare && !originalAppletBehavior && appletItem.pressed
                 && (root.durationTime > 0) && !indicators.info.providesClickedAnimation

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
