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

import org.kde.latte.core 0.2 as LatteCore

import "../../code/MathTools.js" as MathTools

Item{
    id: wrapper

    width: {
        if (appletItem.isInternalViewSplitter && !root.inConfigureAppletsMode)
            return 0;

        if (isSeparator && root.parabolicEffectEnabled && root.isHorizontal) {
            return -1;
        }

        //! width for applets that use fillWidth/fillHeight such plasma taskmanagers and AWC
        if (appletItem.needsFillSpace && root.isHorizontal) {
            if (root.panelAlignment !== LatteCore.Types.Justify) {
                var maximumValue = (applet.Layout.maximumWidth === Infinity) || applet.Layout.maximumWidth === -1 ?
                            appletItem.sizeForFill : Math.min(appletItem.sizeForFill, applet.Layout.maximumWidth);

                var constrainedWidth = MathTools.bound(applet.Layout.minimumWidth, applet.Layout.preferredWidth, maximumValue);

                return root.inConfigureAppletsMode ? Math.max(constrainedWidth, appletItem.metrics.iconSize) : constrainedWidth;
            }

            if(appletItem.sizeForFill>-1){
                return appletItem.sizeForFill;
            }
        }

        if (appletItem.latteApplet) {
            return latteApplet.tasksWidth;
        } else {
            if (root.isHorizontal && root.inConfigureAppletsMode) {
                return Math.max(Math.min(appletItem.metrics.iconSize, root.minAppletLengthInConfigure), scaledWidth);
            }

            return root.isVertical ? scaledWidth + appletItem.metrics.margin.screenEdge : scaledWidth;
        }
    }

    height: {
        if (appletItem.isInternalViewSplitter && !root.inConfigureAppletsMode)
            return 0;

        if (isSeparator && root.parabolicEffectEnabled && root.isVertical) {
            return -1;
        }

        //! height for applets that use fillWidth/fillHeight such plasma taskmanagers and AWC
        if (appletItem.needsFillSpace && root.isVertical) {
            if (root.panelAlignment !== LatteCore.Types.Justify) {
                var maximumValue = (applet.Layout.maximumHeight === Infinity) || applet.Layout.maximumHeight === -1 ?
                            appletItem.sizeForFill : Math.min(appletItem.sizeForFill, applet.Layout.maximumHeight);

                var constrainedHeight = MathTools.bound(applet.Layout.minimumHeight, applet.Layout.preferredHeight, maximumValue);

                return root.inConfigureAppletsMode ? Math.max(constrainedHeight, appletItem.metrics.iconSize) : constrainedHeight;
            }

            if (appletItem.sizeForFill>-1){
                return appletItem.sizeForFill;
            }
        }

        if (appletItem.latteApplet) {
            return latteApplet.tasksHeight;
        } else {
            if (root.isVertical && root.inConfigureAppletsMode) {
                return Math.max(Math.min(appletItem.metrics.iconSize, root.minAppletLengthInConfigure), scaledHeight);
            }

            return root.isHorizontal ? scaledHeight + appletItem.metrics.margin.screenEdge : scaledHeight;
        }
    }

    opacity: appletColorizer.mustBeShown && graphicsSystem.isAccelerated ? 0 : 1

    property bool disableScaleWidth: false
    property bool disableScaleHeight: false
    property bool editMode: root.inConfigureAppletsMode

    property bool edgeLengthMarginsDisabled: isSeparator || !communicator.requires.lengthMarginsEnabled || !canBeHovered

    property int appletWidth: applet ?  applet.width : -1
    property int appletHeight: applet ?  applet.height : -1

    property int appletMinimumWidth: applet && applet.Layout ?  applet.Layout.minimumWidth : -1
    property int appletMinimumHeight: applet && applet.Layout ? applet.Layout.minimumHeight : -1

    property int appletPreferredWidth: applet && applet.Layout ?  applet.Layout.preferredWidth : -1
    property int appletPreferredHeight: applet && applet.Layout ?  applet.Layout.preferredHeight : -1

    property int appletMaximumWidth: applet && applet.Layout ?  applet.Layout.maximumWidth : -1
    property int appletMaximumHeight: applet && applet.Layout ?  applet.Layout.maximumHeight : -1

    property int iconSize: appletItem.metrics.iconSize

    property int marginWidth: root.isVertical ?
                                  appletItem.metrics.totals.thicknessEdges :
                                  (root.inFullJustify && atScreenEdge && !parabolicEffectMarginsEnabled ? edgeLengthMargins : localLengthMargins)  //Fitt's Law
    property int marginHeight: root.isHorizontal ?
                                   appletItem.metrics.totals.thicknessEdges :
                                   (root.inFullJustify && atScreenEdge && !parabolicEffectMarginsEnabled ? edgeLengthMargins : localLengthMargins)  //Fitt's Law

    property int localLengthMargins: isSeparator || !communicator.requires.lengthMarginsEnabled || isInternalViewSplitter ? 0 : appletItem.lengthAppletFullMargins
    property int edgeLengthMargins: edgeLengthMarginsDisabled ? 0 : appletItem.lengthAppletPadding * 2

    property real scaledWidth: zoomScaleWidth * (layoutWidth + marginWidth)
    property real scaledHeight: zoomScaleHeight * (layoutHeight + marginHeight)
    property real zoomScaleWidth: disableScaleWidth ? 1 : zoomScale
    property real zoomScaleHeight: disableScaleHeight ? 1 : zoomScale

    property int layoutWidthResult: 0

    property int layoutWidth
    property int layoutHeight

    property real center:root.isHorizontal ?
                             (width + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2 :
                             (height + hiddenSpacerLeft.separatorSpace + hiddenSpacerRight.separatorSpace) / 2

    property real zoomScale: 1

    property int index: appletItem.index

    property Item wrapperContainer: _wrapperContainer
    property Item clickedEffect: _clickedEffect
    property Item containerForOverlayIcon: _containerForOverlayIcon
    property Item overlayIconLoader: _overlayIconLoader

    Behavior on opacity {
        NumberAnimation {
            duration: 0.8 * appletItem.animations.duration.proposed
            easing.type: Easing.OutCubic
        }
    }

    // property int pHeight: applet ? applet.Layout.preferredHeight : -10

    /*function debugLayouts(){
        if(applet){
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
            console.log("Can be hovered: " + canBeHovered);
            console.log("Icon size: " + appletItem.metrics.iconSize);
            console.log("Thick Margins: " + appletItem.metrics.totals.thicknessEdges);
            console.log("Intern. Margins: " + (appletItem.metrics.padding.length * 2));
            console.log("Intern. Margins: " + (appletItem.metrics.margin.length * 2));
            console.log("Max hovered criteria: " + (appletItem.metrics.iconSize + metrics.totals.thicknessEdges));
            console.log("-----");
            console.log("LayoutW: " + layoutWidth);
            console.log("LayoutH: " + layoutHeight);
        }
    }

    onLayoutWidthChanged: {
        debugLayouts();
    }

    onLayoutHeightChanged: {
        debugLayouts();
    }*/

    onAppletWidthChanged: {
        if(zoomScale === 1) {
            checkCanBeHovered();
        }
    }

    onAppletHeightChanged: {
        if(zoomScale === 1) {
            checkCanBeHovered();
        }
    }

    onAppletMinimumWidthChanged: {
        if(zoomScale === 1) {
            checkCanBeHovered();
        }

        updateLayoutWidth();
    }
    onAppletMinimumHeightChanged: {
        if(zoomScale === 1) {
            checkCanBeHovered();
        }

        updateLayoutHeight();
    }

    onAppletPreferredWidthChanged: updateLayoutWidth();
    onAppletPreferredHeightChanged: updateLayoutHeight();

    onAppletMaximumWidthChanged: updateLayoutWidth();
    onAppletMaximumHeightChanged: updateLayoutHeight();

    Connections {
        target: appletItem
        onCanBeHoveredChanged: {
            updateLayoutWidth();
            updateLayoutHeight();
        }
    }

    onIconSizeChanged: {
        updateLayoutWidth();
        updateLayoutHeight();
    }

    onEditModeChanged: {
        updateLayoutWidth();
        updateLayoutHeight();
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

    Connections {
        target: root
        onIsVerticalChanged: {
            if (appletItem.latteApplet) {
                return;
            }

            wrapper.disableScaleWidth = false;
            wrapper.disableScaleHeight = false;

            if (root.isVertical)  {
                wrapper.updateLayoutHeight();
                wrapper.updateLayoutWidth();
            } else {
                wrapper.updateLayoutWidth();
                wrapper.updateLayoutHeight();
            }
        }
    }

    function updateLayoutHeight(){
        appletItem.movingForResize = true;

        if (appletItem.needsFillSpace && root.isVertical) {
            layoutsContainer.updateSizeForAppletsInFill();
            return;
        }

        var blockParabolicEffect = false;

        if (isLattePlasmoid) {
            return;
        } else if (appletItem.isInternalViewSplitter){
            if(!root.inConfigureAppletsMode) {
                layoutHeight = 0;
            } else {
                layoutHeight = (root.isHorizontal ? appletItem.metrics.iconSize : Math.min(appletItem.metrics.iconSize, root.maxJustifySplitterSize));
            }
        }
        else if(appletItem.isSystray && root.isHorizontal){
            layoutHeight = appletItem.metrics.iconSize;
        }
        else{
            if(applet && (applet.Layout.minimumHeight > appletItem.metrics.iconSize) && root.isVertical && !canBeHovered && !communicator.overlayLatteIconIsActive){
                blockParabolicEffect = true;
                layoutHeight = applet.Layout.minimumHeight;
            } //it is used for plasmoids that need to scale only one axis... e.g. the Weather Plasmoid
            else if(applet
                    && ( applet.Layout.maximumHeight < appletItem.metrics.iconSize
                        || applet.Layout.preferredHeight > appletItem.metrics.iconSize
                        || appletItem.originalAppletBehavior)
                    && root.isVertical
                    && !disableScaleWidth
                    && !communicator.overlayLatteIconIsActive) {
                //this way improves performance, probably because during animation the preferred sizes update a lot
                if((applet.Layout.maximumHeight < appletItem.metrics.iconSize)){
                    layoutHeight = applet.Layout.maximumHeight;
                } else if (applet.Layout.minimumHeight > appletItem.metrics.iconSize){
                    blockParabolicEffect = true;
                    layoutHeight = applet.Layout.minimumHeight;
                } else if ((applet.Layout.preferredHeight > appletItem.metrics.iconSize)
                           || (appletItem.originalAppletBehavior && applet.Layout.preferredHeight > 0 )){
                    blockParabolicEffect = true;
                    layoutHeight = applet.Layout.preferredHeight;
                } else{
                    layoutHeight = appletItem.metrics.iconSize;
                }
            } else {
                layoutHeight = appletItem.metrics.iconSize;
            }
        }

        if (wrapper.zoomScale === 1) {
            if (blockParabolicEffect) {
                disableScaleHeight = true;
            } else {
                disableScaleHeight = false;
            }
        }
    }

    function updateLayoutWidth(){
        appletItem.movingForResize = true;

        if (appletItem.needsFillSpace && root.isHorizontal) {
            layoutsContainer.updateSizeForAppletsInFill();
            return;
        }

        var blockParabolicEffect = false;

        if (isLattePlasmoid) {
            return;
        } else if (appletItem.isInternalViewSplitter){
            if(!root.inConfigureAppletsMode) {
                layoutWidth = 0;
            } else {
                layoutWidth = (root.isVertical ? appletItem.metrics.iconSize : Math.min(appletItem.metrics.iconSize, root.maxJustifySplitterSize));
            }
        }
        else if(appletItem.isSystray && root.isVertical){
            layoutWidth = appletItem.metrics.iconSize;
        }
        else{
            if(applet && (applet.Layout.minimumWidth > appletItem.metrics.iconSize) && root.isHorizontal && !canBeHovered && !communicator.overlayLatteIconIsActive){
                blockParabolicEffect = true;
                layoutWidth = applet.Layout.minimumWidth;
            } //it is used for plasmoids that need to scale only one axis... e.g. the Weather Plasmoid
            else if(applet
                    && ( applet.Layout.maximumWidth < appletItem.metrics.iconSize
                        || applet.Layout.preferredWidth > appletItem.metrics.iconSize
                        || appletItem.originalAppletBehavior)
                    && root.isHorizontal
                    && !disableScaleHeight
                    && !communicator.overlayLatteIconIsActive){

                //this way improves performance, probably because during animation the preferred sizes update a lot
                if((applet.Layout.maximumWidth < appletItem.metrics.iconSize)){
                    //   return applet.Layout.maximumWidth;
                    layoutWidth = applet.Layout.maximumWidth;
                } else if (applet.Layout.minimumWidth > appletItem.metrics.iconSize){
                    blockParabolicEffect = true;
                    layoutWidth = applet.Layout.minimumWidth;
                } else if ((applet.Layout.preferredWidth > appletItem.metrics.iconSize)
                           || (appletItem.originalAppletBehavior && applet.Layout.preferredWidth > 0 )){
                    blockParabolicEffect = true;
                    layoutWidth = applet.Layout.preferredWidth;
                } else{
                    layoutWidth = appletItem.metrics.iconSize;
                }
            } else{
                layoutWidth = appletItem.metrics.iconSize;
            }
        }

        if (wrapper.zoomScale === 1) {
            if (blockParabolicEffect) {
                disableScaleWidth = true;
            } else {
                disableScaleWidth = false;
            }
        }
    }

    Item{
        id:_wrapperContainer

        width:{
            if (appletItem.needsFillSpace && (appletItem.sizeForFill>-1) && root.isHorizontal){
                return wrapper.width;
            }

            if (appletItem.isInternalViewSplitter) {
                return wrapper.layoutWidth;
            } else {
                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    var wrapperContainerThickness = parent.zoomScaleWidth * (appletItem.metrics.totals.thickness);
                    return appletItem.screenEdgeMarginSupported ? wrapperContainerThickness + appletItem.metrics.margin.screenEdge : wrapperContainerThickness;
                } else {
                    return parent.zoomScaleWidth * wrapper.layoutWidth;
                }
            }
        }

        height:{
            if (appletItem.needsFillSpace && (appletItem.sizeForFill>-1) && root.isVertical){
                return wrapper.height;
            }

            if (appletItem.isInternalViewSplitter) {
                return wrapper.layoutHeight;
            } else {
                if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                    var wrapperContainerThickness = parent.zoomScaleHeight * (appletItem.metrics.totals.thickness);
                    return appletItem.screenEdgeMarginSupported ? wrapperContainerThickness + appletItem.metrics.margin.screenEdge : wrapperContainerThickness;
                } else {
                    return parent.zoomScaleHeight * wrapper.layoutHeight;
                }
            }

        }

        opacity: appletShadow.active ? 0 : 1

        readonly property int appliedEdgeMargin: {
            if (appletItem.isInternalViewSplitter) {
                return appletItem.metrics.margin.screenEdge + appletItem.metrics.margin.thickness;
            }

            return appletItem.screenEdgeMarginSupported ? 0 : appletItem.metrics.margin.screenEdge;
        }

        ///Secret MouseArea to be used by the folder widget
        Loader{
            anchors.fill: parent
            active: communicator.overlayLatteIconIsActive
                    && applet.pluginName === "org.kde.plasma.folder"
                    && !appletItem.acceptMouseEvents

            sourceComponent: MouseArea{
                onClicked: latteView.extendedInterface.toggleAppletExpanded(applet.id);
            }
        }

        Item{
            id: _containerForOverlayIcon
            anchors.fill: parent
        }

        Loader{
            id: _overlayIconLoader
            anchors.fill: parent
            active: communicator.overlayLatteIconIsActive

            property color backgroundColor: "black"
            property color glowColor: "white"

            sourceComponent: LatteCore.IconItem{
                id: overlayIconItem
                anchors.fill: parent
                source: {
                    if (communicator.appletIconItemIsShown())
                        return communicator.appletIconItem.source;
                    else if (communicator.appletImageItemIsShown())
                        return communicator.appletImageItem.source;

                    return "";
                }

                providesColors: indicators.info.needsIconColors && source != ""
                usesPlasmaTheme: communicator.appletIconItemIsShown() ? communicator.appletIconItem.usesPlasmaTheme : false

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
                    active: root.debugModeOverloadedIcons
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
                    anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: _wrapperContainer.appliedEdgeMargin;
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
                    anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin: _wrapperContainer.appliedEdgeMargin;    anchors.bottomMargin: 0;
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
                    anchors.leftMargin: _wrapperContainer.appliedEdgeMargin;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: 0;
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
                    anchors.leftMargin: 0;    anchors.rightMargin: _wrapperContainer.appliedEdgeMargin;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                    anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                }
            }
        ]
    }

    Loader{
        anchors.fill: _wrapperContainer
        active: appletItem.isInternalViewSplitter && root.inConfigureAppletsMode

        sourceComponent: Item {
            anchors.fill: parent

            PlasmaCore.SvgItem{
                id:splitterImage
                anchors.centerIn: parent
                width: Math.min(root.maxJustifySplitterSize, appletItem.metrics.iconSize)
                height: width
                rotation: root.isVertical ? 90 : 0

                svg: PlasmaCore.Svg{
                    imagePath: root.universalSettings.splitterIconPath()
                }

                layer.enabled: graphicsSystem.isAccelerated
                layer.effect: DropShadow {
                    radius: root.appShadowSize
                    fast: true
                    samples: 2 * radius
                    color: root.appShadowColor

                    verticalOffset: 2
                }
            }
        }
    }

    ///Shadow in applets
    Loader{
        id: appletShadow
        anchors.fill: appletItem.appletWrapper

        active: appletItem.applet
                && graphicsSystem.isAccelerated
                && !appletColorizer.mustBeShown
                && (root.enableShadows && applet.pluginName !== root.plasmoidName)

        onActiveChanged: {
            if (active && !isSeparator && graphicsSystem.isAccelerated) {
                wrapperContainer.opacity = 0;
            } else {
                wrapperContainer.opacity = 1;
            }
        }

        opacity: isSeparator ? 0.4 : 1

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: root.appShadowColor //"#ff080808"
            fast: true
            samples: 2 * radius
            source: communicator.overlayLatteIconIsActive ? _wrapperContainer : appletItem.applet
            radius: shadowSize
            verticalOffset: root.forceTransparentPanel || root.forcePanelForBusyBackground ? 0 : 2

            property int shadowSize : root.appShadowSize
        }
    }

    BrightnessContrast{
        id:hoveredImage
        anchors.fill: _wrapperContainer
        source: _wrapperContainer

        enabled: appletItem.isSquare && !originalAppletBehavior && !indicators.info.providesHoveredAnimation && opacity != 0 ? true : false
        opacity: appletMouseArea.containsMouse ? 1 : 0
        brightness: 0.25
        contrast: 0.15
        visible: !indicators.info.providesHoveredAnimation

        Behavior on opacity {
            NumberAnimation { duration: appletItem.animations.speedFactor.current*appletItem.animations.duration.large }
        }
    }

    BrightnessContrast {
        id: _clickedEffect
        anchors.fill: _wrapperContainer
        source: _wrapperContainer

        visible: clickedAnimation.running && !indicators.info.providesClickedAnimation
    }

    /*   onHeightChanged: {
        if ((index == 1)|| (index==3)){
            console.log("H: "+index+" ("+zoomScale+"). "+currentLayout.children[1].height+" - "+currentLayout.children[3].height+" - "+(currentLayout.children[1].height+currentLayout.children[3].height));
        }
    }

    onZoomScaleChanged:{
        if ((index == 1)|| (index==3)){
            console.log(index+" ("+zoomScale+"). "+currentLayout.children[1].height+" - "+currentLayout.children[3].height+" - "+(currentLayout.children[1].height+currentLayout.children[3].height));
        }
    }*/

    Loader{
        anchors.fill: parent
        active: root.debugMode

        sourceComponent: Rectangle{
            anchors.fill: parent
            color: "transparent"
            //! red visualizer, in debug mode for the applets that use fillWidth or fillHeight
            //! green, for the rest
            border.color:  (appletItem.needsFillSpace && (appletItem.sizeForFill>-1) && root.isHorizontal) ? "red" : "green"
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

    function calculateScales( currentMousePosition ){
        if (parabolic.factor.zoom===1) {
            return;
        }

        //use the new parabolic effect manager in order to handle all parabolic effect messages
        var scales = parabolic.applyParabolicEffect(index, currentMousePosition, center);

        //Left hiddenSpacer
        if(appletItem.firstAppletInContainer){
            hiddenSpacerLeft.nScale = scales.leftScale - 1;
        }

        //Right hiddenSpacer  ///there is one more item in the currentLayout ????
        if(appletItem.lastAppletInContainer){
            hiddenSpacerRight.nScale =  scales.rightScale - 1;
        }

        zoomScale = parabolic.factor.zoom;
    } //scale


    function signalUpdateScale(nIndex, nScale, step){
        if(appletItem && !appletItem.containsMouse && (appletItem.index === nIndex)){
            if ( ((canBeHovered && !appletItem.originalAppletBehavior) || appletItem.latteApplet)
                    && (applet && applet.status !== PlasmaCore.Types.HiddenStatus)
                    ){
                if(!appletItem.latteApplet){
                    if(nScale >= 0)
                        zoomScale = nScale + step;
                    else
                        zoomScale = zoomScale + step;
                }
            }
        }
    }

    function sltUpdateLowerItemScale(delegateIndex, newScale, step) {
        if (delegateIndex === appletItem.index) {
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateLowerItemScale(newScale, step);
                return;
            }

            if (!appletItem.isSeparator && !appletItem.isHidden) {
                //! when accepted
                signalUpdateScale(delegateIndex, newScale, step);

                if (newScale > 1) { // clear lower items
                    parabolic.sglUpdateLowerItemScale(delegateIndex-1, 1, 0);
                }
            } else {
                parabolic.sglUpdateLowerItemScale(delegateIndex-1, newScale, step);
            }
        } else if ((newScale === 1) && (appletItem.index < delegateIndex)) {
            //! apply zoom clearing
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateLowerItemScale(1, step);
            } else {
                signalUpdateScale(appletItem.index, 1, 0);
            }
        }
    }

    function sltUpdateHigherItemScale(delegateIndex, newScale, step) {
        if (delegateIndex === appletItem.index) {          
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateHigherItemScale(newScale, step);
                return;
            }

            if (!appletItem.isSeparator && !appletItem.isHidden) {
                //! when accepted
                signalUpdateScale(delegateIndex, newScale, step);

                if (newScale > 1) { // clear higher items
                    parabolic.sglUpdateHigherItemScale(delegateIndex+1, 1, 0);
                }
            } else {
                parabolic.sglUpdateHigherItemScale(delegateIndex+1, newScale, step);
            }
        } else if ((newScale === 1) && (appletItem.index > delegateIndex)) {
            //! apply zoom clearing
            if (communicator.parabolicEffectIsSupported) {
                communicator.bridge.parabolic.client.hostRequestUpdateHigherItemScale(1, step);
            } else {
                signalUpdateScale(appletItem.index, 1, 0);
            }
        }
    }

    Component.onCompleted: {
        parabolic.sglUpdateLowerItemScale.connect(sltUpdateLowerItemScale);
        parabolic.sglUpdateHigherItemScale.connect(sltUpdateHigherItemScale);
    }

    Component.onDestruction: {
        parabolic.sglUpdateLowerItemScale.disconnect(sltUpdateLowerItemScale);
        parabolic.sglUpdateHigherItemScale.disconnect(sltUpdateHigherItemScale);
    }
}// Main task area // id:wrapper
