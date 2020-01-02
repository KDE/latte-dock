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

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

import "applet" as Applet
import "colorizer" as Colorizer
import "editmode" as EditMode
import "indicators" as Indicators
import "layouts" as Layouts
import "../code/LayoutManager.js" as LayoutManager

Item {
    id: root
    objectName: "containmentViewLayout"

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft && !root.isVertical
    LayoutMirroring.childrenInherit: true

    //// BEGIN SIGNALS
    signal clearZoomSignal();
    signal destroyInternalViewSplitters();
    signal emptyAreasWheel(QtObject wheel);
    signal separatorsUpdated();
    signal signalActivateEntryAtIndex(int entryIndex);
    signal signalNewInstanceForEntryAtIndex(int entryIndex);
    signal updateEffectsArea();
    signal updateIndexes();
    signal updateScale(int delegateIndex, real newScale, real step);

    signal broadcastedToApplet(string pluginName, string action, variant value);
    //// END SIGNALS

    ////BEGIN properties
    property bool debugMode: Qt.application.arguments.indexOf("--graphics")>=0
    property bool debugModeSpacers: Qt.application.arguments.indexOf("--spacers")>=0
    property bool debugModeTimers: Qt.application.arguments.indexOf("--timers")>=0
    property bool debugModeWindow: Qt.application.arguments.indexOf("--with-window")>=0
    property bool debugModeOverloadedIcons: Qt.application.arguments.indexOf("--overloaded-icons")>=0

    readonly property int version: Latte.WindowSystem.makeVersion(0,9,4)

    property bool globalDirectRender: false //it is used as a globalDirectRender for all elements in the dock
    property int directRenderAnimationTime: 0

    property bool addLaunchersMessage: false
    property bool addLaunchersInTaskManager: plasmoid.configuration.addLaunchersInTaskManager
    property bool backgroundOnlyOnMaximized: plasmoid.configuration.backgroundOnlyOnMaximized
    property bool behaveAsPlasmaPanel: {
        if (!latteView || !latteView.visibility) {
            return false;
        }

        if (screenEdgeMarginEnabled && plasmoid.configuration.fittsLawIsRequested) {
            //! dont use when floating views are requesting Fitt's Law
            return false;
        }

        return (visibilityManager.panelIsBiggerFromIconSize && (maxZoomFactor === 1.0)
                //&& (latteView.visibility.mode === Latte.Types.AlwaysVisible || latteView.visibility.mode === Latte.Types.WindowsGoBelow)
                && (plasmoid.configuration.panelPosition === Latte.Types.Justify)
                && !root.editMode);
    }

    property int viewType: {
        if ((plasmoid.configuration.panelPosition === Latte.Types.Justify)
                && (plasmoid.configuration.useThemePanel)
                && (plasmoid.configuration.panelSize === 100)
                && (maxZoomFactor === 1.0)) {
            return Latte.Types.PanelView;
        }

        return Latte.Types.DockView;
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

    property bool containsOnlyPlasmaTasks: false //this is flag to indicate when from tasks only a plasma based one is found
    property bool dockContainsMouse: latteView && latteView.visibility ? latteView.visibility.containsMouse : false

    property bool disablePanelShadowMaximized: plasmoid.configuration.disablePanelShadowForMaximized && Latte.WindowSystem.compositingActive
    property bool drawShadowsExternal: panelShadowsActive && behaveAsPlasmaPanel && !visibilityManager.inTempHiding
    property bool editMode: editModeVisual.inEditMode
    property bool windowIsTouching: latteView && latteView.windowsTracker
                                    && (latteView.windowsTracker.currentScreen.activeWindowTouching || hasExpandedApplet)

    property bool forceSolidPanel: (latteView && latteView.visibility
                                    && Latte.WindowSystem.compositingActive
                                    && !inConfigureAppletsMode
                                    && userShowPanelBackground
                                    && ( (plasmoid.configuration.solidBackgroundForMaximized
                                          && !(hasExpandedApplet && !plasmaBackgroundForPopups)
                                          && latteView.windowsTracker.currentScreen.existsWindowTouching)
                                        || (hasExpandedApplet && plasmaBackgroundForPopups) ))
                                   || solidBusyForTouchingBusyVerticalView
                                   || plasmaStyleBusyForTouchingBusyVerticalView
                                   || !Latte.WindowSystem.compositingActive

    property bool forceTransparentPanel: root.backgroundOnlyOnMaximized
                                         && latteView && latteView.visibility
                                         && Latte.WindowSystem.compositingActive
                                         && !inConfigureAppletsMode
                                         && !forceSolidPanel
                                         && !latteView.windowsTracker.currentScreen.existsWindowTouching
                                         && !(windowColors === Latte.Types.ActiveWindowColors && selectedWindowsTracker.existsWindowActive)

    property bool forcePanelForBusyBackground: userShowPanelBackground && (root.themeColors === Latte.Types.SmartThemeColors)
                                               && ( (root.forceTransparentPanel && colorizerManager.backgroundIsBusy)
                                                   || normalBusyForTouchingBusyVerticalView )

    property bool normalBusyForTouchingBusyVerticalView: (latteView && latteView.windowsTracker /*is touching a vertical view that is in busy state and the user prefers isBusy transparency*/
                                                          && latteView.windowsTracker.currentScreen.isTouchingBusyVerticalView
                                                          && root.themeColors === Latte.Types.SmartThemeColors
                                                          && plasmoid.configuration.backgroundOnlyOnMaximized
                                                          /*&& !plasmoid.configuration.solidBackgroundForMaximized
                                                          && !plasmaBackgroundForPopups*/)

    property bool solidBusyForTouchingBusyVerticalView: false //DISABLED, until to check if the normalBusyForTouchingBusyVerticalView is enough to catch and handle the case
    /*(latteView && latteView.windowsTracker /*is touching a vertical view that is in busy state and the user prefers solidness*/
    /* && latteView.windowsTracker.currentScreen.isTouchingBusyVerticalView
                                                         && root.themeColors === Latte.Types.SmartThemeColors
                                                         && plasmoid.configuration.backgroundOnlyOnMaximized
                                                         && plasmoid.configuration.solidBackgroundForMaximized
                                                         && !plasmaBackgroundForPopups)*/

    property bool plasmaStyleBusyForTouchingBusyVerticalView: false //DISABLED, until to check if the normalBusyForTouchingBusyVerticalView is enough to catch and handle the case
    //(latteView && latteView.windowsTracker /*is touching a vertical view that is in busy state and the user prefers solidness*/
    /* && latteView.windowsTracker.currentScreen.isTouchingBusyVerticalView
                                                               && root.themeColors === Latte.Types.SmartThemeColors
                                                               && plasmoid.configuration.backgroundOnlyOnMaximized
                                                               && plasmaBackgroundForPopups)*/

    property bool hideThickScreenGap: screenEdgeMarginEnabled
                                      && plasmoid.configuration.hideScreenGapForMaximized
                                      && latteView && latteView.windowsTracker
                                      && latteView.windowsTracker.currentScreen.existsWindowMaximized

    property bool hideLengthScreenGaps: hideThickScreenGap
                                        && (latteView.visibility.mode === Latte.Types.AlwaysVisible
                                            || latteView.visibility.mode === Latte.Types.WindowsGoBelow)
                                        && (plasmoid.configuration.panelPosition === Latte.Types.Justify)
                                        && plasmoid.configuration.maxLength>85
                                        && !root.editMode



    property int themeColors: plasmoid.configuration.themeColors
    property int windowColors: plasmoid.configuration.windowColors

    property bool colorizerEnabled: themeColors !== Latte.Types.PlasmaThemeColors || windowColors !== Latte.Types.NoneWindowColors

    property bool plasmaBackgroundForPopups: plasmoid.configuration.plasmaBackgroundForPopups

    readonly property bool hasExpandedApplet: plasmoid.applets.some(function (item) {
        return (item.status >= PlasmaCore.Types.NeedsAttentionStatus && item.status !== PlasmaCore.Types.HiddenStatus
                && item.pluginName !== root.plasmoidName
                && item.pluginName !== "org.kde.plasma.appmenu"
                && item.pluginName !== "org.kde.windowappmenu"
                && item.pluginName !== "org.kde.activeWindowControl");
    })

    readonly property bool hasUserSpecifiedBackground: (latteView && latteView.layout && latteView.layout.background.startsWith("/")) ?
                                                           true : false

    readonly property bool inConfigureAppletsMode: root.editMode && (plasmoid.configuration.inConfigureAppletsMode || !Latte.WindowSystem.compositingActive)
    readonly property bool parabolicEffectEnabled: zoomFactor>1 && !inConfigureAppletsMode

    property bool dockIsShownCompletely: !(dockIsHidden || inSlidingIn || inSlidingOut) && !root.editMode
    property bool dragActiveWindowEnabled: plasmoid.configuration.dragActiveWindowEnabled
    property bool immutable: plasmoid.immutable
    property bool inFullJustify: (plasmoid.configuration.panelPosition === Latte.Types.Justify) && (maxLengthPerCentage===100)
    property bool inSlidingIn: visibilityManager ? visibilityManager.inSlidingIn : false
    property bool inSlidingOut: visibilityManager ? visibilityManager.inSlidingOut : false
    property bool inStartup: true
    property bool isHalfShown: false //is used to disable the zoom hovering effect at sliding in-out the dock
    property bool isHorizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    property bool isReady: !(dockIsHidden || inSlidingIn || inSlidingOut)
    property bool isVertical: !isHorizontal
    property bool isHovered: latteApplet ? ((latteAppletHoveredIndex !== -1) || (layoutsContainer.hoveredIndex !== -1)) //|| wholeArea.containsMouse
                                         : (layoutsContainer.hoveredIndex !== -1) //|| wholeArea.containsMouse
    property bool mouseWheelActions: plasmoid.configuration.mouseWheelActions
    property bool onlyAddingStarup: true //is used for the initialization phase in startup where there aren't removals, this variable provides a way to grow icon size
    property bool shrinkThickMargins: plasmoid.configuration.shrinkThickMargins
    property bool showLatteShortcutBadges: false
    property bool showAppletShortcutBadges: false
    property bool showMetaBadge: false
    property int applicationLauncherId: -1

    //FIXME: possibly this is going to be the default behavior, this user choice
    //has been dropped from the Dock Configuration Window
    //property bool smallAutomaticIconJumps: plasmoid.configuration.smallAutomaticIconJumps
    property bool smallAutomaticIconJumps: true

    property bool userShowPanelBackground: Latte.WindowSystem.compositingActive ? plasmoid.configuration.useThemePanel : true
    property bool useThemePanel: noApplets === 0 || !Latte.WindowSystem.compositingActive ?
                                     true : (plasmoid.configuration.useThemePanel || plasmoid.configuration.solidBackgroundForMaximized)

    property alias hoveredIndex: layoutsContainer.hoveredIndex
    property alias directRenderDelayerIsRunning: directRenderDelayerForEnteringTimer.running

    property int actionsBlockHiding: 0 //actions that block hiding

    property int animationsNeedBothAxis:0 //animations need space in both axes, e.g zooming a task
    property int animationsNeedLength: 0 // animations need length, e.g. adding a task
    property int animationsNeedThickness: 0 // animations need thickness, e.g. bouncing animation
    readonly property bool thickAnimated: animationsNeedBothAxis>0 || animationsNeedThickness>0

    property int animationTime: durationTime*2.8*units.longDuration

    property int appletsNeedWindowsTracking: 0

    //what is the highest icon size based on what icon size is used, screen calculated or user specified
    property int maxIconSize: proportionIconSize!==-1 ? proportionIconSize : plasmoid.configuration.iconSize
    property int iconSize: automaticItemSizer.automaticIconSizeBasedSize > 0 && automaticItemSizer.isActive ?
                               Math.min(automaticItemSizer.automaticIconSizeBasedSize, root.maxIconSize) :
                               root.maxIconSize

    property int proportionIconSize: { //icon size based on screen height
        if ((plasmoid.configuration.proportionIconSize===-1) || !latteView)
            return -1;

        return Math.max(16,Math.round(latteView.screenGeometry.height * plasmoid.configuration.proportionIconSize/100/8)*8);
    }

    property int latteAppletPos: -1
    property int maxLengthPerCentage: hideLengthScreenGaps ? 100 : plasmoid.configuration.maxLength

    property int maxLength: {
        if (root.isHorizontal) {
            return behaveAsPlasmaPanel ? width : width * (maxLengthPerCentage/100)
        } else {
            return behaveAsPlasmaPanel ? height : height * (maxLengthPerCentage/100)
        }
    }

    property int leftClickAction: plasmoid.configuration.leftClickAction
    property int middleClickAction: plasmoid.configuration.middleClickAction
    property int hoverAction: plasmoid.configuration.hoverAction
    property int modifier: plasmoid.configuration.modifier
    property int modifierClickAction: plasmoid.configuration.modifierClickAction
    property int modifierClick: plasmoid.configuration.modifierClick
    property int scrollAction: plasmoid.configuration.scrollAction

    property bool panelOutline: plasmoid.configuration.panelOutline
    property int panelEdgeSpacing: Math.max(panelBoxBackground.lengthMargins, 1.5*appShadowSize)
    property int panelTransparency: plasmoid.configuration.panelTransparency //user set
    property int currentPanelTransparency: 0 //application override

    readonly property real currentPanelOpacity: currentPanelTransparency/100

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

        var transparencyCheck = (blurEnabled || (!blurEnabled && currentPanelTransparency>20));

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
    property int appShadowSize: enableShadows ? (0.5*root.iconSize) * (plasmoid.configuration.shadowSize/100) : 0
    property int appShadowSizeOriginal: enableShadows ? (0.5*maxIconSize) * (plasmoid.configuration.shadowSize/100) : 0

    property string appChosenShadowColor: {
        if (plasmoid.configuration.shadowColorType === Latte.Types.ThemeColorShadow) {
            var strC = String(theme.textColor);
            return strC.indexOf("#") === 0 ? strC.substr(1) : strC;
        } else if (plasmoid.configuration.shadowColorType === Latte.Types.UserColorShadow) {
            return plasmoid.configuration.shadowColor;
        }

        // default shadow color
        return "080808";
    }

    property string appShadowColor: "#" + decimalToHex(appShadowOpacity) + appChosenShadowColor
    property string appShadowColorSolid: "#" + appChosenShadowColor

    property int totalPanelEdgeSpacing: 0 //this is set by PanelBox
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
    property int offsetFixed: (offset===0 || panelAlignment === Latte.Types.Center || plasmoid.configuration.panelPosition === Latte.Types.Justify)?
                                  offset : offset+panelMarginLength/2+totalPanelEdgeSpacing/2

    property int realPanelSize: 0
    property int realPanelLength: 0
    property int realPanelThickness: 0
    //this is set by the PanelBox
    property int panelThickMarginBase: 0
    property int panelThickMarginHigh: 0
    property int panelMarginLength: 0
    property int panelShadow: 0 //shadowsSize
    property int editShadow: {
        if (!Latte.WindowSystem.compositingActive) {
            return 0;
        } else if (latteView && latteView.screenGeometry) {
            return latteView.screenGeometry.height/90;
        } else {
            return 7;
        }
    }

    property int themePanelThickness: {
        var panelBase = root.panelThickMarginHigh;
        var margin = shrinkThickMargins ? 0 : thickMargins + localScreenEdgeMargin;
        var maxPanelSize = (iconSize + margin) - panelBase;
        var percentage = Latte.WindowSystem.compositingActive ? plasmoid.configuration.panelSize/100 : 1;
        return Math.max(panelBase, panelBase + percentage*maxPanelSize);
    }

    property int lengthIntMargin: lengthIntMarginFactor * root.iconSize
    property int lengthExtMargin: lengthExtMarginFactor * root.iconSize
    property real lengthIntMarginFactor: indicators.isEnabled ? indicators.padding : 0
    property real lengthExtMarginFactor: plasmoid.configuration.lengthExtMargin / 100

    property real lengthAppletIntMarginFactor: indicators.infoLoaded ? indicators.info.appletLengthPadding : -1

    property real thickMarginFactor: {
        if (shrinkThickMargins) {
            return indicators.info.minThicknessPadding;
        }

        //0.075 old statesLineSize and 0.06 old default thickMargin
        return  Math.max(indicators.info.minThicknessPadding, plasmoid.configuration.thickMargin / 100)
    }
    property int thickMargin: thickMarginFactor * root.iconSize

    property bool screenEdgeMarginEnabled: plasmoid.configuration.screenEdgeMargin >= 0 && !plasmoid.configuration.shrinkThickMargins
    property int screenEdgeMargin: {
        //! is used for window geometry calculations
        if (!screenEdgeMarginEnabled
                || (hideThickScreenGap && localScreenEdgeMargin === 0)) {
                /*window geometry is updated after the local screen margin animation was zeroed*/
            return 0;
        }

        return plasmoid.configuration.screenEdgeMargin;
    }
    property int localScreenEdgeMargin: (screenEdgeMarginEnabled && behaveAsPlasmaPanel)
                                        || !screenEdgeMarginEnabled
                                        || hideThickScreenGap ? 0 : plasmoid.configuration.screenEdgeMargin

    //! thickness margins are always two and equal in order for items
    //! to be always correctly centered
    property int thickMargins: 2 * thickMargin

    //it is used in order to not break the calculations for the thickness placement
    //especially in automatic icon sizes calculations
    property int maxThickMargin: thickMarginFactor * maxIconSize

    property int lengthMargin: lengthIntMargin + lengthExtMargin
    property int lengthMargins: 2 * lengthMargin

    property int widthMargins: root.isVertical ? thickMargins : lengthMargins
    property int heightMargins: root.isHorizontal ? thickMargins : lengthMargins

    ///FIXME: <delete both> I can't remember why this is needed, maybe for the anchorings!!! In order for the Double Layout to not mess the anchorings...
    //property int layoutsContainer.mainLayoutPosition: !plasmoid.immutable ? Latte.Types.Center : (root.isVertical ? Latte.Types.Top : Latte.Types.Left)
    //property int panelAlignment: plasmoid.configuration.panelPosition !== Latte.Types.Justify ? plasmoid.configuration.panelPosition : layoutsContainer.mainLayoutPosition

    property int panelAlignment: !root.inConfigureAppletsMode ? plasmoid.configuration.panelPosition :
                                                                ( plasmoid.configuration.panelPosition === Latte.Types.Justify ?
                                                                     Latte.Types.Center : plasmoid.configuration.panelPosition )

    property int panelUserSetAlignment: plasmoid.configuration.panelPosition

    property real zoomFactor: Latte.WindowSystem.compositingActive && root.animationsEnabled ? ( 1 + (plasmoid.configuration.zoomLevel / 20) ) : 1

    readonly property string plasmoidName: "org.kde.latte.plasmoid"

    property var badgesForActivate: {
        if (!shortcutsEngine) {
            return ['1','2','3','4','5','6','7','8','9','0', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.'];
        }

        return shortcutsEngine.badgesForActivate;
    }

    property var iconsArray: [16, 22, 32, 48, 64, 96, 128, 256]

    property Item dragOverlay
    property Item toolBox
    property Item latteAppletContainer
    property Item latteApplet
    readonly property Item indicatorsManager: indicators
    readonly property Item parabolicManager: _parabolicManager
    readonly property Item maskManager: visibilityManager
    readonly property Item layoutsContainerItem: layoutsContainer

    property QtObject latteView: null
    property QtObject shortcutsEngine: null
    property QtObject themeExtended: null
    property QtObject universalSettings: null
    property QtObject layoutsManager: null
    property QtObject viewLayout: latteView && latteView.layout ? latteView.layout : null
    property QtObject selectedWindowsTracker: {
        if (latteView && latteView.windowsTracker) {
            switch(plasmoid.configuration.activeWindowFilter) {
            case Latte.Types.ActiveInCurrentScreen:
                return latteView.windowsTracker.currentScreen;
            case Latte.Types.ActiveFromAllScreens:
                return latteView.windowsTracker.allScreens;
            }
        }

        return null;
    }

    // TO BE DELETED, if not needed: property int counter:0;

    ///BEGIN properties provided to Latte Plasmoid
    //shadows for applets, it should be removed as the appleitems don't need it any more
    property bool badges3DStyle: universalSettings ? universalSettings.badges3DStyle : true
    property bool enableShadows: plasmoid.configuration.shadows || (root.forceTransparentPanel && plasmoid.configuration.shadows>0)
    property bool dockIsHidden: latteView ? latteView.visibility.isHidden : true
    property bool groupTasksByDefault: plasmoid.configuration.groupTasksByDefault

    property bool showInfoBadge: plasmoid.configuration.showInfoBadge
    property bool showProgressBadge: plasmoid.configuration.showProgressBadge
    property bool showAudioBadge: plasmoid.configuration.showAudioBadge
    property bool audioBadgeActionsEnabled: plasmoid.configuration.audioBadgeActionsEnabled
    property bool infoBadgeProminentColorEnabled: plasmoid.configuration.infoBadgeProminentColorEnabled

    property bool scrollTasksEnabled: plasmoid.configuration.scrollTasksEnabled
    property bool autoScrollTasksEnabled: plasmoid.configuration.autoScrollTasksEnabled
    property int manualScrollTasksType: plasmoid.configuration.manualScrollTasksType

    property bool showWindowActions: plasmoid.configuration.showWindowActions
    property bool showWindowsOnlyFromLaunchers: plasmoid.configuration.showWindowsOnlyFromLaunchers
    property bool showOnlyCurrentScreen: plasmoid.configuration.showOnlyCurrentScreen
    property bool showOnlyCurrentDesktop: plasmoid.configuration.showOnlyCurrentDesktop
    property bool showOnlyCurrentActivity: plasmoid.configuration.showOnlyCurrentActivity

    property bool titleTooltips: plasmoid.configuration.titleTooltips
    property bool unifiedGlobalShortcuts: plasmoid.configuration.unifiedGlobalShortcuts

    readonly property bool hasInternalSeparator: latteApplet ? latteApplet.hasInternalSeparator : false

    property int animationStep: {
        if (!universalSettings || universalSettings.mouseSensitivity === Latte.Types.HighSensitivity) {
            return 1;
        } else if (universalSettings.mouseSensitivity === Latte.Types.MediumSensitivity) {
            return Math.max(3, root.iconSize / 18);
        } else if (universalSettings.mouseSensitivity === Latte.Types.LowSensitivity) {
            return Math.max(5, root.iconSize / 10);
        }
    }

    property int latteAppletHoveredIndex: latteApplet ? latteApplet.hoveredIndex : -1
    property int launchersGroup: plasmoid.configuration.launchersGroup
    property int tasksCount: latteApplet ? latteApplet.tasksCount : 0

    //! Animations
    property bool animationsEnabled: plasmoid.configuration.animationsEnabled && Latte.WindowSystem.compositingActive
    property bool animationLauncherBouncing: animationsEnabled && latteApplet && plasmoid.configuration.animationLauncherBouncing
    property bool animationWindowInAttention: animationsEnabled && latteApplet && plasmoid.configuration.animationWindowInAttention
    property bool animationNewWindowSliding: animationsEnabled && latteApplet && plasmoid.configuration.animationNewWindowSliding
    property bool animationWindowAddedInGroup: animationsEnabled && latteApplet && plasmoid.configuration.animationWindowAddedInGroup
    property bool animationWindowRemovedFromGroup: animationsEnabled && latteApplet && plasmoid.configuration.animationWindowRemovedFromGroup

    property real appliedDurationTime: animationsEnabled ? durationTime : 2
    property real durationTime: {
        if (!animationsEnabled) {
            return 0;
        }

        /*if ((latteView && latteView.effects && latteView.effects.animationsBlocked)
                || !animationsEnabled) {
            return 0;
        }*/

        if (plasmoid.configuration.durationTime === 0 || plasmoid.configuration.durationTime === 2 )
            return plasmoid.configuration.durationTime;

        if (plasmoid.configuration.durationTime === 1)
            return 1.65;
        else if (plasmoid.configuration.durationTime === 3)
            return 2.35;

        return 2;
    }

    property real animationsZoomFactor : {
        if (!animationsEnabled) {
            return 1;
        }

        if (latteApplet && (animationLauncherBouncing || animationWindowInAttention || animationWindowAddedInGroup)) {
            return 1.65;
        }

        return 1;
    }

    property real maxZoomFactor: Math.max(zoomFactor, animationsZoomFactor)

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
            if(iconsArray[i] === iconSize){
                return i;
            }
        }
        return 3;
    }

    //// END properties in functions

    ////////////////END properties

    //// BEGIN OF Behaviors
    Behavior on thickMargin {
        NumberAnimation {
            duration: 0.8 * root.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on lengthIntMargin {
        NumberAnimation {
            duration: 0.8 * root.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on lengthExtMargin {
        NumberAnimation {
            duration: 0.8 * root.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on localScreenEdgeMargin {
        enabled: !behaveAsPlasmaPanel

        NumberAnimation {
            duration: 0.8 * root.animationTime
            easing.type: Easing.OutCubic
        }
    }

    Behavior on iconSize {
        enabled: !(root.editMode && root.behaveAsPlasmaPanel)
        NumberAnimation {
            duration: 0.8 * root.animationTime

            onRunningChanged: {
                if (!running) {
                    delayUpdateMaskArea.start();
                }
            }
        }
    }

    Behavior on offset {
        enabled: editModeVisual.editAnimationInFullThickness
        NumberAnimation {
            id: offsetAnimation

            duration: 0.8 * root.animationTime
            easing.type: Easing.OutCubic
        }
    }
    //// END OF Behaviors

    //////////////START OF CONNECTIONS
    onEditModeChanged: {
        if (editMode) {
            visibilityManager.updateMaskArea();
            clearZoom();
        } else {
            layoutsContainer.updateSizeForAppletsInFill();
        }

        //! This is used in case the dndspacer has been left behind
        //! e.g. the user drops a folder and a context menu is appearing
        //! but the user decides to not make a choice for the applet type
        if (dndSpacer.parent !== root) {
            dndSpacer.parent = root;
        }
    }

    onInConfigureAppletsModeChanged: {
        if (inConfigureAppletsMode && panelUserSetAlignment===Latte.Types.Justify) {
            joinLayoutsToMainLayout();
        } else if (!inConfigureAppletsMode) {
            splitMainLayoutToLayouts();
        }

        updateIndexes();
    }

    //! It is used only when the user chooses different alignment types
    //! and not during startup
    onPanelUserSetAlignmentChanged: {
        if (!root.editMode) {
            return;
        }

        if (!inConfigureAppletsMode){
            if (panelUserSetAlignment===Latte.Types.Justify) {
                addInternalViewSplitters();
                splitMainLayoutToLayouts();
            } else {
                joinLayoutsToMainLayout();
                root.destroyInternalViewSplitters();
            }
        } else {
            if (panelUserSetAlignment===Latte.Types.Justify) {
                addInternalViewSplitters();
            } else {
                root.destroyInternalViewSplitters();
            }
        }

        LayoutManager.save();
        updateIndexes();
    }

    onLatteViewChanged: {
        if (latteView) {
            latteView.onXChanged.connect(visibilityManager.updateMaskArea);
            latteView.onYChanged.connect(visibilityManager.updateMaskArea);
            latteView.onWidthChanged.connect(visibilityManager.updateMaskArea);
            latteView.onHeightChanged.connect(visibilityManager.updateMaskArea);

            latteView.positioner.hideDockDuringLocationChangeStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
            latteView.positioner.showDockAfterLocationChangeFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
            latteView.positioner.hideDockDuringScreenChangeStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
            latteView.positioner.showDockAfterScreenChangeFinished.connect(visibilityManager.slotShowDockAfterLocationChange);
            latteView.positioner.hideDockDuringMovingToLayoutStarted.connect(visibilityManager.slotHideDockDuringLocationChange);
            latteView.positioner.showDockAfterMovingToLayoutFinished.connect(visibilityManager.slotShowDockAfterLocationChange);


            latteView.visibility.onContainsMouseChanged.connect(visibilityManager.slotContainsMouseChanged);
            latteView.visibility.onMustBeHide.connect(visibilityManager.slotMustBeHide);
            latteView.visibility.onMustBeShown.connect(visibilityManager.slotMustBeShown);

            updateContainsOnlyPlasmaTasks();
        }
    }

    onDockContainsMouseChanged: {
        if (!dockContainsMouse) {
            initializeHoveredIndexes();
        }
    }

    onMaxLengthChanged: {
        layoutsContainer.updateSizeForAppletsInFill();
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
            if (plasmoid.configuration.panelPosition === Latte.Types.Left)
                plasmoid.configuration.panelPosition = Latte.Types.Top;
            else if (plasmoid.configuration.panelPosition === Latte.Types.Right)
                plasmoid.configuration.panelPosition = Latte.Types.Bottom;
        } else {
            if (plasmoid.configuration.panelPosition === Latte.Types.Top)
                plasmoid.configuration.panelPosition = Latte.Types.Left;
            else if (plasmoid.configuration.panelPosition === Latte.Types.Bottom)
                plasmoid.configuration.panelPosition = Latte.Types.Right;
        }
    }

    //  onIconSizeChanged: console.log("Icon Size Changed:"+iconSize);

    Component.onCompleted: {
        //  currentLayout.isLayoutHorizontal = isHorizontal
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = layoutsContainer.mainLayout;
        LayoutManager.layoutS = layoutsContainer.startLayout;
        LayoutManager.layoutE = layoutsContainer.endLayout;
        LayoutManager.lastSpacer = lastSpacer;
        LayoutManager.restore();
        plasmoid.action("configure").visible = !plasmoid.immutable;
        plasmoid.action("configure").enabled = !plasmoid.immutable;

        inStartupTimer.start();
    }

    Component.onDestruction: {
        console.debug("Destroying Latte Dock Containment ui...");

        if (latteView) {
            latteView.onXChanged.disconnect(visibilityManager.updateMaskArea);
            latteView.onYChanged.disconnect(visibilityManager.updateMaskArea);
            latteView.onWidthChanged.disconnect(visibilityManager.updateMaskArea);
            latteView.onHeightChanged.disconnect(visibilityManager.updateMaskArea);

            latteView.positioner.hideDockDuringLocationChangeStarted.disconnect(visibilityManager.slotHideDockDuringLocationChange);
            latteView.positioner.showDockAfterLocationChangeFinished.disconnect(visibilityManager.slotShowDockAfterLocationChange);
            latteView.positioner.hideDockDuringScreenChangeStarted.disconnect(visibilityManager.slotHideDockDuringLocationChange);
            latteView.positioner.showDockAfterScreenChangeFinished.disconnect(visibilityManager.slotShowDockAfterLocationChange);
            latteView.positioner.hideDockDuringMovingToLayoutStarted.disconnect(visibilityManager.slotHideDockDuringLocationChange);
            latteView.positioner.showDockAfterMovingToLayoutFinished.disconnect(visibilityManager.slotShowDockAfterLocationChange);

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

        console.log(applet.pluginName);
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
            latteView.setBlockHiding(true);

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
            latteView.setBlockHiding(false);

            if (latteView.visibility.isHidden) {
                latteView.visibility.mustBeShown();
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
        ///they break the !immutable experience, the latteView becomes too small
        ///to add applets
        /*   if (plasmoid.immutable) {
            if(root.isHorizontal) {
                root.Layout.preferredWidth = (plasmoid.configuration.panelPosition === Latte.Types.Justify ?
                                                  layoutsContainer.width + 0.5*iconMargin : layoutsContainer.mainLayout.width + iconMargin);
            } else {
                root.Layout.preferredHeight = (plasmoid.configuration.panelPosition === Latte.Types.Justify ?
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
                LayoutManager.insertAtIndex(container, pos);
            } else {
                LayoutManager.insertAtIndex(container, Math.floor(layoutsContainer.mainLayout.count / 2));
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

    function clearZoom(){
        if (latteApplet){
            latteApplet.clearZoom();
        }

        root.clearZoomSignal();
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


    function disableDirectRender(){
        //  root.globalDirectRender = false;
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

    function initializeHoveredIndexes() {
        layoutsContainer.hoveredIndex = -1;
        layoutsContainer.currentSpot = -1000;
        if (latteApplet) {
            latteApplet.initializeHoveredIndex();
        }
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
        if (latteApplet && latteApplet.containsMouse())
            return true;

        var applets = layoutsContainer.startLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.originalAppletBehavior && applet.canBeHovered){
                return true;
            }
        }

        applets = layoutsContainer.mainLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.originalAppletBehavior && applet.canBeHovered){
                return true;
            }
        }

        ///check second layout also
        applets = layoutsContainer.endLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.originalAppletBehavior && applet.canBeHovered){
                return true;
            }
        }

        return false;
    }

    function mouseInHoverableArea() {
        return (latteView.visibility.containsMouse && !rootMouseArea.containsMouse && mouseInCanBeHoveredApplet());
    }

    function setHoveredIndex(ind) {
        layoutsContainer.hoveredIndex = ind;
    }

    function hideTooltipLabel(debug){
        titleTooltipDialog.hide(debug);
    }

    //! this is called from globalshortcuts c++ side
    function setShowAppletShortcutBadges(showLatteShortcuts, showShortcuts, showMeta, applicationLauncher){
        if (latteApplet) {
            var base = unifiedGlobalShortcuts ? parabolicManager.pseudoAppletIndex(latteAppletPos) : 1;
            latteApplet.setTasksBaseIndex(base - 1);
            latteApplet.setShowTaskShortcutBadges(showLatteShortcuts);
        }

        showLatteShortcutBadges = showLatteShortcuts;
        showAppletShortcutBadges = showShortcuts;
        showMetaBadge = showMeta;
        applicationLauncherId = applicationLauncher;

        if (latteApplet) {
            latteApplet.parabolicManager.updateTasksEdgesIndexes();
        }
    }

    //! this is called from Latte::View::ContainmentInterface
    function activateEntryAtIndex(index) {
        if (typeof index !== "number") {
            return;
        }

        if (latteApplet) {
            var base = unifiedGlobalShortcuts ? parabolicManager.pseudoAppletIndex(latteAppletPos) : 1;
            latteApplet.setTasksBaseIndex(base - 1);
            latteApplet.parabolicManager.updateTasksEdgesIndexes();
        }

        signalActivateEntryAtIndex(index);
    }

    //! this is called from Latte::View::ContainmentInterface
    function newInstanceForEntryAtIndex(index) {
        if (typeof index !== "number") {
            return;
        }

        if (latteApplet) {
            var base = unifiedGlobalShortcuts ? parabolicManager.pseudoAppletIndex(latteAppletPos) : 1;
            latteApplet.setTasksBaseIndex(base - 1);
            latteApplet.parabolicManager.updateTasksEdgesIndexes();
        }

        signalNewInstanceForEntryAtIndex(index);
    }

    //! this is called from Latte::View::ContainmentInterface
    function appletIdForIndex(index) {
        if (!root.unifiedGlobalShortcuts || parabolicManager.pseudoIndexBelongsToLatteApplet(index)) {
            return -1;
        }

        for (var i=0; i<layoutsContainer.startLayout.children.length; ++i){
            var appletItem = layoutsContainer.startLayout.children[i];

            if (appletItem && appletItem.refersEntryIndex(index)) {
                return appletItem.applet.id;
            }
        }

        for (var j=0; j<layoutsContainer.mainLayout.children.length; ++j){
            var appletItem2 = layoutsContainer.mainLayout.children[j];

            if (appletItem2 && appletItem2.refersEntryIndex(index)) {
                return appletItem2.applet.id;
            }
        }

        for (var k=0; j<layoutsContainer.endLayout.children.length; ++k){
            var appletItem3 = layoutsContainer.endLayout.children[k];

            if (appletItem3 && appletItem3.refersEntryIndex(index)) {
                return appletItem3.applet.id;
            }
        }

        return -1;
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

    function slotAppletsNeedWindowsTracking(step) {
        if (step === 0) {
            return;
        }

        appletsNeedWindowsTracking = Math.max(appletsNeedWindowsTracking + step, 0);
    }

    //this is used when dragging a task in order to not hide the dock
    //and also by the menu appearing from tasks for the same reason
    function slotActionsBlockHiding(step) {
        //if (root.editMode) {
        //    return;
        // }

        if ((step === 0) || (!latteView)) {
            return;
        }

        actionsBlockHiding = Math.max(actionsBlockHiding + step, 0);

        if (actionsBlockHiding > 0){
            latteView.setBlockHiding(true);
        } else {
            if (!root.editMode)
                latteView.setBlockHiding(false);
        }
    }

    function slotPreviewsShown(){
        if (latteView) {
            latteView.deactivateApplets();
        }
    }

    function startCheckRestoreZoomTimer(){
        checkRestoreZoom.start();
    }

    function stopCheckRestoreZoomTimer(){
        checkRestoreZoom.stop();
    }

    function startDirectRenderDelayerDuringEntering(){
        directRenderDelayerForEnteringTimer.start();
    }

    function setGlobalDirectRender(value) {
        if (latteApplet && latteApplet.tasksExtendedManager.waitingLaunchersLength() > 0)
            return;

        if (value === true) {
            if (mouseInCanBeHoveredApplet()) {
                root.globalDirectRender = true;
            } else {
                //    console.log("direct render true ignored...");
            }
        } else {
            root.globalDirectRender = false;
        }
    }

    function updateContainsOnlyPlasmaTasks() {
        if (latteView) {
            root.containsOnlyPlasmaTasks = (latteView.tasksPresent() && !latteApplet);
        } else {
            root.containsOnlyPlasmaTasks = false;
        }
    }

    function updateSizeForAppletsInFill() {
        layoutsContainer.updateSizeForAppletsInFill();
    }

    function splitMainLayoutToLayouts() {
        if (internalViewSplittersCount() === 2) {
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

    function joinLayoutsToMainLayout() {
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
        target: latteView

        onContextMenuIsShownChanged: {
            if (!latteView.contextMenuIsShown) {
                checkRestoreZoom.start();
            } else {
                root.setGlobalDirectRender(false);
            }
        }
    }

    Connections{
        target: latteView && latteView.visibility ? latteView.visibility : root

        ignoreUnknownSignals : true

        onContainsMouseChanged: {
            if (mouseInHoverableArea()) {
                stopCheckRestoreZoomTimer();
            } else {
                startCheckRestoreZoomTimer();
            }
        }
    }

    Connections{
        target: layoutsContainer

        onHoveredIndexChanged: {
            if (latteApplet && layoutsContainer.hoveredIndex>-1){
                latteApplet.setHoveredIndex(-1);
            }

            if (latteApplet && latteApplet.windowPreviewIsShown && layoutsContainer.hoveredIndex>-1) {
                latteApplet.hidePreview();
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
        Applet.AppletItem{}
    }

    ParabolicManager{
        id: _parabolicManager
    }

    Indicators.Manager{
        id: indicators
    }

    Item {
        id: graphicsSystem
        readonly property bool isAccelerated: (GraphicsInfo.api !== GraphicsInfo.Software)
                                              && (GraphicsInfo.api !== GraphicsInfo.Unknown)
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
        sourceComponent: DebugWindow{}
    }

    //! Load a sepia background in order to avoid black background
    Loader{
        anchors.fill: parent
        active: !Latte.WindowSystem.compositingActive
        sourceComponent: Image{
            anchors.fill: parent
            fillMode: Image.Tile
            source: root.hasUserSpecifiedBackground ? latteView.layout.background : "../icons/wheatprint.jpg"
        }
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

    AutomaticItemSizer {
        id: automaticItemSizer
    }

    VisibilityManager{ id: visibilityManager }

    DragDropArea {
        id: backDropArea
        anchors.fill: parent
        readonly property bool higherPriority: latteView && latteView.containsDrag
                                               && ((root.dragInfo.isPlasmoid && root.dragInfo.isSeparator)
                                                   || (foreDropArea.dragInfo.computationsAreValid && !root.dragInfo.isPlasmoid && !root.dragInfo.onlyLaunchers))

        Item{
            id: panelBox
            anchors.fill: layoutsContainer

            PanelBox{
                id: panelBoxBackground
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

        readonly property int length: root.iconSize + root.lengthMargins
        readonly property int thickness: root.iconSize + root.thickMargins + root.localScreenEdgeMargin

        Layout.preferredWidth: width
        Layout.preferredHeight: height
        opacity: 0
        z:1500

        LatteComponents.AddItem{
            id: dndSpacerAddItem
            width: root.isHorizontal ? parent.width : parent.width - root.localScreenEdgeMargin
            height: root.isHorizontal ? parent.height - root.localScreenEdgeMargin : parent.height

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
                        anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: root.localScreenEdgeMargin;
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
                        anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin: root.localScreenEdgeMargin;    anchors.bottomMargin: 0;
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
                        anchors.leftMargin: root.localScreenEdgeMargin;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: 0;
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
                        anchors.leftMargin: 0;    anchors.rightMargin: root.localScreenEdgeMargin;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                }
            ]
        }
    }

    ///////////////END UI elements

    ///////////////BEGIN TIMER elements


    //Timer to check if the mouse is still outside the latteView in order to restore zooms to 1.0
    Timer{
        id:checkRestoreZoom
        interval: 90

        onTriggered: {
            if (latteApplet && (latteApplet.previewContainsMouse() || latteApplet.contextMenu))
                return;

            if (latteView.contextMenuIsShown)
                return;

            if (!mouseInHoverableArea()) {
                setGlobalDirectRender(false);
                root.initializeHoveredIndexes();
                root.clearZoom();
            }

            if (root.debugModeTimers) {
                console.log("containment timer: checkRestoreZoom called...");
            }
        }
    }

    //! Delayer in order to not activate directRendering when the mouse
    //! enters until the timer has ended. This way we make sure that the
    //! zoom-in animations will have ended.
    Timer{
        id:directRenderDelayerForEnteringTimer
        interval: 3.2 * root.durationTime * units.shortDuration
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
}
