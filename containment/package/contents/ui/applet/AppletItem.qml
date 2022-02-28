/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

import org.kde.latte.abilities.items 0.1 as AbilityItem

import "colorizer" as Colorizer
import "communicator" as Communicator
import "../debugger" as Debugger

Item {
    id: appletItem
    width: isInternalViewSplitter && !root.inConfigureAppletsMode ? 0 : computeWidth
    height: isInternalViewSplitter && !root.inConfigureAppletsMode ? 0 : computeHeight

    //any applets that exceed their limits should not take events from their surrounding applets
    //clip: !isSeparator && !parabolicAreaLoader.active

    signal mousePressed(int x, int y, int button);
    signal mouseReleased(int x, int y, int button);

    property bool animationsEnabled: true
    property bool indexerIsSupported: communicator.indexerIsSupported
    property bool parabolicEffectIsSupported: true
    property bool canShowAppletNumberBadge: !indexerIsSupported
                                            && !isSeparator
                                            && !isMarginsAreaSeparator
                                            && !isHidden
                                            && !isSpacer
                                            && !isInternalViewSplitter

    readonly property bool canFillScreenEdge: communicator.requires.screenEdgeMarginSupported || communicator.indexerIsSupported
    readonly property bool canFillThickness: applet && applet.hasOwnProperty("constraintHints")
                                             && ((applet.constraintHints & PlasmaCore.Types.CanFillArea) === PlasmaCore.Types.CanFillArea);

    readonly property bool isMarginsAreaSeparator: applet && applet.hasOwnProperty("constraintHints")
                                                   && ((applet.constraintHints & PlasmaCore.Types.MarginAreasSeparator) === PlasmaCore.Types.MarginAreasSeparator);

    readonly property color highlightColor: theme.buttonFocusColor

    //! Fill Applet(s)
    property bool inFillCalculations: false //temp record, is used in calculations for fillWidth,fillHeight applets
    property bool isAutoFillApplet:  isRequestingFill
    property bool isParabolicEdgeSpacer: false

    property bool isRequestingFill: {
        if (!applet || !applet.Layout) {
            return false;
        }

        if ((root.isHorizontal && applet.Layout.fillWidth===true)
                || (root.isVertical && applet.Layout.fillHeight===true)) {
            return !isHidden;
        }

        return false;
    }

    property int maxAutoFillLength: -1 //it is used in calculations for fillWidth,fillHeight applets
    property int minAutoFillLength: -1 //it is used in calculations for fillWidth,fillHeight applets

    readonly property bool inConfigureAppletsDragging: root.dragOverlay
                                                       && root.dragOverlay.currentApplet
                                                       && root.dragOverlay.pressed

    property bool appletBlocksColorizing: !communicator.requires.latteSideColoringEnabled || communicator.indexerIsSupported
    property bool appletBlocksParabolicEffect: communicator.requires.parabolicEffectLocked
    readonly property bool lockZoom: !parabolicEffectIsSupported
                                     || appletBlocksParabolicEffect
                                     || (fastLayoutManager && applet && (fastLayoutManager.lockedZoomApplets.indexOf(applet.id)>=0))
    readonly property bool userBlocksColorizing: appletBlocksColorizing
                                                 || (fastLayoutManager && applet && (fastLayoutManager.userBlocksColorizingApplets.indexOf(applet.id)>=0))

    property bool isActive: (isExpanded
                             && !appletItem.communicator.indexerIsSupported
                             && applet.pluginName !== "org.kde.activeWindowControl"
                             && applet.pluginName !== "org.kde.plasma.appmenu")

    property bool isExpanded: false

    property bool isScheduledForDestruction: (fastLayoutManager && applet && fastLayoutManager.appletsInScheduledDestruction.indexOf(applet.id)>=0)
    property bool isHidden: (!root.inConfigureAppletsMode && ((applet && applet.status === PlasmaCore.Types.HiddenStatus ) || isInternalViewSplitter)) || isScheduledForDestruction
    property bool isInternalViewSplitter: (internalSplitterId > 0)
    property bool isZoomed: false
    property bool isPlaceHolder: false
    property bool isPressed: viewSignalsConnector.pressed
    property bool isSeparator: applet && (applet.pluginName === "audoban.applet.separator"
                                          || applet.pluginName === "org.kde.latte.separator")
    property bool isSpacer: applet && (applet.pluginName === "org.kde.latte.spacer")
    property bool isSystray: applet && (applet.pluginName === "org.kde.plasma.systemtray" || applet.pluginName === "org.nomad.systemtray" )

    property bool firstChildOfStartLayout: index === appletItem.layouter.startLayout.firstVisibleIndex
    property bool firstChildOfMainLayout: index === appletItem.layouter.mainLayout.firstVisibleIndex
    property bool lastChildOfMainLayout: index === appletItem.layouter.mainLayout.lastVisibleIndex
    property bool lastChildOfEndLayout: index === appletItem.layouter.endLayout.lastVisibleIndex

    readonly property bool atScreenEdge: {
        if (appletItem.myView.alignment === LatteCore.Types.Center) {
            return false;
        }

        if (appletItem.myView.alignment === LatteCore.Types.Justify) {
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

        if (appletItem.myView.alignment === LatteCore.Types.Left && plasmoid.configuration.offset===0) {
            //! Left case
            return firstChildOfMainLayout;
        } else if (appletItem.myView.alignment === LatteCore.Types.Right && plasmoid.configuration.offset===0) {
            //! Right case
            return lastChildOfMainLayout;
        }

        if (appletItem.myView.alignment === LatteCore.Types.Top && plasmoid.configuration.offset===0) {
            return firstChildOfMainLayout && latteView && latteView.y === latteView.screenGeometry.y;
        } else if (appletItem.myView.alignment === LatteCore.Types.Bottom && plasmoid.configuration.offset===0) {
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

    readonly property bool acceptMouseEvents: applet
                                              && !indexerIsSupported
                                              && !originalAppletBehavior
                                              && parabolicEffectIsSupported
                                              && !appletItem.isSeparator
                                              && !communicator.requires.parabolicEffectLocked

    //! This property is an effort in order to group behaviors into one property. This property is responsible to enable/disable
    //! Applets OnTop MouseArea which is used for ParabolicEffect and ThinTooltips. For Latte panels things
    //! are pretty straight, the original plasma behavior is replicated so parabolic effect and thin tooltips are disabled.
    //! For Latte docks things are a bit more complicated. Applets that can not support parabolic effect inside docks
    //! are presenting their original plasma behavior and also applets that even though can be zoomed user has
    //! chosed to lock its parabolic effect.
    readonly property bool originalAppletBehavior: root.behaveAsPlasmaPanel
                                                   || !parabolicEffectIsSupported
                                                   || (root.behaveAsDockWithMask && !parabolicEffectIsSupported)
                                                   || (root.behaveAsDockWithMask && parabolicEffectIsSupported && lockZoom)

    readonly property bool isIndicatorDrawn: indicatorBackLayer.level.isDrawn
    readonly property bool isSquare: parabolicEffectIsSupported
    readonly property bool screenEdgeMarginSupported: communicator.requires.screenEdgeMarginSupported || communicator.indexerIsSupported

    property int animationTime: appletItem.animations.speedFactor.normal * (1.2*appletItem.animations.duration.small)
    property int index: -1
    property int maxWidth: root.isHorizontal ? root.height : root.width
    property int maxHeight: root.isHorizontal ? root.height : root.width
    property int internalSplitterId: 0

    property int previousIndex: -1
    property int spacersMaxSize: Math.max(0,Math.ceil(0.55 * metrics.iconSize) - metrics.totals.lengthEdges)
    property int status: applet ? applet.status : -1

    //! some metrics
    readonly property int appletMinimumLength: _wrapper.appletMinimumLength
    readonly property int appletPreferredLength: _wrapper.appletPreferredLength
    readonly property int appletMaximumLength: _wrapper.appletMaximumLength

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

    readonly property bool inMarginsArea: {
        if (isMarginsAreaSeparator || appletItem.indexer.marginsAreaSeparators.length === 0) {
            return false;
        }

        var tailMarginsAreaSeparatorsCount = 0;

        for(var i=0; i<appletItem.indexer.marginsAreaSeparators.length; ++i) {
            if (appletItem.indexer.marginsAreaSeparators[i] < index) {
                tailMarginsAreaSeparatorsCount++;
            }
        }

        //! even number of margins area separators before this applet means that this applet
        //! is inside margins area
        return (tailMarginsAreaSeparatorsCount % 2 === 1);
    }

    //! local margins
    readonly property bool parabolicEffectMarginsEnabled: appletItem.parabolic.factor.zoom>1 && !originalAppletBehavior && !communicator.parabolicEffectIsSupported

    property int lengthAppletPadding:{
        if (!isIndicatorDrawn) {
            return 0;
        }

        return metrics.fraction.lengthAppletPadding === -1 || parabolicEffectMarginsEnabled ? metrics.padding.length : metrics.padding.lengthApplet;
    }

    property int lengthAppletFullMargin: 0
    property int lengthAppletFullMargins: 2 * lengthAppletFullMargin

    property int internalWidthMargins: root.isVertical ? metrics.totals.thicknessEdges : 2 * lengthAppletPadding
    property int internalHeightMargins: root.isHorizontal ? root.metrics.totals.thicknessEdges : 2 * lengthAppletPadding

    readonly property string pluginName: isInternalViewSplitter ? "org.kde.latte.splitter" : (applet ? applet.pluginName : "")

    //! are set by the indicator
    readonly property int iconOffsetX: indicatorBackLayer.level.requested.iconOffsetX
    readonly property int iconOffsetY: indicatorBackLayer.level.requested.iconOffsetY
    readonly property int iconTransformOrigin: indicatorBackLayer.level.requested.iconTransformOrigin
    readonly property real iconOpacity: indicatorBackLayer.level.requested.iconOpacity
    readonly property real iconRotation: indicatorBackLayer.level.requested.iconRotation
    readonly property real iconScale: indicatorBackLayer.level.requested.iconScale

    property real computeWidth: root.isVertical ? wrapper.width :
                                                  hiddenSpacerLeft.width+wrapper.width+hiddenSpacerRight.width

    property real computeHeight: root.isVertical ? hiddenSpacerLeft.height + wrapper.height + hiddenSpacerRight.height :
                                                   wrapper.height

    property Item applet: null
    property Item latteStyleApplet: applet && ((applet.pluginName === "org.kde.latte.spacer") || (applet.pluginName === "org.kde.latte.separator")) ?
                                        (applet.children[0] ? applet.children[0] : null) : null

    property Item appletWrapper: wrapper.wrapperContainer

    property Item tooltipVisualParent: titleTooltipParent

    readonly property alias communicator: _communicator
    readonly property alias wrapper: _wrapper
    readonly property alias restoreAnimation: _restoreAnimation

    property Item animations: null
    property Item debug: null
    property Item environment: null
    property Item indexer: null
    property Item indicators: null
    property Item launchers: null
    property Item layouter: null
    property Item layouts: null
    property Item metrics: null
    property Item myView: null
    property Item parabolic: null
    property Item shortcuts: null
    property Item thinTooltip: null
    property Item userRequests: null

    property bool containsMouse: parabolicAreaLoader.active && parabolicAreaLoader.item.containsMouse
    property bool pressed: viewSignalsConnector.pressed


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
        } else if (inDraggingOverAppletOrOutOfContainment) {
            return;
        }

        var draggingAppletInConfigure = root.dragOverlay && root.dragOverlay.currentApplet;
        var isCurrentAppletInDragging = draggingAppletInConfigure && (root.dragOverlay.currentApplet === appletItem);
        var dropApplet = root.dragInfo.entered && root.dragInfo.isPlasmoid

        if ((isCurrentAppletInDragging || !draggingAppletInConfigure) && !dropApplet) {
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
        } else if (inDraggingOverAppletOrOutOfContainment) {
            return;
        }

        var draggingAppletInConfigure = root.dragOverlay && root.dragOverlay.currentApplet;
        var isCurrentAppletInDragging = draggingAppletInConfigure && (root.dragOverlay.currentApplet === appletItem);
        var dropApplet = root.dragInfo.entered && root.dragInfo.isPlasmoid

        if ((isCurrentAppletInDragging || !draggingAppletInConfigure) && !dropApplet) {
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

        var startAppletIndex = -1;
        for(var i=0; i<appletItem.layouter.startLayout.count; ++i){
            var child = layoutsContainer.startLayout.children[i];
            if (child.isParabolicEdgeSpacer || child.isInternalViewSplitter) {
                continue;
            }

            startAppletIndex++;
            if (child === appletItem){
                index = layoutsContainer.startLayout.beginIndex + startAppletIndex;
                break;
            }
        }

        var mainAppletIndex = -1;
        for(var i=0; i<appletItem.layouter.mainLayout.count; ++i){
            var child = layoutsContainer.mainLayout.children[i];
            if (child.isParabolicEdgeSpacer || child.isInternalViewSplitter) {
                continue;
            }

            mainAppletIndex++;
            if (child === appletItem){
                index = layoutsContainer.mainLayout.beginIndex + mainAppletIndex;
                break;
            }
        }

        var endAppletIndex = -1;
        for(var i=0; i<appletItem.layouter.endLayout.count; ++i){
            var child = layoutsContainer.endLayout.children[i];
            if (child.isParabolicEdgeSpacer || child.isInternalViewSplitter) {
                continue;
            }

            endAppletIndex++;
            if (child === appletItem){
                //create a very high index in order to not need to exchange hovering messages
                //between layoutsContainer.mainLayout and layoutsContainer.endLayout
                index = layoutsContainer.endLayout.beginIndex + endAppletIndex;
                break;
            }
        }
    }

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

            if (appletItem.communicator.indexerIsSupported) {
                appletItem.parabolicEffectIsSupported = true;
                return;
            }

            var maxSize = 1.5 * appletItem.metrics.iconSize;
            var maxForMinimumSize = 1.5 * appletItem.metrics.iconSize;

            if ( isSystray
                    || appletItem.isAutoFillApplet
                    || (((applet && root.isHorizontal && (applet.width > maxSize || applet.Layout.minimumWidth > maxForMinimumSize))
                         || (applet && root.isVertical && (applet.height > maxSize || applet.Layout.minimumHeight > maxForMinimumSize)))
                        && !appletItem.isSpacer) ) {
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
        if (index>-1) {
            previousIndex = index;
        }
    }

    onIsSystrayChanged: {
        updateParabolicEffectIsSupported();
    }

    onIsAutoFillAppletChanged: updateParabolicEffectIsSupported();
    onParentChanged: checkIndex()

    Component.onCompleted: {
        checkIndex();
        root.updateIndexes.connect(checkIndex);
        root.destroyInternalViewSplitters.connect(slotDestroyInternalViewSplitters);

        parabolic.sglClearZoom.connect(sltClearZoom);
    }

    Component.onDestruction: {
        appletItem.animations.needBothAxis.removeEvent(appletItem);

        root.updateIndexes.disconnect(checkIndex);
        root.destroyInternalViewSplitters.disconnect(slotDestroyInternalViewSplitters);

        parabolic.sglClearZoom.disconnect(sltClearZoom);
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
        enabled: !appletItem.indexerIsSupported && !appletItem.isSeparator && !appletItem.isSpacer && !appletItem.isHidden

        property bool pressed: false
        property bool blockWheel: false

        onMousePressed: {
            if (appletItem.containsPos(pos)) {
                viewSignalsConnector.pressed = true;
                var local = appletItem.mapFromItem(root, pos.x, pos.y);

                appletItem.mousePressed(local.x, local.y, button);
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
            if (!appletItem.applet || !root.mouseWheelActions || viewSignalsConnector.blockWheel || !appletItem.myView.isShownFully) {
                return;
            }

            blockWheel = true;
            scrollDelayer.start();

            if (appletItem.containsPos(pos)
                    && (root.latteView.extendedInterface.appletIsExpandable(applet.id)
                        || (root.latteView.extendedInterface.appletIsActivationTogglesExpanded(applet.id)))) {
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
        enabled: !appletItem.indexerIsSupported && !appletItem.isSeparator && !appletItem.isSpacer && !appletItem.isHidden

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
    }

    /*  Rectangle{
        anchors.fill: parent
        color: "transparent"
        border.color: "green"
        border.width: 1
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

            AbilityItem.IndicatorObject {
                id: appletIndicatorObj
                animations: appletItem.animations
                metrics: appletItem.metrics
                host: appletItem.indicators

                isApplet: true

                isActive: appletItem.isActive
                isHovered: appletItem.containsMouse
                isPressed: appletItem.isPressed
                isSquare: appletItem.isSquare

                hasActive: appletItem.isActive

                scaleFactor: appletItem.wrapper.zoomScale
                panelOpacity: root.background.currentOpacity
                shadowColor: appletItem.myView.itemShadow.shadowSolidColor

                palette: colorizerManager.applyTheme

                //!icon colors
                iconBackgroundColor: appletItem.wrapper.overlayIconLoader.backgroundColor
                iconGlowColor: appletItem.wrapper.overlayIconLoader.glowColor
            }

            //! InConfigureApplets visual paddings
            Loader {
                anchors.fill: _wrapper
                active: root.inConfigureAppletsMode && !isInternalViewSplitter
                sourceComponent: PaddingsInConfigureApplets{
                    color: appletItem.highlightColor
                }
            }

            //! Indicator Back Layer
            IndicatorLevel{
                id: indicatorBackLayer
                level.isBackground: true
                level.indicator: appletIndicatorObj

                Loader{
                    anchors.fill: parent
                    active: appletItem.debug.graphicsEnabled && parent.active
                    sourceComponent: Rectangle{
                        color: "transparent"
                        border.width: 1
                        border.color: "purple"
                        opacity: 0.4
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
            IndicatorLevel{
                id: indicatorFrontLayer
                level.isForeground: true
                level.indicator: appletIndicatorObj
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
                        var marginthickness = appletItem.metrics.margin.tailThickness * wrapper.zoomMarginScale;
                        return (appletItem.metrics.iconSize * wrapper.zoomScale) + marginthickness + appletItem.metrics.margin.screenEdge;
                    }

                    var marginthickness = appletItem.metrics.margin.headThickness * wrapper.zoomMarginScale;
                    return (appletItem.metrics.iconSize * wrapper.zoomScale) + marginthickness;
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
        HiddenSpacer{id: hiddenSpacerRight; isRightSpacer: true}
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
        id: parabolicAreaLoader
        width: root.isHorizontal ? appletItem.width : appletItem.metrics.mask.thickness.zoomedForItems
        height: root.isHorizontal ? appletItem.metrics.mask.thickness.zoomedForItems : appletItem.height
        //! must be enabled even for applets that are hidden in order to forward
        //! parabolic effect messages properly to surrounding plasma applets
        active: isParabolicEnabled || isThinTooltipEnabled || hasParabolicMessagesEnabled

        //! in hidden state applets must pass on parabolic messages to neighbours
        readonly property bool isParabolicEnabled: appletItem.parabolic.isEnabled && !lockZoom
        readonly property bool isThinTooltipEnabled: appletItem.thinTooltip.isEnabled && !isHidden
        readonly property bool hasParabolicMessagesEnabled: appletItem.parabolic.isEnabled && (!lockZoom || isSeparator || isMarginsAreaSeparator || isHidden)

        sourceComponent: ParabolicArea{}

        states:[
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target: parabolicAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: parabolicAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: parabolicAreaLoader
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
            },
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: parabolicAreaLoader
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
            }
        ]
    }

    //! Debug Elements
    Loader{
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        active: appletItem.debug.layouterEnabled
        sourceComponent: Debugger.Tag{
            label.text: (root.isHorizontal ? appletItem.width : appletItem.height) + labeltext
            label.color: appletItem.isAutoFillApplet ? "green" : "white"

            readonly property string labeltext: {
                if (appletItem.isAutoFillApplet) {
                    return " || max_fill:"+appletItem.maxAutoFillLength + " / min_fill:"+appletItem.minAutoFillLength;
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
        id: _restoreAnimation

        PropertyAnimation {
            target: wrapper
            property: "zoomScale"
            to: 1
            duration: 3 * appletItem.animationTime
            easing.type: Easing.InCubic
        }
    }
}
