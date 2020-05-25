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
import org.kde.kquickcontrolsaddons 2.0

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

import "colorizer" as Colorizer
import "communicator" as Communicator
import "indicator" as Indicator
import "../debug" as Debug

Item {
    id: appletItem

    visible: false
    width: isInternalViewSplitter && !root.inConfigureAppletsMode ? 0 : computeWidth
    height: isInternalViewSplitter && !root.inConfigureAppletsMode ? 0 : computeHeight

    //any applets that exceed their limits should not take events from their surrounding applets
    clip: !isSeparator

    signal mousePressed(int x, int y, int button);
    signal mouseReleased(int x, int y, int button);

    property bool animationsEnabled: true
    property bool parabolicEffectIsSupported: true
    property bool canShowAppletNumberBadge: !isSeparator && !isHidden && !isLattePlasmoid
                                            && !isSpacer && !isInternalViewSplitter

    //! Fill Applet(s)
    property bool inFillCalculations: false //temp record, is used in calculations for fillWidth,fillHeight applets
    property bool isAutoFillApplet: {
        if (!applet || !applet.Layout)
            return false;

        if (((root.isHorizontal && applet.Layout.fillWidth===true)
             || (root.isVertical && applet.Layout.fillHeight===true))
                && (applet.status !== PlasmaCore.Types.HiddenStatus))
            return true;
        else
            return false;
    }
    property int maxAutoFillLength: -1 //it is used in calculations for fillWidth,fillHeight applets
    property int minAutoFillLength: -1 //it is used in calculations for fillWidth,fillHeight applets

    property bool userBlocksColorizing: false
    property bool appletBlocksColorizing: !communicator.requires.latteSideColoringEnabled
    property bool appletBlocksParabolicEffect: communicator.requires.parabolicEffectLocked
    property bool lockZoom: false

    property bool isActive: (isExpanded
                             && applet.pluginName !== root.plasmoidName
                             && applet.pluginName !== "org.kde.activeWindowControl"
                             && applet.pluginName !== "org.kde.plasma.appmenu")

    property bool isExpanded: false

    property bool isHidden: (applet && applet.status === PlasmaCore.Types.HiddenStatus)
                            || (isInternalViewSplitter && !root.inConfigureAppletsMode)
    property bool isInternalViewSplitter: (internalSplitterId > 0)
    property bool isLattePlasmoid: latteApplet !== null
    property bool isZoomed: false
    property bool isSeparator: applet && (applet.pluginName === "audoban.applet.separator"
                                          || applet.pluginName === "org.kde.latte.separator")
    property bool isSpacer: applet && (applet.pluginName === "org.kde.latte.spacer")
    property bool isSystray: applet && (applet.pluginName === "org.kde.plasma.systemtray" || applet.pluginName === "org.nomad.systemtray" )

    property bool firstChildOfStartLayout: index === appletItem.layouter.startLayout.firstVisibleIndex
    property bool firstChildOfMainLayout: index === appletItem.layouter.mainLayout.firstVisibleIndex
    property bool lastChildOfMainLayout: index === appletItem.layouter.mainLayout.lastVisibleIndex
    property bool lastChildOfEndLayout: index === appletItem.layouter.endLayout.lastVisibleIndex

    readonly property bool atScreenEdge: {
        if (root.panelAlignment === LatteCore.Types.Center) {
            return false;
        }

        if (root.panelAlignment === LatteCore.Types.Justify) {
            //! Justify case
            if (root.maxLengthPerCentage!==100 || plasmoid.configuration.offset!==0) {
                return false;
            }

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

        if (root.panelAlignment === LatteCore.Types.Left && plasmoid.configuration.offset===0) {
            //! Left case
            return firstChildOfMainLayout;
        } else if (root.panelAlignment === LatteCore.Types.Right && plasmoid.configuration.offset===0) {
            //! Right case
            return lastChildOfMainLayout;
        }

        if (root.panelAlignment === LatteCore.Types.Top && plasmoid.configuration.offset===0) {
            return firstChildOfMainLayout && latteView && latteView.y === latteView.screenGeometry.y;
        } else if (root.panelAlignment === LatteCore.Types.Bottom && plasmoid.configuration.offset===0) {
            return lastChildOfMainLayout && latteView && ((latteView.y + latteView.height) === (latteView.screenGeometry.y + latteView.screenGeometry.height));
        }

        return false;
    }

    //applet is in starting edge
    property bool firstAppletInContainer: (index >=0) &&
                                          ((index === layouter.startLayout.firstVisibleIndex)
                                           || (index === layouter.mainLayout.firstVisibleIndex)
                                           || (index === layouter.endLayout.firstVisibleIndex))
    //applet is in ending edge
    property bool lastAppletInContainer: (index >=0) &&
                                         ((index === layouter.startLayout.lastVisibleIndex)
                                          || (index === layouter.mainLayout.lastVisibleIndex)
                                          || (index === layouter.endLayout.lastVisibleIndex))

    readonly property bool acceptMouseEvents: applet && !isLattePlasmoid && !originalAppletBehavior && !appletItem.isSeparator && !communicator.requires.parabolicEffectLocked
    readonly property bool originalAppletBehavior: (appletItem.parabolic.factor.zoom === 1 && !lockZoom /*hacky flag to keep Latte behavior*/)
                                                   || (appletItem.parabolic.factor.zoom>1 && !parabolicEffectIsSupported)
                                                   || (appletItem.parabolic.factor.zoom>1 && parabolicEffectIsSupported && lockZoom)

    readonly property bool isSquare: communicator.overlayLatteIconIsActive
    readonly property bool screenEdgeMarginSupported: communicator.requires.screenEdgeMarginSupported

    property int animationTime: appletItem.animations.speedFactor.normal * (1.2*appletItem.animations.duration.small)
    property int index: -1
    property int maxWidth: root.isHorizontal ? root.height : root.width
    property int maxHeight: root.isHorizontal ? root.height : root.width
    property int internalSplitterId: 0

    property int previousIndex: -1
    property int spacersMaxSize: Math.max(0,Math.ceil(0.55 * metrics.iconSize) - metrics.totals.lengthEdges)
    property int status: applet ? applet.status : -1

    //! separators tracking
    readonly property bool tailAppletIsSeparator: {
        if (isSeparator || index<0) {
            return false;
        }

        var tail = index - 1;

        while(tail>=0 && indexer.hidden.indexOf(tail)>=0) {
            //! when a tail applet contains sub-indexing and does not influence
            //! tracking is considered hidden
            tail = tail - 1;
        }

        if (tail >= 0 && indexer.clients.indexOf(tail)>=0) {
            //! tail applet contains items sub-indexing
            var tailBridge = indexer.getClientBridge(tail);

            if (tailBridge && tailBridge.client) {
                return tailBridge.client.lastHeadItemIsSeparator;
            }
        }

        // tail applet is normal
        return (indexer.separators.indexOf(tail)>=0);
    }

    readonly property bool headAppletIsSeparator: {
        if (isSeparator || index<0) {
            return false;
        }

        var head = index + 1;

        while(head>=0 && indexer.hidden.indexOf(head)>=0) {
            //! when a head applet contains sub-indexing and does not influence
            //! tracking is considered hidden
            head = head + 1;
        }

        if (head >= 0 && indexer.clients.indexOf(head)>=0) {
            //! head applet contains items sub-indexing
            var headBridge = indexer.getClientBridge(head);

            if (headBridge && headBridge.client) {
                return headBridge.client.firstTailItemIsSeparator;
            }
        }

        // head applet is normal
        return (indexer.separators.indexOf(head)>=0);
    }

    //! local margins
    readonly property bool parabolicEffectMarginsEnabled: appletItem.parabolic.factor.zoom>1 && !originalAppletBehavior && !communicator.parabolicEffectIsSupported

    property int lengthAppletPadding: metrics.fraction.lengthAppletPadding === -1 || parabolicEffectMarginsEnabled ?
                                          metrics.padding.length : metrics.padding.lengthApplet

    property int lengthAppletFullMargin: 0
    property int lengthAppletFullMargins: 2 * lengthAppletFullMargin

    property int internalWidthMargins: {
        if (root.isVertical) {
            return metrics.totals.thicknessEdges;
        }

        /*TODO, Fitt's case: is temporary until the atScreenEdge applets are aligned properly to the corner and the wrapper
          is taking all the space needed in order to fill right. For atScreenEdge appplets that should be: applet size + lengthAppletPadding + lengthAppletExtMargin.
          The indicator should follow also the applet alignment in this in order to feel right
          */
        return 2 * lengthAppletPadding;
    }

    property int internalHeightMargins: {
        if (root.isHorizontal) {
            return root.metrics.totals.thicknessEdges;
        }

        /*TODO,Fitt's case: is temporary until the atScreenEdge applets are aligned properly to the corner and the wrapper
          is taking all the space needed in order to fill right. For atScreenEdge appplets that should be: applet size + lengthAppletPadding + lengthAppletExtMargin.
          The indicator should follow also the applet alignment in this in order to feel right
          */
        return 2 * lengthAppletPadding;
    }

    //! are set by the indicator
    property int iconOffsetX: 0
    property int iconOffsetY: 0

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

    property Item appletWrapper: applet && (applet.pluginName === root.plasmoidName )? wrapper : wrapper.wrapperContainer

    property Item tooltipVisualParent: titleTooltipParent

    readonly property alias communicator: _communicator
    readonly property alias wrapper: _wrapper

    property Item animations: null
    property Item indexer: null
    property Item layouter: null
    property Item metrics: null
    property Item parabolic: null
    property Item shortcuts: null

    property bool containsMouse: appletMouseArea.containsMouse || (isLattePlasmoid && latteApplet.containsMouse)
    property bool pressed: viewSignalsConnector.pressed || clickedAnimation.running


    //// BEGIN :: Animate Applet when a new applet is dragged in the view

    //when the applet moves caused by its resize, don't animate.
    //this is completely heuristic, but looks way less "jumpy"
    property bool movingForResize: false
    property int oldX: x
    property int oldY: y

    onXChanged: {
        if (root.isVertical) {
            return;
        }

        if (movingForResize) {
            movingForResize = false;
            return;
        }

        var draggingAppletInConfigure = root.dragOverlay && root.dragOverlay.currentApplet;
        var isCurrentAppletInDragging = draggingAppletInConfigure && (root.dragOverlay.currentApplet === appletItem);
        var dropApplet = root.dragInfo.entered && foreDropArea.visible

        if (isCurrentAppletInDragging || !draggingAppletInConfigure && !dropApplet) {
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
        if (root.isHorizontal) {
            return;
        }

        if (movingForResize) {
            movingForResize = false;
            return;
        }

        var draggingAppletInConfigure = root.dragOverlay && root.dragOverlay.currentApplet;
        var isCurrentAppletInDragging = draggingAppletInConfigure && (root.dragOverlay.currentApplet === appletItem);
        var dropApplet = root.dragInfo.entered && foreDropArea.visible

        if (isCurrentAppletInDragging || !draggingAppletInConfigure && !dropApplet) {
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
        duration: appletItem.animations.duration.large
        easing.type: Easing.InOutQuad
        target: translation
        properties: "x,y"
        to: 0
    }

    Behavior on lengthAppletPadding {
        NumberAnimation {
            duration: 0.8 * appletItem.animations.duration.proposed
            easing.type: Easing.OutCubic
        }
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

        var inThicknessNeutralArea = !wrapperContainsMouse && (appletItem.metrics.margin.screenEdge>0);
        var appletNeutralAreaEnabled = !(inThicknessNeutralArea && root.dragActiveWindowEnabled);

        if (appletItemContainsMouse && !wrapperContainsMouse && appletNeutralAreaEnabled) {
            //console.log("PASSED");
            latteView.extendedInterface.toggleAppletExpanded(applet.id);
        } else {
            //console.log("REJECTED");
        }
    }

    function checkIndex(){
        index = -1;

        for(var i=0; i<appletItem.layouter.startLayout.count; ++i){
            var child = layoutsContainer.startLayout.children[i];
            if (child === appletItem){
                index = layoutsContainer.startLayout.beginIndex + i;
                break;
            }
        }

        for(var i=0; i<appletItem.layouter.mainLayout.count; ++i){
            var child = layoutsContainer.mainLayout.children[i];
            if (child === appletItem){
                index = layoutsContainer.mainLayout.beginIndex + i;
                break;
            }
        }

        for(var i=0; i<appletItem.layouter.endLayout.count; ++i){
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

            if( index === layoutsContainer.startLayout.beginIndex + appletItem.layouter.startLayout.count - 1
                    || index===layoutsContainer.mainLayout.beginIndex + appletItem.layouter.mainLayout.count - 1
                    || index === layoutsContainer.endLayout.beginIndex + appletItem.layouter.endLayout.count - 1)
                latteApplet.disableRightSpacer = false;
            else
                latteApplet.disableRightSpacer = true;
        }
    }

    //this functions gets the signal from the plasmoid, it can be used for signal items
    //outside the LatteApplet Plasmoid
    //property int debCounter: 0;

    function sltClearZoom(){
        if (communicator.parabolicEffectIsSupported) {
            communicator.bridge.parabolic.client.sglClearZoom();
        } else {
            restoreAnimation.start();
        }
    }

    function updateParabolicEffectIsSupported(){
        parabolicEffectIsSupportedTimer.start();
    }

    //! Reduce calculations and give the time to applet to adjust to prevent binding loops
    Timer{
        id: parabolicEffectIsSupportedTimer
        interval: 100
        onTriggered: {
            if (wrapper.zoomScale !== 1) {
                return;
            }

            if (appletItem.isLattePlasmoid) {
                appletItem.parabolicEffectIsSupported = true;
                return;
            }

            var maxSize = appletItem.metrics.iconSize + lengthAppletFullMargins;
            var maxForMinimumSize = appletItem.metrics.iconSize + lengthAppletFullMargins;

            if ( isSystray
                    || appletItem.isAutoFillApplet
                    || (((applet && root.isHorizontal && (applet.width > maxSize || applet.Layout.minimumWidth > maxForMinimumSize))
                         || (applet && root.isVertical && (applet.height > maxSize || applet.Layout.minimumHeight > maxForMinimumSize)))
                        && !appletItem.isSpacer
                        && !communicator.canShowOverlaiedLatteIcon) ) {
                appletItem.parabolicEffectIsSupported = false;
            } else {
                appletItem.parabolicEffectIsSupported = true;
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

    onIndexChanged: {
        if (appletItem.latteApplet) {
            root.latteAppletPos = index;
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

    onIsSystrayChanged: {
        updateParabolicEffectIsSupported();
    }

    onLatteAppletChanged: {
        if(appletItem.latteApplet){
            root.latteApplet = appletItem.latteApplet;
            root.latteAppletContainer = appletItem;
            root.latteAppletPos = index;
            appletItem.latteApplet.latteView = root;
            appletItem.latteApplet.forceHidePanel = true;

            appletItem.latteApplet.signalPreviewsShown.connect(slotPreviewsShown);
        }
    }

    onIsAutoFillAppletChanged: updateParabolicEffectIsSupported();

    Component.onCompleted: {
        checkIndex();
        root.updateIndexes.connect(checkIndex);
        root.destroyInternalViewSplitters.connect(slotDestroyInternalViewSplitters);

        parabolic.sglClearZoom.connect(sltClearZoom);
    }

    Component.onDestruction: {
        appletItem.animations.needBothAxis.removeEvent(appletItem);

        if(root.latteAppletPos>=0 && root.latteAppletPos === index){
            root.latteApplet = null;
            root.latteAppletContainer = null;
            root.latteAppletPos = -1;
        }

        root.updateIndexes.disconnect(checkIndex);
        root.destroyInternalViewSplitters.disconnect(slotDestroyInternalViewSplitters);

        parabolic.sglClearZoom.disconnect(sltClearZoom);

        if (appletItem.latteApplet) {
            appletItem.latteApplet.signalPreviewsShown.disconnect(slotPreviewsShown);
        }
    }

    //! Bindings

    Binding {
        //! is used to aboid loop binding warnings on startup
        target: appletItem
        property: "lengthAppletFullMargin"
        when: !communicator.inStartup
        value: lengthAppletPadding + metrics.margin.length;
    }

    //! Connections
    Connections{
        target: appletItem.shortcuts

        onSglActivateEntryAtIndex: {
            if (!appletItem.shortcuts.unifiedGlobalShortcuts) {
                return;
            }

            var visibleIndex = appletItem.indexer.visibleIndex(appletItem.index);

            if (visibleIndex === entryIndex && !communicator.positionShortcutsAreSupported) {
                latteView.extendedInterface.toggleAppletExpanded(applet.id);
            }
        }

        onSglNewInstanceForEntryAtIndex: {
            if (!appletItem.shortcuts.unifiedGlobalShortcuts) {
                return;
            }

            var visibleIndex = appletItem.indexer.visibleIndex(appletItem.index);

            if (visibleIndex === entryIndex && !communicator.positionShortcutsAreSupported) {
                latteView.extendedInterface.toggleAppletExpanded(applet.id);
            }
        }
    }

    Connections {
        id: viewSignalsConnector
        target: root.latteView ? root.latteView : null
        enabled: !appletItem.isLattePlasmoid && !appletItem.isSeparator && !appletItem.isSpacer && !appletItem.isHidden

        property bool pressed: false
        property bool blockWheel: false

        onMousePressed: {
            if (appletItem.containsPos(pos)) {
                viewSignalsConnector.pressed = true;
                var local = appletItem.mapFromItem(root, pos.x, pos.y);

                appletItem.mousePressed(local.x, local.y, button);

                if (button === Qt.LeftButton) {
                    appletItem.activateAppletForNeutralAreas(local);
                }
            }
        }

        onMouseReleased: {
            if (appletItem.containsPos(pos)) {
                viewSignalsConnector.pressed = false;
                var local = appletItem.mapFromItem(root, pos.x, pos.y);
                appletItem.mouseReleased(local.x, local.y, button);
            }
        }

        onWheelScrolled: {
            if (!appletItem.applet || !root.mouseWheelActions || viewSignalsConnector.blockWheel
                    || (root.latteViewIsHidden || root.inSlidingIn || root.inSlidingOut)) {
                return;
            }

            blockWheel = true;
            scrollDelayer.start();

            if (appletItem.containsPos(pos) && root.latteView.extendedInterface.appletIsExpandable(applet.id)) {
                var angle = angleDelta.y / 8;
                var expanded = root.latteView.extendedInterface.appletIsExpanded(applet.id);

                if ((angle > 12 && !expanded) /*positive direction*/
                        || (angle < -12 && expanded) /*negative direction*/) {
                    latteView.extendedInterface.toggleAppletExpanded(applet.id);
                }
            }
        }
    }

    Connections {
        target: root.latteView ? root.latteView.extendedInterface : null
        enabled: !appletItem.isLattePlasmoid && !appletItem.isSeparator && !appletItem.isSpacer && !appletItem.isHidden

        onExpandedAppletStateChanged: {
            if (latteView.extendedInterface.hasExpandedApplet && appletItem.applet) {
                appletItem.isExpanded = latteView.extendedInterface.appletIsExpandable(appletItem.applet.id)
                        && latteView.extendedInterface.appletIsExpanded(appletItem.applet.id);
            } else {
                appletItem.isExpanded = false;
            }
        }
    }
    ///END connections

    //! It is used for any communication needed with the underlying applet
    Communicator.Engine{
        id: _communicator

        //set up the overlayed appletItems and properties for when a overlaiedIconItem must be presented to the user
        //because the plasma widgets specific implementation breaks the Latte experience
        onOverlayLatteIconIsActiveChanged:{
            if (!overlayLatteIconIsActive && applet.opacity===0) {
                applet.opacity = 1;
            } else if (overlayLatteIconIsActive && applet.opacity>0) {
                applet.opacity = 0;

                if (applet.pluginName === "org.kde.plasma.folder") {
                    applet.parent =  wrapper.containerForOverlayIcon;
                    applet.anchors.fill = wrapper.containerForOverlayIcon;
                }
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
                    id: backLevelOptions
                    isBackground: true
                    bridge: indicatorBridge

                    Binding {
                        target: appletItem
                        property: "iconOffsetX"
                        value: backLevelOptions.requested.iconOffsetX
                    }

                    Binding {
                        target: appletItem
                        property: "iconOffsetY"
                        value: backLevelOptions.requested.iconOffsetY
                    }
                }
            }

            ItemWrapper{
                id: _wrapper

                TitleTooltipParent{
                    id: titleTooltipParent
                    metrics: appletItem.metrics
                    parabolic: appletItem.parabolic
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
                        duration: 1.2 * appletItem.animations.duration.proposed
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
                id: shortcutBadgeContainer

                width: {
                    if (root.isHorizontal) {
                        return appletItem.metrics.iconSize * wrapper.zoomScale
                    } else {
                        return badgeThickness;
                    }
                }

                height: {
                    if (root.isHorizontal) {
                        return badgeThickness;
                    } else {
                        return appletItem.metrics.iconSize * wrapper.zoomScale
                    }
                }

                readonly property int badgeThickness: {
                    if (plasmoid.location === PlasmaCore.Types.BottomEdge
                            || plasmoid.location === PlasmaCore.Types.RightEdge) {
                        return ((appletItem.metrics.iconSize + appletItem.metrics.margin.thickness) * wrapper.zoomScale) + appletItem.metrics.margin.screenEdge;
                    }

                    return ((appletItem.metrics.iconSize + appletItem.metrics.margin.thickness) * wrapper.zoomScale);
                }

                ShortcutBadge{
                    anchors.fill: parent
                }

                states:[
                    State{
                        name: "horizontal"
                        when: plasmoid.formFactor === PlasmaCore.Types.Horizontal

                        AnchorChanges{
                            target: shortcutBadgeContainer;
                            anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                            anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                        }
                    },
                    State{
                        name: "vertical"
                        when: plasmoid.formFactor === PlasmaCore.Types.Vertical

                        AnchorChanges{
                            target: shortcutBadgeContainer;
                            anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                            anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                        }
                    }
                ]
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

    Loader {
        id: addingAreaLoader
        width: root.isHorizontal ? parent.width : parent.width - appletItem.metrics.margin.screenEdge
        height: root.isHorizontal ? parent.height - appletItem.metrics.margin.screenEdge : parent.height

        active: isLattePlasmoid

        sourceComponent: LatteComponents.AddingArea{
            id: addingAreaItem
            anchors.fill: parent
            // width: root.isHorizontal ? parent.width : parent.width - appletItem.metrics.margin.screenEdge
            // height: root.isHorizontal ? parent.height - appletItem.metrics.margin.screenEdge : parent.height

            radius: appletItem.metrics.iconSize/10
            opacity: root.addLaunchersMessage ? 1 : 0
            backgroundOpacity: 0.75
            duration: appletItem.animations.speedFactor.current

            title: i18n("Tasks Area")
        }

        //! AddingAreaItem States
        states:[
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: addingAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
                PropertyChanges{
                    target: addingAreaLoader
                    anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: appletItem.metrics.margin.screenEdge;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: addingAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
                PropertyChanges{
                    target: addingAreaLoader
                    anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin: appletItem.metrics.margin.screenEdge;    anchors.bottomMargin: 0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: addingAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
                PropertyChanges{
                    target: addingAreaLoader
                    anchors.leftMargin: appletItem.metrics.margin.screenEdge;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: addingAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
                PropertyChanges{
                    target: addingAreaLoader
                    anchors.leftMargin: 0;    anchors.rightMargin: appletItem.metrics.margin.screenEdge;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            }
        ]
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
            appletItem.parabolic.stopRestoreZoomTimer();

            if (restoreAnimation.running) {
                restoreAnimation.stop();
            }

            if (!(isSeparator || isSpacer)) {
                root.showTooltipLabel(appletItem, applet.title);
            }

            if (originalAppletBehavior || communicator.requires.parabolicEffectLocked || !parabolicEffectIsSupported) {
                return;
            }

            if (root.isHalfShown || (root.latteApplet
                                     && (root.latteApplet.noTasksInAnimation>0 || root.latteApplet.contextMenu))) {
                return;
            }

            if (root.isHorizontal){
                layoutsContainer.currentSpot = mouseX;
                wrapper.calculateParabolicScales(mouseX);
            }
            else{
                layoutsContainer.currentSpot = mouseY;
                wrapper.calculateParabolicScales(mouseY);
            }
        }

        onExited:{
            if (communicator.appletIconItemIsShown()) {
                communicator.setAppletIconItemActive(false);
            }

            root.hideTooltipLabel();

            if (appletItem.parabolic.factor.zoom>1){
                appletItem.parabolic.startRestoreZoomTimer();
            }
        }

        onPositionChanged: {
            if (originalAppletBehavior || !parabolicEffectIsSupported) {
                mouse.accepted = false;
                return;
            }

            if (root.isHalfShown || (root.latteApplet
                                     && (root.latteApplet.noTasksInAnimation>0 || root.latteApplet.contextMenu))) {
                return;
            }

            var rapidMovement = appletItem.parabolic.lastIndex>=0 && Math.abs(appletItem.parabolic.lastIndex-index)>1;

            if (rapidMovement) {
                parabolic.setDirectRenderingEnabled(true);
            }

            if( ((wrapper.zoomScale == 1 || wrapper.zoomScale === appletItem.parabolic.factor.zoom) && !parabolic.directRenderingEnabled) || parabolic.directRenderingEnabled) {
                if (root.isHorizontal){
                    var step = Math.abs(layoutsContainer.currentSpot-mouse.x);
                    if (step >= appletItem.animations.hoverPixelSensitivity){
                        layoutsContainer.currentSpot = mouse.x;

                        wrapper.calculateParabolicScales(mouse.x);
                    }
                }
                else{
                    var step = Math.abs(layoutsContainer.currentSpot-mouse.y);
                    if (step >= appletItem.animations.hoverPixelSensitivity){
                        layoutsContainer.currentSpot = mouse.y;

                        wrapper.calculateParabolicScales(mouse.y);
                    }
                }
            }

            mouse.accepted = false;
        }

        //! these are needed in order for these events to be really forwarded underneath
        //! otherwise there were applets that did not receive them e.g. lock/logout applet
        //! when parabolic effect was used
        onPressed: mouse.accepted = false;
        onReleased: mouse.accepted = false;
    }

    //! Debug Elements
    Loader{
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        active: root.debugModeLayouter
        sourceComponent: Debug.Tag{
            label.text: (root.isHorizontal ? appletItem.width : appletItem.height) + labeltext
            label.color: appletItem.isAutoFillApplet ? "green" : "white"

            readonly property string labeltext: {
                if (appletItem.isAutoFillApplet) {
                    return " / max_fill:"+appletItem.maxAutoFillLength + " / min_fill:"+appletItem.minAutoFillLength;
                }

                return "";
            }
        }
    }

    //! A timer is needed in order to handle also touchpads that probably
    //! send too many signals very fast. This way the signals per sec are limited.
    //! The user needs to have a steady normal scroll in order to not
    //! notice a annoying delay
    Timer{
        id: scrollDelayer
        interval: 500

        onTriggered: viewSignalsConnector.blockWheel = false;
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
            duration: 3 * appletItem.animationTime
            easing.type: Easing.InCubic
        }
    }


    /////Clicked Animation/////
    SequentialAnimation{
        id: clickedAnimation
        alwaysRunToEnd: true
        running: appletItem.isSquare && !originalAppletBehavior && appletItem.pressed
                 && (appletItem.animations.speedFactor.current > 0) && !indicators.info.providesClickedAnimation

        ParallelAnimation{
            PropertyAnimation {
                target: wrapper.clickedEffect
                property: "brightness"
                to: -0.35
                duration: appletItem.animations.duration.large
                easing.type: Easing.OutQuad
            }
        }
        ParallelAnimation{
            PropertyAnimation {
                target: wrapper.clickedEffect
                property: "brightness"
                to: 0
                duration: appletItem.animations.duration.large
                easing.type: Easing.OutQuad
            }
        }
    }
    //END animations
}
