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
import "./debug" as Debug
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
    property bool debugMode: Qt.application.arguments.indexOf("--graphics")>=0
    property bool debugModeLayouter: Qt.application.arguments.indexOf("--layouter")>=0
    property bool debugModeLocalGeometry: Qt.application.arguments.indexOf("--localgeometry")>=0
    property bool debugModeSpacers: Qt.application.arguments.indexOf("--spacers")>=0
    property bool debugModeTimers: Qt.application.arguments.indexOf("--timers")>=0
    property bool debugModeWindow: Qt.application.arguments.indexOf("--with-window")>=0
    property bool debugModeOverloadedIcons: Qt.application.arguments.indexOf("--overloaded-icons")>=0

    readonly property int version: LatteCore.Environment.makeVersion(0,9,4)

    property bool addLaunchersMessage: false
    property bool addLaunchersInTaskManager: plasmoid.configuration.addLaunchersInTaskManager
                                             && latteView
                                             && (latteView.extendedInterface.latteTasksModel.count === 1)
                                             && (latteView.extendedInterface.plasmaTasksModel.count === 0)

    property bool backgroundOnlyOnMaximized: plasmoid.configuration.backgroundOnlyOnMaximized
    property bool behaveAsPlasmaPanel: {
        if (!LatteCore.WindowSystem.compositingActive) {
            //! In NOCOMPOSITING mode VIEWS should behave as real windows and that way
            //! we gain also the snapping features from KWin
            return true;
        }

        if (!latteView || !latteView.visibility) {
            return false;
        }

        if (screenEdgeMarginEnabled && plasmoid.configuration.fittsLawIsRequested) {
            //! dont use when floating views are requesting Fitt's Law
            return false;
        }

        var staticLayout = (plasmoid.configuration.minLength === plasmoid.configuration.maxLength);

        return (visibilityManager.panelIsBiggerFromIconSize
                && (parabolic.factor.maxZoom === 1.0)
                && (plasmoid.configuration.alignment === LatteCore.Types.Justify || staticLayout)
                && !root.editMode
                && !visibilityManager.inLocationAnimation);
    }

    readonly property bool behaveAsDockWithMask: !behaveAsPlasmaPanel

    readonly property bool viewIsAvailable: latteView && latteView.visibility && latteView.effects

    property int viewType: {
        var staticLayout = (plasmoid.configuration.minLength === plasmoid.configuration.maxLength);

        if ((plasmoid.configuration.alignment === LatteCore.Types.Justify || staticLayout)
                && (plasmoid.configuration.useThemePanel)
                && (plasmoid.configuration.panelSize === 100)
                && (parabolic.factor.maxZoom === 1.0)) {
            return LatteCore.Types.PanelView;
        }

        return LatteCore.Types.DockView;
    }

    property bool blurEnabled: plasmoid.configuration.blurEnabled && (!forceTransparentPanel || forcePanelForBusyBackground)

    readonly property bool ignoreRegularFilesDragging: !root.editMode
                                                       && (dragInfo.computationsAreValid || foreDropArea.dragInfo.computationsAreValid)
                                                       && !root.dragInfo.isPlasmoid
                                                       && !root.dragInfo.onlyLaunchers
    readonly property Item dragInfo: Item {
        property bool entered: backDropArea.dragInfo.entered || foreDropArea.dragInfo.entered
        property bool isTask: backDropArea.dragInfo.isTask || foreDropArea.dragInfo.isTask
        property bool isPlasmoid: backDropArea.dragInfo.isPlasmoid || foreDropArea.dragInfo.isPlasmoid
        property bool isSeparator: backDropArea.dragInfo.isSeparator || foreDropArea.dragInfo.isSeparator
        property bool isLatteTasks: backDropArea.dragInfo.isLatteTasks || foreDropArea.dragInfo.isLatteTasks
        property bool onlyLaunchers: backDropArea.dragInfo.onlyLaunchers || foreDropArea.dragInfo.onlyLaunchers

        //  onIsPlasmoidChanged: console.log("isPlasmoid :: " + backDropArea.dragInfo.isPlasmoid + " _ " + foreDropArea.dragInfo.isPlasmoid );
        //  onEnteredChanged: console.log("entered :: " + backDropArea.dragInfo.entered + " _ " + foreDropArea.dragInfo.entered );
    }

    property bool containsOnlyPlasmaTasks: latteView ? latteView.extendedInterface.hasPlasmaTasks && !latteView.extendedInterface.hasLatteTasks : false
    property bool dockContainsMouse: latteView && latteView.visibility ? latteView.visibility.containsMouse : false

    property bool disablePanelShadowMaximized: plasmoid.configuration.disablePanelShadowForMaximized && LatteCore.WindowSystem.compositingActive
    property bool drawShadowsExternal: panelShadowsActive && behaveAsPlasmaPanel && !visibilityManager.inTempHiding

    property bool editMode: editModeVisual.inEditMode
    property bool windowIsTouching: latteView && latteView.windowsTracker
                                    && (latteView.windowsTracker.currentScreen.activeWindowTouching
                                        || latteView.windowsTracker.currentScreen.activeWindowTouchingEdge
                                        || hasExpandedApplet)

    property bool forceSolidPanel: (latteView && latteView.visibility
                                    && LatteCore.WindowSystem.compositingActive
                                    && !inConfigureAppletsMode
                                    && userShowPanelBackground
                                    && ( (plasmoid.configuration.solidBackgroundForMaximized
                                          && !(hasExpandedApplet && !plasmaBackgroundForPopups)
                                          && (latteView.windowsTracker.currentScreen.existsWindowTouching
                                              || latteView.windowsTracker.currentScreen.existsWindowTouchingEdge))
                                        || (hasExpandedApplet && plasmaBackgroundForPopups) ))
                                   || solidBusyForTouchingBusyVerticalView
                                   || plasmaStyleBusyForTouchingBusyVerticalView
                                   || !LatteCore.WindowSystem.compositingActive

    property bool forceTransparentPanel: root.backgroundOnlyOnMaximized
                                         && latteView && latteView.visibility
                                         && LatteCore.WindowSystem.compositingActive
                                         && !inConfigureAppletsMode
                                         && !forceSolidPanel
                                         && !(latteView.windowsTracker.currentScreen.existsWindowTouching
                                              || latteView.windowsTracker.currentScreen.existsWindowTouchingEdge)
                                         && !(windowColors === LatteContainment.Types.ActiveWindowColors && selectedWindowsTracker.existsWindowActive)

    property bool forcePanelForBusyBackground: userShowPanelBackground && (root.themeColors === LatteContainment.Types.SmartThemeColors)
                                               && ( (root.forceTransparentPanel && colorizerManager.backgroundIsBusy)
                                                   || normalBusyForTouchingBusyVerticalView )

    property bool normalBusyForTouchingBusyVerticalView: (latteView && latteView.windowsTracker /*is touching a vertical view that is in busy state and the user prefers isBusy transparency*/
                                                          && latteView.windowsTracker.currentScreen.isTouchingBusyVerticalView
                                                          && root.themeColors === LatteContainment.Types.SmartThemeColors
                                                          && plasmoid.configuration.backgroundOnlyOnMaximized
                                                          /*&& !plasmoid.configuration.solidBackgroundForMaximized
                                                          && !plasmaBackgroundForPopups*/)

    property bool solidBusyForTouchingBusyVerticalView: false //DISABLED, until to check if the normalBusyForTouchingBusyVerticalView is enough to catch and handle the case
    /*(latteView && latteView.windowsTracker /*is touching a vertical view that is in busy state and the user prefers solidness*/
    /* && latteView.windowsTracker.currentScreen.isTouchingBusyVerticalView
                                                         && root.themeColors === LatteContainment.Types.SmartThemeColors
                                                         && plasmoid.configuration.backgroundOnlyOnMaximized
                                                         && plasmoid.configuration.solidBackgroundForMaximized
                                                         && !plasmaBackgroundForPopups)*/

    property bool plasmaStyleBusyForTouchingBusyVerticalView: false //DISABLED, until to check if the normalBusyForTouchingBusyVerticalView is enough to catch and handle the case
    //(latteView && latteView.windowsTracker /*is touching a vertical view that is in busy state and the user prefers solidness*/
    /* && latteView.windowsTracker.currentScreen.isTouchingBusyVerticalView
                                                               && root.themeColors === LatteContainment.Types.SmartThemeColors
                                                               && plasmoid.configuration.backgroundOnlyOnMaximized
                                                               && plasmaBackgroundForPopups)*/

    property bool hideThickScreenGap: screenEdgeMarginEnabled
                                      && plasmoid.configuration.hideScreenGapForMaximized
                                      && latteView && latteView.windowsTracker
                                      && latteView.windowsTracker.currentScreen.existsWindowMaximized

    property bool hideLengthScreenGaps: hideThickScreenGap
                                        && (latteView.visibility.mode === LatteCore.Types.AlwaysVisible
                                            || latteView.visibility.mode === LatteCore.Types.WindowsGoBelow)
                                        && (plasmoid.configuration.alignment === LatteCore.Types.Justify)
                                        && plasmoid.configuration.maxLength>85
                                        && !root.editMode



    property int themeColors: plasmoid.configuration.themeColors
    property int windowColors: plasmoid.configuration.windowColors

    property bool colorizerEnabled: themeColors !== LatteContainment.Types.PlasmaThemeColors || windowColors !== LatteContainment.Types.NoneWindowColors

    property bool plasmaBackgroundForPopups: plasmoid.configuration.plasmaBackgroundForPopups

    readonly property bool hasExpandedApplet: latteView && latteView.extendedInterface.hasExpandedApplet;
    readonly property bool hasUserSpecifiedBackground: (latteView && latteView.layout && latteView.layout.background.startsWith("/")) ?
                                                           true : false

    readonly property bool inConfigureAppletsMode: root.editMode && (plasmoid.configuration.inConfigureAppletsMode || !LatteCore.WindowSystem.compositingActive)
    readonly property bool parabolicEffectEnabled: parabolic.factor.zoom>1 && !inConfigureAppletsMode

    property bool dockIsShownCompletely: !(dockIsHidden || inSlidingIn || inSlidingOut) && !root.editMode
    property bool closeActiveWindowEnabled: plasmoid.configuration.closeActiveWindowEnabled
    property bool dragActiveWindowEnabled: plasmoid.configuration.dragActiveWindowEnabled
    property bool immutable: plasmoid.immutable
    property bool inFullJustify: (plasmoid.configuration.alignment === LatteCore.Types.Justify) && (maxLengthPerCentage===100)
    property bool inSlidingIn: visibilityManager ? visibilityManager.inSlidingIn : false
    property bool inSlidingOut: visibilityManager ? visibilityManager.inSlidingOut : false
    property bool inStartup: true
    property bool isHalfShown: false //is used to disable the zoom hovering effect at sliding in-out the dock
    property bool isHorizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    property bool isReady: !(dockIsHidden || inSlidingIn || inSlidingOut)
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
        if (root.panelAlignment === LatteCore.Types.Justify) {
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

    property int panelAlignment: plasmoid.configuration.alignment

    readonly property string plasmoidName: "org.kde.latte.plasmoid"

    property var iconsArray: [16, 22, 32, 48, 64, 96, 128, 256]

    property Item dragOverlay
    property Item toolBox
    property Item latteAppletContainer
    property Item latteApplet

    readonly property alias animations: _animations
    readonly property alias background: _background
    readonly property alias autosize: _autosize
    readonly property alias indexer: _indexer
    readonly property alias indicatorsManager: indicators
    readonly property alias layouter: _layouter
    readonly property alias metrics: _metrics
    readonly property alias parabolic: _parabolic

    readonly property alias maskManager: visibilityManager
    readonly property alias layoutsContainerItem: layoutsContainer

    readonly property alias latteView: _interfaces.view
    readonly property alias layoutsManager: _interfaces.layoutsManager
    readonly property alias shortcutsEngine: _interfaces.globalShortcuts
    readonly property alias themeExtended: _interfaces.themeExtended
    readonly property alias universalSettings: _interfaces.universalSettings

    readonly property QtObject viewLayout: latteView && latteView.layout ? latteView.layout : null
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
    property bool dockIsHidden: latteView && latteView.visibility ? latteView.visibility.isHidden : true

    property bool titleTooltips: plasmoid.configuration.titleTooltips

    property int tasksCount: latteApplet ? latteApplet.tasksCount : 0  

    property rect screenGeometry: latteView ? latteView.screenGeometry : plasmoid.screenGeometry

    readonly property color minimizedDotColor: colorizerManager.minimizedDotColor
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

    //// BEGIN OF Behaviors
    Behavior on offset {
        enabled: editModeVisual.editAnimationInFullThickness
        NumberAnimation {
            id: offsetAnimation

            duration: 0.8 * animations.duration.proposed
            easing.type: Easing.OutCubic
        }
    }
    //// END OF Behaviors

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
    }

    onInConfigureAppletsModeChanged: {
        updateIndexes();
    }

    //! It is used only when the user chooses different alignment types
    //! and not during startup
    onPanelAlignmentChanged: {
        if (!root.editMode) {
            return;
        }

        if (root.editMode/*!inConfigureAppletsMode*/){
            if (panelAlignment===LatteCore.Types.Justify) {
                addInternalViewSplitters();
                splitMainLayoutToLayouts();
            } else {
                joinLayoutsToMainLayout();
                root.destroyInternalViewSplitters();
            }
        }

        LayoutManager.save();
        updateIndexes();
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

    onIsReadyChanged: {
        if (isReady && !titleTooltipDialog.visible && titleTooltipDialog.activeItemHovered){
            titleTooltipDialog.show(titleTooltipDialog.activeItem, titleTooltipDialog.activeItemText);
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
        if (plasmoid.immutable) {
            if (dragOverlay) {
                dragOverlay.destroy();
            }
            return;
        }

        // console.debug("user configuring", plasmoid.userConfiguring)

        if (plasmoid.userConfiguring) {
            //  console.log("applets------");
            for (var i = 0; i < plasmoid.applets.length; ++i) {
                //    console.log("applet:"+i);
                plasmoid.applets[i].expanded = false;
            }
            if (!dragOverlay) {
                var component = Qt.createComponent("editmode/ConfigOverlay.qml");
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
        ///they break the !immutable experience, the latteView becomes too small
        ///to add applets
        /*   if (plasmoid.immutable) {
            if(root.isHorizontal) {
                root.Layout.preferredWidth = (plasmoid.configuration.alignment === LatteCore.Types.Justify ?
                                                  layoutsContainer.width + 0.5*iconMargin : layoutsContainer.mainLayout.width + iconMargin);
            } else {
                root.Layout.preferredHeight = (plasmoid.configuration.alignment === LatteCore.Types.Justify ?
                                                   layoutsContainer.height + 0.5*iconMargin : layoutsContainer.mainLayout.height + iconMargin);
            }
        } else {
            if (root.isHorizontal) {
                root.Layout.preferredWidth = Screen.width;
            } else {
                root.Layout.preferredHeight = Screen.height;
            }
        }*/

        visibilityManager.updateMaskArea();
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

    function addInternalViewSplitters(){
        if (internalViewSplittersCount() === 0) {
            addInternalViewSplitter(plasmoid.configuration.splitterPosition);
            addInternalViewSplitter(plasmoid.configuration.splitterPosition2);
        }        
    }

    function addInternalViewSplitter(pos){
        var splittersCount = internalViewSplittersCount();
        if(splittersCount<2){
            var container = appletContainerComponent.createObject(root);

            container.internalSplitterId = splittersCount+1;
            container.visible = true;

            if(pos>=0 ){
                LayoutManager.insertAtIndex(layoutsContainer.mainLayout, container, pos);
            } else {
                LayoutManager.insertAtIndex(layoutsContainer.mainLayout, container, Math.floor(layouter.mainLayout.count / 2));
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

    function containmentActions(){
        return latteView.containmentActions();
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

    function mouseInHoverableArea() {
        return (latteView.visibility.containsMouse && !rootMouseArea.containsMouse && mouseInCanBeHoveredApplet());
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

    function layoutManagerMoveAppletsBasedOnJustifyAlignment() {
        if (plasmoid.configuration.alignment !== 10) {
            return;
        }
        layouter.appletsInParentChange = true;

        var splitter = -1;
        var startChildrenLength = layoutsContainer.startLayout.children.length;

        //! Check if there is a splitter inside start layout after the user was dragging its applets
        for (var i=0; i<startChildrenLength; ++i) {
            var item = layoutsContainer.startLayout.children[i];
            if(item.isInternalViewSplitter) {
                splitter = i;
                break;
            }
        }

        //! If a splitter was found inside the startlayout move the head applets after splitter as tail
        //! applets of mainlayout
        if (splitter>=0) {
            for (var i=startChildrenLength-1; i>=splitter; --i){
                var item = layoutsContainer.startLayout.children[i];
                LayoutManager.insertAtIndex(layoutsContainer.mainLayout, item, 0);
            }
        }

        var splitter2 = -1;
        var endChildrenLength = layoutsContainer.endLayout.children.length;

        //! Check if there is a splitter inside endlayout after the user was dragging its applets
        for (var i=0; i<endChildrenLength; ++i) {
            var item = layoutsContainer.endLayout.children[i];
            if(item.isInternalViewSplitter) {
                splitter2 = i;
                break;
            }
        }

        //! If a splitter was found inside the endlayout move the tail applets until splitter as head
        //! applets of mainlayout
        if (splitter2>=0) {
            for (var i=0; i<=splitter2; ++i){
                var item = layoutsContainer.endLayout.children[0];
                item.parent = layoutsContainer.mainLayout;
            }
        }

        //! Validate applets positioning and move applets out of splitters to start/endlayouts accordingly
        splitMainLayoutToLayouts();

        layouter.appletsInParentChange = false;
    }

    function splitMainLayoutToLayouts() {
        if (internalViewSplittersCount() === 2) {
            layouter.appletsInParentChange = true;

            console.log("LAYOUTS: Moving applets from MAIN to THREE Layouts mode...");
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

            if (splitter > 0) {
                for (var i=0; i<splitter; ++i){
                    var item = layoutsContainer.mainLayout.children[0];
                    item.parent = layoutsContainer.startLayout;
                }
            }

            if (splitter2 > 0) {
                splitter2 = splitter2 - splitter;
                // console.log("update layouts 2:"+splitter + " - "+splitter2);

                totalChildren = layoutsContainer.mainLayout.children.length;

                for (var i=totalChildren-1; i>=splitter2+1; --i){
                    var item = layoutsContainer.mainLayout.children[i];
                    LayoutManager.insertAtIndex(layoutsContainer.endLayout, item, 0);
                }
            }

            layouter.appletsInParentChange = false;
        }
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


    ////BEGIN interfaces

    Connections {
        target: LatteCore.WindowSystem

        onCompositingActiveChanged: {
            visibilityManager.updateMaskArea();
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

            if (isReady) {
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
            }

            if (root.debugModeTimers) {
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
            indexer: _indexer
            layouter: _layouter
            metrics: _metrics
            parabolic: _parabolic
            shortcuts: _shortcuts
        }
    }

    Indicators.Manager{
        id: indicators
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

    //it is used to check if the mouse is outside the layoutsContainer borders,
    //so in that case the onLeave event behavior should be trigerred
    RootMouseArea{
        id: rootMouseArea
    }

    Loader{
        active: root.debugModeWindow
        sourceComponent: Debug.DebugWindow{}
    }

    EditMode.Visual{
        id:editModeVisual
        //   z: root.behaveAsPlasmaPanel ? 1 : 0
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
        active: root.debugMode
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
        applets: layoutsContainer.applets
    }

    DragDropArea {
        id: backDropArea
        anchors.fill: parent
        readonly property bool higherPriority: latteView && latteView.containsDrag
                                               && ((root.dragInfo.isPlasmoid && root.dragInfo.isSeparator)
                                                   || (foreDropArea.dragInfo.computationsAreValid && !root.dragInfo.isPlasmoid && !root.dragInfo.onlyLaunchers))

        Item{
            anchors.fill: layoutsContainer

            Background.MultiLayered{
                id: _background
            }
        }

        Layouts.LayoutsContainer {
            id: layoutsContainer
        }

        DragDropArea {
            id: foreDropArea
            anchors.fill: parent
            visible: !backDropArea.higherPriority
            isForeground: true

            /* Rectangle {
                anchors.fill: parent
                color: "blue"
                opacity: 0.5
            }*/
        }
    }

    Colorizer.Manager {
        id: colorizerManager
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

    Ability.Indexer {
        id: _indexer
        layouts: layoutsContainer
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
        indicators: indicatorsManager
    }

    Ability.ParabolicEffect {
        id: _parabolic
        animations: _animations
        applets: layoutsContainer.applets
        view: latteView
    }

    Ability.PositionShortcuts {
        id: _shortcuts
        layouts: layoutsContainer
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
        active: root.debugModeLocalGeometry
        sourceComponent: Rectangle{
            x: latteView.localGeometry.x
            y: latteView.localGeometry.y
            width: latteView.localGeometry.width
            height: latteView.localGeometry.height

            color: "blue"
            border.width: 2
            border.color: "red"

            opacity: 0.35
        }
    }
}
