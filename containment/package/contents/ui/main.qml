/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.8
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.abilities.host 0.1 as AbilityHost

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents
import org.kde.latte.private.app 0.1 as LatteApp
import org.kde.latte.private.containment 0.1 as LatteContainment

import "abilities" as Ability
import "applet" as Applet
import "colorizer" as Colorizer
import "editmode" as EditMode
import "layouts" as Layouts
import "./background" as Background
import "./debugger" as Debugger

Item {
    id: root
    objectName: "containmentViewLayout"

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft && !root.isVertical
    LayoutMirroring.childrenInherit: true

    //// BEGIN SIGNALS
    signal destroyInternalViewSplitters();
    signal emptyAreasWheel(QtObject wheel);
    signal separatorsUpdated();
    signal updateEffectsArea();
    signal updateIndexes();

    signal broadcastedToApplet(string pluginName, string action, variant value);
    //// END SIGNALS

    ////BEGIN properties
    readonly property int version: LatteCore.Environment.makeVersion(0,9,75)
    readonly property bool kirigamiLibraryIsFound: LatteCore.Environment.frameworksVersion >= LatteCore.Environment.makeVersion(5,69,0)

    property bool backgroundOnlyOnMaximized: plasmoid.configuration.backgroundOnlyOnMaximized
    readonly property bool behaveAsPlasmaPanel: viewType === LatteCore.Types.PanelView
    readonly property bool behaveAsDockWithMask: !behaveAsPlasmaPanel

    readonly property bool viewIsAvailable: latteView && latteView.visibility && latteView.effects

    property int viewType: {
        if (!latteView || !latteView.visibility) {
            return LatteCore.Types.DockView;
        }

        if (screenEdgeMarginEnabled && root.floatingInternalGapIsForced) {
            //! dont use when floating views are requesting internal floating gap which is in client side
            return LatteCore.Types.DockView;
        }

        return viewTypeInQuestion;
    }

    property int viewTypeInQuestion: {
        //! viewType as chosen before considering other optios such as floating internal gap enforcement.
        //! It helps with binding loops
        if (!latteView || !latteView.visibility) {
            return LatteCore.Types.DockView;
        }

        if (background.customShadowedRectangleIsEnabled) {
            return LatteCore.Types.DockView;
        }

        var staticLayout = (plasmoid.configuration.minLength === plasmoid.configuration.maxLength);

        if ((plasmoid.configuration.alignment === LatteCore.Types.Justify || staticLayout)
                && background.isGreaterThanItemThickness
                && (parabolic.factor.maxZoom === 1.0)) {
            return LatteCore.Types.PanelView;
        }

        return LatteCore.Types.DockView;
    }

    property bool blurEnabled: plasmoid.configuration.blurEnabled && (!forceTransparentPanel || forcePanelForBusyBackground)

    readonly property bool inDraggingOverAppletOrOutOfContainment: latteView && latteView.containsDrag && !backDropArea.containsDrag

    readonly property Item dragInfo: Item {
        property bool entered: backDropArea.dragInfo.entered
        property bool isTask: backDropArea.dragInfo.isTask
        property bool isPlasmoid: backDropArea.dragInfo.isPlasmoid
        property bool isSeparator: backDropArea.dragInfo.isSeparator
        property bool isLatteTasks: backDropArea.dragInfo.isLatteTasks
        property bool onlyLaunchers: backDropArea.dragInfo.onlyLaunchers
    }

    property bool containsOnlyPlasmaTasks: latteView ? latteView.extendedInterface.hasPlasmaTasks && !latteView.extendedInterface.hasLatteTasks : false
    property bool dockContainsMouse: latteView && latteView.visibility ? latteView.visibility.containsMouse : false

    property bool disablePanelShadowMaximized: plasmoid.configuration.disablePanelShadowForMaximized && LatteCore.WindowSystem.compositingActive
    property bool drawShadowsExternal: panelShadowsActive && behaveAsPlasmaPanel

    property bool editMode: plasmoid.userConfiguring
    property bool windowIsTouching: latteView && latteView.windowsTracker
                                    && (latteView.windowsTracker.currentScreen.activeWindowTouching
                                        || latteView.windowsTracker.currentScreen.activeWindowTouchingEdge
                                        || hasExpandedApplet)

    property bool floatingInternalGapIsForced: plasmoid.configuration.floatingInternalGapIsForced

    property bool hasFloatingGapInputEventsDisabled: root.screenEdgeMarginEnabled
                                                     && !latteView.byPassWM
                                                     && !root.inConfigureAppletsMode
                                                     && !parabolic.isEnabled
                                                     && (root.behaveAsPlasmaPanel || (root.behaveAsDockWithMask && !root.floatingInternalGapIsForced))

    property bool forceSolidPanel: (latteView && latteView.visibility
                                    && LatteCore.WindowSystem.compositingActive
                                    && !inConfigureAppletsMode
                                    && userShowPanelBackground
                                    && ( (plasmoid.configuration.solidBackgroundForMaximized
                                          && !(hasExpandedApplet && !plasmaBackgroundForPopups)
                                          && (latteView.windowsTracker.currentScreen.existsWindowTouching
                                              || latteView.windowsTracker.currentScreen.existsWindowTouchingEdge))
                                        || (hasExpandedApplet && plasmaBackgroundForPopups) ))
                                   || !LatteCore.WindowSystem.compositingActive

    property bool forceTransparentPanel: root.backgroundOnlyOnMaximized
                                         && latteView && latteView.visibility
                                         && LatteCore.WindowSystem.compositingActive
                                         && !inConfigureAppletsMode
                                         && !forceSolidPanel
                                         && !(latteView.windowsTracker.currentScreen.existsWindowTouching
                                              || latteView.windowsTracker.currentScreen.existsWindowTouchingEdge)
                                         && !(windowColors === LatteContainment.Types.ActiveWindowColors && selectedWindowsTracker.existsWindowActive)

    property bool forcePanelForBusyBackground: userShowPanelBackground && (normalBusyForTouchingBusyVerticalView
                                                                           || ( root.forceTransparentPanel
                                                                               && colorizerManager.backgroundIsBusy
                                                                               && root.themeColors === LatteContainment.Types.SmartThemeColors))

    property bool normalBusyForTouchingBusyVerticalView: (latteView && latteView.windowsTracker /*is touching a vertical view that is in busy state and the user prefers isBusy transparency*/
                                                          && latteView.windowsTracker.currentScreen.isTouchingBusyVerticalView
                                                          && plasmoid.configuration.backgroundOnlyOnMaximized)

    property bool appletIsDragged: root.dragOverlay && root.dragOverlay.pressed
    property bool hideThickScreenGap: false /*set through binding*/
    property bool hideLengthScreenGaps: false /*set through binding*/

    property bool mirrorScreenGap: screenEdgeMarginEnabled
                                   && plasmoid.configuration.floatingGapIsMirrored
                                   && latteView.visibility.mode === LatteCore.Types.AlwaysVisible



    property int themeColors: plasmoid.configuration.themeColors
    property int windowColors: plasmoid.configuration.windowColors

    property bool colorizerEnabled: themeColors !== LatteContainment.Types.PlasmaThemeColors || windowColors !== LatteContainment.Types.NoneWindowColors

    property bool plasmaBackgroundForPopups: plasmoid.configuration.plasmaBackgroundForPopups

    readonly property bool hasExpandedApplet: latteView && latteView.extendedInterface.hasExpandedApplet;
    readonly property bool hasUserSpecifiedBackground: (latteView && latteView.layout && latteView.layout.background.startsWith("/")) ?
                                                           true : false

    readonly property bool inConfigureAppletsMode: root.editMode && universalSettings && universalSettings.inConfigureAppletsMode

    property bool closeActiveWindowEnabled: plasmoid.configuration.closeActiveWindowEnabled
    property bool dragActiveWindowEnabled: plasmoid.configuration.dragActiveWindowEnabled
    property bool immutable: plasmoid.immutable
    property bool inFullJustify: (plasmoid.configuration.alignment === LatteCore.Types.Justify) && (maxLengthPerCentage===100)
    property bool inStartup: true

    property bool isHorizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    property bool isVertical: !isHorizontal

    property bool mouseWheelActions: plasmoid.configuration.mouseWheelActions
    property bool onlyAddingStarup: true //is used for the initialization phase in startup where there aren't removals, this variable provides a way to grow icon size

    //FIXME: possibly this is going to be the default behavior, this user choice
    //has been dropped from the Dock Configuration Window
    //property bool smallAutomaticIconJumps: plasmoid.configuration.smallAutomaticIconJumps
    property bool smallAutomaticIconJumps: true

    property bool userShowPanelBackground: LatteCore.WindowSystem.compositingActive ? plasmoid.configuration.useThemePanel : true
    property bool useThemePanel: noApplets === 0 || !LatteCore.WindowSystem.compositingActive ?
                                     true : (plasmoid.configuration.useThemePanel || plasmoid.configuration.solidBackgroundForMaximized)

    property bool plasma515: LatteCore.Environment.plasmaDesktopVersion >= LatteCore.Environment.makeVersion(5,15,0)
    property bool plasma518: LatteCore.Environment.plasmaDesktopVersion >= LatteCore.Environment.makeVersion(5,18,0)

    readonly property int minAppletLengthInConfigure: 16
    readonly property int maxJustifySplitterSize: 64

    property real minLengthPerCentage: plasmoid.configuration.minLength
    property real maxLengthPerCentage: hideLengthScreenGaps ? 100 : plasmoid.configuration.maxLength

    property int minLength: {
        if (myView.alignment === LatteCore.Types.Justify) {
            return maxLength;
        }

        if (root.isHorizontal) {
            return behaveAsPlasmaPanel && LatteCore.WindowSystem.compositingActive ? width : width * (minLengthPerCentage/100)
        } else {
            return behaveAsPlasmaPanel && LatteCore.WindowSystem.compositingActive ? height : height * (minLengthPerCentage/100)
        }
    }

    property int maxLength: {
        if (root.isHorizontal) {
            return behaveAsPlasmaPanel ? width : width * (maxLengthPerCentage/100)
        } else {
            return behaveAsPlasmaPanel ? height : height * (maxLengthPerCentage/100)
        }
    }

    property int scrollAction: plasmoid.configuration.scrollAction

    property bool panelOutline: plasmoid.configuration.panelOutline
    property int panelEdgeSpacing: Math.max(background.lengthMargins, 1.5*myView.itemShadow.size)

    property bool backgroundShadowsInRegularStateEnabled: LatteCore.WindowSystem.compositingActive
                                                          && userShowPanelBackground
                                                          && plasmoid.configuration.panelShadows

    property bool panelShadowsActive: {
        if (!userShowPanelBackground) {
            return false;
        }

        if (inConfigureAppletsMode) {
            return plasmoid.configuration.panelShadows;
        }

        var forcedNoShadows = (plasmoid.configuration.panelShadows && disablePanelShadowMaximized
                               && latteView && latteView.windowsTracker && latteView.windowsTracker.currentScreen.activeWindowMaximized);

        if (forcedNoShadows) {
            return false;
        }

        var transparencyCheck = (blurEnabled || (!blurEnabled && background.currentOpacity>20));

        //! Draw shadows for isBusy state only when current background opacity is greater than 10%
        if (plasmoid.configuration.panelShadows && root.forcePanelForBusyBackground && transparencyCheck) {
            return true;
        }

        if (( (plasmoid.configuration.panelShadows && !root.backgroundOnlyOnMaximized)
             || (plasmoid.configuration.panelShadows && root.backgroundOnlyOnMaximized && !root.forceTransparentPanel))
                && !forcedNoShadows) {
            return true;
        }

        if (hasExpandedApplet && plasmaBackgroundForPopups) {
            return true;
        }

        return false;
    }

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

    property int editShadow: {
        if (!LatteCore.WindowSystem.compositingActive) {
            return 0;
        } else if (latteView && latteView.screenGeometry) {
            return latteView.screenGeometry.height/90;
        } else {
            return 7;
        }
    }

    property bool screenEdgeMarginEnabled: plasmoid.configuration.screenEdgeMargin >= 0

    property int widthMargins: root.isVertical ? metrics.totals.thicknessEdges : metrics.totals.lengthEdges
    property int heightMargins: root.isHorizontal ? metrics.totals.thicknessEdges : metrics.totals.lengthEdges

    property var iconsArray: [16, 22, 32, 48, 64, 96, 128, 256]

    property Item dragOverlay: _dragOverlay
    property Item toolBox

    readonly property alias animations: _animations
    readonly property alias autosize: _autosize
    readonly property alias background: _background
    readonly property alias debug: _debug
    readonly property alias environment: _environment
    readonly property alias indexer: _indexer
    readonly property alias indicators: _indicators
    readonly property alias layouter: _layouter
    readonly property alias launchers: _launchers
    readonly property alias metrics: _metrics
    readonly property alias myView: _myView
    readonly property alias parabolic: _parabolic
    readonly property alias thinTooltip: _thinTooltip
    readonly property alias userRequests: _userRequests

    readonly property alias maskManager: visibilityManager
    readonly property alias layoutsContainerItem: layoutsContainer

    readonly property alias latteView: _interfaces.view
    readonly property alias layoutsManager: _interfaces.layoutsManager
    readonly property alias shortcutsEngine: _interfaces.globalShortcuts
    readonly property alias themeExtended: _interfaces.themeExtended
    readonly property alias universalSettings: _interfaces.universalSettings

    readonly property QtObject selectedWindowsTracker: {
        if (latteView && latteView.windowsTracker) {
            switch(plasmoid.configuration.activeWindowFilter) {
            case LatteContainment.Types.ActiveInCurrentScreen:
                return latteView.windowsTracker.currentScreen;
            case LatteContainment.Types.ActiveFromAllScreens:
                return latteView.windowsTracker.allScreens;
            }
        }

        return null;
    }

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    //// BEGIN properties in functions
    property int noApplets: {
        var count1 = 0;
        var count2 = 0;

        count1 = layoutsContainer.mainLayout.children.length;
        var tempLength = layoutsContainer.mainLayout.children.length;

        for (var i=tempLength-1; i>=0; --i) {
            var applet = layoutsContainer.mainLayout.children[i];
            if (applet && (applet === dndSpacer ||  applet.isInternalViewSplitter))
                count1--;
        }

        count2 = layoutsContainer.endLayout.children.length;
        tempLength = layoutsContainer.endLayout.children.length;

        for (var i=tempLength-1; i>=0; --i) {
            var applet = layoutsContainer.endLayout.children[i];
            if (applet && (applet === dndSpacer || applet.isInternalViewSplitter))
                count2--;
        }

        return (count1 + count2);
    }

    ///The index of user's current icon size
    property int currentIconIndex:{
        for(var i=iconsArray.length-1; i>=0; --i){
            if(iconsArray[i] === metrics.iconSize){
                return i;
            }
        }
        return 3;
    }

    //// END properties in functions

    ////////////////END properties

    //////////////START OF BINDINGS

    //! Wait until the mouse leaves the view
    Binding {
        target: root
        property: "hideThickScreenGap"
        when: !(plasmoid.configuration.floatingGapHidingWaitsMouse && dockContainsMouse)
        value: screenEdgeMarginEnabled
               && plasmoid.configuration.hideFloatingGapForMaximized
               && latteView && latteView.windowsTracker
               && latteView.windowsTracker.currentScreen.existsWindowMaximized
    }

    //! Binding is needed in order for hideLengthScreenGaps to be activated or not only after
    //! View sliding in/out has finished. This way the animation is smoother for behaveAsPlasmaPanels
    Binding{
        target: root
        property: "hideLengthScreenGaps"
        when: latteView && latteView.positioner && latteView.visibility
              && ((root.behaveAsPlasmaPanel && latteView.positioner.slideOffset === 0)
                  || root.behaveAsDockWithMask)
              && !(plasmoid.configuration.floatingGapHidingWaitsMouse && dockContainsMouse)
        value: (hideThickScreenGap
                && (latteView.visibility.mode === LatteCore.Types.AlwaysVisible
                    || latteView.visibility.mode === LatteCore.Types.WindowsGoBelow)
                && (plasmoid.configuration.alignment === LatteCore.Types.Justify)
                && plasmoid.configuration.maxLength>85)
    }

    //////////////END OF BINDINGS


    //////////////START OF CONNECTIONS
    onEditModeChanged: {
        if (!editMode) {
            layouter.updateSizeForAppletsInFill();
        }

        //! This is used in case the dndspacer has been left behind
        //! e.g. the user drops a folder and a context menu is appearing
        //! but the user decides to not make a choice for the applet type
        if (dndSpacer.parent !== root) {
            dndSpacer.parent = root;
        }

        //Block Hiding events
        if (editMode) {
            latteView.visibility.addBlockHidingEvent("main[qml]::inEditMode()");
        } else {
            latteView.visibility.removeBlockHidingEvent("main[qml]::inEditMode()");
        }
    }

    onInConfigureAppletsModeChanged: {
        updateIndexes();
    }

    //! It is used only when the user chooses different alignment types and not during startup
    Connections {
        target: latteView ? latteView : null
        onAlignmentChanged: {
            if (latteView.alignment === LatteCore.Types.NoneAlignment) {
                return;
            }

            var previousalignment = plasmoid.configuration.alignment;

            if (latteView.alignment===LatteCore.Types.Justify && previousalignment!==LatteCore.Types.Justify) { // main -> justify
                layouter.appletsInParentChange = true;
                fastLayoutManager.addJustifySplittersInMainLayout();
                console.log("LAYOUTS: Moving applets from MAIN to THREE Layouts mode...");
                fastLayoutManager.moveAppletsBasedOnJustifyAlignment();
                layouter.appletsInParentChange = false;
            } else if (latteView.alignment!==LatteCore.Types.Justify && previousalignment===LatteCore.Types.Justify ) { // justify ->main
                layouter.appletsInParentChange = true;
                console.log("LAYOUTS: Moving applets from THREE to MAIN Layout mode...");
                fastLayoutManager.joinLayoutsToMainLayout();
                layouter.appletsInParentChange = false;
            }

            root.updateIndexes();
            plasmoid.configuration.alignment = latteView.alignment;
            fastLayoutManager.save();
        }
    }

    onLatteViewChanged: {
        if (latteView) {
            if (latteView.positioner) {
                latteView.positioner.hidingForRelocationStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showingAfterRelocationFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
            }

            if (latteView.visibility) {
                latteView.visibility.onContainsMouseChanged.connect(visibilityManager.slotContainsMouseChanged);
                latteView.visibility.onMustBeHide.connect(visibilityManager.slotMustBeHide);
                latteView.visibility.onMustBeShown.connect(visibilityManager.slotMustBeShown);
            }
        }
    }

    Connections {
        target: latteView
        onPositionerChanged: {
            if (latteView.positioner) {
                latteView.positioner.hidingForRelocationStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showingAfterRelocationFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
            }
        }

        onVisibilityChanged: {
            if (latteView.visibility) {
                latteView.visibility.onContainsMouseChanged.connect(visibilityManager.slotContainsMouseChanged);
                latteView.visibility.onMustBeHide.connect(visibilityManager.slotMustBeHide);
                latteView.visibility.onMustBeShown.connect(visibilityManager.slotMustBeShown);
            }
        }
    }


    onMaxLengthChanged: {
        layouter.updateSizeForAppletsInFill();
    }

    onToolBoxChanged: {
        if (toolBox) {
            toolBox.visible = false;
        }
    }

    onIsVerticalChanged: {
        if (isVertical) {
            if (plasmoid.configuration.alignment === LatteCore.Types.Left)
                plasmoid.configuration.alignment = LatteCore.Types.Top;
            else if (plasmoid.configuration.alignment === LatteCore.Types.Right)
                plasmoid.configuration.alignment = LatteCore.Types.Bottom;
        } else {
            if (plasmoid.configuration.alignment === LatteCore.Types.Top)
                plasmoid.configuration.alignment = LatteCore.Types.Left;
            else if (plasmoid.configuration.alignment === LatteCore.Types.Bottom)
                plasmoid.configuration.alignment = LatteCore.Types.Right;
        }
    }

    Component.onCompleted: {
        upgrader_v010_alignment();

        fastLayoutManager.restore();
        plasmoid.action("configure").visible = !plasmoid.immutable;
        plasmoid.action("configure").enabled = !plasmoid.immutable;
    }

    Component.onDestruction: {
        console.debug("Destroying Latte Dock Containment ui...");

        layouter.appletsInParentChange = true;
        fastLayoutManager.save();

        if (latteView) {
            if (latteView.positioner) {
                latteView.positioner.hidingForRelocationStarted.disconnect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showingAfterRelocationFinished.disconnect(visibilityManager.slotShowDockAfterLocationChange);
            }

            if (latteView.visibility) {
                latteView.visibility.onContainsMouseChanged.disconnect(visibilityManager.slotContainsMouseChanged);
                latteView.visibility.onMustBeHide.disconnect(visibilityManager.slotMustBeHide);
                latteView.visibility.onMustBeShown.disconnect(visibilityManager.slotMustBeShown);
            }
        }
    }

    Containment.onAppletAdded: {
        if (fastLayoutManager.isMasqueradedIndex(x, y)) {
            var index = fastLayoutManager.masquearadedIndex(x, y);
            fastLayoutManager.addAppletItem(applet, index);
        } else {
            fastLayoutManager.addAppletItem(applet, x, y);
        }
    }

    Containment.onAppletRemoved: fastLayoutManager.removeAppletItem(applet);

    Plasmoid.onUserConfiguringChanged: {
        if (plasmoid.userConfiguring) {
            for (var i = 0; i < plasmoid.applets.length; ++i) {
                plasmoid.applets[i].expanded = false;
            }
        }
    }

    Plasmoid.onImmutableChanged: {
        plasmoid.action("configure").visible = !plasmoid.immutable;
        plasmoid.action("configure").enabled = !plasmoid.immutable;
    }
    //////////////END OF CONNECTIONS

    //////////////START OF FUNCTIONS
    function createAppletItem(applet) {
        var appletContainer = appletItemComponent.createObject(dndSpacer.parent);
        initAppletContainer(appletContainer, applet);

        // don't show applet if it chooses to be hidden but still make it  accessible in the panelcontroller
        appletContainer.visible = Qt.binding(function() {
            return (appletContainer.applet && appletContainer.applet.status !== PlasmaCore.Types.HiddenStatus || (!plasmoid.immutable && root.inConfigureAppletsMode)) && !appletContainer.isHidden;
        });
        return appletContainer;
    }

    function initAppletContainer(appletContainer, applet) {
        appletContainer.applet = applet;
        applet.parent = appletContainer.appletWrapper;
        applet.anchors.fill = appletContainer.appletWrapper;
        applet.visible = true;
    }

    function createJustifySplitter() {
        var splitter = appletItemComponent.createObject(root);
        splitter.internalSplitterId = internalViewSplittersCount()+1;
        splitter.visible = true;
        return splitter;
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

    function internalViewSplittersCount(){
        var splitters = 0;
        for (var container in layoutsContainer.startLayout.children) {
            var item = layoutsContainer.startLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters = splitters + 1;
            }
        }

        for (var container in layoutsContainer.mainLayout.children) {
            var item = layoutsContainer.mainLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters = splitters + 1;
            }
        }

        for (var container in layoutsContainer.endLayout.children) {
            var item = layoutsContainer.endLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters = splitters + 1;
            }
        }

        return splitters;
    }

    function mouseInCanBeHoveredApplet(){
        var applets = layoutsContainer.startLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.originalAppletBehavior && applet.parabolicEffectIsSupported){
                return true;
            }
        }

        applets = layoutsContainer.mainLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.originalAppletBehavior && applet.parabolicEffectIsSupported){
                return true;
            }
        }

        ///check second layout also
        applets = layoutsContainer.endLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.originalAppletBehavior && applet.parabolicEffectIsSupported){
                return true;
            }
        }

        return false;
    }

    function sizeIsFromAutomaticMode(size){

        for(var i=iconsArray.length-1; i>=0; --i){
            if(iconsArray[i] === size){
                return true;
            }
        }

        return false;
    }

    function upgrader_v010_alignment() {
        //! IMPORTANT, special case because it needs to be loaded on Component constructor
        if (!plasmoid.configuration.alignmentUpgraded) {
            plasmoid.configuration.alignment = plasmoid.configuration.panelPosition;
            plasmoid.configuration.alignmentUpgraded = true;
        }
    }
    //END functions

    ///////////////BEGIN components
    Component {
        id: appletItemComponent
        Applet.AppletItem{
            animations: _animations
            debug: _debug
            environment: _environment
            indexer: _indexer
            indicators: _indicators
            launchers: _launchers
            layouter: _layouter
            layouts: layoutsContainer
            metrics: _metrics
            myView: _myView
            parabolic: _parabolic
            shortcuts: _shortcuts
            thinTooltip: _thinTooltip
            userRequests: _userRequests
        }
    }

    Upgrader {
        id: upgrader
    }

    ///////////////END components

    PlasmaCore.ColorScope{
        id: colorScopePalette
    }

    LatteContainment.LayoutManager{
        id:fastLayoutManager
        plasmoidObj: plasmoid
        rootItem: root
        dndSpacerItem: dndSpacer
        mainLayout: layoutsContainer.mainLayout
        startLayout: layoutsContainer.startLayout
        endLayout: layoutsContainer.endLayout
        metrics: _metrics

        onAppletOrderChanged: root.updateIndexes();
        onSplitterPositionChanged: root.updateIndexes();
        onSplitterPosition2Changed: root.updateIndexes();
    }

    ///////////////BEGIN UI elements

    Loader{
        active: debug.windowEnabled
        sourceComponent: Debugger.DebugWindow{}
    }

    Loader{
        anchors.fill: parent
        active: debug.graphicsEnabled
        z:10

        sourceComponent: Item{
            Rectangle{
                anchors.fill: parent
                color: "yellow"
                opacity: 0.06
            }
        }
    }

    BindingsExternal {
        id: bindingsExternal
    }

    VisibilityManager{
        id: visibilityManager
        layouts: layoutsContainer
    }

    DragDropArea {
        id: backDropArea
        anchors.fill: parent

        Item{
            anchors.fill: layoutsContainer

            Background.MultiLayered{
                id: _background
            }
        }

        Layouts.LayoutsContainer {
            id: layoutsContainer
        }
    }

    Colorizer.Manager {
        id: colorizerManager
    }

    EditMode.ConfigOverlay{
        id: _dragOverlay
    }

    Item {
        id: dndSpacer

        width: root.isHorizontal ? length : thickness
        height: root.isHorizontal ? thickness : length

        property int length: opacity > 0 ? (dndSpacerAddItem.length + metrics.totals.lengthEdges + metrics.totals.lengthPaddings) : 0

        readonly property bool isDndSpacer: true
        readonly property int thickness: metrics.totals.thickness + metrics.margin.screenEdge
        readonly property int maxLength: 96

        Layout.minimumWidth: width
        Layout.minimumHeight: height
        Layout.preferredWidth: Layout.minimumWidth
        Layout.preferredHeight: Layout.minimumHeight
        Layout.maximumWidth: Layout.minimumWidth
        Layout.maximumHeight: Layout.minimumHeight
        opacity: 0
        visible: parent === layoutsContainer.startLayout
                 || parent === layoutsContainer.mainLayout
                 || parent === layoutsContainer.endLayout

        z:1500

        Behavior on length {
            NumberAnimation {
                duration: animations.duration.large
                easing.type: Easing.InQuad
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: animations.duration.large
                easing.type: Easing.InQuad
            }
        }

        Item {
            id: dndSpacerAddItemContainer
            width: root.isHorizontal ? parent.length : parent.thickness - metrics.margin.screenEdge
            height: root.isHorizontal ? parent.thickness - metrics.margin.screenEdge : parent.length

            property int thickMargin: metrics.margin.screenEdge

            LatteComponents.AddItem{
                id: dndSpacerAddItem
                anchors.centerIn: parent
                width: length
                height: width

                readonly property int length: Math.min(metrics.iconSize, 96)
            }

            states:[
                State{
                    name: "bottom"
                    when: plasmoid.location === PlasmaCore.Types.BottomEdge

                    AnchorChanges{
                        target: dndSpacerAddItemContainer;
                        anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                    }
                    PropertyChanges{
                        target: dndSpacerAddItemContainer;
                        anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: dndSpacerAddItemContainer.thickMargin;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "top"
                    when: plasmoid.location === PlasmaCore.Types.TopEdge

                    AnchorChanges{
                        target: dndSpacerAddItemContainer;
                        anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: dndSpacerAddItemContainer;
                        anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin: dndSpacerAddItemContainer.thickMargin;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "left"
                    when: plasmoid.location === PlasmaCore.Types.LeftEdge

                    AnchorChanges{
                        target: dndSpacerAddItemContainer;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                        anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: dndSpacerAddItemContainer;
                        anchors.leftMargin: dndSpacerAddItemContainer.thickMargin;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "right"
                    when: plasmoid.location === PlasmaCore.Types.RightEdge

                    AnchorChanges{
                        target: dndSpacerAddItemContainer;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                        anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: dndSpacerAddItemContainer;
                        anchors.leftMargin: 0;    anchors.rightMargin: dndSpacerAddItemContainer.thickMargin;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                }
            ]
        }
    }

    Behavior on maxLengthPerCentage {
        enabled: root.behaveAsDockWithMask && plasmoid.configuration.floatingGapHidingWaitsMouse && dockContainsMouse
        NumberAnimation {
            duration: animations.duration.short
            easing.type: Easing.InQuad
        }
    }

    ///////////////END UI elements

    ///////////////BEGIN ABILITIES

    Ability.Animations {
        id: _animations
        layouts: layoutsContainer
        metrics: _metrics
        settings: universalSettings
    }

    Ability.AutoSize {
        id: _autosize
        layouts: layoutsContainer
        layouter: _layouter
        metrics: _metrics
        visibility: visibilityManager
    }

    Ability.Debug {
        id: _debug
    }

    AbilityHost.Environment{
        id: _environment
    }

    Ability.Indexer {
        id: _indexer
        layouts: layoutsContainer
    }

    Ability.Indicators{
        id: _indicators
        view: latteView
    }

    Ability.Launchers {
        id: _launchers
        layouts: layoutsContainer
        layoutName: latteView && latteView.layout ? latteView.layout.name : ""
    }

    Ability.Layouter {
        id: _layouter
        animations: _animations
        indexer: _indexer
        layouts: layoutsContainer
    }

    Ability.Metrics {
        id: _metrics
        animations: _animations
        autosize: _autosize
        background: _background
        indicators: _indicators
        parabolic: _parabolic
    }

    Ability.MyView {
        id: _myView
        layouts: layoutsContainer
    }

    Ability.ParabolicEffect {
        id: _parabolic
        animations: _animations
        debug: _debug
        layouts: layoutsContainer
        view: latteView
        settings: universalSettings
    }

    Ability.PositionShortcuts {
        id: _shortcuts
        layouts: layoutsContainer
    }

    Ability.ThinTooltip {
        id: _thinTooltip
        debug: _debug
        layouts: layoutsContainer
        view: latteView
    }

    Ability.UserRequests {
        id: _userRequests
        view: latteView
    }

    LatteApp.Interfaces {
        id: _interfaces
        plasmoidInterface: plasmoid

        Component.onCompleted: {
            view.interfacesGraphicObj = _interfaces;
        }

        onViewChanged: {
            if (view) {
                view.interfacesGraphicObj = _interfaces;

                if (!root.inStartup) {
                    //! used from recreating views
                    root.inStartup = true;
                    startupDelayer.start();
                }
            }
        }
    }

    ///////////////END ABILITIES

    ///////////////BEGIN TIMER elements

    //! It is used in order to slide-in the latteView on startup
    onInStartupChanged: {
        if (!inStartup) {
            latteView.positioner.startupFinished();
            latteView.positioner.slideInDuringStartup();
            visibilityManager.slotMustBeShown();
        }
    }

    Connections {
        target:fastLayoutManager
        onHasRestoredAppletsChanged: {
            if (fastLayoutManager.hasRestoredApplets) {
                startupDelayer.start();
            }
        }
    }

    Timer {
        //! Give a little more time to layouter and applets to load and be positioned properly during startup when
        //! the view is drawn out-of-screen and afterwards trigger the startup animation sequence:
        //! 1.slide-out when out-of-screen //slotMustBeHide()
        //! 2.be positioned properly at correct screen //slideInDuringStartup(), triggers Positioner::syncGeometry()
        //! 3.slide-in properly in correct screen //slotMustBeShown();
        id: startupDelayer
        interval: 1000
        onTriggered: {
            visibilityManager.slotMustBeHide();
        }
    }

    ///////////////END TIMER elements

    Loader{
        anchors.fill: parent
        active: debug.localGeometryEnabled
        sourceComponent: Rectangle{
            x: latteView.localGeometry.x
            y: latteView.localGeometry.y
            //! when view is resized there is a chance that geometry is faulty stacked in old values
            width: Math.min(latteView.localGeometry.width, root.width) //! fixes updating
            height: Math.min(latteView.localGeometry.height, root.height) //! fixes updating

            color: "blue"
            border.width: 2
            border.color: "red"

            opacity: 0.35
        }
    }

    Loader{
        anchors.fill: parent
        active: latteView && latteView.effects && debug.inputMaskEnabled
        sourceComponent: Rectangle{
            x: latteView.effects.inputMask.x
            y: latteView.effects.inputMask.y
            //! when view is resized there is a chance that geometry is faulty stacked in old values
            width: Math.min(latteView.effects.inputMask.width, root.width) //! fixes updating
            height: Math.min(latteView.effects.inputMask.height, root.height) //! fixes updating

            color: "purple"
            border.width: 1
            border.color: "black"

            opacity: 0.20
        }
    }
}
