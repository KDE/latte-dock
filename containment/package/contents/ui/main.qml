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

import QtQuick 2.8
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents
import org.kde.latte.private.app 0.1 as LatteApp
import org.kde.latte.private.containment 0.1 as LatteContainment

import "abilities" as Ability
import "applet" as Applet
import "colorizer" as Colorizer
import "editmode" as EditMode
import "indicators" as Indicators
import "layouts" as Layouts
import "./background" as Background
import "./debugger" as Debugger
import "../code/LayoutManager.js" as LayoutManager

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

    property bool floatingInternalGapIsForced: {
        if (plasmoid.configuration.floatingInternalGapIsForced === Qt.UnChecked) {
            return false;
        } else if (plasmoid.configuration.floatingInternalGapIsForced === Qt.Checked) {
            return true;
        }  else if (plasmoid.configuration.floatingInternalGapIsForced === Qt.PartiallyChecked /*Auto*/) {
            return viewTypeInQuestion === LatteCore.Types.PanelView ? false : true;
        }

        return true;
    }

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

    property bool hideThickScreenGap: screenEdgeMarginEnabled
                                      && plasmoid.configuration.hideFloatingGapForMaximized
                                      && latteView && latteView.windowsTracker
                                      && latteView.windowsTracker.currentScreen.existsWindowMaximized

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

    readonly property bool inConfigureAppletsMode: root.editMode && plasmoid.configuration.inConfigureAppletsMode

    property bool closeActiveWindowEnabled: plasmoid.configuration.closeActiveWindowEnabled
    property bool dragActiveWindowEnabled: plasmoid.configuration.dragActiveWindowEnabled
    property bool immutable: plasmoid.immutable
    property bool inFullJustify: (plasmoid.configuration.alignment === LatteCore.Types.Justify) && (maxLengthPerCentage===100)
    property bool inStartup: true
    property bool isHorizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    property bool isVertical: !isHorizontal

    property bool mouseWheelActions: plasmoid.configuration.mouseWheelActions
    property bool onlyAddingStarup: true //is used for the initialization phase in startup where there aren't removals, this variable provides a way to grow icon size
    property bool shrinkThickMargins: plasmoid.configuration.shrinkThickMargins

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

    property int latteAppletPos: -1
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
    property int panelEdgeSpacing: Math.max(background.lengthMargins, 1.5*appShadowSize)
    property int panelTransparency: plasmoid.configuration.panelTransparency //user set

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

        //! Draw shadows for isBusy state only when current panelTransparency is greater than 10%
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

    property int appShadowOpacity: (plasmoid.configuration.shadowOpacity/100) * 255
    property int appShadowSize: enableShadows ? (0.5*metrics.iconSize) * (plasmoid.configuration.shadowSize/100) : 0
    property int appShadowSizeOriginal: enableShadows ? (0.5*metrics.maxIconSize) * (plasmoid.configuration.shadowSize/100) : 0

    property string appChosenShadowColor: {
        if (plasmoid.configuration.shadowColorType === LatteContainment.Types.ThemeColorShadow) {
            var strC = String(theme.textColor);
            return strC.indexOf("#") === 0 ? strC.substr(1) : strC;
        } else if (plasmoid.configuration.shadowColorType === LatteContainment.Types.UserColorShadow) {
            return plasmoid.configuration.shadowColor;
        }

        // default shadow color
        return "080808";
    }

    property string appShadowColor: "#" + decimalToHex(appShadowOpacity) + appChosenShadowColor
    property string appShadowColorSolid: "#" + appChosenShadowColor

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

    property bool screenEdgeMarginEnabled: plasmoid.configuration.screenEdgeMargin >= 0 && !plasmoid.configuration.shrinkThickMargins

    property int widthMargins: root.isVertical ? metrics.totals.thicknessEdges : metrics.totals.lengthEdges
    property int heightMargins: root.isHorizontal ? metrics.totals.thicknessEdges : metrics.totals.lengthEdges

    readonly property string plasmoidName: "org.kde.latte.plasmoid"

    property var iconsArray: [16, 22, 32, 48, 64, 96, 128, 256]

    property Item dragOverlay: _dragOverlay
    property Item toolBox
    property Item latteAppletContainer
    property Item latteApplet

    readonly property alias animations: _animations
    readonly property alias autosize: _autosize
    readonly property alias background: _background
    readonly property alias debug: _debug
    readonly property alias indexer: _indexer
    readonly property alias indicators: _indicators
    readonly property alias layouter: _layouter
    readonly property alias launchers: _launchers
    readonly property alias metrics: _metrics
    readonly property alias myView: _myView
    readonly property alias parabolic: _parabolic
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

    // TO BE DELETED, if not needed: property int counter:0;

    ///BEGIN properties provided to Latte Plasmoid
    //shadows for applets, it should be removed as the appleitems don't need it any more
    property bool badges3DStyle: universalSettings ? universalSettings.badges3DStyle : true
    property bool enableShadows: plasmoid.configuration.appletShadowsEnabled

    property bool titleTooltips: {
        if (behaveAsPlasmaPanel) {
            return false;
        }

        return plasmoid.configuration.titleTooltips;
    }

    property int tasksCount: latteApplet ? latteApplet.tasksCount : 0

    property rect screenGeometry: latteView ? latteView.screenGeometry : plasmoid.screenGeometry
    ///END properties from latteApplet


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
            if(iconsArray[i] === metrics.iconSize){
                return i;
            }
        }
        return 3;
    }

    //// END properties in functions

    ////////////////END properties

    //////////////START OF BINDINGS

    //! Binding is needed in order for hideLengthScreenGaps to be activated or not only after
    //! View sliding in/out has finished. This way the animation is smoother for behaveAsPlasmaPanels
    Binding{
        target: root
        property: "hideLengthScreenGaps"
        when: latteView && latteView.positioner && latteView.visibility
              && ((root.behaveAsPlasmaPanel && latteView.positioner.slideOffset === 0)
                  || root.behaveAsDockWithMask)
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

    //! It is used only when the user chooses different alignment types
    //! and not during startup
    Connections {
        target: myView
        onAlignmentChanged: {
            if (!root.editMode) {
                return;
            }

            if (root.editMode){
                if (root.myView.alignment===LatteCore.Types.Justify) {
                    root.addInternalViewSplittersInMainLayout();
                    root.moveAppletsBasedOnJustifyAlignment();
                } else {
                    root.joinLayoutsToMainLayout();
                    root.destroyInternalViewSplitters();
                }
            }

            LayoutManager.save();
            root.updateIndexes();
        }
    }

    onLatteViewChanged: {
        if (latteView) {
            if (latteView.positioner) {
                latteView.positioner.hideDockDuringLocationChangeStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterLocationChangeFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
                latteView.positioner.hideDockDuringScreenChangeStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterScreenChangeFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
                latteView.positioner.hideDockDuringMovingToLayoutStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterMovingToLayoutFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
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
                latteView.positioner.hideDockDuringLocationChangeStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterLocationChangeFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
                latteView.positioner.hideDockDuringScreenChangeStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterScreenChangeFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
                latteView.positioner.hideDockDuringMovingToLayoutStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterMovingToLayoutFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
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

    Connections {
        target: myView
        onIsShownFullyChanged: {
            if (root.myView.isShownFully && !titleTooltipDialog.visible && titleTooltipDialog.activeItemHovered) {
                titleTooltipDialog.show(titleTooltipDialog.activeItem, titleTooltipDialog.activeItemText);
            }
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
        //  currentLayout.isLayoutHorizontal = isHorizontal
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = layoutsContainer.mainLayout;
        LayoutManager.layoutS = layoutsContainer.startLayout;
        LayoutManager.layoutE = layoutsContainer.endLayout;
        LayoutManager.lastSpacer = lastSpacer;
        LayoutManager.metrics = metrics;

        upgrader_v010_alignment();

        LayoutManager.restore();
        plasmoid.action("configure").visible = !plasmoid.immutable;
        plasmoid.action("configure").enabled = !plasmoid.immutable;

        inStartupTimer.start();
    }

    Component.onDestruction: {
        console.debug("Destroying Latte Dock Containment ui...");

        layouter.appletsInParentChange = true;

        if (latteView) {
            if (latteView.positioner) {
                latteView.positioner.hideDockDuringLocationChangeStarted.disconnect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterLocationChangeFinished.disconnect(visibilityManager.slotShowDockAfterLocationChange);
                latteView.positioner.hideDockDuringScreenChangeStarted.disconnect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterScreenChangeFinished.disconnect(visibilityManager.slotShowDockAfterLocationChange);
                latteView.positioner.hideDockDuringMovingToLayoutStarted.disconnect(visibilityManager.slotHideDockDuringLocationChange);
                latteView.positioner.showDockAfterMovingToLayoutFinished.disconnect(visibilityManager.slotShowDockAfterLocationChange);
            }

            if (latteView.visibility) {
                latteView.visibility.onContainsMouseChanged.disconnect(visibilityManager.slotContainsMouseChanged);
                latteView.visibility.onMustBeHide.disconnect(visibilityManager.slotMustBeHide);
                latteView.visibility.onMustBeShown.disconnect(visibilityManager.slotMustBeShown);
            }
        }
    }

    Containment.onAppletAdded: {
        addApplet(applet, x, y);
        console.log(applet.pluginName);
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
    }

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
    function addApplet(applet, x, y) {
        var container = appletContainerComponent.createObject(dndSpacer.parent)

        container.applet = applet;
        applet.parent = container.appletWrapper;

        applet.anchors.fill = container.appletWrapper;

        applet.visible = true;


        // don't show applet if it chooses to be hidden but still make it
        // accessible in the panelcontroller
        container.visible = Qt.binding(function() {
            return applet.status !== PlasmaCore.Types.HiddenStatus || (!plasmoid.immutable && root.inConfigureAppletsMode)
        })

        addContainerInLayout(container, applet, x, y);
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
            // frequent-enough occurrence that we'd like to abstract it further in the future
            // and get rid of the ugliness of parties external to the containment adding applets
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

    function addInternalViewSplittersInMainLayout(){
        if (internalViewSplittersCount() === 0) {
            addInternalViewSplitterInMain(plasmoid.configuration.splitterPosition);
            addInternalViewSplitterInMain(plasmoid.configuration.splitterPosition2);
        }
    }

    function addInternalViewSplitterInStart(pos){
        addInternalViewSplitterInLayout(layoutsContainer.startLayout, pos);
    }

    function addInternalViewSplitterInMain(pos){
        addInternalViewSplitterInLayout(layoutsContainer.mainLayout, pos);
    }

    function addInternalViewSplitterInEnd(pos){
        addInternalViewSplitterInLayout(layoutsContainer.endLayout, pos);
    }

    function addInternalViewSplitterInLayout(area, pos){
        var splittersCount = internalViewSplittersCount();
        if(splittersCount<2){
            var splitter = appletContainerComponent.createObject(root);

            splitter.internalSplitterId = splittersCount+1;
            splitter.visible = true;

            if(pos>=0 ){
                LayoutManager.insertAtIndex(area, splitter, pos);
            } else {
                LayoutManager.insertAtIndex(area, splitter, Math.floor(area.count / 2));
            }
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

    function decimalToHex(d, padding) {
        var hex = Number(d).toString(16);
        padding = typeof (padding) === "undefined" || padding === null ? padding = 2 : padding;

        while (hex.length < padding) {
            hex = "0" + hex;
        }

        return hex;
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

    function layoutManager() {
        return LayoutManager;
    }

    function layoutManagerInsertBefore(place, item) {
        LayoutManager.insertBefore(place, item);
    }

    function layoutManagerInsertAfter(place, item) {
        LayoutManager.insertAfter(place, item);
    }

    function layoutManagerSave() {
        LayoutManager.save();
    }

    function layoutManagerSaveOptions() {
        LayoutManager.saveOptions();
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

    function slotPreviewsShown(){
        if (latteView) {
            latteView.extendedInterface.deactivateApplets();
        }
    }

    function moveAppletsBasedOnJustifyAlignment() {
        layouter.appletsInParentChange = true;

        if (latteView) {
            latteView.extendedInterface.moveAppletsInJustifyAlignment(layoutsContainer.startLayout,
                                                                      layoutsContainer.mainLayout,
                                                                      layoutsContainer.endLayout);
        }

        layouter.appletsInParentChange = false;
    }

    function joinLayoutsToMainLayout() {
        layouter.appletsInParentChange = true;

        console.log("LAYOUTS: Moving applets from THREE to MAIN Layout mode...");
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

        layouter.appletsInParentChange = false;
    }

    function upgrader_v010_alignment() {
        //! IMPORTANT, special case because it needs to be loaded on Component constructor
        if (!plasmoid.configuration.alignmentUpgraded) {
            plasmoid.configuration.alignment = plasmoid.configuration.panelPosition;
            plasmoid.configuration.alignmentUpgraded = true;
        }
    }
    //END functions

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
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                Layout.topMargin: 2
                Layout.bottomMargin: 2
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
            parabolic.sglClearZoom.connect(titleTooltipDialog.hide);
        }

        Component.onDestruction: {
            parabolic.sglClearZoom.disconnect(titleTooltipDialog.hide);
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

            if (root.myView.isShownFully) {
                showTitleTooltipTimer.start();
            }

        }

        function update() {
            activeItemHovered = true
            title = activeItemText;
            visualParent = activeItemTooltipParent;
            if (latteApplet && latteApplet.windowPreviewIsShown) {
                latteApplet.hidePreview();
            }

            visible = true;
        }
    }

    Timer {
        id: showTitleTooltipTimer
        interval: 100
        onTriggered: {
            if (latteView && latteView.visibility && latteView.visibility.containsMouse) {
                titleTooltipDialog.update();
            }

            if (titleTooltipDialog.visible) {
                titleTooltipCheckerToNotShowTimer.start();
            }

            if (debug.timersEnabled) {
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
            }

            if (debug.timersEnabled) {
                console.log("containment timer: hideTitleTooltipTimer called...");
            }

        }
    }

    //! Timer to fix #811, rare cases that both a window preview and context menu are
    //! shown
    Timer {
        id: titleTooltipCheckerToNotShowTimer
        interval: 250

        onTriggered: {
            if (titleTooltipDialog.visible && latteApplet && (latteApplet.contextMenu || latteApplet.windowPreviewIsShown)) {
                titleTooltipDialog.visible = false;
            }
        }
    }
    /////END: Title Tooltip///////////

    ///////////////BEGIN components
    Component {
        id: appletContainerComponent
        Applet.AppletItem{
            animations: _animations
            debug: _debug
            indexer: _indexer
            launchers: _launchers
            layouter: _layouter
            layouts: layoutsContainer
            metrics: _metrics
            myView: _myView
            parabolic: _parabolic
            shortcuts: _shortcuts
            userRequests: _userRequests
        }
    }

    Item {
        id: graphicsSystem
        readonly property bool isAccelerated: (GraphicsInfo.api !== GraphicsInfo.Software)
                                              && (GraphicsInfo.api !== GraphicsInfo.Unknown)
    }

    Upgrader {
        id: upgrader
    }

    ///////////////END components

    PlasmaCore.ColorScope{
        id: colorScopePalette
    }


    ///////////////BEGIN UI elements

    Loader{
        active: debug.windowEnabled
        sourceComponent: Debugger.DebugWindow{}
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

        readonly property bool isDndSpacer: true
        readonly property int length: metrics.totals.length
        readonly property int thickness: metrics.totals.thickness + metrics.margin.screenEdge

        Layout.preferredWidth: width
        Layout.preferredHeight: height
        opacity: 0
        z:1500

        LatteComponents.AddItem{
            id: dndSpacerAddItem
            width: root.isHorizontal ? parent.width : parent.width - metrics.margin.screenEdge
            height: root.isHorizontal ? parent.height - metrics.margin.screenEdge: parent.height

            states:[
                State{
                    name: "bottom"
                    when: plasmoid.location === PlasmaCore.Types.BottomEdge

                    AnchorChanges{
                        target: dndSpacerAddItem;
                        anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                    }
                    PropertyChanges{
                        target: dndSpacerAddItem;
                        anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: metrics.margin.screenEdge;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "top"
                    when: plasmoid.location === PlasmaCore.Types.TopEdge

                    AnchorChanges{
                        target: dndSpacerAddItem;
                        anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: dndSpacerAddItem;
                        anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin: metrics.margin.screenEdge;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "left"
                    when: plasmoid.location === PlasmaCore.Types.LeftEdge

                    AnchorChanges{
                        target: dndSpacerAddItem;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                        anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: dndSpacerAddItem;
                        anchors.leftMargin: metrics.margin.screenEdge;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "right"
                    when: plasmoid.location === PlasmaCore.Types.RightEdge

                    AnchorChanges{
                        target: dndSpacerAddItem;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                        anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: dndSpacerAddItem;
                        anchors.leftMargin: 0;    anchors.rightMargin: metrics.margin.screenEdge;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                }
            ]
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

    Ability.Indexer {
        id: _indexer
        layouts: layoutsContainer
    }

    Indicators.Manager{
        id: _indicators
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
    }

    Ability.ParabolicEffect {
        id: _parabolic
        animations: _animations
        debug: _debug
        layouts: layoutsContainer
        view: latteView
    }

    Ability.PositionShortcuts {
        id: _shortcuts
        layouts: layoutsContainer
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
            }
        }
    }

    ///////////////END ABILITIES

    ///////////////BEGIN TIMER elements

    //! It is used in order to slide-in the latteView on startup
    Timer{
        id: inStartupTimer
        interval: 1500
        repeat: false
        onTriggered: {
            if (inStartup) {
                visibilityManager.slotMustBeShown();
            }
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
