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

Item {
    id: container

    visible: false
    width: isInternalViewSplitter && !root.editMode ? 0 : (root.isHorizontal ? computeWidth : computeWidth + shownAppletMargin)
    height: isInternalViewSplitter && !root.editMode ? 0 : (root.isVertical ?  computeHeight : computeHeight + shownAppletMargin)

    property bool animationsEnabled: true
    property bool animationWasSent: false  //protection flag for animation broadcasting
    property bool canBeHovered: true
    property bool inFillCalculations: false //it is used in calculations for fillWidth,fillHeight applets
    property bool needsFillSpace: { //it is used in calculations for fillWidth,fillHeight applets
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
    property bool isInternalViewSplitter: (internalSplitterId > 0)
    property bool isZoomed: false

    //applet is in starting edge
    /*property bool startEdge: index < endLayout.beginIndex ? (index === 0)&&(mainLayout.count > 1) :
                                                               (index === endLayout.beginIndex)&&(endLayout.count > 1)*/
    property bool startEdge: (index === startLayout.beginIndex) || (index === mainLayout.beginIndex) || (index === endLayout.beginIndex)
    //applet is in ending edge
    property bool endEdge: plasmoid.configuration.panelPosition !== Latte.Dock.Justify ? (index === mainLayout.beginIndex + mainLayout.count - 1)&&(mainLayout.count>1) :
                                                                                         (((index === startLayout.beginIndex+startLayout.count-2)&&(startLayout.count>2))
                                                                                          ||((index === mainLayout.beginIndex+mainLayout.count-2)&&(mainLayout.count>2))
                                                                                          ||((index === endLayout.beginIndex+endLayout.count-1)&&(endLayout.count>1)))



    property int animationTime: root.durationTime* (1.2 *units.shortDuration) // 70
    property int hoveredIndex: layoutsContainer.hoveredIndex
    property int index: -1
    property int appletMargin: (applet && (applet.pluginName === root.plasmoidName))
                               || isInternalViewSplitter
                               || root.reverseLinesPosition ? 0 : root.statesLineSize
    property int maxWidth: root.isHorizontal ? root.height : root.width
    property int maxHeight: root.isHorizontal ? root.height : root.width
    property int shownAppletMargin: applet && (applet.pluginName === "org.kde.plasma.systemtray") ? 0 : appletMargin
    property int internalSplitterId: 0
    property int sizeForFill: -1 //it is used in calculations for fillWidth,fillHeight applets
    property int spacersMaxSize: Math.max(0,Math.ceil(0.55*root.iconSize) - root.iconMargin)
    property int status: applet ? applet.status : -1

    //property real animationStep: root.iconSize / 8
    property real animationStep: 2 //6
    property real computeWidth: root.isVertical ? wrapper.width :
                                                  hiddenSpacerLeft.width+wrapper.width+hiddenSpacerRight.width

    property real computeHeight: root.isVertical ? hiddenSpacerLeft.height + wrapper.height + hiddenSpacerRight.height :
                                                   wrapper.height

    property string title: isInternalViewSplitter ? "Now Dock Splitter" : ""

    property Item applet
    property Item latteApplet: applet && (applet.pluginName === root.plasmoidName) ?
                                   (applet.children[0] ? applet.children[0] : null) : null
    property Item appletWrapper: applet &&
                                 ((applet.pluginName === root.plasmoidName) ||
                                  (applet.pluginName === "org.kde.plasma.systemtray")) ? wrapper : wrapperContainer
    property Item appletIconItem; //first applet's IconItem, to be activated onExit signal

    //this is used for folderView and icon widgets to fake their visual
    property bool fakeIconItem: applet && appletIconItem && (applet.pluginName === "org.kde.plasma.folder" || applet.pluginName === "org.kde.plasma.icon")

    property alias containsMouse: appletMouseArea.containsMouse
    property alias pressed: appletMouseArea.pressed

    /*onComputeHeightChanged: {
        if(index==0)
            console.log(computeHeight);
    }*/

    //set up the fake containers and properties for when a fakeIconItem must be presented to the user
    //because the plasma widgets specific implementation breaks the Latte experience
    onFakeIconItemChanged: {
        if (fakeIconItem) {
            applet.opacity = 0;

            if (applet.pluginName === "org.kde.plasma.folder") {
                applet.parent =  fakeIconItemContainer;
                applet.anchors.fill = fakeIconItemContainer;
            }

            wrapper.disableScaleWidth = false;
            wrapper.disableScaleHeight = false;

            wrapper.updateLayoutWidth();
            wrapper.updateLayoutHeight();
        }
    }

    onIndexChanged: {
        if (container.latteApplet) {
            root.latteAppletPos = index;
        }
    }

    /// BEGIN functions
    function checkIndex(){
        index = -1;

        for(var i=0; i<startLayout.count; ++i){
            if(startLayout.children[i] === container){
                index = startLayout.beginIndex + i;
                break;
            }
        }

        for(var i=0; i<mainLayout.count; ++i){
            if(mainLayout.children[i] === container){
                index = mainLayout.beginIndex + i;
                break;
            }
        }

        for(var i=0; i<endLayout.count; ++i){
            if(endLayout.children[i] === container){
                //create a very high index in order to not need to exchange hovering messages
                //between mainLayout and endLayout
                index = endLayout.beginIndex + i;
                break;
            }
        }


        if(container.latteApplet){
            if(index===startLayout.beginIndex || index===mainLayout.beginIndex || index===endLayout.beginIndex)
                latteApplet.disableLeftSpacer = false;
            else
                latteApplet.disableLeftSpacer = true;

            if( index === startLayout.beginIndex + startLayout.count - 1
                    || index===mainLayout.beginIndex + mainLayout.count - 1
                    || index === endLayout.beginIndex + endLayout.count - 1)
                latteApplet.disableRightSpacer = false;
            else
                latteApplet.disableRightSpacer = true;
        }
    }

    //this functions gets the signal from the plasmoid, it can be used for signal items
    //outside the LatteApplet Plasmoid
    //property int debCounter: 0;
    function clearZoom(){
        if (root.globalDirectRender){
            wrapper.zoomScale = 1;
        } else {
            restoreAnimation.start();
        }

        //if (restoreAnimation)
        //    restoreAnimation.start();
        // if(wrapper)
        //     wrapper.zoomScale = 1;
    }

    function checkCanBeHovered(){
        if ( ((applet && (applet.Layout.minimumWidth > root.iconSize) && root.isHorizontal) ||
              (applet && (applet.Layout.minimumHeight > root.iconSize) && root.isVertical))
                && (applet && applet.pluginName !== "org.kde.plasma.panelspacer")
                && !container.fakeIconItem){
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

    function reconsiderAppletIconItem(){
        if (container.appletIconItem || !applet)
            return;

        //! searching to find for that applet the first IconItem
        //! which is going to be used in order to deactivate its active
        //! from our MouseArea
        for(var i=0; i<applet.children.length; ++i){
            for(var j=0; j<applet.children[i].children.length; ++j){
                if (typeOf(applet.children[i].children[j], "IconItem")) {
                    appletIconItem = applet.children[i].children[j];
                    return;
                } else if (typeOf(applet.children[i].children[j], "CompactRepresentation")) {
                    for(var k=0; k<applet.children[i].children[j].children.length; ++k){
                        if (typeOf(applet.children[i].children[j].children[k], "IconItem")) {
                            appletIconItem = applet.children[i].children[j].children[k];
                            return;
                        }
                    }
                }
            }
        }
    }

    function typeOf(obj, className){
        var name = obj.toString();
        return ((name.indexOf(className + "(") === 0) || (name.indexOf(className + "_QML") === 0));
    }
    ///END functions

    //BEGIN connections
    onAppletChanged: {
        if (!applet) {
            destroy();
        } else {
            reconsiderAppletIconItem()
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

    onLatteAppletChanged: {
        if(container.latteApplet){
            root.latteApplet = container.latteApplet;
            root.latteAppletContainer = container;
            root.latteAppletPos = index;
            latteApplet.latteDock = root;
            latteApplet.forceHidePanel = true;
        }
    }

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

    Component.onCompleted: {
        checkIndex();
        root.updateIndexes.connect(checkIndex);
        root.clearZoomSignal.connect(clearZoom);
    }

    Component.onDestruction: {
        root.updateIndexes.disconnect(checkIndex);
        root.clearZoomSignal.disconnect(clearZoom);
    }

    ///END connections


    PlasmaComponents.BusyIndicator {
        z: 1000
        visible: applet && applet.busy
        running: visible
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width
    }

    /*  Rectangle{
        anchors.fill: parent
        color: "transparent"
        border.color: "green"
        border.width: 1
    } */

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
        Item{
            id: hiddenSpacerLeft
            //we add one missing pixel from calculations
            width: root.isHorizontal ? nHiddenSize : wrapper.width
            height: root.isHorizontal ? wrapper.height : nHiddenSize

            ///check also if this is the first plasmoid in anylayout
            visible: container.startEdge

            property real nHiddenSize: (nScale > 0) ? container.spacersMaxSize * nScale : 0
            property real nScale: 0

            Behavior on nScale {
                enabled: !root.globalDirectRender
                NumberAnimation { duration: 3*container.animationTime }
            }

            Behavior on nScale {
                enabled: root.globalDirectRender
                NumberAnimation { duration: root.directRenderAnimationTime }
            }

            Loader{
                anchors.fill: parent
                active: root.debugMode

                sourceComponent: Rectangle{
                    height: 1
                    width: parent.width
                    y: parent.width/2
                    border.width: 1
                    border.color: "red"
                    color: "transparent"
                }
            }
        }


        Item{
            id: wrapper

            width: {
                if (container.isInternalViewSplitter && !root.editMode)
                    return 0;

                if (container.needsFillSpace && (container.sizeForFill>-1) && root.isHorizontal){
                    //! in edit mode shrink a bit the fill sizes because the splitters are shown
                    return root.editMode && container.needsFillSpace && (container.sizeForFill > 5*root.iconSize) ?
                                container.sizeForFill - 2.5*root.iconSize : container.sizeForFill;
                    //return container.sizeForFill;
                }

                if (latteApplet) {
                    if (container.showZoomed && root.isVertical)
                        return root.statesLineSize + root.thickMargin + root.iconSize + 1;
                    else
                        return latteApplet.tasksWidth;
                } else {
                    return scaledWidth;
                }
            }

            height: {
                if (container.isInternalViewSplitter && !root.editMode)
                    return 0;

                if (container.needsFillSpace && (container.sizeForFill>-1) && root.isVertical){
                    //! in edit mode shrink a bit the fill sizes because the splitters are shown
                    return root.editMode && container.needsFillSpace && (container.sizeForFill > 5*root.iconSize) ?
                                container.sizeForFill - 2.5*root.iconSize : container.sizeForFill;

                    //return container.sizeForFill;
                }

                if (latteApplet) {
                    if (container.showZoomed && root.isHorizontal)
                        return root.statesLineSize + root.thickMargin + root.iconSize + 1;
                    else
                        return latteApplet.tasksHeight;
                } else {
                    return scaledHeight;
                }
            }

            //width: container.isInternalViewSplitter && !root.editMode ? 0 : Math.round( latteApplet ? ((container.showZoomed && root.isVertical) ?
            //                                                                        scaledWidth : latteApplet.tasksWidth) : scaledWidth )
            //height: container.isInternalViewSplitter&& !root.editMode ? 0 : Math.round( latteApplet ? ((container.showZoomed && root.isHorizontal) ?
            //                                                                          scaledHeight : latteApplet.tasksHeight ): scaledHeight )

            property bool disableScaleWidth: false
            property bool disableScaleHeight: false
            property bool editMode: root.editMode

            property int appletMinimumWidth: applet && applet.Layout ?  applet.Layout.minimumWidth : -1
            property int appletMinimumHeight: applet && applet.Layout ? applet.Layout.minimumHeight : -1

            property int appletPreferredWidth: applet && applet.Layout ?  applet.Layout.preferredWidth : -1
            property int appletPreferredHeight: applet && applet.Layout ?  applet.Layout.preferredHeight : -1

            property int appletMaximumWidth: applet && applet.Layout ?  applet.Layout.maximumWidth : -1
            property int appletMaximumHeight: applet && applet.Layout ?  applet.Layout.maximumHeight : -1

            property int iconSize: root.iconSize

            property int marginWidth: root.isVertical ?
                                          (applet && (applet.pluginName === "org.kde.plasma.systemtray") ? root.thickMarginBase : root.thickMargin ) :
                                          root.iconMargin
            property int marginHeight: root.isHorizontal ?
                                           (applet && (applet.pluginName === "org.kde.plasma.systemtray") ? root.thickMarginBase : root.thickMargin ) :
                                           root.iconMargin

            property real scaledWidth: zoomScaleWidth * (layoutWidth + marginWidth)
            property real scaledHeight: zoomScaleHeight * (layoutHeight + marginHeight)
            property real zoomScaleWidth: disableScaleWidth ? 1 : zoomScale
            property real zoomScaleHeight: disableScaleHeight ? 1 : zoomScale

            property int layoutWidthResult: 0

            property int layoutWidth
            property int layoutHeight

            // property int localMoreSpace: root.reverseLinesPosition ? root.statesLineSize + 2 : appletMargin
            property int localMoreSpace: appletMargin

            property int moreHeight: ((applet && (applet.pluginName === "org.kde.plasma.systemtray")) || root.reverseLinesPosition)
                                     && root.isHorizontal ? localMoreSpace : 0
            property int moreWidth: ((applet && (applet.pluginName === "org.kde.plasma.systemtray")) || root.reverseLinesPosition)
                                    && root.isVertical ? localMoreSpace : 0

            property real center: width / 2
            property real zoomScale: 1

            property alias index: container.index
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

            onAppletMinimumWidthChanged: {
                if(zoomScale == 1)
                    checkCanBeHovered();

                updateLayoutWidth();
            }

            onAppletMinimumHeightChanged: {
                if(zoomScale == 1)
                    checkCanBeHovered();

                updateLayoutHeight();
            }

            onAppletPreferredWidthChanged: updateLayoutWidth();
            onAppletPreferredHeightChanged: updateLayoutHeight();

            onAppletMaximumWidthChanged: updateLayoutWidth();
            onAppletMaximumHeightChanged: updateLayoutHeight();

            onIconSizeChanged: {
                updateLayoutWidth();
                updateLayoutHeight();
            }

            onEditModeChanged: {
                updateLayoutWidth();
                updateLayoutHeight();
            }

            onZoomScaleChanged: {
                if ((zoomScale === root.zoomFactor) && !enableDirectRenderTimer.running && !root.globalDirectRender) {
                    enableDirectRenderTimer.start();
                }

                if ((zoomScale > 1) && !container.isZoomed) {
                    container.isZoomed = true;
                    if (!root.editMode && !animationWasSent) {
                        root.slotAnimationsNeedBothAxis(1);
                        animationWasSent = true;
                    }
                } else if ((zoomScale == 1) && container.isZoomed) {
                    container.isZoomed = false;
                    if (!root.editMode && animationWasSent) {
                        root.slotAnimationsNeedBothAxis(-1);
                        animationWasSent = false;
                    }
                }
            }

            Connections {
                target: root
                onIsVerticalChanged: {
                    if (container.latteApplet) {
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

                if(container.isInternalViewSplitter){
                    if(!root.editMode)
                        layoutHeight = 0;
                    else
                        layoutHeight = root.iconSize;// + moreHeight + root.statesLineSize;
                }
                else if(applet && applet.pluginName === "org.kde.plasma.panelspacer"){
                    layoutHeight = root.iconSize + moreHeight;
                }
                else if(applet && applet.pluginName === "org.kde.plasma.systemtray" && root.isHorizontal){
                    layoutHeight = root.statesLineSize + root.iconSize;
                }
                else{
                    if(applet && (applet.Layout.minimumHeight > root.iconSize) && root.isVertical && !canBeHovered && !container.fakeIconItem){
                        // return applet.Layout.minimumHeight;
                        layoutHeight = applet.Layout.minimumHeight;
                    } //it is used for plasmoids that need to scale only one axis... e.g. the Weather Plasmoid
                    else if(applet
                            && ( (applet.Layout.maximumHeight < root.iconSize) || (applet.Layout.preferredHeight > root.iconSize))
                            && root.isVertical
                            && !disableScaleWidth
                            && !container.fakeIconItem) {
                        //&& !root.editMode ){
                        disableScaleHeight = true;
                        //this way improves performance, probably because during animation the preferred sizes update a lot
                        if((applet.Layout.maximumHeight < root.iconSize)){
                            layoutHeight = applet.Layout.maximumHeight;
                        }
                        else if (applet.Layout.minimumHeight > root.iconSize){
                            layoutHeight = applet.Layout.minimumHeight;
                        }
                        else if ((applet.Layout.preferredHeight > root.iconSize)){
                            layoutHeight = applet.Layout.preferredHeight;
                        }
                        else{
                            layoutHeight = root.iconSize + moreHeight;
                        }
                    }
                    else
                        layoutHeight = root.iconSize + moreHeight;
                }
                //return root.iconSize + moreHeight;
            }

            function updateLayoutWidth(){

                if(container.isInternalViewSplitter){
                    if(!root.editMode)
                        layoutWidth = 0;
                    else
                        layoutWidth = root.iconSize; //+ moreWidth+ root.statesLineSize;
                }
                else if(applet && applet.pluginName === "org.kde.plasma.panelspacer"){
                    layoutWidth = root.iconSize + moreWidth;
                }
                else if(applet && applet.pluginName === "org.kde.plasma.systemtray" && root.isVertical){
                    layoutWidth = root.statesLineSize + root.iconSize;
                }
                else{
                    if(applet && (applet.Layout.minimumWidth > root.iconSize) && root.isHorizontal && !canBeHovered && !container.fakeIconItem){
                        layoutWidth = applet.Layout.minimumWidth;
                    } //it is used for plasmoids that need to scale only one axis... e.g. the Weather Plasmoid
                    else if(applet
                            && ( (applet.Layout.maximumWidth < root.iconSize) || (applet.Layout.preferredWidth > root.iconSize))
                            && root.isHorizontal
                            && !disableScaleHeight
                            && !container.fakeIconItem){
                        //  && !root.editMode){
                        disableScaleWidth = true;
                        //this way improves performance, probably because during animation the preferred sizes update a lot
                        if((applet.Layout.maximumWidth < root.iconSize)){
                            //   return applet.Layout.maximumWidth;
                            layoutWidth = applet.Layout.maximumWidth;
                        }
                        else if (applet.Layout.minimumWidth > root.iconSize){
                            layoutWidth = applet.Layout.minimumWidth;
                        }
                        else if (applet.Layout.preferredWidth > root.iconSize){
                            layoutWidth = applet.Layout.preferredWidth;
                        }
                        else{
                            layoutWidth = root.iconSize + moreWidth;
                        }
                    }
                    else{
                        //return root.iconSize + moreWidth;
                        layoutWidth = root.iconSize + moreWidth;
                    }
                }
            }

            Item{
                id:wrapperContainer

                width:{
                    if (container.needsFillSpace && (container.sizeForFill>-1) && root.isHorizontal){
                        return wrapper.width;
                    }

                    if (container.isInternalViewSplitter)
                        return wrapper.layoutWidth;
                    else
                        return parent.zoomScaleWidth * wrapper.layoutWidth;
                }

                height:{
                    if (container.needsFillSpace && (container.sizeForFill>-1) && root.isVertical){
                        return wrapper.height;
                    }

                    if (container.isInternalViewSplitter)
                        return wrapper.layoutHeight;
                    else
                        return parent.zoomScaleHeight * wrapper.layoutHeight;
                }

                //width: Math.round( container.isInternalViewSplitter ? wrapper.layoutWidth : parent.zoomScaleWidth * wrapper.layoutWidth )
                //height: Math.round( container.isInternalViewSplitter ? wrapper.layoutHeight : parent.zoomScaleHeight * wrapper.layoutHeight )

                anchors.rightMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? root.thickMarginBase : 0
                anchors.leftMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? root.thickMarginBase : 0
                anchors.topMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? root.thickMarginBase : 0
                anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? root.thickMarginBase : 0

                //BEGIN states
                states: [
                    State {
                        name: "left"
                        when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

                        AnchorChanges {
                            target: wrapperContainer
                            anchors{ verticalCenter:wrapper.verticalCenter; horizontalCenter:undefined;
                                top:undefined; bottom:undefined; left:parent.left; right:undefined;}
                        }
                    },
                    State {
                        name: "right"
                        when: (plasmoid.location === PlasmaCore.Types.RightEdge)

                        AnchorChanges {
                            target: wrapperContainer
                            anchors{ verticalCenter:wrapper.verticalCenter; horizontalCenter:undefined;
                                top:undefined; bottom:undefined; left:undefined; right:parent.right;}
                        }
                    },
                    State {
                        name: "bottom"
                        when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

                        AnchorChanges {
                            target: wrapperContainer
                            anchors{ verticalCenter:undefined; horizontalCenter:wrapper.horizontalCenter;
                                top:undefined; bottom:parent.bottom; left:undefined; right:undefined;}
                        }
                    },
                    State {
                        name: "top"
                        when: (plasmoid.location === PlasmaCore.Types.TopEdge)

                        AnchorChanges {
                            target: wrapperContainer
                            anchors{  verticalCenter:undefined; horizontalCenter:wrapper.horizontalCenter;
                                top:parent.top; bottom:undefined; left:undefined; right:undefined;}
                        }
                    }
                ]
                //END states

                ///Secret MouseArea to be used by the folder widget
                Loader{
                    anchors.fill: parent
                    active: container.fakeIconItem && applet.pluginName === "org.kde.plasma.folder"
                    sourceComponent: MouseArea{
                        onClicked: dock.toggleAppletExpanded(applet.id);
                    }
                }

                Item{
                    id: fakeIconItemContainer
                    anchors.centerIn: parent

                    //we setup as maximum for hidden container of some applets that break
                    //the Latte experience the size:96 . This is why after that size
                    //the folder widget changes to fullRepresentation instead of compact one
                    width: Math.min(96, parent.width)
                    height: width
                }

                Loader{
                    anchors.fill: parent
                    active: container.fakeIconItem
                    sourceComponent: Latte.IconItem{
                        id: fakeAppletIconItem
                        anchors.fill: parent
                        source: appletIconItem.source
                    }
                }
            }

            //spacer background
            Loader{
                anchors.fill: wrapperContainer
                active: applet && (applet.pluginName === "org.kde.plasma.panelspacer") && root.editMode

                sourceComponent: Rectangle{
                    anchors.fill: parent
                    border.width: 1
                    border.color: theme.textColor
                    color: "transparent"
                    opacity: 0.7

                    radius: root.iconMargin
                    Rectangle{
                        anchors.centerIn: parent
                        color: parent.border.color

                        width: parent.width - 1
                        height: parent.height - 1

                        opacity: 0.2
                    }
                }
            }

            Loader{
                anchors.fill: wrapperContainer
                active: container.isInternalViewSplitter
                        && root.editMode

                rotation: root.isVertical ? 90 : 0

                sourceComponent: Image{
                    id:splitterImage
                    anchors.fill: parent

                    source: (container.internalSplitterId===1) ? "../icons/splitter.png" : "../icons/splitter2.png"

                    layer.enabled: true
                    layer.effect: DropShadow {
                        radius: shadowSize
                        samples: 2 * radius
                        color: "#ff080808"

                        verticalOffset: 2

                        property int shadowSize : Math.ceil(root.iconSize / 10)
                    }

                    Component.onCompleted: wrapper.zoomScale = 1.1
                }
            }

            ///Shadow in applets
            Loader{
                anchors.fill: container.appletWrapper

                active: container.applet
                        &&((plasmoid.configuration.shadows === 1 /*Locked Applets*/
                            && (!container.canBeHovered || (container.lockZoom && (applet.pluginName !== root.plasmoidName))) )
                           || (plasmoid.configuration.shadows === 2 /*All Applets*/
                               && (applet.pluginName !== root.plasmoidName)))

                sourceComponent: DropShadow{
                    anchors.fill: parent
                    color: "#ff080808"
                    samples: 2 * radius
                    source: container.fakeIconItem ? wrapperContainer : container.applet
                    radius: shadowSize
                    verticalOffset: 2

                    property int shadowSize : Math.ceil(root.iconSize / 12)
                }
            }

            BrightnessContrast{
                id:hoveredImage
                anchors.fill: wrapperContainer
                source: wrapperContainer

                enabled: opacity != 0 ? true : false
                opacity: appletMouseArea.containsMouse ? 1 : 0
                brightness: 0.25
                contrast: 0.15

                Behavior on opacity {
                    NumberAnimation { duration: root.durationTime*units.longDuration }
                }
            }

            BrightnessContrast {
                id: clickedEffect
                anchors.fill: wrapperContainer
                source: wrapperContainer

                visible: clickedAnimation.running
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
                    border.color:  (container.needsFillSpace && (container.sizeForFill>-1) && root.isHorizontal) ? "red" : "green"
                    border.width: 1
                }
            }

            Behavior on zoomScale {
                enabled: !root.globalDirectRender
                NumberAnimation { duration: 3*container.animationTime }
            }

            Behavior on zoomScale {
                enabled: root.globalDirectRender
                NumberAnimation { duration: root.directRenderAnimationTime }
            }

            //!this is used in order to update the index when the signal is for the internal latte plasmoid
            function updateIdSendScale(indx, zScale, zStep){
                if(root.latteApplet && ((index<root.latteAppletPos && indx>=root.latteAppletPos)
                                        || (index>root.latteAppletPos && indx<=root.latteAppletPos)) ){
                    var appStep = Math.abs(root.latteAppletPos-index);
                    var signalStep = Math.abs(indx - index);

                    if(index<root.latteAppletPos){
                        root.latteApplet.updateScale(signalStep-appStep, zScale,zStep);
                    } else if (index>root.latteAppletPos){
                        root.latteApplet.updateScale(root.tasksCount-1 - (signalStep-appStep), zScale,zStep);
                    }
                } else {
                    root.updateScale(indx, zScale, zStep);
                }
            }


            function calculateScales( currentMousePosition ){
                if (root.editMode || root.zoomFactor===1 || root.durationTime===0) {
                    return;
                }

                var distanceFromHovered = Math.abs(index - layoutsContainer.hoveredIndex);

                // A new algorithm tryig to make the zoom calculation only once
                // and at the same time fixing glitches
                if ((distanceFromHovered == 0)&&
                        (currentMousePosition  > 0) ){

                    var rDistance = Math.abs(currentMousePosition  - center);

                    //check if the mouse goes right or down according to the center
                    var positiveDirection =  ((currentMousePosition  - center) >= 0 );


                    //finding the zoom center e.g. for zoom:1.7, calculates 0.35
                    var zoomCenter = (root.zoomFactor - 1) / 2

                    //computes the in the scale e.g. 0...0.35 according to the mouse distance
                    //0.35 on the edge and 0 in the center
                    var firstComputation = (rDistance / center) * zoomCenter;

                    //calculates the scaling for the neighbour tasks
                    var bigNeighbourZoom = Math.min(1 + zoomCenter + firstComputation, root.zoomFactor);
                    var smallNeighbourZoom = Math.max(1 + zoomCenter - firstComputation, 1);

                    //bigNeighbourZoom = Number(bigNeighbourZoom.toFixed(4));
                    //smallNeighbourZoom = Number(smallNeighbourZoom.toFixed(4));

                    var leftScale;
                    var rightScale;

                    if(positiveDirection === true){
                        rightScale = bigNeighbourZoom;
                        leftScale = smallNeighbourZoom;
                    }
                    else {
                        rightScale = smallNeighbourZoom;
                        leftScale = bigNeighbourZoom;
                    }

                    //! compute the neighbour separator scales
                    var bsNeighbourZoom = 1;
                    var ssNeighbourZoom = 1;

                    if(root.latteApplet && root.latteApplet.internalSeparatorPos>=0) {
                        var latApp = root.latteApplet;
                        if((latApp.internalSeparatorPos === 0) || (latApp.internalSeparatorPos === root.tasksCount-1) ){
                            var sepZoomDifference = (5+root.iconMargin) / root.realSize;

                            bsNeighbourZoom = Math.max(1,bigNeighbourZoom - sepZoomDifference);
                            ssNeighbourZoom = Math.max(1,smallNeighbourZoom - sepZoomDifference);
                        }
                    }

                    if(!root.latteApplet || Math.abs(root.latteAppletPos-index)>1 || !root.hasInternalSeparator
                            || (root.hasInternalSeparator
                                && ((root.latteApplet.internalSeparatorPos>0 && root.latteApplet.internalSeparatorPos<root.tasksCount-1)
                                    || (root.latteApplet.internalSeparatorPos===0 && index>root.latteAppletPos)
                                    || (root.latteApplet.internalSeparatorPos===root.tasksCount-1 && index<root.latteAppletPos)))
                            ){
                        updateIdSendScale(index-1, leftScale, 0);
                        updateIdSendScale(index+1, rightScale, 0);

                        updateIdSendScale(index-2, 1, 0);
                        updateIdSendScale(index+2, 1 ,0);
                    } else{
                        if(latApp.internalSeparatorPos === 0){
                            if (!positiveDirection) {
                                updateIdSendScale(index+2, ssNeighbourZoom, 0);
                            } else {
                                updateIdSendScale(index+2, bsNeighbourZoom, 0);
                            }

                            updateIdSendScale(index-1, leftScale, 0);
                            updateIdSendScale(index+1, rightScale, 0);

                            updateIdSendScale(index+3, 1, 0);
                            updateIdSendScale(index-2, 1, 0);
                        } else if(root.hasInternalSeparator && latApp.internalSeparatorPos === root.tasksCount-1) {
                            if (!positiveDirection) {
                                updateIdSendScale(index-2, bsNeighbourZoom, 0);
                            } else {
                                updateIdSendScale(index-2, ssNeighbourZoom, 0);
                            }

                            updateIdSendScale(index-1, leftScale, 0);
                            updateIdSendScale(index+1, rightScale, 0);

                            updateIdSendScale(index+2, 1, 0);
                            updateIdSendScale(index-3, 1, 0);
                        }
                    }

                    if (root.latteApplet && Math.abs(index - root.latteAppletPos) > 2){
                        root.latteApplet.clearZoom();
                    }

                    //Left hiddenSpacer
                    if(container.startEdge){
                        hiddenSpacerLeft.nScale = leftScale - 1;
                    }

                    //Right hiddenSpacer  ///there is one more item in the currentLayout ????
                    if(container.endEdge){
                        hiddenSpacerRight.nScale =  rightScale - 1;
                    }

                    zoomScale = root.zoomFactor;
                }

            } //scale


            function signalUpdateScale(nIndex, nScale, step){
                if(container && (container.index === nIndex)){
                    reconsiderAppletIconItem();

                    if ( ((canBeHovered && !lockZoom ) || container.latteApplet)
                            && (applet && applet.status !== PlasmaCore.Types.HiddenStatus)
                            //&& (index != currentLayout.hoveredIndex)
                            ){
                        if(!container.latteApplet){
                            if(nScale >= 0)
                                zoomScale = nScale + step;
                            else
                                zoomScale = zoomScale + step;
                        }
                    }  ///if the applet is hidden must forward its scale events to its neighbours
                    else if ((applet && (applet.status === PlasmaCore.Types.HiddenStatus))
                             || container.isInternalViewSplitter){
                        if(layoutsContainer.hoveredIndex>index)
                            root.updateScale(index-1, nScale, step);
                        else if((layoutsContainer.hoveredIndex<index))
                            root.updateScale(index+1, nScale, step);
                    }
                }
            }

            Component.onCompleted: {
                root.updateScale.connect(signalUpdateScale);
            }
        }// Main task area // id:wrapper

        // a hidden spacer on the right for the last item to add stability
        Item{
            id: hiddenSpacerRight
            //we add one missing pixel from calculations
            width: root.isHorizontal ? nHiddenSize : wrapper.width
            height: root.isHorizontal ? wrapper.height : nHiddenSize

            //check if this last plasmoid in any layout
            visible: container.endEdge

            property real nHiddenSize: (nScale > 0) ? (container.spacersMaxSize * nScale) : 0
            property real nScale: 0

            Behavior on nScale {
                enabled: !root.globalDirectRender
                NumberAnimation { duration: 3*container.animationTime }
            }

            Behavior on nScale {
                enabled: root.globalDirectRender
                NumberAnimation { duration: root.directRenderAnimationTime }
            }

            Loader{
                anchors.fill: parent
                active: root.debugMode

                sourceComponent: Rectangle{
                    height: 1
                    width: parent.width
                    y: parent.width/2
                    border.width: 1
                    border.color: "red"
                    color: "transparent"
                }
            }
        }

    }// Flow with hidden spacers inside

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
            level: 3
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            elide: Text.ElideRight

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
        enabled: (!latteApplet)&&(canBeHovered)&&(!root.editMode)//&&(!lockZoom)
        hoverEnabled: !root.editMode && (!latteApplet) ? true : false
        propagateComposedEvents: true

        //! a way must be found in order for this be enabled
        //! only to support springloading for plasma 5.10
        //! also on this is based the tooltips behavior by enabling it
        //! plasma tooltips are disabled
        visible: !lockZoom && canBeHovered && (root.zoomFactor>1)

        property bool pressed: false

        onClicked: {
            pressed = false;
            mouse.accepted = false;
        }

        onEntered: {
            reconsiderAppletIconItem();

            if (lockZoom || !canBeHovered) {
                return;
            }

            layoutsContainer.hoveredIndex = index;

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
            if (appletIconItem)
                appletIconItem.active = false;

            if (root.zoomFactor>1){
                checkRestoreZoom.start();
            }
        }

        onPositionChanged: {
            //  if(!pressed){
            if (lockZoom || !canBeHovered) {
                mouse.accepted = false;
                return;
            }

            if (root.isHorizontal){
                var step = Math.abs(layoutsContainer.currentSpot-mouse.x);
                if (step >= container.animationStep){
                    layoutsContainer.hoveredIndex = index;
                    layoutsContainer.currentSpot = mouse.x;

                    wrapper.calculateScales(mouse.x);
                }
            }
            else{
                var step = Math.abs(layoutsContainer.currentSpot-mouse.y);
                if (step >= container.animationStep){
                    layoutsContainer.hoveredIndex = index;
                    layoutsContainer.currentSpot = mouse.y;

                    wrapper.calculateScales(mouse.y);
                }
            }
            //  }
            mouse.accepted = false;
        }

        onPressed: {
            pressed = true;
            mouse.accepted = false;
        }

        onReleased: {
            pressed = false;
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
            duration: 3 * container.animationTime
            easing.type: Easing.Linear
        }

        PropertyAnimation {
            target: hiddenSpacerLeft
            property: "nScale"
            to: 0
            duration: 3 * container.animationTime
            easing.type: Easing.Linear
        }

        PropertyAnimation {
            target: hiddenSpacerRight
            property: "nScale"
            to: 0
            duration: 3 * container.animationTime
            easing.type: Easing.Linear
        }
    }


    /////Clicked Animation/////
    SequentialAnimation{
        id: clickedAnimation
        alwaysRunToEnd: true
        running: appletMouseArea.pressed && (root.durationTime > 0)

        onStopped: appletMouseArea.pressed = false;

        ParallelAnimation{
            PropertyAnimation {
                target: clickedEffect
                property: "brightness"
                to: -0.35
                duration: units.longDuration
                easing.type: Easing.OutQuad
            }
        }
        ParallelAnimation{
            PropertyAnimation {
                target: clickedEffect
                property: "brightness"
                to: 0
                duration: units.longDuration
                easing.type: Easing.OutQuad
            }
        }
    }
    //END animations
}
