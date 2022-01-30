/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

import "../../code/MathTools.js" as MathTools

Item{
    id: wrapper
    width: root.isHorizontal ? length : thickness
    height: root.isHorizontal ? thickness : length

    readonly property real length: {
        if (appletItem.isScheduledForDestruction) {
            return 0;
        }

        if (appletItem.isInternalViewSplitter) {
            if (!root.inConfigureAppletsMode) {
                return 0;
            } else {
                return appletItem.inConfigureAppletsDragging && root.dragOverlay.currentApplet.isInternalViewSplitter && root.dragOverlay.currentApplet === appletItem ?
                            appletMinimumLength : internalSplitterComputedLength;
            }
        }

        if ((isSeparator && appletItem.parabolic.isEnabled)
                || (isMarginsAreaSeparator && (!root.inConfigureAppletsMode || appletItem.parabolic.isEnabled))) {
            return -1;
        }

        if (appletItem.isAutoFillApplet) {
            if (appletItem.inConfigureAppletsDragging && root.dragOverlay.currentApplet === appletItem) {
                return root.dragOverlay.draggedPlaceHolder.length;
            }

            //! dont miss 1pixel gap when the two internal splitters are met inConfigure and Justify mode
            //! a good example is a big vertical right sidebar to observe that gap
            if (appletItem.layouter.maxMetricsInHigherPriority) {                
                return isInternalViewSplitter ? appletItem.maxAutoFillLength + 1 : appletItem.maxAutoFillLength;
            }

            var result = Math.max(appletItem.minAutoFillLength,Math.min(appletPreferredLength,appletItem.maxAutoFillLength));
            return isInternalViewSplitter? result + 1 : result;
        }

        return root.inConfigureAppletsMode ? Math.max(Math.min(appletItem.metrics.iconSize, root.minAppletLengthInConfigure), scaledLength) : scaledLength;
    }

    readonly property real thickness: {
        if (appletItem.isInternalViewSplitter && !root.inConfigureAppletsMode) {
            return 0;
        }

        return communicator.parabolicEffectIsSupported ? appletPreferredThickness : scaledThickness + appletItem.metrics.margin.screenEdge;
    }

    opacity: appletColorizer.mustBeShown && appletItem.environment.isGraphicsSystemAccelerated ? 0 : 1

    property bool disableLengthScale: false
    property bool disableThicknessScale: false

    property bool editMode: root.inConfigureAppletsMode

    property real appletWidth: applet ?  applet.width : -1
    property real appletHeight: applet ?  applet.height : -1

    property real appletMinimumWidth: applet && applet.Layout ?  applet.Layout.minimumWidth : -1
    property real appletMinimumHeight: applet && applet.Layout ? applet.Layout.minimumHeight : -1

    property real appletPreferredWidth: applet && applet.Layout ?  applet.Layout.preferredWidth : -1
    property int appletPreferredHeight: applet && applet.Layout ?  applet.Layout.preferredHeight : -1

    property real appletMaximumWidth: applet && applet.Layout ?  applet.Layout.maximumWidth : -1
    property real appletMaximumHeight: applet && applet.Layout ?  applet.Layout.maximumHeight : -1

    readonly property int proposedItemThickness: appletItem.inMarginsArea && !appletItem.canFillThickness ?
                                                     Math.max(16,appletItem.metrics.marginsArea.iconSize) :
                                                     appletItem.metrics.iconSize

    readonly property real appletLength: root.isHorizontal ? appletWidth : appletHeight
    readonly property real appletThickness: root.isHorizontal ? appletHeight : appletWidth

    readonly property int appletMinimumLength : {
        if (isInternalViewSplitter) {
            return root.maxJustifySplitterSize;
        }

        return root.isHorizontal ? appletMinimumWidth : appletMinimumHeight
    }

    readonly property real appletPreferredLength: {
        if (isInternalViewSplitter) {
            return appletMinimumLength;
        }
        return root.isHorizontal ? appletPreferredWidth : appletPreferredHeight;
    }

    readonly property real appletMaximumLength: {
        if (isInternalViewSplitter) {
            return Infinity;
        }

        root.isHorizontal ? appletMaximumWidth : appletMaximumHeight;
    }

    readonly property real appletMinimumThickness: root.isHorizontal ? appletMinimumHeight : appletMinimumWidth
    readonly property real appletPreferredThickness: root.isHorizontal ? appletPreferredHeight : appletPreferredWidth
    readonly property real appletMaximumThickness: root.isHorizontal ? appletMaximumHeight : appletMaximumWidth

    property int iconSize: appletItem.metrics.iconSize

    property int marginsThickness: {
        if (appletItem.inMarginsArea) {
            return appletItem.metrics.marginsArea.thicknessEdges;
        }

        return appletItem.metrics.totals.thicknessEdges;
    }

    property int marginsLength: 0   //Fitt's Law, through Binding to avoid Binding loops

    property int localLengthMargins: isSeparator
                                     || appletItem.isMarginsAreaSeparator
                                     || !communicator.requires.lengthMarginsEnabled
                                     || communicator.indexerIsSupported
                                     || isInternalViewSplitter
                                     ? 0 : appletItem.lengthAppletFullMargins

    property real scaledLength: zoomScaleLength * (layoutLength + marginsLength)
    property real scaledThickness: zoomScaleThickness * layoutThickness + (zoomMarginScale*marginsThickness)

    property real zoomScaleLength: disableLengthScale ? 1 : zoomScale
    property real zoomScaleThickness: disableThicknessScale ? 1 : zoomScale

    property real layoutLength: 0
    property real layoutThickness: 0

    property real zoomScale: 1
    readonly property real zoomMarginScale: 1 + (zoomScale - 1) * parabolic.factor.marginThicknessZoomInPercentage

    readonly property alias headThicknessMargin: _wrapperContainer.headThicknessMargin
    readonly property alias tailThicknessMargin: _wrapperContainer.tailThicknessMargin
    readonly property alias appletScreenMargin: _wrapperContainer.appliedEdgeMargin

    property int index: appletItem.index

    property Item wrapperContainer: _wrapperContainer
    property Item overlayIconLoader: _overlayIconLoader

    readonly property int internalSplitterComputedLength: {
        if (!appletItem.isInternalViewSplitter) {
            return 0;
        }

        var parentLayoutLength = 0;
        var parentTwinLayoutLength = 0;

        if (appletItem.parent === layoutsContainer.startLayout) {
            parentLayoutLength = appletItem.layouter.startLayout.lengthWithoutSplitters;
            parentTwinLayoutLength = appletItem.layouter.endLayout.lengthWithoutSplitters;
        } else if (appletItem.parent === layoutsContainer.endLayout) {
            parentLayoutLength = appletItem.layouter.endLayout.lengthWithoutSplitters;
            parentTwinLayoutLength = appletItem.layouter.startLayout.lengthWithoutSplitters;
        } else {
            return 0;
        }

        var parentLayoutCenter = (appletItem.layouter.maxLength - layoutsContainer.mainLayout.length)/2;
        var twinLayoutExceededCenter = Math.max(0, (parentTwinLayoutLength + root.maxJustifySplitterSize) - parentLayoutCenter);
        var availableLength = Math.max(0, parentLayoutCenter - twinLayoutExceededCenter) + 1; //add one pixel in order to not leave a gap inConfigureAppletsMode

        return Math.max(root.maxJustifySplitterSize, availableLength - parentLayoutLength);
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 0.8 * appletItem.animations.duration.proposed
            easing.type: Easing.OutCubic
        }
    }

    // property int pHeight: applet ? applet.Layout.preferredHeight : -10

    /*function debugLayouts(){
        if(applet && applet.pluginName==="org.kde.plasma.systemtray"){
            console.log("---------- "+ applet.pluginName +" ----------");
            console.log("MinW "+applet.Layout.minimumWidth);
            console.log("PW "+applet.Layout.preferredWidth);
            console.log("MaxW "+applet.Layout.maximumWidth);
            console.log("FillW "+applet.Layout.fillWidth);
            console.log("-----");
            console.log("MinH "+applet.Layout.minimumHeight);
            console.log("PH "+applet.Layout.preferredHeight);
            console.log("MaxH "+applet.Layout.maximumHeight);
            console.log("FillH "+applet.Layout.fillHeight);
            console.log("-----");
            console.log("Real Applet Width: "+applet.width);
            console.log("Real Applet Height: "+applet.height);
            console.log("-----");
            console.log("Real Wrapper Width: "+wrapper.width);
            console.log("Real Wrapper Height: "+wrapper.height);
            console.log("-----");
            console.log("Can be hovered: " + parabolicEffectIsSupported);
            console.log("Icon size: " + appletItem.metrics.iconSize);
            console.log("Thick Margins: " + appletItem.metrics.totals.thicknessEdges);
            console.log("Intern. Margins: " + (appletItem.metrics.padding.length * 2));
            console.log("Intern. Margins: " + (appletItem.metrics.margin.length * 2));
            console.log("Max hovered criteria: " + (appletItem.metrics.iconSize + metrics.totals.thicknessEdges));
            console.log("-----");
            console.log("LayoutLength: " + layoutLength);
            console.log("LayoutThickness: " + layoutThickness);
        }
    }*/

    onAppletLengthChanged: {
        if(zoomScale === 1) {
            appletItem.updateParabolicEffectIsSupported();
        }
    }

    onAppletThicknessChanged: {
        if(zoomScale === 1) {
            appletItem.updateParabolicEffectIsSupported();
        }
    }

    onAppletMinimumLengthChanged: {
        if(zoomScale === 1) {
            appletItem.updateParabolicEffectIsSupported();
        }

        updateAutoFillLength();
    }

    onAppletMinimumThicknessChanged: {
        if(zoomScale === 1) {
            appletItem.updateParabolicEffectIsSupported();
        }
    }

    onAppletPreferredLengthChanged: {
        updateAutoFillLength();
    }

    onAppletMaximumLengthChanged: {
        updateAutoFillLength();
    }

    onZoomScaleChanged: {
        if ((zoomScale === appletItem.parabolic.factor.zoom) && !appletItem.parabolic.directRenderingEnabled) {
            appletItem.parabolic.setDirectRenderingEnabled(true);
        }

        if ((zoomScale > 1) && !appletItem.isZoomed) {
            appletItem.isZoomed = true;
            appletItem.animations.needBothAxis.addEvent(appletItem);
        } else if (zoomScale == 1) {
            appletItem.isZoomed = false;
            appletItem.animations.needBothAxis.removeEvent(appletItem);
        }
    }

    Binding {
        target: wrapper
        property: "layoutThickness"
        when: latteView && (wrapper.zoomScale === 1 || communicator.parabolicEffectIsSupported)
        value: {
            if (appletItem.isInternalViewSplitter){
                return !root.inConfigureAppletsMode ? 0 : proposedItemThickness;
            }

            // avoid binding loops on startup
            if (communicator.parabolicEffectIsSupported && !communicator.inStartup) {
                return Math.min(appletPreferredThickness, appletItem.metrics.totals.thickness);
            }

            return proposedItemThickness;
        }
    }

    Binding {
        target: wrapper
        property: "layoutLength"
        when: latteView && !appletItem.isAutoFillApplet && (wrapper.zoomScale === 1)
        value: {
            if (applet && ( appletMaximumLength < appletItem.metrics.iconSize
                           || appletPreferredLength > appletItem.metrics.iconSize
                           || appletItem.originalAppletBehavior)) {

                //this way improves performance, probably because during animation the preferred sizes update a lot
                if (appletMaximumLength>=0 && appletMaximumLength < appletItem.metrics.iconSize){
                    return appletMaximumLength;
                } else if (appletMinimumLength > appletItem.metrics.iconSize){
                    return appletMinimumLength;
                } else if ((appletPreferredLength > appletItem.metrics.iconSize)
                           || (appletItem.originalAppletBehavior && appletPreferredLength > 0)){
                    return appletPreferredLength;
                }
            }

            return appletItem.metrics.iconSize;
        }
    }

    Binding {
        target: wrapper
        property: "disableLengthScale"
        when: latteView && !(appletItem.isAutoFillApplet || appletItem.indexerIsSupported)
        value: {
            var blockParabolicEffectInLength = false;

            if (communicator.parabolicEffectIsSupported) {
                return true;
            }

            if (appletItem.isInternalViewSplitter){
                return false;
            } else {
                if(applet && (appletMinimumLength > appletItem.metrics.iconSize) && !appletItem.parabolicEffectIsSupported){
                    return (wrapper.zoomScale === 1);
                } //it is used for plasmoids that need to scale only one axis... e.g. the Weather Plasmoid
                else if(applet
                        && ( appletMaximumLength < appletItem.metrics.iconSize
                            || appletPreferredLength > appletItem.metrics.iconSize
                            || appletItem.originalAppletBehavior)) {

                    //this way improves performance, probably because during animation the preferred sizes update a lot
                    if (appletMaximumLength>0 && appletMaximumLength < appletItem.metrics.iconSize){
                        return false;
                    } else if (appletMinimumLength > appletItem.metrics.iconSize){
                        return (wrapper.zoomScale === 1);
                    } else if ((appletPreferredLength > appletItem.metrics.iconSize)
                               || (appletItem.originalAppletBehavior && appletPreferredLength > 0 )){
                        return (wrapper.zoomScale === 1);
                    }
                }
            }

            return false;
        }
    }

    Binding {
        target: wrapper
        property: "marginsLength"
        when: latteView && (!root.inStartup || visibilityManager.inRelocationHiding)
        value: localLengthMargins
    }

    function updateAutoFillLength() {
        if (appletItem.isAutoFillApplet) {
            appletItem.layouter.updateSizeForAppletsInFill();
        }
    }

    //! Applet Highlight when requested
    Loader {
        id: visualIndicator
        anchors.fill: parent
        active: showVisualIndicatorRequested

        property bool showVisualIndicatorRequested: false

        Connections {
            target: root.latteView ? root.latteView.extendedInterface : null
            onAppletRequestedVisualIndicator: {
                if (plasmoidId === appletItem.applet.id) {
                    visualIndicator.showVisualIndicatorRequested = true;
                }
            }
        }

        sourceComponent: PlasmaComponents.Highlight {
            id: visualIndicatorRectangle
            opacity: 0

            Component.onCompleted: showVisualIndicatorAnimation.running = true;

            SequentialAnimation{
                id: showVisualIndicatorAnimation
                alwaysRunToEnd: true

                PropertyAnimation {
                    target: visualIndicatorRectangle
                    property: "opacity"
                    to: 1
                    duration: 3*appletItem.animations.duration.large
                    easing.type: Easing.OutQuad
                }

                PauseAnimation {
                    duration: 1500
                }

                PropertyAnimation {
                    target: visualIndicatorRectangle
                    property: "opacity"
                    to: 0
                    duration: 3*appletItem.animations.duration.large
                    easing.type: Easing.OutQuad
                }

                ScriptAction {
                    script: {
                        visualIndicator.showVisualIndicatorRequested = false;
                    }
                }
            }
        }
    }

    ///Shadow in applets
    Loader{
        id: appletShadow
        anchors.fill: _wrapperContainer

        active: appletItem.applet
                && appletItem.environment.isGraphicsSystemAccelerated
                && !appletColorizer.mustBeShown
                && (appletItem.myView.itemShadow.isEnabled && !appletItem.communicator.indexerIsSupported)

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: appletItem.myView.itemShadow.shadowColor
            fast: true
            samples: 2 * radius
            source: _wrapperContainer
            radius: appletItem.myView.itemShadow.size
            verticalOffset: root.forceTransparentPanel || root.forcePanelForBusyBackground ? 0 : 2
        }
    }

    //! Applet Main Container
    Item{
        id:_wrapperContainer
        width: root.isHorizontal ? _length : _thickness
        height: root.isHorizontal ? _thickness : _length

        property int _length:0 // through Binding to avoid binding loops
        property int _thickness:0 // through Binding to avoid binding loops

        readonly property int appliedEdgeMargin: appletItem.canFillScreenEdge ? 0 : appletItem.metrics.margin.screenEdge
        readonly property int tailThicknessMargin: {
            if (appletItem.canFillScreenEdge) {
                return 0;
            } else if (appletItem.canFillThickness) {
                return appliedEdgeMargin;
            } else if (appletItem.inMarginsArea) {
                return appliedEdgeMargin + (wrapper.zoomMarginScale * appletItem.metrics.marginsArea.tailThickness);
            }

            return appliedEdgeMargin + (wrapper.zoomMarginScale * appletItem.metrics.margin.tailThickness);
        }

        readonly property int headThicknessMargin: {
            if (appletItem.canFillThickness || appletItem.canFillScreenEdge) {
                return 0;
            } else if (appletItem.inMarginsArea) {
                return (wrapper.zoomMarginScale * appletItem.metrics.marginsArea.headThickness);
            }

            return wrapper.zoomMarginScale * appletItem.metrics.margin.headThickness;
        }

        readonly property real zoomScaleThickness: wrapper.zoomScaleThickness

        Binding {
            target: _wrapperContainer
            property: "_thickness"
            when: !visibilityManager.inRelocationHiding
            value: {
                if (appletItem.isInternalViewSplitter) {
                    return wrapper.layoutThickness;
                }

                var canfillthickness = (appletItem.canFillScreenEdge || appletItem.canFillThickness);
                var wrapperContainerThickness =  canfillthickness ? appletItem.metrics.totals.thickness : wrapper.zoomScaleThickness * wrapper.proposedItemThickness;
                return appletItem.canFillScreenEdge ? wrapperContainerThickness + appletItem.metrics.margin.screenEdge : wrapperContainerThickness;
            }
        }

        Binding {
            target: _wrapperContainer
            property: "_length"
            when: !visibilityManager.inRelocationHiding
            value: {
                if (appletItem.isAutoFillApplet && (appletItem.maxAutoFillLength>-1)){
                    return wrapper.length;
                }

                return wrapper.zoomScaleLength * wrapper.layoutLength;
            }
        }

        Loader{
            anchors.fill: parent
            active: appletItem.debug.graphicsEnabled && !isInternalViewSplitter
            sourceComponent: Rectangle{
                width: 30
                height: 30
                color: "transparent"
                border.width: 1
                border.color: "yellow"
            }
        }

        Loader{
            id: _overlayIconLoader
            anchors.fill: parent
            active: communicator.appletMainIconIsFound && indicators.info.needsIconColors

            property color backgroundColor: "transparent"
            property color glowColor: "transparent"

            //! Plasma Applets RoundToIconSize should be ALWAYS false when Latte has discovered
            //! the iconitem for these applets. This way we can be sure that applets and latte
            //! tasks will follow the same icon size if it is possible; in any other case
            //! the plasma applets might look to small
            readonly property bool isRoundToIconSizeEnabled: !communicator.appletIconItem

            onIsRoundToIconSizeEnabledChanged: {
                if (!communicator.appletIconItem) {
                    return;
                }

                communicator.appletIconItem.roundToIconSize = isRoundToIconSizeEnabled;
            }

            sourceComponent: LatteCore.IconItem{
                id: overlayIconItem
                anchors.fill: parent
                visible: false

                source: {
                    if (communicator.appletIconItem) {
                        return communicator.appletIconItem.source;
                    } else if (communicator.appletImageItem) {
                        return communicator.appletImageItem.source;
                    }

                    return "";
                }

                providesColors: source != ""
                usesPlasmaTheme: communicator.appletIconItem && communicator.appletIconItem.visible ? communicator.appletIconItem.usesPlasmaTheme : false

                Binding{
                    target: _overlayIconLoader
                    property: "backgroundColor"
                    when: overlayIconItem.providesColors
                    value: overlayIconItem.backgroundColor
                }

                Binding{
                    target: _overlayIconLoader
                    property: "glowColor"
                    when: overlayIconItem.providesColors
                    value: overlayIconItem.glowColor
                }

                Loader{
                    anchors.centerIn: parent
                    active: appletItem.debug.overloadedIconsEnabled && !isInternalViewSplitter
                    sourceComponent: Rectangle{
                        width: 30
                        height: 30
                        color: "green"
                        opacity: 0.65
                    }
                }
            }
        }

        //! WrapperContainer States
        states:[
            State{
                name: "bottom"
                when: plasmoid.location === PlasmaCore.Types.BottomEdge

                AnchorChanges{
                    target: _wrapperContainer;
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                }
                PropertyChanges{
                    target: _wrapperContainer;
                    anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: _wrapperContainer.tailThicknessMargin
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State{
                name: "top"
                when: plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges{
                    target:_wrapperContainer;
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                    anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                }
                PropertyChanges{
                    target: _wrapperContainer;
                    anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin: _wrapperContainer.tailThicknessMargin;    anchors.bottomMargin: 0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State{
                name: "left"
                when: plasmoid.location === PlasmaCore.Types.LeftEdge

                AnchorChanges{
                    target: _wrapperContainer;
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                }
                PropertyChanges{
                    target: _wrapperContainer;
                    anchors.leftMargin: _wrapperContainer.tailThicknessMargin;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            },
            State{
                name: "right"
                when: plasmoid.location === PlasmaCore.Types.RightEdge

                AnchorChanges{
                    target: _wrapperContainer;
                    anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                    anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                }
                PropertyChanges{
                    target: _wrapperContainer;
                    anchors.leftMargin: 0;    anchors.rightMargin: _wrapperContainer.tailThicknessMargin;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            }
        ]
    }

    //! EventsSink
    Loader {
        id: eventsSinkLoader
        anchors.fill: _wrapperContainer
        active: !communicator.parabolicEffectIsSupported && !isSeparator && !isSpacer && !isHidden && !isInternalViewSplitter
        //! The following can be added in case EventsSink creates slaginess with parabolic effect
        //!(appletItem.lockZoom || !appletItem.parabolic.isEnabled || !appletItem.parabolicEffectIsSupported)

        sourceComponent: EventsSink {
            destination: _wrapperContainer
        }
    }

    //! InternalViewSplitter
    Loader{
        anchors.fill: parent //_wrapperContainer
        anchors.topMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? wrapper.appletScreenMargin : 0
        anchors.leftMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? wrapper.appletScreenMargin : 0
        anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? wrapper.appletScreenMargin : 0
        anchors.rightMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? wrapper.appletScreenMargin : 0

        active: appletItem.isInternalViewSplitter && root.inConfigureAppletsMode

        sourceComponent: LatteComponents.SpriteRectangle {
            isHorizontal: root.isHorizontal
            color: appletItem.highlightColor
            spriteSize: 8
            spriteMargin: 4
            spritePosition: {
                if (root.isHorizontal) {
                    return appletItem.parent === appletItem.layouts.startLayout ?
                                PlasmaCore.Types.RightPositioned : PlasmaCore.Types.LeftPositioned;
                } else {
                    return appletItem.parent === appletItem.layouts.startLayout ?
                                PlasmaCore.Types.BottomPositioned : PlasmaCore.Types.TopPositioned;
                }
            }
        }
    }

    Loader{
        anchors.fill: parent
        active: appletItem.debug.graphicsEnabled

        sourceComponent: Rectangle{
            anchors.fill: parent
            color: "transparent"
            //! red visualizer, in debug mode for the applets that use fillWidth or fillHeight
            //! green, for the rest
            border.color:  (appletItem.isAutoFillApplet && (appletItem.maxAutoFillLength>-1) && root.isHorizontal) ? "red" : "green"
            border.width: 1
        }
    }

    Behavior on zoomScale {
        id: animatedScaleBehavior
        enabled: !appletItem.parabolic.directRenderingEnabled || restoreAnimation.running
        NumberAnimation {
            duration: 3 * appletItem.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on zoomScale {
        enabled: !animatedScaleBehavior.enabled
        NumberAnimation { duration: 0 }
    }
}// Main task area // id:wrapper
