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
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

import "applet" as Applet

import "../code/LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root
    objectName: "dockLayoutView"

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    //// BEGIN SIGNALS
    signal clearZoomSignal();
    signal separatorsUpdated();
    signal updateEffectsArea();
    signal updateIndexes();
    signal updateScale(int delegateIndex, real newScale, real step);
    //// END SIGNALS

    ////BEGIN properties
    property bool debugMode: Qt.application.arguments.indexOf("--graphics")>=0
    property bool debugModeSpacers: Qt.application.arguments.indexOf("--spacers")>=0
    property bool debugModeTimers: Qt.application.arguments.indexOf("--timers")>=0
    property bool debugModeWindow: Qt.application.arguments.indexOf("--with-window")>=0

    property bool globalDirectRender: false //it is used as a globalDirectRender for all elements in the dock
    property bool directRenderTimerIsRunning: enableDirectRenderTimer.running
    property int directRenderAnimationTime: 0

    property bool addLaunchersMessage: false
    property bool addLaunchersInTaskManager: plasmoid.configuration.addLaunchersInTaskManager
    // when there are only plasma style task managers the automatic icon size algorithm should better be disabled
    property bool autoDecreaseIconSize: plasmoid.configuration.autoDecreaseIconSize && !containsOnlyPlasmaTasks
    property bool backgroundOnlyOnMaximized: plasmoid.configuration.backgroundOnlyOnMaximized
    property bool behaveAsPlasmaPanel: {
        if (!dock || !dock.visibility)
            return false;

        return (visibilityManager.panelIsBiggerFromIconSize && (zoomFactor === 1.0)
                && (dock.visibility.mode === Latte.Dock.AlwaysVisible || dock.visibility.mode === Latte.Dock.WindowsGoBelow)
                && (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && !(root.solidPanel && panelShadowsActive));
    }

    property bool blurEnabled: plasmoid.configuration.blurEnabled && !root.forceTransparentPanel
                               || (hasExpandedApplet && zoomFactor===1 && plasmoid.configuration.panelSize===100)

    property bool confirmedDragEntered: false
    property bool containsOnlyPlasmaTasks: false //this is flag to indicate when from tasks only a plasma based one is found
    property bool dockContainsMouse: dock && dock.visibility ? dock.visibility.containsMouse : false

    property bool disablePanelShadowMaximized: plasmoid.configuration.disablePanelShadowForMaximized
    property bool drawShadowsExternal: panelShadowsActive && behaveAsPlasmaPanel
    property bool editMode: plasmoid.userConfiguring
    property bool forceSolidPanel:  plasmoid.configuration.solidBackgroundForMaximized && windowsModel.hasMaximizedWindow
    property bool forceTransparentPanel: root.backgroundOnlyOnMaximized && !windowsModel.hasMaximizedWindow && Latte.WindowSystem.compositingActive
                                         && !(hasExpandedApplet && zoomFactor===1 && plasmoid.configuration.panelSize===100)

    readonly property bool hasExpandedApplet: plasmoid.applets.some(function (item) {
        return (item.status >= PlasmaCore.Types.NeedsAttentionStatus
                && item.status !== PlasmaCore.Types.HiddenStatus);
    })

    property bool immutable: plasmoid.immutable
    property bool indicateAudioStreams: plasmoid.configuration.indicateAudioStreams
    property bool inFullJustify: (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && (plasmoid.configuration.maxLength===100)
    property bool inSlidingIn: visibilityManager ? visibilityManager.inSlidingIn : false
    property bool inSlidingOut: visibilityManager ? visibilityManager.inSlidingOut : false
    property bool inStartup: true
    property bool isHalfShown: false //is used to disable the zoom hovering effect at sliding in-out the dock
    property bool isHorizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    property bool isVertical: !isHorizontal
    property bool isHovered: latteApplet ? ((latteAppletHoveredIndex !== -1) && (layoutsContainer.hoveredIndex !== -1)) //|| wholeArea.containsMouse
                                         : (layoutsContainer.hoveredIndex !== -1) //|| wholeArea.containsMouse
    property bool mouseWheelActions: plasmoid.configuration.mouseWheelActions
    property bool normalState : false
    property bool onlyAddingStarup: true //is used for the initialization phase in startup where there arent removals, this variable provides a way to grow icon size
    property bool shrinkThickMargins: plasmoid.configuration.shrinkThickMargins
    property bool solidPanel: Latte.WindowSystem.compositingActive ? plasmoid.configuration.solidPanel : true

    //FIXME: possibly this is going to be the default behavior, this user choice
    //has been dropped from the Dock Configuration Window
    //property bool smallAutomaticIconJumps: plasmoid.configuration.smallAutomaticIconJumps
    property bool smallAutomaticIconJumps: true
    property bool useThemePanel: noApplets === 0 || !Latte.WindowSystem.compositingActive ? true : plasmoid.configuration.useThemePanel

    property alias hoveredIndex: layoutsContainer.hoveredIndex

    property int activeIndicator: plasmoid.configuration.activeIndicator

    property int actionsBlockHiding: 0 //actions that block hiding

    property int animationsNeedBothAxis:0 //animations need space in both axes, e.g zooming a task
    property int animationsNeedLength: 0 // animations need length, e.g. adding a task
    property int animationsNeedThickness: 0 // animations need thickness, e.g. bouncing animation

    property int automaticIconSizeBasedSize: -1 //it is not set, this is the defautl

    //what is the highest icon size based on what icon size is used, screen calculated or user specified
    property int maxIconSize: proportionIconSize!==-1 ? proportionIconSize : plasmoid.configuration.iconSize
    property int iconSize: automaticIconSizeBasedSize > 0 && autoDecreaseIconSize ?
                               Math.min(automaticIconSizeBasedSize, root.maxIconSize) :
                               root.maxIconSize

    property int proportionIconSize: { //icon size based on screen height
        if ((plasmoid.configuration.proportionIconSize===-1) || !dock)
            return -1;

        return Math.max(16,Math.round(dock.screenGeometry.height * plasmoid.configuration.proportionIconSize/100/8)*8);
    }

    property int iconStep: 8
    property int latteAppletPos: -1
    property int maxLength: root.isHorizontal ? width * (plasmoid.configuration.maxLength/100)
                                              : height * (plasmoid.configuration.maxLength/100)

    property int middleClickAction: plasmoid.configuration.middleClickAction
    property int modifier: plasmoid.configuration.modifier
    property int modifierClickAction: plasmoid.configuration.modifierClickAction
    property int modifierClick: plasmoid.configuration.modifierClick

    property int panelEdgeSpacing: iconSize / 3
    property int panelTransparency: plasmoid.configuration.panelTransparency
    property bool panelShadowsActive: (( (plasmoid.configuration.panelShadows && !root.backgroundOnlyOnMaximized)
                                      || (plasmoid.configuration.panelShadows &&  root.backgroundOnlyOnMaximized && !root.forceTransparentPanel))
                                      && !(disablePanelShadowMaximized && windowsModel.hasMaximizedWindow))
                                      || (hasExpandedApplet && zoomFactor===1 && plasmoid.configuration.panelSize===100 && !(root.solidPanel && !plasmoid.configuration.panelShadows) )


    property int appShadowOpacity: (plasmoid.configuration.shadowOpacity/100) * 255
    property int appShadowSize: (0.4*root.iconSize) * (plasmoid.configuration.shadowSize/100)
    property string appShadowColor: "#" + decimalToHex(appShadowOpacity) + plasmoid.configuration.shadowColor

    property int totalPanelEdgeSpacing: 0 //this is set by PanelBox
    //FIXME: this is not needed any more probably
    property int previousAllTasks: -1    //is used to forbit updateAutomaticIconSize when hovering
    property int offset: {
        if (behaveAsPlasmaPanel) {
            return 0;
        }

        if (root.isHorizontal) {
            return width * (plasmoid.configuration.offset/100);
        } else {
            height * (plasmoid.configuration.offset/100)
        }
    }

    //center the layout correctly when the user uses an offset
    property int offsetFixed: (offset===0 || panelAlignment === Latte.Dock.Center || plasmoid.configuration.panelPosition === Latte.Dock.Justify)?
                                  offset : offset+panelMarginLength/2+totalPanelEdgeSpacing/2

    property int realSize: iconSize + iconMargin
    property int realPanelSize: 0
    property int realPanelLength: 0
    property int realPanelThickness: 0
    //this is set by the PanelBox
    property int panelMargin: 0
    property int panelMarginLength: 0
    property int panelShadow: 0 //shadowsSize
    property int editShadow: 7
    property int themePanelSize: {
        //root.statesLineSize + root.iconSize + root.iconMargin + 1
        var panelBase = root.statesLineSize + root.panelMargin;
        var margin = latteApplet ? thickMargin : 0;
        var maxPanelSize = (root.statesLineSize + iconSize + margin + 1) - panelBase;
        var percentage = Latte.WindowSystem.compositingActive ? plasmoid.configuration.panelSize/100 : 1;
        return Math.max(panelBase, panelBase + percentage*maxPanelSize);
    }

    //decouple iconMargin which now is used only for length calculations with thickMargins
    //which are used for thickness calculations
    property int thickMarginBase: shrinkThickMargins ? 0 : Math.ceil(0.06 * iconSize)
    property int thickMarginHigh: shrinkThickMargins ? 0 : Math.ceil(0.06 * iconSize)
    property int thickMargin: thickMarginBase + thickMarginHigh

    //it is used in order to not break the calculations for the thickness placement
    //especially in automatic icon sizes calculations
    property int thickMarginOriginal: Math.ceil(0.12 * maxIconSize)

    //! iconMargin from configuration is a percentage. The calculation provides a length
    //! for that value between 0.04 - 0.5 of iconSize, this way 100% iconMargin means
    //! equal to the iconSize
    property int iconMargin: Math.ceil( ((0.5 * (plasmoid.configuration.iconMargin))/100) * iconSize)
    property int statesLineSize: latteApplet ?  Math.ceil( root.iconSize/13 ) : 0


    ///FIXME: <delete both> I can't remember why this is needed, maybe for the anchorings!!! In order for the Double Layout to not mess the anchorings...
    //property int layoutsContainer.mainLayoutPosition: !plasmoid.immutable ? Latte.Dock.Center : (root.isVertical ? Latte.Dock.Top : Latte.Dock.Left)
    //property int panelAlignment: plasmoid.configuration.panelPosition !== Latte.Dock.Justify ? plasmoid.configuration.panelPosition : layoutsContainer.mainLayoutPosition

    property int panelAlignment: !root.editMode ? plasmoid.configuration.panelPosition :
                                                  ( plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                                       Latte.Dock.Center : plasmoid.configuration.panelPosition )

    property real zoomFactor: (Latte.WindowSystem.compositingActive && durationTime>0) ? ( 1 + (plasmoid.configuration.zoomLevel / 20) ) : 1

    readonly property string plasmoidName: "org.kde.latte.plasmoid"

    property var iconsArray: [16, 22, 32, 48, 64, 96, 128, 256]
    property var layoutManager: LayoutManager

    property Item dragOverlay
    property Item toolBox
    property Item latteAppletContainer
    property Item latteApplet
    property Item parabolicManager: _parabolicManager

    property QtObject dock
    property QtObject universalSettings
    property QtObject universalLayoutManager


    // TO BE DELETED, if not needed: property int counter:0;

    ///BEGIN properties provided to Latte Plasmoid
    //shadows for applets, it should be removed as the appleitems dont need it any more
    property bool enableShadows: plasmoid.configuration.shadows || root.forceTransparentPanel
    property bool dockIsHidden: dock ? dock.visibility.isHidden : true
    property bool dotsOnActive: plasmoid.configuration.dotsOnActive
    property bool highlightWindows: plasmoid.configuration.highlightWindows
    property bool reverseLinesPosition: !latteApplet && plasmoid.configuration.panelSize===100 ? true : plasmoid.configuration.reverseLinesPosition
    property bool showGlow: plasmoid.configuration.showGlow
    property bool glow3D: plasmoid.configuration.glow3D
    property bool showToolTips: plasmoid.configuration.showToolTips
    property bool showWindowActions: plasmoid.configuration.showWindowActions
    property bool showOnlyCurrentScreen: plasmoid.configuration.showOnlyCurrentScreen
    property bool showOnlyCurrentDesktop: plasmoid.configuration.showOnlyCurrentDesktop
    property bool showOnlyCurrentActivity: plasmoid.configuration.showOnlyCurrentActivity

    property bool smartLaunchersEnabled: plasmoid.configuration.smartLaunchersEnabled
    property bool threeColorsWindows: plasmoid.configuration.threeColorsWindows
    property bool titleTooltips: plasmoid.configuration.titleTooltips

    readonly property bool hasInternalSeparator: latteApplet ? latteApplet.hasInternalSeparator : false

    property int activeIndicatorType: plasmoid.configuration.activeIndicatorType
    property int glowOption: plasmoid.configuration.glowOption
    property real glowOpacity: plasmoid.configuration.glowOpacity/100

    property int latteAppletHoveredIndex: latteApplet ? latteApplet.hoveredIndex : -1
    property int launchersGroup: plasmoid.configuration.launchersGroup
    property int tasksCount: latteApplet ? latteApplet.tasksCount : 0

    property real durationTime: {
        if (plasmoid.configuration.durationTime === 0 || plasmoid.configuration.durationTime === 2 )
            return plasmoid.configuration.durationTime;

        if (plasmoid.configuration.durationTime === 1)
            return 1.65;
        else if (plasmoid.configuration.durationTime === 3)
            return 2.35;

        return 2;
    }

    property rect screenGeometry: dock ? dock.screenGeometry : plasmoid.screenGeometry
    ///END properties from latteApplet

    /* Layout.preferredWidth: plasmoid.immutable ?
                               (plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                    layoutsContainer.width + 0.5*iconMargin : layoutsContainer.mainLayout.width + iconMargin) :
                               Screen.width //on unlocked state use the maximum

    Layout.preferredHeight: plasmoid.immutable ?
                               (plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                    layoutsContainer.height + 0.5*iconMargin : layoutsContainer.mainLayout.height + iconMargin) :
                               Screen.height //on unlocked state use the maximum*/

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    //// BEGIN properties in functions
    property int noApplets: {
        var count1 = 0;
        var count2 = 0;

        count1 = layoutsContainer.mainLayout.children.length;
        var tempLength = layoutsContainer.mainLayout.children.length;

        for (var i=tempLength-1; i>=0; --i) {
            var applet = layoutsContainer.mainLayout.children[i];
            if (applet && (applet === dndSpacer || applet === lastSpacer ||  applet.isInternalViewSplitter))
                count1--;
        }

        count2 = layoutsContainer.endLayout.children.length;
        tempLength = layoutsContainer.endLayout.children.length;

        for (var i=tempLength-1; i>=0; --i) {
            var applet = layoutsContainer.endLayout.children[i];
            if (applet && (applet === dndSpacer || applet === lastSpacer  || applet.isInternalViewSplitter))
                count2--;
        }

        return (count1 + count2);
    }

    ///The index of user's current icon size
    property int currentIconIndex:{
        for(var i=iconsArray.length-1; i>=0; --i){
            if(iconsArray[i] === iconSize){
                return i;
            }
        }
        return 3;
    }

    //// END properties in functions

    ////////////////END properties




    //// BEGIN OF Behaviors
    Behavior on iconSize {
        NumberAnimation {
            duration: 200

            onRunningChanged: {
                if (!running) {
                    delayUpdateMaskArea.start();
                }
            }
        }
    }
    //// END OF Behaviors

    //////////////START OF CONNECTIONS
    onContainsOnlyPlasmaTasksChanged: updateAutomaticIconSize();

    onEditModeChanged: {
        if (editMode) {
            visibilityManager.updateMaskArea();
        } else {
            updateAutomaticIconSize();
            layoutsContainer.updateSizeForAppletsInFill();
        }

        updateLayouts();

        //! This is used in case the dndspacer has been left behind
        //! e.g. the user drops a folder and a context menu is appearing
        //! but the user decides to not make a choice for the applet type
        if (dndSpacer.parent !== root) {
            dndSpacer.parent = root;
        }
    }

    onDockChanged: {
        if (dock) {
            dock.onAddInternalViewSplitter.connect(addInternalViewSplitters);
            dock.onRemoveInternalViewSplitter.connect(removeInternalViewSplitters);

            dock.onXChanged.connect(visibilityManager.updateMaskArea);
            dock.onYChanged.connect(visibilityManager.updateMaskArea);
            dock.onWidthChanged.connect(visibilityManager.updateMaskArea);
            dock.onHeightChanged.connect(visibilityManager.updateMaskArea);

            dock.visibility.onContainsMouseChanged.connect(visibilityManager.slotContainsMouseChanged);
            dock.visibility.onMustBeHide.connect(visibilityManager.slotMustBeHide);
            dock.visibility.onMustBeShown.connect(visibilityManager.slotMustBeShown);

            updateContainsOnlyPlasmaTasks();
        }
    }

    onDragEnter: {
        if (plasmoid.immutable) {
            event.ignore();
            return;
        }

        if (event.mimeData.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") >= 0) {
            return;
        }

        if (latteApplet) {
            if (latteApplet.launchersDrop(event)) {
                root.addLaunchersMessage = true;
                if (root.addLaunchersInTaskManager) {
                    return;
                }
            }
        }

        if (!confirmedDragEntered) {
            confirmedDragEntered = true;
            slotAnimationsNeedLength(1);
        }

        if (!latteApplet || (latteApplet && !dock.mimeContainsPlasmoid(event.mimeData, "org.kde.latte.plasmoid"))) {
            LayoutManager.insertAtCoordinates2(dndSpacer, event.x, event.y)
            dndSpacer.opacity = 1;
        }
    }

    onDragMove: {
        if (event.mimeData.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") >= 0) {
            return;
        }

        if (latteApplet) {
            if (latteApplet.launchersDrop(event)) {
                root.addLaunchersMessage = true;
                if (root.addLaunchersInTaskManager) {
                    return;
                }
            }
        }

        if (!latteApplet || (latteApplet && !dock.mimeContainsPlasmoid(event.mimeData, "org.kde.latte.plasmoid"))) {
            LayoutManager.insertAtCoordinates2(dndSpacer, event.x, event.y)
            dndSpacer.opacity = 1;
        }
    }

    onDragLeave: {

        if (confirmedDragEntered) {
            slotAnimationsNeedLength(-1);
            confirmedDragEntered = false;
        }

        root.addLaunchersMessage = false;
        dndSpacer.opacity = 0;
        dndSpacer.parent = root;
    }

    onDrop: {
        //var relevantLayout = layoutsContainer.mainLayout.mapFromItem(root, event.x, event.y);
        //plasmoid.processMimeData(event.mimeData, relevantLayout.x, relevantLayout.y);
        //launchersDropped
        if (event.mimeData.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") < 0) {
            if (latteApplet && latteApplet.launchersDrop(event) && root.addLaunchersInTaskManager) {
                latteApplet.launchersDropped(event.mimeData.urls);
            } else if (!latteApplet || (latteApplet && !dock.mimeContainsPlasmoid(event.mimeData, "org.kde.latte.plasmoid"))) {
                plasmoid.processMimeData(event.mimeData, event.x, event.y);
                event.accept(event.proposedAction);
            }
        }

        if (confirmedDragEntered) {
            slotAnimationsNeedLength(-1);
            confirmedDragEntered = false;
        }

        root.addLaunchersMessage = false;
        dndSpacer.opacity = 0;
        //! this line is very important because it positions correctly the new applets
        //dndSpacer.parent = root;
    }

    onMaxLengthChanged: layoutsContainer.updateSizeForAppletsInFill();

    onToolBoxChanged: {
        if (toolBox) {
            toolBox.visible = false;
        }
    }

    property bool automaticSizeAnimation: false;
    onAutomaticIconSizeBasedSizeChanged: {
        if (!automaticSizeAnimation) {
            automaticSizeAnimation = true;
            slotAnimationsNeedBothAxis(1);
        }
    }

    onIconSizeChanged: {
        if (((iconSize === automaticIconSizeBasedSize) || (iconSize === root.maxIconSize)) && automaticSizeAnimation){
            slotAnimationsNeedBothAxis(-1);
            automaticSizeAnimation=false;
        }
    }

    onIsVerticalChanged: {
        if (isVertical) {
            if (plasmoid.configuration.panelPosition === Latte.Dock.Left)
                plasmoid.configuration.panelPosition = Latte.Dock.Top;
            else if (plasmoid.configuration.panelPosition === Latte.Dock.Right)
                plasmoid.configuration.panelPosition = Latte.Dock.Bottom;
        } else {
            if (plasmoid.configuration.panelPosition === Latte.Dock.Top)
                plasmoid.configuration.panelPosition = Latte.Dock.Left;
            else if (plasmoid.configuration.panelPosition === Latte.Dock.Bottom)
                plasmoid.configuration.panelPosition = Latte.Dock.Right;
        }
    }

    onProportionIconSizeChanged: {
        if (proportionIconSize!==-1)
            updateAutomaticIconSize();
    }

    //  onIconSizeChanged: console.log("Icon Size Changed:"+iconSize);

    Component.onCompleted: {
        //  currentLayout.isLayoutHorizontal = isHorizontal
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = layoutsContainer.mainLayout;
        LayoutManager.layoutS = layoutsContainer.startLayout;
        layoutManager.layoutE = layoutsContainer.endLayout;
        LayoutManager.lastSpacer = lastSpacer;
        LayoutManager.restore();
        plasmoid.action("configure").visible = !plasmoid.immutable;
        plasmoid.action("configure").enabled = !plasmoid.immutable;

        if (!plasmoid.immutable) {
            inStartup = false;
        }
    }

    Component.onDestruction: {
        console.debug("Destroying Latte Dock Containment ui...");

        if (dock) {
            dock.onAddInternalViewSplitter.disconnect(addInternalViewSplitters);
            dock.onRemoveInternalViewSplitter.disconnect(removeInternalViewSplitters);

            dock.onXChanged.disconnect(visibilityManager.updateMaskArea);
            dock.onYChanged.disconnect(visibilityManager.updateMaskArea);
            dock.onWidthChanged.disconnect(visibilityManager.updateMaskArea);
            dock.onHeightChanged.disconnect(visibilityManager.updateMaskArea);

            if (dock.visibility) {
                dock.visibility.onContainsMouseChanged.disconnect(visibilityManager.slotContainsMouseChanged);
                dock.visibility.onMustBeHide.disconnect(visibilityManager.slotMustBeHide);
                dock.visibility.onMustBeShown.disconnect(visibilityManager.slotMustBeShown);
            }
        }
    }

    Containment.onAppletAdded: {
        addApplet(applet, x, y);
        LayoutManager.save();
        updateIndexes();
    }

    Containment.onAppletRemoved: {
        LayoutManager.removeApplet(applet);
        var flexibleFound = false;
        for (var i = 0; i < layoutsContainer.mainLayout.children.length; ++i) {
            var applet = layoutsContainer.mainLayout.children[i].applet;
            if (applet && ((root.isHorizontal && applet.Layout.fillWidth) ||
                           (!root.isHorizontal && applet.Layout.fillHeight)) &&
                    applet.visible) {
                flexibleFound = true;
                break
            }
        }
        if (!flexibleFound) {
            lastSpacer.parent = layoutsContainer.mainLayout;
        }

        LayoutManager.save();

        updateIndexes();
        updateContainsOnlyPlasmaTasks();
    }

    Plasmoid.onUserConfiguringChanged: {
        if (plasmoid.immutable) {
            if (dragOverlay) {
                dragOverlay.destroy();
            }
            return;
        }

        // console.debug("user configuring", plasmoid.userConfiguring)

        if (plasmoid.userConfiguring) {
            dock.setBlockHiding(true);

            //  console.log("applets------");
            for (var i = 0; i < plasmoid.applets.length; ++i) {
                //    console.log("applet:"+i);
                plasmoid.applets[i].expanded = false;
            }
            if (!dragOverlay) {
                var component = Qt.createComponent("ConfigOverlay.qml");
                if (component.status == Component.Ready) {
                    dragOverlay = component.createObject(root);
                } else {
                    console.log("Could not create ConfigOverlay");
                    console.log(component.errorString());
                }
                component.destroy();
            } else {
                dragOverlay.visible = true;
            }
        } else {
            dock.setBlockHiding(false);

            if (dock.visibility.isHidden) {
                dock.visibility.mustBeShown();
            }

            if (dragOverlay) {
                dragOverlay.visible = false;
                dragOverlay.destroy();
            }
        }
    }

    Plasmoid.onImmutableChanged: {
        plasmoid.action("configure").visible = !plasmoid.immutable;
        plasmoid.action("configure").enabled = !plasmoid.immutable;

        ///Set Preferred Sizes///
        ///Notice: they are set here because if they are set with a binding
        ///they break the !immutable experience, the dock becomes too small
        ///to add applets
        if (plasmoid.immutable) {
            if(root.isHorizontal) {
                root.Layout.preferredWidth = (plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                                  layoutsContainer.width + 0.5*iconMargin : layoutsContainer.mainLayout.width + iconMargin);
            } else {
                root.Layout.preferredHeight = (plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                                   layoutsContainer.height + 0.5*iconMargin : layoutsContainer.mainLayout.height + iconMargin);
            }
        } else {
            if (root.isHorizontal) {
                root.Layout.preferredWidth = Screen.width;
            } else {
                root.Layout.preferredHeight = Screen.height;
            }
        }

        visibilityManager.updateMaskArea();
    }
    //////////////END OF CONNECTIONS

    //////////////START OF FUNCTIONS
    function addApplet(applet, x, y) {
        var container = appletContainerComponent.createObject(root)

        container.applet = applet;
        applet.parent = container.appletWrapper;

        applet.anchors.fill = container.appletWrapper;

        applet.visible = true;


        // don't show applet if it choses to be hidden but still make it
        // accessible in the panelcontroller
        container.visible = Qt.binding(function() {
            return applet.status !== PlasmaCore.Types.HiddenStatus || (!plasmoid.immutable && plasmoid.userConfiguring)
        })

        addContainerInLayout(container, applet, x, y);

        updateContainsOnlyPlasmaTasks();
    }

    function addContainerInLayout(container, applet, x, y){
        // Is there a DND placeholder? Replace it!
        if ( (dndSpacer.parent === layoutsContainer.mainLayout)
                || (dndSpacer.parent === layoutsContainer.startLayout)
                || (dndSpacer.parent===layoutsContainer.endLayout)) {
            LayoutManager.insertBeforeForLayout(dndSpacer.parent, dndSpacer, container);
            dndSpacer.parent = root;
            return;
            // If the provided position is valid, use it.
        } else if (x >= 0 && y >= 0) {
            var index = LayoutManager.insertAtCoordinates2(container, x , y);

            // Fall through to determining an appropriate insert position.
        } else {
            var before = null;
            container.animationsEnabled = false;

            if (lastSpacer.parent === layoutsContainer.mainLayout) {
                before = lastSpacer;
            }

            // Insert icons to the left of whatever is at the center (usually a Task Manager),
            // if it exists.
            // FIXME TODO: This is a real-world fix to produce a sensible initial position for
            // launcher icons added by launcher menu applets. The basic approach has been used
            // since Plasma 1. However, "add launcher to X" is a generic-enough concept and
            // frequent-enough occurence that we'd like to abstract it further in the future
            // and get rid of the uglyness of parties external to the containment adding applets
            // of a specific type, and the containment caring about the applet type. In a better
            // system the containment would be informed of requested launchers, and determine by
            // itself what it wants to do with that information.
            if (applet.pluginName == "org.kde.plasma.icon") {
                var middle = layoutsContainer.mainLayout.childAt(root.width / 2, root.height / 2);

                if (middle) {
                    before = middle;
                }

                // Otherwise if lastSpacer is here, enqueue before it.
            }

            if (before) {
                LayoutManager.insertBefore(before, container);

                // Fall through to adding at the end.
            } else {
                container.parent = layoutsContainer.mainLayout;
            }
        }

        //Important, removes the first children of the layoutsContainer.mainLayout after the first
        //applet has been added
        lastSpacer.parent = root;

        updateIndexes();
    }

    function addInternalViewSplitters(){
        addInternalViewSplitter(-1);
        addInternalViewSplitter(-1);
    }

    function addInternalViewSplitter(pos){
        var splittersCount = internalViewSplittersCount();
        if(splittersCount<2){
            var container = appletContainerComponent.createObject(root);

            container.internalSplitterId = splittersCount+1;
            container.visible = true;

            if(pos>=0 ){
                layoutManager.insertAtIndex(container, pos);
            } else {
                layoutManager.insertAtIndex(container, Math.floor(layoutsContainer.mainLayout.count / 2));
            }

            layoutManager.save();
        }
    }

    //! it is used in order to check the right click position
    //! the only way to take into account the visual appearance
    //! of the applet (including its spacers)
    function appletContainsPos(appletId, pos){
        for (var i = 0; i < layoutsContainer.startLayout.children.length; ++i) {
            var child = layoutsContainer.startLayout.children[i];

            if (child && child.applet && child.applet.id === appletId && child.containsPos(pos))
                return true;
        }

        for (var i = 0; i < layoutsContainer.mainLayout.children.length; ++i) {
            var child = layoutsContainer.mainLayout.children[i];

            if (child && child.applet && child.applet.id === appletId && child.containsPos(pos))
                return true;
        }

        for (var i = 0; i < layoutsContainer.endLayout.children.length; ++i) {
            var child = layoutsContainer.endLayout.children[i];

            if (child && child.applet && child.applet.id === appletId && child.containsPos(pos))
                return true;
        }

        return false;
    }

    function checkLastSpacer() {
        lastSpacer.parent = root

        var expands = false;

        if (isHorizontal) {
            for (var container in layoutsContainer.mainLayout.children) {
                var item = layoutsContainer.mainLayout.children[container];
                if (item.Layout && item.Layout.fillWidth) {
                    expands = true;
                }
            }
        } else {
            for (var container in layoutsContainer.mainLayout.children) {
                var item = layoutsContainer.mainLayout.children[container];
                if (item.Layout && item.Layout.fillHeight) {
                    expands = true;
                }
            }
        }
        if (!expands) {
            lastSpacer.parent = layoutsContainer.mainLayout
        }
    }

    function clearZoom(){
        layoutsContainer.currentSpot = -1000;
        layoutsContainer.hoveredIndex = -1;

        if (latteApplet){
            latteApplet.clearZoom();
        }

        root.clearZoomSignal();
    }

    function containmentActions(){
        return dock.containmentActions();
    }

    function decimalToHex(d, padding) {
        var hex = Number(d).toString(16);
        padding = typeof (padding) === "undefined" || padding === null ? padding = 2 : padding;

        while (hex.length < padding) {
            hex = "0" + hex;
        }

        return hex;
    }


    function disableDirectRender(){
        root.globalDirectRender = false;
    }

    function internalViewSplittersCount(){
        var splitters = 0;
        for (var container in layoutsContainer.startLayout.children) {
            var item = layoutsContainer.startLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters++;
            }
        }

        for (var container in layoutsContainer.mainLayout.children) {
            var item = layoutsContainer.mainLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters++;
            }
        }

        for (var container in layoutsContainer.endLayout.children) {
            var item = layoutsContainer.endLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters++;
            }
        }

        return splitters;
    }


    function mouseInCanBeHoveredApplet(){
        if (latteApplet && latteApplet.containsMouse())
            return true;

        var applets = layoutsContainer.startLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.lockZoom && applet.canBeHovered){
                return true;
            }
        }

        applets = layoutsContainer.mainLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.lockZoom && applet.canBeHovered){
                return true;
            }
        }

        ///check second layout also
        applets = layoutsContainer.endLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.lockZoom && applet.canBeHovered){
                return true;
            }
        }

        return false;
    }

    function removeInternalViewSplitters(){
        for (var container in layoutsContainer.mainLayout.children) {
            var item = layoutsContainer.mainLayout.children[container];
            if(item && item.isInternalViewSplitter)
                item.destroy();
        }

        layoutManager.save();
    }

    function hideTooltipLabel(debug){
        titleTooltipDialog.hide(debug);
    }

    function showTooltipLabel(taskItem, text){
        titleTooltipDialog.show(taskItem, text);
    }

    function sizeIsFromAutomaticMode(size){

        for(var i=iconsArray.length-1; i>=0; --i){
            if(iconsArray[i] === size){
                return true;
            }
        }

        return false;
    }

    function slotAnimationsNeedBothAxis(step) {
        if (step === 0) {
            return;
        }

        animationsNeedBothAxis = Math.max(animationsNeedBothAxis + step, 0);

        visibilityManager.updateMaskArea();
    }

    function slotAnimationsNeedLength(step) {
        if (step === 0) {
            return;
        }

        animationsNeedLength = Math.max(animationsNeedLength + step, 0);

        //when need length animations are ended it would be a good idea
        //to update the tasks geometries in the plasmoid
        if(animationsNeedLength === 0 && latteApplet) {
            latteApplet.publishTasksGeometries();
        }

        visibilityManager.updateMaskArea();
    }

    function slotAnimationsNeedThickness(step) {
        if (step === 0) {
            return;
        }

        animationsNeedThickness = Math.max(animationsNeedThickness + step, 0);

        visibilityManager.updateMaskArea();
    }

    //this is used when dragging a task in order to not hide the dock
    //and also by the menu appearing from tasks for the same reason
    function slotActionsBlockHiding(step) {
        //if (root.editMode) {
        //    return;
        // }

        if ((step === 0) || (!dock)) {
            return;
        }

        actionsBlockHiding = Math.max(actionsBlockHiding + step, 0);

        if (actionsBlockHiding > 0){
            dock.setBlockHiding(true);
        } else {
            if (!root.editMode)
                dock.setBlockHiding(false);
        }
    }

    function slotPreviewsShown(){
        if (dock) {
            dock.deactivateApplets();
        }
    }

    function startEnableDirectRenderTimer(){
        if (!enableDirectRenderTimer.running)
            enableDirectRenderTimer.start();
    }

    function startCheckRestoreZoomTimer(){
        checkRestoreZoom.start();
    }

    function setGlobalDirectRender(value) {
        if (latteApplet && latteApplet.waitingLaunchers.length > 0)
            return;

        root.globalDirectRender = value;
    }

    function updateAutomaticIconSize() {
        if ((visibilityManager.normalState && !root.editMode
             && (root.autoDecreaseIconSize || (!root.autoDecreaseIconSize && root.iconSize!=root.maxIconSize)))
                && (iconSize===root.maxIconSize || iconSize === automaticIconSizeBasedSize) ) {
            var layoutLength;
            var maxLength = root.maxLength;

            //console.log("------Entered check-----");
            //console.log("max length: "+ maxLength);

            if (root.isVertical) {
                layoutLength = (plasmoid.configuration.panelPosition === Latte.Dock.Justify) ?
                            layoutsContainer.startLayout.height+layoutsContainer.mainLayout.height+layoutsContainer.endLayout.height : layoutsContainer.mainLayout.height
            } else {
                layoutLength = (plasmoid.configuration.panelPosition === Latte.Dock.Justify) ?
                            layoutsContainer.startLayout.width+layoutsContainer.mainLayout.width+layoutsContainer.endLayout.width : layoutsContainer.mainLayout.width
            }

            var toShrinkLimit = maxLength-((root.zoomFactor-1)*(iconSize+2*iconMargin));
            var toGrowLimit = maxLength-1.5*((root.zoomFactor-1)*(iconSize+2*iconMargin));

            if (layoutLength > toShrinkLimit) { //must shrink
                //  console.log("step3");
                var nextIconSize = root.maxIconSize;

                do {
                    nextIconSize = nextIconSize - iconStep;
                    var factor = nextIconSize / iconSize;
                    var nextLength = factor * layoutLength;

                } while ( (nextLength>toShrinkLimit) && (nextIconSize !== 16));

                automaticIconSizeBasedSize = nextIconSize;
                console.log("Step 3 - found:"+automaticIconSizeBasedSize);
            } else if ((layoutLength<toGrowLimit
                        && (iconSize === automaticIconSizeBasedSize)) ) { //must grow probably
                //  console.log("step4");
                var nextIconSize2 = automaticIconSizeBasedSize;
                var foundGoodSize = -1;

                do {
                    nextIconSize2 = nextIconSize2 + iconStep;
                    var factor2 = nextIconSize2 / automaticIconSizeBasedSize;
                    var nextLength2 = factor2 * layoutLength;

                    if (nextLength2 < toGrowLimit) {
                        foundGoodSize = nextIconSize2;
                    }
                } while ( (nextLength2<toGrowLimit) && (nextIconSize2 !== root.maxIconSize ));

                if (foundGoodSize > 0) {
                    if (foundGoodSize === root.maxIconSize) {
                        automaticIconSizeBasedSize = -1;
                    } else {
                        automaticIconSizeBasedSize = foundGoodSize;
                    }
                    //        console.log("Step 4 - found:"+automaticIconSizeBasedSize);
                } else {
                    //       console.log("Step 4 - did not found...");
                }
            }
        }
    }

    function updateContainsOnlyPlasmaTasks() {
        if (dock) {
            root.containsOnlyPlasmaTasks = (dock.tasksPresent() && !dock.latteTasksPresent());
        } else {
            root.containsOnlyPlasmaTasks = false;
        }
    }

    function updateSizeForAppletsInFill() {
        layoutsContainer.updateSizeForAppletsInFill();
    }

    function updateLayouts(){
        if(!root.editMode){
            //    console.log("update layout - internal view splitters count:"+internalViewSplittersCount());
            if (internalViewSplittersCount() === 2) {
                var splitter = -1;
                var splitter2 = -1;

                var totalChildren = layoutsContainer.mainLayout.children.length;
                for (var i=0; i<totalChildren; ++i) {
                    var item = layoutsContainer.mainLayout.children[i];

                    if(item.isInternalViewSplitter && splitter === -1) {
                        splitter = i;
                    } else if (item.isInternalViewSplitter && splitter>=0 && splitter2 === -1) {
                        splitter2 = i;
                    }
                }

                // console.log("update layouts 1:"+splitter + " - "+splitter2);
                for (var i=0; i<=splitter; ++i){
                    var item = layoutsContainer.mainLayout.children[0];
                    item.parent = layoutsContainer.startLayout;
                }

                splitter2 = splitter2 - splitter - 1;
                // console.log("update layouts 2:"+splitter + " - "+splitter2);

                totalChildren = layoutsContainer.mainLayout.children.length;
                for (var i=splitter2+1; i<totalChildren; ++i){
                    var item = layoutsContainer.mainLayout.children[splitter2+1];
                    item.parent = layoutsContainer.endLayout;
                }
            }
        }
        else{
            if (internalViewSplittersCount() === 2) {
                var totalChildren1 = layoutsContainer.mainLayout.children.length;
                for (var i=totalChildren1-1; i>=0; --i) {
                    var item1 = layoutsContainer.mainLayout.children[0];
                    item1.parent = layoutsContainer.startLayout;
                }

                var totalChildren2 = layoutsContainer.endLayout.children.length;

                for (var i=totalChildren2-1; i>=0; --i) {
                    var item2 = layoutsContainer.endLayout.children[0];
                    item2.parent = layoutsContainer.startLayout;
                }

                var totalChildrenL = layoutsContainer.startLayout.children.length;
                for (var i=totalChildrenL-1; i>=0; --i) {
                    var itemL = layoutsContainer.startLayout.children[0];
                    itemL.parent = layoutsContainer.mainLayout;
                }
            }
        }

        updateIndexes();
    }
    //END functions


    ////BEGIN interfaces

    Connections {
        target: Latte.WindowSystem

        onCompositingActiveChanged: {
            visibilityManager.updateMaskArea();
        }
    }

    Connections {
        target: dock
        onWidthChanged:{
            if (root.isHorizontal && proportionIconSize!==-1)
                updateAutomaticIconSize();
        }

        onHeightChanged:{
            if (root.isVertical && proportionIconSize!==-1)
                updateAutomaticIconSize();
        }

        onContextMenuIsShownChanged: {
            if (!dock.contextMenuIsShown) {
                checkRestoreZoom.start();
            } else {
                root.globalDirectRender = false;
            }
        }
    }

    Connections{
        target: dock && dock.visibility ? dock.visibility : root

        ignoreUnknownSignals : true

        onContainsMouseChanged: {
            if (dock.visibility.containsMouse) {
                if (checkRestoreZoom.running)
                    checkRestoreZoom.stop();
            } else {
                checkRestoreZoom.start();
            }
        }
    }

    ////END interfaces

    /////BEGIN: Title Tooltip///////////
    PlasmaCore.Dialog{
        id: titleTooltipDialog

        type: PlasmaCore.Dialog.Tooltip
        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.ToolTip

        location: plasmoid.location
        mainItem: RowLayout{
            Layout.fillWidth: true
            Layout.fillHeight: true
            PlasmaComponents.Label{
                id:titleLbl
                text: titleTooltipDialog.title
            }
        }

        visible: false

        property string title: ""

        property bool activeItemHovered: false
        property Item activeItem: null
        property Item activeItemTooltipParent: null
        property string activeItemText: ""


        Component.onCompleted: {
            root.clearZoomSignal.connect(titleTooltipDialog.hide);
        }

        Component.onDestruction: {
            root.clearZoomSignal.disconnect(titleTooltipDialog.hide);
        }

        function hide(debug){
            if (!root.titleTooltips)
                return;

            activeItemHovered = false;
            hideTitleTooltipTimer.start();
        }

        function show(taskItem, text){
            if (!root.titleTooltips || (latteApplet && latteApplet.contextMenu)){
                return;
            }

            activeItemHovered = true;

            if (activeItem !== taskItem) {
                activeItem = taskItem;
                activeItemTooltipParent = taskItem.tooltipVisualParent;
                activeItemText = text;
            }

            showTitleTooltipTimer.start();
        }

        function update() {
            activeItemHovered = true
            title = activeItemText;
            visualParent = activeItemTooltipParent;
            visible = true;
        }
    }

    Timer {
        id: showTitleTooltipTimer
        interval: 100
        onTriggered: {
            if (dock && dock.visibility && dock.visibility.containsMouse) {
                titleTooltipDialog.update();
            }

            if (root.debugModeTimers) {
                console.log("containment timer: showTitleTooltipTimer called...");
            }
        }
    }

    Timer {
        id: hideTitleTooltipTimer
        interval: 200
        onTriggered: {
            if (!titleTooltipDialog.activeItemHovered) {
                titleTooltipDialog.visible = false;
                if (dock && dock.visibility && !dock.visibility.containsMouse) {
                    enableDirectRenderTimer.stop();
                    setGlobalDirectRender(false);
                }
            }

            if (root.debugModeTimers) {
                console.log("containment timer: hideTitleTooltipTimer called...");
            }

        }
    }
    /////END: Title Tooltip///////////

    ///////////////BEGIN components
    Component {
        id: appletContainerComponent
        Applet.AppletItem{}
    }

    ParabolicManager{
        id: _parabolicManager
    }

    ///////////////END components

    PlasmaCore.ColorScope{
        id: colorScopePalette
    }


    ///////////////BEGIN UI elements

    //it is used to check if the mouse is outside the layoutsContainer borders,
    //so in that case the onLeave event behavior should be trigerred
    MouseArea{
        id: rootMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            if(!root.editMode)
                checkRestoreZoom.start();
        }
    }

    Loader{
        active: root.debugModeWindow
        sourceComponent: DebugWindow{}
    }

    //! Load a sepia background in order to avoid black background
    Loader{
        anchors.fill: parent
        active: !Latte.WindowSystem.compositingActive
        sourceComponent: Image{
            anchors.fill: parent
            fillMode: Image.Tile
            source: "../icons/wheatprint.jpg"
        }
    }

    Loader{
        id: windowsModel

        active: (plasmoid.configuration.backgroundOnlyOnMaximized || root.disablePanelShadowMaximized || plasmoid.configuration.solidBackgroundForMaximized) && dock

        property bool hasMaximizedWindow: active && item ? item.maximizedWindowOnScreen : false
        sourceComponent: WindowsModel{}
    }


    EditModeVisual{
        id:editModeVisual
        z: root.behaveAsPlasmaPanel ? 1 : 0
    }

    Item{
        anchors.fill:layoutsContainer
        z: root.behaveAsPlasmaPanel ? 0 : 1

        PanelBox{}
    }

    Item {
        id: lastSpacer
        parent: layoutsContainer.mainLayout

        Layout.fillWidth: true
        Layout.fillHeight: true
        z:10

        Rectangle{
            anchors.fill: parent
            color: "transparent"
            border.color: "yellow"
            border.width: 1
        }
    }

    Item {
        id: dndSpacer

        property int normalSize: root.statesLineSize + root.iconSize + root.thickMargin - 1
        //visibilityManager.statesLineSizeOriginal + root.maxIconSize + visibilityManager.iconMarginOriginal - 1

        width: normalSize
        height: normalSize

        Layout.preferredWidth: width
        Layout.preferredHeight: height
        opacity: 0
        z:10

        AddWidgetVisual{}
    }

    Loader{
        anchors.fill: parent
        active: root.debugMode
        z:10

        sourceComponent: Item{
            Rectangle{
                anchors.fill: parent
                color: "yellow"
                opacity: 0.30
            }
        }
    }

    VisibilityManager{ id: visibilityManager }

    LayoutsContainer { id: layoutsContainer }


    ///////////////END UI elements

    ///////////////BEGIN TIMER elements


    //Timer to check if the mouse is still outside the dock in order to restore zooms to 1.0
    Timer{
        id:checkRestoreZoom
        repeat:false;
        interval: 150;

        onTriggered: {
            if (latteApplet && (latteApplet.previewContainsMouse() || latteApplet.contextMenu))
                return;

            if (dock.contextMenuIsShown)
                return;

            //! dropped because was creating a glitch when the user was clicking the area
            //! underneath the applet until the screen edge and the applet was zoomed
            // || (rootMouseArea.containsMouse && !root.editMode)
            if (!dock.visibility.containsMouse || !mouseInCanBeHoveredApplet()) {
                if (enableDirectRenderTimer.running)
                    enableDirectRenderTimer.stop();

                root.globalDirectRender = false;
                root.clearZoom();
            }

            if (root.debugModeTimers) {
                console.log("containment timer: checkRestoreZoom called...");
            }
        }
    }

    //this timer adds a delay into enabling direct rendering...
    //it gives the time to neighbour tasks to complete their animation
    //during first hovering phase
    Timer {
        id: enableDirectRenderTimer
        interval: 4 * root.durationTime * units.shortDuration
        onTriggered: {
            if (latteApplet && latteApplet.waitingLaunchers.length > 0)
                return;

            if (dock.visibility.containsMouse)
                root.globalDirectRender = true;

            if (root.debugModeTimers) {
                console.log("containment timer: enableDirectRenderTimer called...");
            }
        }
    }


    //this is a delayer to update mask area, it is used in cases
    //that animations can not catch up with animations signals
    //e.g. the automaicIconSize case
    Timer{
        id:delayUpdateMaskArea
        repeat:false;
        interval:300;

        onTriggered: {
            if (layoutsContainer.animationSent) {
                root.slotAnimationsNeedLength(-1);
                layoutsContainer.animationSent = false;
            }

            visibilityManager.updateMaskArea();

            if (root.debugModeTimers) {
                console.log("containment timer: delayUpdateMaskArea called...");
            }
        }
    }

    ///////////////END TIMER elements
}
