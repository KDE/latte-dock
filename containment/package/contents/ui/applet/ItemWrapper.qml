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

import org.kde.latte 0.2 as Latte

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
            if (root.panelAlignment !== Latte.Types.Justify) {
                var maximumValue = (applet.Layout.maximumWidth === Infinity) || applet.Layout.maximumWidth === -1 ?
                            appletItem.sizeForFill : Math.min(appletItem.sizeForFill, applet.Layout.maximumWidth);

                var constrainedWidth = MathTools.bound(applet.Layout.minimumWidth, applet.Layout.preferredWidth, maximumValue);

                return root.inConfigureAppletsMode ? Math.max(constrainedWidth, root.iconSize) : constrainedWidth;
            }

            if(appletItem.sizeForFill>-1){
                return appletItem.sizeForFill;
            }
        }

        if (appletItem.latteApplet) {
            return latteApplet.tasksWidth;
        } else {
            return (root.isHorizontal && root.inConfigureAppletsMode) ? Math.max(root.iconSize, scaledWidth) : scaledWidth;
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
            if (root.panelAlignment !== Latte.Types.Justify) {
                var maximumValue = (applet.Layout.maximumHeight === Infinity) || applet.Layout.maximumHeight === -1 ?
                            appletItem.sizeForFill : Math.min(appletItem.sizeForFill, applet.Layout.maximumHeight);

                var constrainedHeight = MathTools.bound(applet.Layout.minimumHeight, applet.Layout.preferredHeight, maximumValue);

                return root.inConfigureAppletsMode ? Math.max(constrainedHeight, root.iconSize) : constrainedHeight;
            }

            if (appletItem.sizeForFill>-1){
                return appletItem.sizeForFill;
            }
        }

        if (appletItem.latteApplet) {
            return latteApplet.tasksHeight;
        } else {
            return (root.isVertical && root.inConfigureAppletsMode) ? Math.max(root.iconSize, scaledHeight) : scaledHeight;
        }
    }

    opacity: appletColorizer.mustBeShown && graphicsSystem.isAccelerated ? 0 : 1

    property bool disableScaleWidth: false
    property bool disableScaleHeight: false
    property bool editMode: root.inConfigureAppletsMode

    property bool edgeLengthMarginsDisabled: isSeparator || !communicator.lengthMarginsEnabled || !canBeHovered

    property int appletWidth: applet ?  applet.width : -1
    property int appletHeight: applet ?  applet.height : -1

    property int appletMinimumWidth: applet && applet.Layout ?  applet.Layout.minimumWidth : -1
    property int appletMinimumHeight: applet && applet.Layout ? applet.Layout.minimumHeight : -1

    property int appletPreferredWidth: applet && applet.Layout ?  applet.Layout.preferredWidth : -1
    property int appletPreferredHeight: applet && applet.Layout ?  applet.Layout.preferredHeight : -1

    property int appletMaximumWidth: applet && applet.Layout ?  applet.Layout.maximumWidth : -1
    property int appletMaximumHeight: applet && applet.Layout ?  applet.Layout.maximumHeight : -1

    property int iconSize: root.iconSize

    property int marginWidth: root.isVertical ?
                                  root.thickMargins :
                                  (root.inFullJustify && atScreenEdge && !parabolicEffectMarginsEnabled ? edgeLengthMargins : localLengthMargins)  //Fitt's Law
    property int marginHeight: root.isHorizontal ?
                                   root.thickMargins :
                                  (root.inFullJustify && atScreenEdge && !parabolicEffectMarginsEnabled ? edgeLengthMargins : localLengthMargins)  //Fitt's Law

    property int localLengthMargins: isSeparator || !communicator.lengthMarginsEnabled ? 0 : appletItem.lengthAppletFullMargins
    property int edgeLengthMargins: edgeLengthMarginsDisabled ? 0 : appletItem.lengthAppletIntMargin * 2

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
            duration: 0.8 * root.animationTime
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
            console.log("Icon size: " + root.iconSize);
            console.log("Thick Margins: " + root.thickMargins);
            console.log("Intern. Margins: " + (root.lengthIntMargin * 2));
            console.log("Intern. Margins: " + (root.lengthExtMargin * 2));
            console.log("Max hovered criteria: " + (root.iconSize + thickMargins));
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
        if ((zoomScale === root.zoomFactor) && !root.globalDirectRender) {
            root.setGlobalDirectRender(true);
        }

        if ((zoomScale > 1) && !appletItem.isZoomed) {
            appletItem.isZoomed = true;
            if (!animationWasSent) {
                root.slotAnimationsNeedBothAxis(1);
                animationWasSent = true;
            }
        } else if (zoomScale == 1) {
            appletItem.isZoomed = false;
            if (animationWasSent) {
                root.slotAnimationsNeedBothAxis(-1);
                animationWasSent = false;
            }
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
            if(!root.inConfigureAppletsMode)
                layoutHeight = 0;
            else
                layoutHeight = root.iconSize;
        }
        else if(appletItem.isSystray && root.isHorizontal){
            layoutHeight = root.iconSize;
        }
        else{
            if(applet && (applet.Layout.minimumHeight > root.iconSize) && root.isVertical && !canBeHovered && !communicator.overlayLatteIconIsActive){
                blockParabolicEffect = true;
                layoutHeight = applet.Layout.minimumHeight;
            } //it is used for plasmoids that need to scale only one axis... e.g. the Weather Plasmoid
            else if(applet
                    && ( applet.Layout.maximumHeight < root.iconSize
                        || applet.Layout.preferredHeight > root.iconSize
                        || appletItem.originalAppletBehavior)
                    && root.isVertical
                    && !disableScaleWidth
                    && !communicator.overlayLatteIconIsActive) {
                //this way improves performance, probably because during animation the preferred sizes update a lot
                if((applet.Layout.maximumHeight < root.iconSize)){
                    layoutHeight = applet.Layout.maximumHeight;
                } else if (applet.Layout.minimumHeight > root.iconSize){
                    blockParabolicEffect = true;
                    layoutHeight = applet.Layout.minimumHeight;
                } else if ((applet.Layout.preferredHeight > root.iconSize)
                           || (appletItem.originalAppletBehavior && applet.Layout.preferredHeight > 0 )){
                    blockParabolicEffect = true;
                    layoutHeight = applet.Layout.preferredHeight;
                } else{
                    layoutHeight = root.iconSize;
                }
            } else {
                layoutHeight = root.iconSize;
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
            if(!root.inConfigureAppletsMode)
                layoutWidth = 0;
            else
                layoutWidth = root.iconSize;
        }
        else if(appletItem.isSystray && root.isVertical){
            layoutWidth = root.iconSize;
        }
        else{
            if(applet && (applet.Layout.minimumWidth > root.iconSize) && root.isHorizontal && !canBeHovered && !communicator.overlayLatteIconIsActive){
                blockParabolicEffect = true;
                layoutWidth = applet.Layout.minimumWidth;
            } //it is used for plasmoids that need to scale only one axis... e.g. the Weather Plasmoid
            else if(applet
                    && ( applet.Layout.maximumWidth < root.iconSize
                        || applet.Layout.preferredWidth > root.iconSize
                        || appletItem.originalAppletBehavior)
                    && root.isHorizontal
                    && !disableScaleHeight
                    && !communicator.overlayLatteIconIsActive){

                //this way improves performance, probably because during animation the preferred sizes update a lot
                if((applet.Layout.maximumWidth < root.iconSize)){
                    //   return applet.Layout.maximumWidth;
                    layoutWidth = applet.Layout.maximumWidth;
                } else if (applet.Layout.minimumWidth > root.iconSize){
                    blockParabolicEffect = true;
                    layoutWidth = applet.Layout.minimumWidth;
                } else if ((applet.Layout.preferredWidth > root.iconSize)
                           || (appletItem.originalAppletBehavior && applet.Layout.preferredWidth > 0 )){
                    blockParabolicEffect = true;
                    layoutWidth = applet.Layout.preferredWidth;
                } else{
                    layoutWidth = root.iconSize;
                }
            } else{
                layoutWidth = root.iconSize;
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
                    return parent.zoomScaleWidth * (root.iconSize + root.thickMargins);
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
                    return parent.zoomScaleHeight * (root.iconSize + root.thickMargins);
                } else {
                    return parent.zoomScaleHeight * wrapper.layoutHeight;
                }
            }

        }

        opacity: appletShadow.active ? 0 : 1
        anchors.centerIn: parent

        ///Secret MouseArea to be used by the folder widget
        Loader{
            anchors.fill: parent
            active: communicator.overlayLatteIconIsActive && applet.pluginName === "org.kde.plasma.folder"
            sourceComponent: MouseArea{
                onClicked: latteView.toggleAppletExpanded(applet.id);
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

            sourceComponent: Latte.IconItem{
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
    }

    Loader{
        anchors.fill: _wrapperContainer
        active: appletItem.isInternalViewSplitter && root.inConfigureAppletsMode

        rotation: root.isVertical ? 90 : 0

        sourceComponent: PlasmaCore.SvgItem{
            id:splitterImage
            anchors.fill: parent

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

            Component.onCompleted: {
                if (root.isVertical)  {
                    wrapper.updateLayoutHeight();
                    wrapper.updateLayoutWidth();
                } else {
                    wrapper.updateLayoutWidth();
                    wrapper.updateLayoutHeight();
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
                && (((plasmoid.configuration.shadows === 1 /*Locked Applets*/
                      && (!appletItem.canBeHovered || (appletItem.originalAppletBehavior && (applet.pluginName !== root.plasmoidName))) )
                     || (plasmoid.configuration.shadows === 2 /*All Applets*/
                         && (applet.pluginName !== root.plasmoidName)))
                    || (root.forceTransparentPanel && plasmoid.configuration.shadows>0 && applet.pluginName !== root.plasmoidName)) /*on forced transparent state*/

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
            NumberAnimation { duration: root.durationTime*units.longDuration }
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
        enabled: !root.globalDirectRender
        NumberAnimation {
            duration: 3 * appletItem.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on zoomScale {
        enabled: root.globalDirectRender && !restoreAnimation.running
        NumberAnimation { duration: root.directRenderAnimationTime }
    }

    function calculateScales( currentMousePosition ){
        if (root.zoomFactor===1) {
            return;
        }

        var distanceFromHovered = Math.abs(index - layoutsContainer.hoveredIndex);

        // A new algorithm trying to make the zoom calculation only once
        // and at the same time fixing glitches
        if ((distanceFromHovered == 0)&&
                (currentMousePosition  > 0) ){

            //use the new parabolicManager in order to handle all parabolic effect messages
            var scales = parabolicManager.applyParabolicEffect(index, currentMousePosition, center);

            /*if (root.latteApplet && Math.abs(index - root.latteAppletPos) > 2){
                root.latteApplet.clearZoom();
            }*/

            //Left hiddenSpacer
            if(appletItem.firstAppletInContainer){
                hiddenSpacerLeft.nScale = scales.leftScale - 1;
            }

            //Right hiddenSpacer  ///there is one more item in the currentLayout ????
            if(appletItem.lastAppletInContainer){
                hiddenSpacerRight.nScale =  scales.rightScale - 1;
            }

            zoomScale = root.zoomFactor;
        }

    } //scale


    function signalUpdateScale(nIndex, nScale, step){
        if(appletItem && !appletItem.containsMouse && (appletItem.index === nIndex)){
            if ( ((canBeHovered && !appletItem.originalAppletBehavior) || appletItem.latteApplet)
                    && (applet && applet.status !== PlasmaCore.Types.HiddenStatus)
                    //&& (index != currentLayout.hoveredIndex)
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

    Component.onCompleted: {
        root.updateScale.connect(signalUpdateScale);
    }

    Component.onDestruction: {
        root.updateScale.disconnect(signalUpdateScale);
    }
}// Main task area // id:wrapper
