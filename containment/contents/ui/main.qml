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

import "LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root
    objectName: "dockLayoutView"

    //// BEGIN SIGNALS
    signal clearZoomSignal();
    signal updateEffectsArea();
    signal updateIndexes();
    ////

    ////BEGIN properties
    property bool debugMode: Qt.application.arguments.indexOf("--graphics")>=0

    property bool globalDirectRender: false //it is used to check both the applet and the containment for direct render

    property bool addLaunchersMessage: false
    property bool addLaunchersInTaskManager: plasmoid.configuration.addLaunchersInTaskManager
    property bool autoDecreaseIconSize: plasmoid.configuration.autoDecreaseIconSize
    property bool blurEnabled: plasmoid.configuration.blurEnabled
    property bool confirmedDragEntered: false
    property bool drawShadowsExternal: visibilityManager.panelIsBiggerFromIconSize && (zoomFactor === 1.0)
                                       && (dock.visibility.mode === Latte.Dock.AlwaysVisible)
                                       && (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && !root.solidPanel

    property bool editMode: plasmoid.userConfiguring
    property bool exposeAltSession: globalSettings ? globalSettings.exposeAltSession : false

    property bool immutable: plasmoid.immutable
    property bool inStartup: true
    property bool isHorizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    property bool isVertical: !isHorizontal
    property bool isHovered: latteApplet ? ((latteAppletHoveredIndex !== -1) && (layoutsContainer.hoveredIndex !== -1)) //|| wholeArea.containsMouse
                                         : (layoutsContainer.hoveredIndex !== -1) //|| wholeArea.containsMouse
    property bool normalState : false
    property bool onlyAddingStarup: true //is used for the initialization phase in startup where there arent removals, this variable provides a way to grow icon size
    property bool shrinkThickMargins: plasmoid.configuration.shrinkThickMargins
    property bool solidPanel: Latte.WindowSystem.compositingActive ? plasmoid.configuration.solidPanel : true
    //FIXME: possibly this is going to be the default behavior, this user choice
    //has been dropped from the Dock Configuration Window
    //property bool smallAutomaticIconJumps: plasmoid.configuration.smallAutomaticIconJumps
    property bool smallAutomaticIconJumps: true
    property bool useThemePanel: noApplets === 0 ? true : plasmoid.configuration.useThemePanel

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
    property int totalPanelEdgeSpacing: 0 //this is set by PanelBox
    //FIXME: this is not needed any more probably
    property int previousAllTasks: -1    //is used to forbit updateAutomaticIconSize when hovering
    property int realSize: iconSize + iconMargin
    property int realPanelSize: 0
    property int realPanelLength: 0
    property int realPanelThickness: 0
    //this is set by the PanelBox
    property int panelMargin: 0
    property int panelMarginLength: 0
    property int panelShadow: 0 //shadowsSize
    property int editShadow: Math.ceil(iconSize / 5)
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
    property int thickMarginBase: shrinkThickMargins ? 1 : Math.ceil(0.06 * iconSize)
    property int thickMarginHigh: shrinkThickMargins ? 1 : Math.ceil(0.06 * iconSize)
    property int thickMargin: thickMarginBase + thickMarginHigh

    //it is used in order to not break the calculations for the thickness placement
    //especially in automatic icon sizes calculations
    property int thickMarginOriginal: Math.ceil(0.12 * maxIconSize)

    //! iconMargin from configuration is a percentage. The calculation provides a length
    //! for that value between 0.12 - 0.5 of iconSize, this way 100% iconMargin means
    //! equal to the iconSize
    property int iconMargin: Math.ceil( (0.12 + (0.38 * (plasmoid.configuration.iconMargin)/100)) * iconSize)
    property int statesLineSize: latteApplet ?  Math.ceil( root.iconSize/13 ) : 0


    ///FIXME: <delete both> I can't remember why this is needed, maybe for the anchorings!!! In order for the Double Layout to not mess the anchorings...
    //property int mainLayoutPosition: !plasmoid.immutable ? Latte.Dock.Center : (root.isVertical ? Latte.Dock.Top : Latte.Dock.Left)
    //property int panelAlignment: plasmoid.configuration.panelPosition !== Latte.Dock.Justify ? plasmoid.configuration.panelPosition : mainLayoutPosition

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
    property QtObject dock
    property QtObject globalSettings

    property QtObject altSessionAction: globalSettings ? globalSettings.altSessionAction : 0

    // TO BE DELETED, if not needed: property int counter:0;

    ///BEGIN properties provided to Latte Plasmoid
    property bool directRender: layoutsContainer.directRender

    property bool enableShadows: plasmoid.configuration.shadows
    property bool dockIsHidden: dock ? dock.visibility.isHidden : true
    property bool dotsOnActive: plasmoid.configuration.dotsOnActive
    property bool highlightWindows: plasmoid.configuration.highlightWindows
    property bool reverseLinesPosition: plasmoid.configuration.reverseLinesPosition// latteApplet ? latteApplet.reverseLinesPosition : false
    property bool showGlow: plasmoid.configuration.showGlow
    property bool showToolTips: plasmoid.configuration.showToolTips
    property bool showWindowActions: plasmoid.configuration.showWindowActions
    property bool showOnlyCurrentScreen: plasmoid.configuration.showOnlyCurrentScreen
    property bool showOnlyCurrentDesktop: plasmoid.configuration.showOnlyCurrentDesktop
    property bool showOnlyCurrentActivity: plasmoid.configuration.showOnlyCurrentActivity

    property bool smartLaunchersEnabled: plasmoid.configuration.smartLaunchersEnabled
    property bool threeColorsWindows: plasmoid.configuration.threeColorsWindows

    property int durationTime: plasmoid.configuration.durationTime
    property int latteAppletHoveredIndex: latteApplet ? latteApplet.hoveredIndex : -1
    property int tasksCount: latteApplet ? latteApplet.tasksCount : 0

    property rect screenGeometry: dock ? dock.screenGeometry : plasmoid.screenGeometry
    ///END properties from latteApplet

    /* Layout.preferredWidth: plasmoid.immutable ?
                               (plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                    layoutsContainer.width + 0.5*iconMargin : mainLayout.width + iconMargin) :
                               Screen.width //on unlocked state use the maximum

    Layout.preferredHeight: plasmoid.immutable ?
                               (plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                    layoutsContainer.height + 0.5*iconMargin : mainLayout.height + iconMargin) :
                               Screen.height //on unlocked state use the maximum*/

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    //// BEGIN properties in functions
    property int noApplets: {
        var count1 = 0;
        var count2 = 0;

        count1 = mainLayout.children.length;
        var tempLength = mainLayout.children.length;

        for (var i=tempLength-1; i>=0; --i) {
            var applet = mainLayout.children[i];
            if (applet && (applet === dndSpacer || applet === lastSpacer ||  applet.isInternalViewSplitter))
                count1--;
        }

        count2 = endLayout.children.length;
        tempLength = endLayout.children.length;

        for (var i=tempLength-1; i>=0; --i) {
            var applet = endLayout.children[i];
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

    //////////////////////////BEGIN states
    //user set Panel Positions
    // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
    states: [
        ///Left Edge
        State {
            name: "leftCenter"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&((root.panelAlignment === Latte.Dock.Center)||(root.panelAlignment === Latte.Dock.Justify))

            AnchorChanges {
                target: mainLayout
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        State {
            name: "leftTop"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Top)

            AnchorChanges {
                target: mainLayout
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        State {
            name: "leftBottom"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Bottom)

            AnchorChanges {
                target: mainLayout
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        ///Right Edge
        State {
            name: "rightCenter"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&((root.panelAlignment === Latte.Dock.Center)||(root.panelAlignment === Latte.Dock.Justify))

            AnchorChanges {
                target: mainLayout
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        State {
            name: "rightTop"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Top)

            AnchorChanges {
                target: mainLayout
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        State {
            name: "rightBottom"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Bottom)

            AnchorChanges {
                target: mainLayout
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        ///Bottom Edge
        State {
            name: "bottomCenter"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&((root.panelAlignment === Latte.Dock.Center)||(root.panelAlignment === Latte.Dock.Justify))

            AnchorChanges {
                target: mainLayout
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        State {
            name: "bottomLeft"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Left)

            AnchorChanges {
                target: mainLayout
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        State {
            name: "bottomRight"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Right)

            AnchorChanges {
                target: mainLayout
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        ///Top Edge
        State {
            name: "topCenter"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&((root.panelAlignment === Latte.Dock.Center)||(root.panelAlignment === Latte.Dock.Justify))

            AnchorChanges {
                target: mainLayout
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        State {
            name: "topLeft"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Left)

            AnchorChanges {
                target: mainLayout
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        },
        State {
            name: "topRight"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Right)

            AnchorChanges {
                target: mainLayout
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
            PropertyChanges{
                target: mainLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
            }
        }
    ]
    ////////////////END states


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
    Connections {
        target: latteApplet
        onDirectRenderChanged: {
            root.globalDirectRender = latteApplet.directRender || layoutsContainer.directRender;
        }
    }

    Connections {
        target: layoutsContainer
        onDirectRenderChanged: {
            root.globalDirectRender = latteApplet ? latteApplet.directRender || layoutsContainer.directRender : layoutsContainer.directRender;
        }
    }

    onEditModeChanged: {
        if (editMode) {
            visibilityManager.updateMaskArea();
        } else {
            updateAutomaticIconSize();
        }

        updateLayouts();
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

        //var relevantLayout = mainLayout.mapFromItem(root, event.x, event.y);
        //LayoutManager.insertAtCoordinates2(dndSpacer, relevantLayout.x, relevantLayout.y)
        LayoutManager.insertAtCoordinates2(dndSpacer, event.x, event.y)
        dndSpacer.opacity = 1;
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

        //var relevantLayout = mainLayout.mapFromItem(root, event.x, event.y);
        //LayoutManager.insertAtCoordinates2(dndSpacer, relevantLayout.x, relevantLayout.y)
        LayoutManager.insertAtCoordinates2(dndSpacer, event.x, event.y)
        dndSpacer.opacity = 1;
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
        //var relevantLayout = mainLayout.mapFromItem(root, event.x, event.y);
        //plasmoid.processMimeData(event.mimeData, relevantLayout.x, relevantLayout.y);
        //launchersDropped
        if (event.mimeData.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") < 0) {
            if (latteApplet && latteApplet.launchersDrop(event) && root.addLaunchersInTaskManager) {
                latteApplet.launchersDropped(event.mimeData.urls);
            } else {
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
        //  dndSpacer.parent = root;
    }

    onLatteAppletChanged: {
        if (latteApplet) {
            latteApplet.signalAnimationsNeedBothAxis.connect(slotAnimationsNeedBothAxis);
            latteApplet.signalAnimationsNeedLength.connect(slotAnimationsNeedLength);
            latteApplet.signalAnimationsNeedThickness.connect(slotAnimationsNeedThickness);
            latteApplet.signalActionsBlockHiding.connect(slotActionsBlockHiding);
        }
    }

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
        LayoutManager.layout = mainLayout;
        LayoutManager.layoutS = startLayout;
        layoutManager.layoutE = endLayout;
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
    }

    Containment.onAppletAdded: {
        addApplet(applet, x, y);
        LayoutManager.save();
        updateIndexes();
    }

    Containment.onAppletRemoved: {
        LayoutManager.removeApplet(applet);
        var flexibleFound = false;
        for (var i = 0; i < mainLayout.children.length; ++i) {
            var applet = mainLayout.children[i].applet;
            if (applet && ((root.isHorizontal && applet.Layout.fillWidth) ||
                           (!root.isHorizontal && applet.Layout.fillHeight)) &&
                    applet.visible) {
                flexibleFound = true;
                break
            }
        }
        if (!flexibleFound) {
            lastSpacer.parent = mainLayout;
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
            dock.visibility.blockHiding = true;

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
            dock.visibility.blockHiding = false;

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
                                                  layoutsContainer.width + 0.5*iconMargin : mainLayout.width + iconMargin);
            } else {
                root.Layout.preferredHeight = (plasmoid.configuration.panelPosition === Latte.Dock.Justify ?
                                                   layoutsContainer.height + 0.5*iconMargin : mainLayout.height + iconMargin);
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
    }

    function addContainerInLayout(container, applet, x, y){
        // Is there a DND placeholder? Replace it!
        if ( (dndSpacer.parent === mainLayout)
                || (dndSpacer.parent === startLayout)
                || (dndSpacer.parent===endLayout)) {
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

            if (lastSpacer.parent === mainLayout) {
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
                var middle = mainLayout.childAt(root.width / 2, root.height / 2);

                if (middle) {
                    before = middle;
                }

                // Otherwise if lastSpacer is here, enqueue before it.
            }

            if (before) {
                LayoutManager.insertBefore(before, container);

                // Fall through to adding at the end.
            } else {
                container.parent = mainLayout;
            }
        }

        //Important, removes the first children of the mainLayout after the first
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
                layoutManager.insertAtIndex(container, Math.floor(mainLayout.count / 2));
            }

            layoutManager.save();
        }
    }

    function checkLastSpacer() {
        lastSpacer.parent = root

        var expands = false;

        if (isHorizontal) {
            for (var container in mainLayout.children) {
                var item = mainLayout.children[container];
                if (item.Layout && item.Layout.fillWidth) {
                    expands = true;
                }
            }
        } else {
            for (var container in mainLayout.children) {
                var item = mainLayout.children[container];
                if (item.Layout && item.Layout.fillHeight) {
                    expands = true;
                }
            }
        }
        if (!expands) {
            lastSpacer.parent = mainLayout
        }
    }

    function clearZoom(){
        if (enableDirectRenderTimer.running)
            enableDirectRenderTimer.stop();

        layoutsContainer.directRender = false;
        layoutsContainer.currentSpot = -1000;
        layoutsContainer.hoveredIndex = -1;
        root.clearZoomSignal();
    }

    function containmentActions(){
        return dock.containmentActions();
    }

    function containsMouse(){
        var result = root.outsideContainsMouse();

        if(result)
            return true;

        if(!result && latteApplet && latteApplet.outsideContainsMouse()){
            layoutsContainer.hoveredIndex = latteAppletContainer.index;
            return true;
        }

        if (latteApplet){
            latteApplet.clearZoom();
        }

        return false;
    }

    function internalViewSplittersCount(){
        var splitters = 0;
        for (var container in startLayout.children) {
            var item = startLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters++;
            }
        }

        for (var container in mainLayout.children) {
            var item = mainLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters++;
            }
        }

        for (var container in endLayout.children) {
            var item = endLayout.children[container];
            if(item && item.isInternalViewSplitter) {
                splitters++;
            }
        }

        return splitters;
    }

    function outsideContainsMouse(){
        var applets = startLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.lockZoom && applet.canBeHovered){
                return true;
            }
        }

        applets = mainLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.lockZoom && applet.canBeHovered){
                return true;
            }
        }

        ///check second layout also
        applets = endLayout.children;

        for(var i=0; i<applets.length; ++i){
            var applet = applets[i];

            if(applet && applet.containsMouse && !applet.lockZoom && applet.canBeHovered){
                return true;
            }
        }

        return false;
    }

    function removeInternalViewSplitters(){
        for (var container in mainLayout.children) {
            var item = mainLayout.children[container];
            if(item && item.isInternalViewSplitter)
                item.destroy();
        }

        layoutManager.save();
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
            dock.visibility.blockHiding = true;
        } else {
            if (!root.editMode)
                dock.visibility.blockHiding = false;
        }
    }

    function updateAutomaticIconSize() {
        if ((visibilityManager.normalState && !root.editMode && root.autoDecreaseIconSize)
                && (iconSize===root.maxIconSize || iconSize === automaticIconSizeBasedSize) ) {
            var layoutLength;
            var maxLength = root.maxLength;

            //console.log("------Entered check-----");
            //console.log("max length: "+ maxLength);

            if (root.isVertical) {
                layoutLength = (plasmoid.configuration.panelPosition === Latte.Dock.Justify) ?
                            startLayout.height+mainLayout.height+endLayout.height : mainLayout.height
            } else {
                layoutLength = (plasmoid.configuration.panelPosition === Latte.Dock.Justify) ?
                            startLayout.width+mainLayout.width+endLayout.width : mainLayout.width
            }

            var toShrinkLimit = maxLength-(root.zoomFactor*(iconSize+2*iconMargin));
            var toGrowLimit = maxLength-1.5*(root.zoomFactor*(iconSize+2*iconMargin));

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

    function updateLayouts(){
        if(!root.editMode){
            //    console.log("update layout - internal view splitters count:"+internalViewSplittersCount());
            if (internalViewSplittersCount() === 2) {
                var splitter = -1;
                var splitter2 = -1;

                var totalChildren = mainLayout.children.length;
                for (var i=0; i<totalChildren; ++i) {
                    var item = mainLayout.children[i];

                    if(item.isInternalViewSplitter && splitter === -1) {
                        splitter = i;
                    } else if (item.isInternalViewSplitter && splitter>=0 && splitter2 === -1) {
                        splitter2 = i;
                    }
                }

                // console.log("update layouts 1:"+splitter + " - "+splitter2);
                for (var i=0; i<=splitter; ++i){
                    var item = mainLayout.children[0];
                    item.parent = startLayout;
                }

                splitter2 = splitter2 - splitter - 1;
                // console.log("update layouts 2:"+splitter + " - "+splitter2);

                totalChildren = mainLayout.children.length;
                for (var i=splitter2+1; i<totalChildren; ++i){
                    var item = mainLayout.children[splitter2+1];
                    item.parent = endLayout;
                }
            }
        }
        else{
            if (internalViewSplittersCount() === 2) {
                var totalChildren1 = mainLayout.children.length;
                for (var i=totalChildren1-1; i>=0; --i) {
                    var item1 = mainLayout.children[0];
                    item1.parent = startLayout;
                }

                var totalChildren2 = endLayout.children.length;

                for (var i=totalChildren2-1; i>=0; --i) {
                    var item2 = endLayout.children[0];
                    item2.parent = startLayout;
                }

                var totalChildrenL = startLayout.children.length;
                for (var i=totalChildrenL-1; i>=0; --i) {
                    var itemL = startLayout.children[0];
                    itemL.parent = mainLayout;
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
    }


    ////END interfaces

    ///////////////BEGIN components
    Component {
        id: appletContainerComponent
        AppletItem{}
    }
    ///////////////END components

    ///////////////BEGIN UI elements

    Loader{
        active: Qt.application.arguments.indexOf("--with-window") >= 0
        sourceComponent: DebugWindow{}
    }

    //! Load a sepia background in order to avoid black background
    Loader{
        anchors.fill: parent
        active: !Latte.WindowSystem.compositingActive
        sourceComponent: Image{
            anchors.fill: parent
            fillMode: Image.Tile
            source: "../icons/sepiaprint.jpg"
        }
    }


    EditModeVisual{
        id:editModeVisual
        z: root.drawShadowsExternal ? 1 : 0
    }

    Item{
        anchors.fill:layoutsContainer
        z: root.drawShadowsExternal ? 0 : 1

        PanelBox{}
    }

    Item {
        id: lastSpacer
        parent: mainLayout

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

    VisibilityManager{
        id: visibilityManager
    }

    Item{
        id: layoutsContainer

        signal updateScale(int delegateIndex, real newScale, real step)

        property int allCount: root.latteApplet ? mainLayout.count-1+latteApplet.tasksCount : mainLayout.count
        property int currentSpot: -1000
        property int hoveredIndex: -1

        property bool directRender: false

        x: {
            if ( dock && (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && root.isHorizontal
                    && !root.editMode && !root.drawShadowsExternal ){
                return ((dock.width/2) - (root.maxLength/2))
            } else {
                if ((visibilityManager.inSlidingIn || visibilityManager.inSlidingOut) && root.isVertical){
                    return;
                }

                if (dock.visibility.isHidden && root.isVertical) {
                    if (Latte.WindowSystem.compositingActive) {
                        return slidingOutToPos;
                    } else {
                        if ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) {
                            return slidingOutToPos + 1;
                        } else {
                            return slidingOutToPos - 1;
                        }
                    }
                } else {
                    return 0;
                }
            }
        }

        y: {
            if ( dock && (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && root.isVertical
                    && !root.editMode && !root.drawShadowsExternal ) {
                return ((dock.height/2) - (root.maxLength/2));
            } else {
                if ((visibilityManager.inSlidingIn || visibilityManager.inSlidingOut) && root.isHorizontal){
                    return;
                }

                if (dock.visibility.isHidden && root.isHorizontal) {
                    if (Latte.WindowSystem.compositingActive) {
                        return slidingOutToPos;
                    } else {
                        if ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) {
                            return slidingOutToPos + 1;
                        } else {
                            return slidingOutToPos - 1;
                        }
                    }
                } else {
                    return 0;
                }
            }
        }

        width: (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && root.isHorizontal && !root.editMode && !root.drawShadowsExternal ?
                   root.maxLength : parent.width
        height: (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && root.isVertical && !root.editMode && !root.drawShadowsExternal ?
                    root.maxLength : parent.height
        z:10

        property bool animationSent: false
        property bool shouldCheckHalfs: (plasmoid.configuration.panelPosition === Latte.Dock.Justify) && (mainLayout.children>1)

        property int contentsWidth: startLayout.width + mainLayout.width + endLayout.width
        property int contentsHeight: startLayout.height + mainLayout.height + endLayout.height

        onContentsWidthChanged: {
            if (root.isHorizontal){
                var firstHalfExited = false;
                var secondHalfExited = false;

                if (shouldCheckHalfs){
                    firstHalfExited = ( (startLayout.width + mainLayout.width/2) >= root.maxLength/2 );
                    secondHalfExited = ( (endLayout.width + mainLayout.width/2) >= root.maxLength/2 );
                }

                if (dock && ((contentsWidth >= root.maxLength) || firstHalfExited || secondHalfExited)) {
                    updateAutomaticIconSize();
                }

                if (!animationSent) {
                    animationSent = true;
                    slotAnimationsNeedLength(1);
                }

                delayUpdateMaskArea.start();
            }
        }

        onContentsHeightChanged: {
            if (root.isVertical){
                var firstHalfExited = false;
                var secondHalfExited = false;

                if (shouldCheckHalfs){
                    firstHalfExited = ( (startLayout.height + mainLayout.height/2) >= root.maxLength/2 );
                    secondHalfExited = ( (endLayout.height + mainLayout.height/2) >= root.maxLength/2 );
                }

                if (dock && ((contentsHeight >= root.maxLength) || firstHalfExited || secondHalfExited)) {
                    updateAutomaticIconSize();
                }

                if (!animationSent) {
                    animationSent = true;
                    slotAnimationsNeedLength(1);
                }

                delayUpdateMaskArea.start();
            }
        }

        onXChanged: root.updateEffectsArea();
        onYChanged: root.updateEffectsArea();

        Grid{
            id:startLayout

            columns: root.isVertical ? 1 : 0
            columnSpacing: 0
            flow: isHorizontal ? Grid.LeftToRight : Grid.TopToBottom
            rows: root.isHorizontal ? 1 : 0
            rowSpacing: 0

            Layout.preferredWidth: width
            Layout.preferredHeight: height

            property int beginIndex: 0
            property int count: children.length

            states:[
                State {
                    name: "bottom"
                    when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                    AnchorChanges {
                        target: startLayout
                        anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                    }
                    PropertyChanges{
                        target: startLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom;
                        anchors.leftMargin: panelEdgeSpacing/2;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    }
                },
                State {
                    name: "left"
                    when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                    AnchorChanges {
                        target: startLayout
                        anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                    }
                    PropertyChanges{
                        target: startLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                        anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:panelEdgeSpacing/2;    anchors.bottomMargin:0;
                    }
                },
                State {
                    name: "right"
                    when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                    AnchorChanges {
                        target: startLayout
                        anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                    }
                    PropertyChanges{
                        target: startLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                        anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:panelEdgeSpacing/2;    anchors.bottomMargin:0;
                    }
                },
                State {
                    name: "top"
                    when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                    AnchorChanges {
                        target: startLayout
                        anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                    }
                    PropertyChanges{
                        target: startLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop;
                        anchors.leftMargin: panelEdgeSpacing/2;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    }
                }
            ]
        }

        // This is the main Layout, in contrary with the others
        Grid{
            id: mainLayout

            columns: root.isVertical ? 1 : 0
            columnSpacing: 0
            flow: isHorizontal ? Grid.LeftToRight : Grid.TopToBottom
            rows: root.isHorizontal ? 1 : 0
            rowSpacing: 0

            Layout.preferredWidth: width
            Layout.preferredHeight: height

            property int beginIndex: 100
            property int count: children.length
        }

        Grid{
            id:endLayout

            columns: root.isVertical ? 1 : 0
            columnSpacing: 0
            flow: isHorizontal ? Grid.LeftToRight : Grid.TopToBottom
            rows: root.isHorizontal ? 1 : 0
            rowSpacing: 0


            Layout.preferredWidth: width
            Layout.preferredHeight: height

            property int beginIndex: 200
            property int count: children.length

            states:[
                State {
                    name: "bottom"
                    when: (plasmoid.location === PlasmaCore.Types.BottomEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                    AnchorChanges {
                        target: endLayout
                        anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                    }
                    PropertyChanges{
                        target: endLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignBottom
                        anchors.leftMargin: 0;    anchors.rightMargin:panelEdgeSpacing/2;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    }
                },
                State {
                    name: "left"
                    when: (plasmoid.location === PlasmaCore.Types.LeftEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                    AnchorChanges {
                        target: endLayout
                        anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
                    }
                    PropertyChanges{
                        target: endLayout; horizontalItemAlignment: Grid.AlignLeft; verticalItemAlignment: Grid.AlignVCenter;
                        anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:panelEdgeSpacing/2;
                    }
                },
                State {
                    name: "right"
                    when: (plasmoid.location === PlasmaCore.Types.RightEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                    AnchorChanges {
                        target: endLayout
                        anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                    }
                    PropertyChanges{
                        target: endLayout; horizontalItemAlignment: Grid.AlignRight; verticalItemAlignment: Grid.AlignVCenter;
                        anchors.leftMargin: 0;    anchors.rightMargin:0;     anchors.topMargin:0;    anchors.bottomMargin:panelEdgeSpacing/2;
                    }
                },
                State {
                    name: "top"
                    when: (plasmoid.location === PlasmaCore.Types.TopEdge)&&(root.panelAlignment === Latte.Dock.Justify)

                    AnchorChanges {
                        target: endLayout
                        anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
                    }
                    PropertyChanges{
                        target: endLayout; horizontalItemAlignment: Grid.AlignHCenter; verticalItemAlignment: Grid.AlignTop
                        anchors.leftMargin: 0;    anchors.rightMargin:panelEdgeSpacing/2;     anchors.topMargin:0;    anchors.bottomMargin:0;
                    }
                }
            ]
        }
    }

    ///Buffers to paint areas outside the maskArea///
    ///this is an approach to try to solve #132 issue///
    Loader{
        anchors.fill: parent
        active: dock & !drawShadowsExternal && Latte.WindowSystem.compositingActive
        z: 1000

        sourceComponent: Item{
            anchors.fill: parent

            Rectangle{
                x:0
                y:0
                width: root.width
                height: dock.maskArea.y - 1

                color: "red"
            }

            Rectangle{
                x: 0
                y: dock.maskArea.y
                width: dock.maskArea.x - 1
                height: dock.maskArea.height

                color: "red"
            }

            Rectangle{
                x: dock.maskArea.x + dock.maskArea.width + 1
                y: dock.maskArea.y
                width: dock.width - x
                height: dock.maskArea.height

                color: "red"
            }

            Rectangle{
                x: 0
                y: dock.maskArea.y + dock.maskArea.height + 1
                width: root.width
                height: root.height - y

                color: "red"
            }
        }
    }

    ///Buffers///


    ///////////////END UI elements

    ///////////////BEGIN TIMER elements

    //Timer to check if the mouse is still inside the ListView
    Timer{
        id:checkListHovered
        repeat:false;
        interval: 150;

        onTriggered: {
            if(!root.containsMouse()) {
                root.clearZoom();
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
        }
    }

    //this timer adds a delay into enabling direct rendering...
    //it gives the time to neighbour tasks to complete their animation
    //during first hovering phase
    Timer {
        id: enableDirectRenderTimer
        interval: 4 * root.durationTime * units.shortDuration
        onTriggered: layoutsContainer.directRender = true;
    }

    ///////////////END TIMER elements
}
