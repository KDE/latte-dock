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

import "../../controls" as LatteExtraControls

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    property alias dockTypeSelection: _dockTypeSelection

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

            LatteExtraControls.Header {
                text: i18n("Type")
            }


            LatteExtraControls.TypeSelection{
                id: _dockTypeSelection
                horizontal: true
            }
        }
        //! END: Inline Dock/Panel Type

        //! BEGIN: Location
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.topMargin: units.smallSpacing

            LatteExtraControls.Header {
                text: i18n("Location")
            }

            RowLayout {
                id: screenRow
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 2
                visible: true

                function updateScreens() {
                    if (universalSettings.screens.length > 1)
                        screenRow.visible = true;
                    else
                        screenRow.visible = false;

                    var screens = []
                    var rtlSpace = Qt.application.layoutDirection === Qt.RightToLeft ? "    " : "";

                    screens.push(rtlSpace + i18n("On Primary"));

                    //check if the screen exists, it is used in cases Latte is moving
                    //the view automatically to primaryScreen in order for the user
                    //to has always a view with tasks shown
                    var screenExists = false
                    for (var i = 0; i < universalSettings.screens.length; i++) {
                        if (universalSettings.screens[i].name === latteView.positioner.currentScreenName)
                            screenExists = true;
                    }

                    if (!screenExists && !latteView.onPrimary)
                        screens.push(rtlSpace + latteView.positioner.currentScreenName);

                    for (var i = 0; i < universalSettings.screens.length; i++) {
                        screens.push(rtlSpace + universalSettings.screens[i].name)
                    }

                    screenCmb.model = screens;

                    if (latteView.onPrimary) {
                        screenCmb.currentIndex = 0;
                    } else {
                        screenCmb.currentIndex = screenCmb.find(latteView.positioner.currentScreenName);
                    }

                    console.log(latteView.positioner.currentScreenName);
                }

                Connections{
                    target: viewConfig
                    onShowSignal: screenRow.updateScreens();
                }

                PlasmaComponents.Label {
                    text: i18n("Screen:")
                    Layout.alignment: Qt.AlignRight
                }

                PlasmaComponents3.ComboBox {
                    id: screenCmb
                    Layout.fillWidth: true
                    Component.onCompleted: screenRow.updateScreens();

                    //they are used to restore the index when the screen edge
                    //is occupied
                    property bool acceptedIndex: true
                    property int previousIndex: -1

                    onCurrentIndexChanged: {
                        //it is used to restore the index when the screen edge
                        //is occupied
                        if (!acceptedIndex) {
                            acceptedIndex = true;
                            currentIndex = previousIndex;
                        }
                    }

                    onActivated: {
                        previousIndex = currentIndex;
                        if (index === 0) {
                            var succeed = latteView.positioner.setCurrentScreen("primary");

                            latteView.onPrimary = true;
                            acceptedIndex = true;
                        } else if (index>0 && (index !== find(latteView.positioner.currentScreenName) || latteView.onPrimary)) {
                            console.log("current index changed!!! :"+ index);
                            console.log("screen must be changed...");

                            var succeed = latteView.positioner.setCurrentScreen(textAt(index));

                            if(succeed) {
                                latteView.onPrimary = false;
                            } else {
                                console.log("the edge is already occupied!!!");
                                acceptedIndex = false;
                            }
                        }
                    }
                }
            }

            RowLayout {
                id: locationLayout
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                LayoutMirroring.enabled: false
                spacing: 2

                Connections{
                    target: latteView
                    onDockLocationChanged: locationLayout.lockReservedEdges();
                }

                Connections{
                    target: latteView.managedLayout
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
                    var edges = latteView.managedLayout.qmlFreeEdges(latteView.positioner.currentScreenId);

                    bottomEdgeBtn.edgeIsFree = (edges.indexOf(bottomEdgeBtn.edge)>=0);
                    topEdgeBtn.edgeIsFree = (edges.indexOf(topEdgeBtn.edge)>=0);
                    leftEdgeBtn.edgeIsFree = (edges.indexOf(leftEdgeBtn.edge)>=0);
                    rightEdgeBtn.edgeIsFree = (edges.indexOf(rightEdgeBtn.edge)>=0);
                }

                PlasmaComponents.Button {
                    id: bottomEdgeBtn
                    Layout.fillWidth: true
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
                    Layout.fillWidth: true
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
                    Layout.fillWidth: true
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
                    Layout.fillWidth: true
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

            LatteExtraControls.Header {
                text: i18n("Alignment")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                LayoutMirroring.enabled: false
                spacing: 2

                property int panelPosition: plasmoid.configuration.panelPosition

                onPanelPositionChanged: {
                    if (panelPosition === Latte.Types.Justify)
                        latteView.addInternalViewSplitter()
                    else
                        latteView.removeInternalViewSplitter()
                }

                Component.onCompleted: {
                    if (panelPosition === Latte.Types.Justify)
                        latteView.addInternalViewSplitter()
                    else
                        latteView.removeInternalViewSplitter()
                }

                ExclusiveGroup {
                    id: alignmentGroup
                    onCurrentChanged: {
                        if (current.checked)
                            plasmoid.configuration.panelPosition = current.position
                    }
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: panelIsVertical ? i18nc("top alignment", "Top") : i18nc("left alignment", "Left")
                    iconSource: panelIsVertical ? "format-align-vertical-top" : "format-justify-left"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: panelIsVertical ? Latte.Types.Top : Latte.Types.Left
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("center alignment", "Center")
                    iconSource: panelIsVertical ? "format-align-vertical-center" : "format-justify-center"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: Latte.Types.Center
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: panelIsVertical ? i18nc("bottom alignment", "Bottom") : i18nc("right alignment", "Right")
                    iconSource: panelIsVertical ? "format-align-vertical-bottom" : "format-justify-right"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: panelIsVertical ? Latte.Types.Bottom : Latte.Types.Right
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
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

            LatteExtraControls.Header {
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

                ExclusiveGroup {
                    id: visibilityGroup
                    onCurrentChanged: {
                        if (current.checked)
                            latteView.visibility.mode = current.mode
                    }
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Always Visible")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.AlwaysVisible
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Auto Hide")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.AutoHide
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Dodge Active")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.DodgeActive
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Dodge Maximized")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.DodgeMaximized
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Dodge All Windows")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.DodgeAllWindows
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Windows Go Below")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Types.WindowsGoBelow
                }
            }
        }
        //! END: Visibility

        //! BEGIN: Delay
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing

            enabled: !(latteView.visibility.mode === Latte.Types.AlwaysVisible
                       || latteView.visibility.mode === Latte.Types.WindowsGoBelow)

            LatteExtraControls.Header {
                Layout.fillWidth: true
                text: i18n("Delay")
            }

            RowLayout {
                Layout.fillWidth: false
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                Layout.alignment: Qt.AlignHCenter

                spacing: 2

                PlasmaComponents.Label {
                    Layout.fillWidth: false
                    Layout.rightMargin: Qt.application.layoutDirection === Qt.RightToLeft ? 0 : units.smallSpacing
                    Layout.leftMargin: Qt.application.layoutDirection === Qt.RightToLeft ? units.smallSpacing : 0
                    horizontalAlignment: Text.AlignRight
                    text: i18n("Show")
                }
                LatteExtraControls.TextField {
                    Layout.preferredWidth: width
                    text: latteView.visibility.timerShow

                    onValueChanged: {
                        latteView.visibility.timerShow = value
                    }
                }

                PlasmaComponents.Label {
                    Layout.fillWidth: false
                    Layout.leftMargin: Qt.application.layoutDirection === Qt.RightToLeft ?
                                           units.smallSpacing : units.largeSpacing
                    Layout.rightMargin: Qt.application.layoutDirection === Qt.RightToLeft ?
                                            units.largeSpacing : units.smallSpacing
                    horizontalAlignment: Text.AlignRight
                    text: i18n("Hide")
                }
                LatteExtraControls.TextField{
                    Layout.preferredWidth: width
                    text: latteView.visibility.timerHide

                    onValueChanged: {
                        latteView.visibility.timerHide = value
                    }
                }
            }
        }
        //! END: Delay

        //! BEGIN: Items
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            visible: dialog.expertLevel

            LatteExtraControls.Header {
                text: i18n("Items")
            }

            PlasmaComponents.CheckBox {
                id: titleTooltipsChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show title tooltips on hovering")
                checked: plasmoid.configuration.titleTooltips

                onClicked: {
                    plasmoid.configuration.titleTooltips = checked;
                }
            }

            PlasmaComponents.CheckBox {
                id: mouseWheelChk
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Activate through mouse wheel")
                checked: plasmoid.configuration.mouseWheelActions
                tooltip: i18n("Enable/Disable the mouse wheel action")
                visible: dialog.highLevel

                onClicked: {
                    plasmoid.configuration.mouseWheelActions = checked
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Activate based on position through global shortcuts")
                checked: latteView.isPreferredForShortcuts

                tooltip: i18n("This view activates its items based on their position through global shortcuts. Take note that only one view can have that option enabled for each layout")

                onClicked: {
                    latteView.isPreferredForShortcuts = checked
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Decrease size automatically when needed")
                checked: plasmoid.configuration.autoDecreaseIconSize
                tooltip: i18n("Items size is decreased automatically when the contents exceed the maximum length \n\nHint: this option is disabled when plasma taskmanagers are present")
                enabled: !(latteView.tasksPresent() && !latteView.latteTasksPresent());

                onClicked: {
                    plasmoid.configuration.autoDecreaseIconSize = checked
                }
            }
        }
        //! END: Items

        //! BEGIN: Adjust
        ColumnLayout {
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            visible: dialog.expertLevel

            LatteExtraControls.Header {
                text: i18n("Environment")
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Activate KWin edge after hiding")
                checked: latteView.visibility.enableKWinEdges
                tooltip: i18n("After the view becomes hidden, KWin is informed to track user feedback. For example an edge visual hint is shown whenever the mouse approaches the hidden view")

                onClicked: {
                    latteView.visibility.enableKWinEdges = checked;
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Can be above fullscreen windows")
                checked: latteView.byPassWM
                enabled: !(latteView.visibility.mode === Latte.Types.AlwaysVisible
                           || latteView.visibility.mode === Latte.Types.WindowsGoBelow)

                tooltip: i18n("BypassWindowManagerHint flag for the window. The view will be above all windows even those set as 'Always On Top'")

                onCheckedChanged: {
                    latteView.byPassWM = checked;
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Raise on desktop change")
                checked: latteView.visibility.raiseOnDesktop
                enabled: latteView.visibility.mode !== Latte.Types.AlwaysVisible

                onClicked: {
                    latteView.visibility.raiseOnDesktop = checked
                }
            }

            PlasmaComponents.CheckBox {
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Raise on activity change")
                checked: latteView.visibility.raiseOnActivity
                enabled: latteView.visibility.mode !== Latte.Types.AlwaysVisible

                onClicked: {
                    latteView.visibility.raiseOnActivity = checked
                }
            }
        }
        //! END: Adjust

        //! Bottom spacer
        PlasmaComponents.Label{
            id: bottomMarginSpacer
            text:" "
        }
    }
}
