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

import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

import "../../controls" as LatteExtraControls

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    ColumnLayout {
        id: content

        width: (dialog.appliedWidth - units.smallSpacing * 2) - Layout.leftMargin * 2
        spacing: dialog.subGroupSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.leftMargin: units.smallSpacing * 2

        //! BEGIN: Inline Dock/Panel Type, it is used only when the secondary window
        //! overlaps the main dock config window
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.topMargin: units.smallSpacing

            visible: dialog.highLevel && viewConfig.showInlineProperties

            LatteComponents.Header {
                text: i18n("Type")
            }


            LatteExtraControls.TypeSelection{
                id: viewTypeSelection
                horizontal: true
            }
        }
        //! END: Inline Dock/Panel Type

        //! BEGIN: Location
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.topMargin: units.smallSpacing

            LatteComponents.Header {
                text: screenRow.visible ? i18n("Screen") : i18n("Location")
            }

            Connections {
                target: universalSettings
                onScreensCountChanged: screenRow.updateScreens()
            }

            RowLayout {
                id: screenRow
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 3
                spacing: 2
                visible: true

                function updateScreens() {
                    if (universalSettings.screens.length > 1) {
                        screenRow.visible = true;
                    } else {
                        screenRow.visible = false;
                    }

                    screensModel.clear();

                    var primary = {name: i18n("On Primary"), icon: 'favorites'};
                    screensModel.append(primary);

                    //check if the screen exists, it is used in cases Latte is moving
                    //the view automatically to primaryScreen in order for the user
                    //to has always a view with tasks shown
                    var screenExists = false
                    for (var i = 0; i < universalSettings.screens.length; i++) {
                        if (universalSettings.screens[i].name === latteView.positioner.currentScreenName) {
                            screenExists = true;
                        }
                    }

                    if (!screenExists && !latteView.onPrimary) {
                        var scr = {name: latteView.positioner.currentScreenName, icon: 'view-fullscreen'};
                        screensModel.append(scr);
                    }

                    for (var i = 0; i < universalSettings.screens.length; i++) {
                        var scr = {name: universalSettings.screens[i].name, icon: 'view-fullscreen'};
                        screensModel.append(scr);
                    }

                    if (latteView.onPrimary) {
                        screenCmb.currentIndex = 0;
                    } else {
                        screenCmb.currentIndex = screenCmb.findScreen(latteView.positioner.currentScreenName);
                    }

                    console.log(latteView.positioner.currentScreenName);
                }

                Connections{
                    target: viewConfig
                    onShowSignal: screenRow.updateScreens();
                }

                ListModel {
                    id: screensModel
                }

                LatteComponents.ComboBox {
                    id: screenCmb
                    Layout.fillWidth: true
                    model: screensModel
                    textRole: "name"
                    iconRole: "icon"

                    Component.onCompleted: screenRow.updateScreens();

                    onActivated: {
                        var succeed = false;

                        if (index === 0) {
                            succeed = latteView.positioner.setCurrentScreen("primary");

                            if (succeed) {
                                latteView.onPrimary = true;
                            } else if (!latteView.onPrimary){
                                console.log("the edge is already occupied!!!");
                                currentIndex = findScreen(latteView.positioner.currentScreenName);
                            }
                        } else if (index>0 && (index !== findScreen(latteView.positioner.currentScreenName) || latteView.onPrimary)) {
                            console.log("current index changed!!! :"+ index);
                            console.log("screen must be changed...");

                            succeed = latteView.positioner.setCurrentScreen(textAt(index));

                            if(succeed) {
                                latteView.onPrimary = false;
                            } else {
                                console.log("the edge is already occupied!!!");
                                currentIndex = findScreen(latteView.positioner.currentScreenName);
                            }
                        }                        
                    }

                    function findScreen(scrName) {
                        for(var i=0; i<screensModel.count; ++i) {
                            if (screensModel.get(i).name === scrName) {
                                return i;
                            }
                        }

                        return 0;
                    }
                }
            }

            RowLayout {
                id: locationLayout
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                Layout.topMargin: screenRow.visible ? units.smallSpacing : 0
                LayoutMirroring.enabled: false
                spacing: 2

                readonly property int buttonSize: (dialog.optionsWidth - (spacing * 3)) / 4

                Connections{
                    target: latteView
                    onDockLocationChanged: locationLayout.lockReservedEdges();
                }

                Connections{
                    target: latteView.layout
                    onViewsCountChanged: locationLayout.lockReservedEdges();
                }

                Connections{
                    target: latteView.positioner
                    onCurrentScreenChanged: locationLayout.lockReservedEdges();
                }

                Component.onCompleted: lockReservedEdges()

                ExclusiveGroup {
                    id: locationGroup
                    property bool inStartup: true

                    onCurrentChanged: {
                        if (current.checked && !inStartup) {
                            latteView.positioner.hideDockDuringLocationChange(current.edge);
                        }
                        inStartup = false;
                    }
                }

                function lockReservedEdges() {
                    var edges = latteView.layout.qmlFreeEdges(latteView.positioner.currentScreenId);

                    bottomEdgeBtn.edgeIsFree = (edges.indexOf(bottomEdgeBtn.edge)>=0);
                    topEdgeBtn.edgeIsFree = (edges.indexOf(topEdgeBtn.edge)>=0);
                    leftEdgeBtn.edgeIsFree = (edges.indexOf(leftEdgeBtn.edge)>=0);
                    rightEdgeBtn.edgeIsFree = (edges.indexOf(rightEdgeBtn.edge)>=0);
                }

                PlasmaComponents.Button {
                    id: bottomEdgeBtn
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18nc("bottom location", "Bottom")
                    iconSource: "arrow-down"
                    checked: latteView.location === edge
                    checkable: true
                    enabled: checked || edgeIsFree
                    exclusiveGroup: locationGroup

                    property bool edgeIsFree: true
                    readonly property int edge: PlasmaCore.Types.BottomEdge
                }
                PlasmaComponents.Button {
                    id: leftEdgeBtn
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18nc("left location", "Left")
                    iconSource: "arrow-left"
                    checked: latteView.location === edge
                    checkable: true
                    enabled: checked || edgeIsFree
                    exclusiveGroup: locationGroup

                    property bool edgeIsFree: true
                    readonly property int edge: PlasmaCore.Types.LeftEdge
                }
                PlasmaComponents.Button {
                    id: topEdgeBtn
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18nc("top location", "Top")
                    iconSource: "arrow-up"
                    checked: latteView.location === edge
                    checkable: true
                    enabled: checked || edgeIsFree
                    exclusiveGroup: locationGroup

                    property bool edgeIsFree: true
                    readonly property int edge: PlasmaCore.Types.TopEdge
                }
                PlasmaComponents.Button {
                    id: rightEdgeBtn
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18nc("right location", "Right")
                    iconSource: "arrow-right"
                    checked: latteView.location === edge
                    checkable: true
                    enabled: checked || edgeIsFree
                    exclusiveGroup: locationGroup

                    property bool edgeIsFree: true
                    readonly property int edge: PlasmaCore.Types.RightEdge
                }
            }
        }
        //! END: Location

        //! BEGIN: Alignment
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            LatteComponents.Header {
                text: i18n("Alignment")
            }

            RowLayout {
                id: alignmentRow
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                LayoutMirroring.enabled: false
                spacing: 2

                readonly property int panelPosition: plasmoid.configuration.panelPosition
                readonly property int buttonSize: (dialog.optionsWidth - (spacing * 3)) / 4

                ExclusiveGroup {
                    id: alignmentGroup
                    onCurrentChanged: {
                        if (current.checked)
                            plasmoid.configuration.panelPosition = current.position
                    }
                }

                PlasmaComponents.Button {
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: panelIsVertical ? i18nc("top alignment", "Top") : i18nc("left alignment", "Left")
                    iconSource: panelIsVertical ? "format-align-vertical-top" : "format-justify-left"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: panelIsVertical ? Latte.Types.Top : Latte.Types.Left
                }
                PlasmaComponents.Button {
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18nc("center alignment", "Center")
                    iconSource: panelIsVertical ? "format-align-vertical-center" : "format-justify-center"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: Latte.Types.Center
                }
                PlasmaComponents.Button {
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: panelIsVertical ? i18nc("bottom alignment", "Bottom") : i18nc("right alignment", "Right")
                    iconSource: panelIsVertical ? "format-align-vertical-bottom" : "format-justify-right"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: panelIsVertical ? Latte.Types.Bottom : Latte.Types.Right
                }

                PlasmaComponents.Button {
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18nc("justify alignment", "Justify")
                    iconSource: "format-justify-fill"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: Latte.Types.Justify
                }
            }
        }
        //! END: Alignment

        //! BEGIN: Visibility
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            LatteComponents.Header {
                text: i18n("Visibility")
            }

            GridLayout {
                width: parent.width
                rowSpacing: 1
                columnSpacing: 2
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                columns: 2

                property int mode: latteView.visibility.mode
                readonly property int buttonSize: (dialog.optionsWidth - (columnSpacing)) / 2

                ExclusiveGroup {
                    id: visibilityGroup
                    onCurrentChanged: {
                        if (current.checked){
                            if (current !== windowsModeBtn.button){
                                latteView.visibility.mode = current.mode;
                            } else {
                                latteView.visibility.mode = windowsModeBtn.mode;
                            }
                        }
                    }
                }

                PlasmaComponents.Button {
                    id:alwaysVisibleBtn
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18n("Always Visible")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.AlwaysVisible
                }
                PlasmaComponents.Button {
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18n("Auto Hide")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.AutoHide
                }
                PlasmaComponents.Button {
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18n("Dodge Active")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.DodgeActive
                }
                PlasmaComponents.Button {
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18n("Dodge Maximized")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.DodgeMaximized
                }
                PlasmaComponents.Button {
                    id: dodgeAllWindowsBtn
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    text: i18n("Dodge All Windows")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.DodgeAllWindows
                }

                LatteExtraControls.CustomWindowsModeButton {
                    id: windowsModeBtn
                    Layout.minimumWidth: parent.buttonSize
                    Layout.maximumWidth: Layout.minimumWidth
                    implicitWidth: dodgeAllWindowsBtn.implicitWidth
                    implicitHeight: dodgeAllWindowsBtn.implicitHeight

                    checked: parent.mode === mode
                    buttonExclusiveGroup:  visibilityGroup
                    comboBoxMinimumPopUpWidth: windowsModeBtn.width
                }
            }
        }
        //! END: Visibility

        //! BEGIN: Delay
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            enabled: !(latteView.visibility.mode === Latte.Types.AlwaysVisible
                       || latteView.visibility.mode === Latte.Types.WindowsGoBelow
                       || latteView.visibility.mode === Latte.Types.WindowsAlwaysCover
                       || latteView.visibility.mode === Latte.Types.SideBar)

            LatteComponents.Header {
                text: i18n("Delay")
            }

            Flow {
                width: dialog.optionsWidth
                Layout.minimumWidth: dialog.optionsWidth
                Layout.maximumWidth: dialog.optionsWidth
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                Layout.topMargin: units.smallSpacing

                spacing: 2

                readonly property bool overlap: showContainer.overlap || hideContainer.overlap

                Item {
                    id: showContainer
                    width: parent.overlap ? dialog.optionsWidth : oneLineWidth
                    height: childrenRect.height
                    implicitWidth: width
                    implicitHeight: height

                    readonly property bool overlap: oneLineWidth > dodgeAllWindowsBtn.width
                    readonly property int oneLineWidth: Math.max(dodgeAllWindowsBtn.width, showTimerRow.width)

                    RowLayout{
                        id: showTimerRow
                        anchors.horizontalCenter: parent.horizontalCenter
                        PlasmaComponents.Label {
                            Layout.leftMargin: Qt.application.layoutDirection === Qt.RightToLeft ? units.smallSpacing : 0
                            Layout.rightMargin: Qt.application.layoutDirection === Qt.RightToLeft ? 0 : units.smallSpacing
                            text: i18n("Show ")
                        }

                        LatteComponents.TextField {
                            Layout.preferredWidth: width
                            text: latteView.visibility.timerShow

                            onValueChanged: {
                                latteView.visibility.timerShow = value
                            }
                        }
                    }
                }

                Item {
                    id: hideContainer
                    width: parent.overlap ? dialog.optionsWidth : oneLineWidth
                    height: childrenRect.height
                    implicitWidth: width
                    implicitHeight: height

                    readonly property bool overlap: oneLineWidth > alwaysVisibleBtn.width
                    readonly property int oneLineWidth: Math.max(alwaysVisibleBtn.width, hideTimerRow.width)

                    RowLayout {
                        id: hideTimerRow
                        anchors.horizontalCenter: parent.horizontalCenter

                        PlasmaComponents.Label {
                            Layout.leftMargin: Qt.application.layoutDirection === Qt.RightToLeft ? units.smallSpacing : 0
                            Layout.rightMargin: Qt.application.layoutDirection === Qt.RightToLeft ? 0 : units.smallSpacing
                            text: i18n("Hide")
                        }

                        LatteComponents.TextField{
                            Layout.preferredWidth: width
                            text: latteView.visibility.timerHide
                            maxValue: 5000

                            onValueChanged: {
                                latteView.visibility.timerHide = value
                            }
                        }
                    }
                }
            }
        }
        //! END: Delay

        //! BEGIN: Actions
        ColumnLayout {
            spacing: units.smallSpacing
            visible: dialog.expertLevel

            LatteComponents.Header {
                text: i18n("Actions")
            }

            ColumnLayout {
                id: actionsPropertiesColumn
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 0

                readonly property int maxLabelWidth: Math.max(trackActiveLbl.implicitWidth,
                                                              mouseWheelLbl.implicitWidth,
                                                              leftBtnLbl.implicitWidth,
                                                              midBtnLbl.implicitWidth)

                LatteComponents.SubHeader {
                    text: i18n("Active Window")
                }

                ColumnLayout {
                    RowLayout {
                        Layout.topMargin: units.smallSpacing

                        PlasmaComponents.Label {
                            id: trackActiveLbl
                            Layout.minimumWidth: actionsPropertiesColumn.maxLabelWidth
                            Layout.maximumWidth: actionsPropertiesColumn.maxLabelWidth
                            text: i18n("Track From")
                        }

                        LatteComponents.ComboBox {
                            id: activeWindowFilterCmb
                            Layout.fillWidth: true
                            model: [i18nc("track from current screen", "Current Screen"),
                                i18nc("track from all screens", "All Screens")]

                            currentIndex: plasmoid.configuration.activeWindowFilter

                            onCurrentIndexChanged: {
                                switch(currentIndex) {
                                case Latte.Types.ActiveInCurrentScreen:
                                    plasmoid.configuration.activeWindowFilter = Latte.Types.ActiveInCurrentScreen;
                                    break;
                                case Latte.Types.ActiveFromAllScreens:
                                    plasmoid.configuration.activeWindowFilter = Latte.Types.ActiveFromAllScreens;
                                    break;
                                }
                            }
                        }
                    }
                }

                LatteComponents.SubHeader {
                    text: i18n("Empty Area")
                }

                ColumnLayout {
                    RowLayout {
                        PlasmaComponents.Label {
                            id: leftBtnLbl
                            Layout.minimumWidth: actionsPropertiesColumn.maxLabelWidth
                            Layout.maximumWidth: actionsPropertiesColumn.maxLabelWidth
                            text: i18n("Left Button")
                        }

                        PlasmaComponents.Button {
                            Layout.fillWidth: true
                            text: i18n("Drag Active Window")
                            checked: plasmoid.configuration.dragActiveWindowEnabled
                            checkable: true
                            tooltip: i18n("The user can use left mouse button to drag and maximized/restore last active window")
                            iconName: "transform-move"

                            onClicked: {
                                plasmoid.configuration.dragActiveWindowEnabled = !plasmoid.configuration.dragActiveWindowEnabled;
                            }
                        }
                    }

                    RowLayout {
                        PlasmaComponents.Label {
                            id: midBtnLbl
                            Layout.minimumWidth: actionsPropertiesColumn.maxLabelWidth
                            Layout.maximumWidth: actionsPropertiesColumn.maxLabelWidth
                            text: i18n("Middle Button")
                        }

                        PlasmaComponents.Button {
                            Layout.fillWidth: true
                            text: i18n("Close Active Window")
                            checked: plasmoid.configuration.closeActiveWindowEnabled
                            checkable: true
                            tooltip: i18n("The user can use middle mouse button to close last active window")
                            iconName: "window-close"

                            onClicked: {
                                plasmoid.configuration.closeActiveWindowEnabled = !plasmoid.configuration.closeActiveWindowEnabled;
                            }
                        }
                    }

                    RowLayout {
                       // Layout.topMargin: units.smallSpacing

                        PlasmaComponents.Label {
                            id: mouseWheelLbl
                            Layout.minimumWidth: actionsPropertiesColumn.maxLabelWidth
                            Layout.maximumWidth: actionsPropertiesColumn.maxLabelWidth
                            text: i18n("Mouse wheel")
                        }

                        LatteComponents.ComboBox {
                            id: scrollAction
                            Layout.fillWidth: true
                            model: [i18nc("none scroll actions", "None Action"),
                                i18n("Cycle Through Desktops"),
                                i18n("Cycle Through Activities"),
                                i18n("Cycle Through Tasks"),
                                i18n("Cycle And Minimize Tasks")
                            ]

                            currentIndex: plasmoid.configuration.scrollAction

                            onCurrentIndexChanged: {
                                switch(currentIndex) {
                                case Latte.Types.ScrollNone:
                                    plasmoid.configuration.scrollAction = Latte.Types.ScrollNone;
                                    break;
                                case Latte.Types.ScrollDesktops:
                                    plasmoid.configuration.scrollAction = Latte.Types.ScrollDesktops;
                                    break;
                                case Latte.Types.ScrollActivities:
                                    plasmoid.configuration.scrollAction = Latte.Types.ScrollActivities;
                                    break;
                                case Latte.Types.ScrollTasks:
                                    plasmoid.configuration.scrollAction = Latte.Types.ScrollTasks;
                                    break;
                                case Latte.Types.ScrollToggleMinimized:
                                    plasmoid.configuration.scrollAction = Latte.Types.ScrollToggleMinimized;
                                    break;
                                }
                            }
                        }
                    }
                }

                LatteComponents.SubHeader {
                    text: i18n("Items")
                }

                LatteComponents.CheckBoxesColumn {
                    LatteComponents.CheckBox {
                        id: titleTooltipsChk
                        Layout.maximumWidth: dialog.optionsWidth
                        text: i18n("Show title tooltips on hovering")
                        tooltip: i18n("Show thinner tooltips produced by Latte for items.\nThese tooltips are not drawn when applets zoom effect is disabled");
                        checked: plasmoid.configuration.titleTooltips
                        //enabled: plasmoid.configuration.zoomLevel > 0

                        onClicked: {
                            plasmoid.configuration.titleTooltips = !plasmoid.configuration.titleTooltips;
                        }
                    }

                    LatteComponents.CheckBox {
                        id: mouseWheelChk
                        Layout.maximumWidth: dialog.optionsWidth
                        text: i18n("Activate through mouse wheel")
                        checked: plasmoid.configuration.mouseWheelActions
                        tooltip: i18n("Enable/Disable the mouse wheel action")
                        visible: dialog.highLevel

                        onClicked: {
                            plasmoid.configuration.mouseWheelActions = checked
                        }
                    }

                    LatteComponents.CheckBox {
                        Layout.maximumWidth: dialog.optionsWidth
                       // Layout.maximumHeight: mouseWheelChk.height
                        text: i18n("➊ Activate based on position through global shortcuts")
                        checked: latteView.isPreferredForShortcuts || (!latteView.layout.preferredForShortcutsTouched && latteView.isHighestPriorityView())
                        tooltip: i18n("This view is used for based on position global shortcuts. Take note that only one view can have that option enabled for each layout")

                        onClicked: {
                            latteView.isPreferredForShortcuts = checked
                            if (!latteView.layout.preferredForShortcutsTouched) {
                                latteView.layout.preferredForShortcutsTouched = true
                            }
                        }
                    }
                }
            }

            LatteComponents.SubHeader {
                id: floatingSubCategory
                text: i18n("Floating")
                enabled: !plasmoid.configuration.shrinkThickMargins && (plasmoid.configuration.screenEdgeMargin >= 0)
            }

            LatteComponents.CheckBoxesColumn {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                enabled: floatingSubCategory.enabled

                LatteComponents.CheckBoxesColumn {
                    LatteComponents.CheckBox {
                        id: fittsLawChk
                        Layout.maximumWidth: dialog.optionsWidth
                        text: i18n("Always use screen gap for user interaction")
                        checked: plasmoid.configuration.fittsLawIsRequested
                        tooltip: i18n("Panels in floating mode use the screen gap for user interaction")

                        onClicked: {
                            plasmoid.configuration.fittsLawIsRequested = checked;
                        }
                    }

                    LatteComponents.CheckBox {
                        id: hideScreenGapForMaximizedChk
                        Layout.maximumWidth: dialog.optionsWidth
                        text: i18n("Hide screen gap for maximized windows")
                        checked: plasmoid.configuration.hideScreenGapForMaximized
                        tooltip: i18n("Panels in floating mode disable their screen gap for maximized windows")

                        onClicked: {
                            plasmoid.configuration.hideScreenGapForMaximized = checked;
                        }
                    }
                }
            }
        }
        //! END: Actions

        //! BEGIN: Adjust
        ColumnLayout {
            spacing: units.smallSpacing

            visible: dialog.expertLevel
            enabled: !(latteView.visibility.mode === Latte.Types.AlwaysVisible
                       || latteView.visibility.mode === Latte.Types.WindowsGoBelow
                       || latteView.visibility.mode === Latte.Types.WindowsCanCover
                       || latteView.visibility.mode === Latte.Types.WindowsAlwaysCover)

            LatteComponents.Header {
                text: i18n("Environment")
            }

            LatteComponents.CheckBoxesColumn {
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Activate KWin edge after hiding")
                    checked: latteView.visibility.enableKWinEdges
                    tooltip: i18n("After the view becomes hidden, KWin is informed to track user feedback. For example an edge visual hint is shown whenever the mouse approaches the hidden view")
                    enabled: !latteView.byPassWM && latteView.visibility.mode !== Latte.Types.SideBar

                    onClicked: {
                        latteView.visibility.enableKWinEdges = checked;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Can be above fullscreen windows")
                    checked: latteView.byPassWM

                    tooltip: i18n("BypassWindowManagerHint flag for the window. The view will be above all windows even those set as 'Always On Top'")

                    onCheckedChanged: {
                        latteView.byPassWM = checked;
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Raise on desktop change")
                    checked: latteView.visibility.raiseOnDesktop

                    onClicked: {
                        latteView.visibility.raiseOnDesktop = checked
                    }
                }

                LatteComponents.CheckBox {
                    Layout.maximumWidth: dialog.optionsWidth
                    text: i18n("Raise on activity change")
                    checked: latteView.visibility.raiseOnActivity

                    onClicked: {
                        latteView.visibility.raiseOnActivity = checked
                    }
                }
            }
        }
        //! END: Adjust

    }
}
