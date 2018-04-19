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

import org.kde.latte 0.1 as Latte

import "../controls" as LatteExtraControls

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    property alias dockTypeSelection: _dockTypeSelection

    ColumnLayout {
        id: content

        width: dialog.maxWidth - 2 * Layout.leftMargin
        spacing: dialog.subGroupSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.leftMargin: units.smallSpacing * 2

        //! BEGIN: Inline Dock/Panel Type, it is used only when the secondary window
        //! overlaps the main dock config window
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            Layout.topMargin: units.smallSpacing

            visible: plasmoid.configuration.advanced && dockConfig.showInlineProperties

            Header {
                text: i18n("Type")
            }

            TypeSelection{
                id: _dockTypeSelection
                horizontal: true
            }
        }
        //! END: Inline Dock/Panel Type

        //! BEGIN: Location
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2
            Layout.topMargin: units.smallSpacing

            Header {
                text: i18n("Location")
            }

            RowLayout {
                id: screenRow
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                spacing: 1
                visible: true

                function updateScreens() {
                    if (dock.screens.length > 1)
                        screenRow.visible = true;
                    else
                        screenRow.visible = false;

                    var screens = []
                    var rtlSpace = Qt.application.layoutDirection === Qt.RightToLeft ? "    " : "";

                    screens.push(rtlSpace + i18n("On Primary"));

                    //check if the screen exists, it is used in cases Latte is moving
                    //the dock automatically to primaryScreen in order for the user
                    //to has always a dock with tasks shown
                    var screenExists = false
                    for (var i = 0; i < dock.screens.length; i++) {
                        if (dock.screens[i].name === dock.currentScreen)
                            screenExists = true;
                    }

                    if (!screenExists && !dock.onPrimary)
                        screens.push(rtlSpace + dock.currentScreen);

                    for (var i = 0; i < dock.screens.length; i++) {
                        screens.push(rtlSpace + dock.screens[i].name)
                    }

                    screenCmb.model = screens;

                    if (dock.onPrimary) {
                        screenCmb.currentIndex = 0;
                    } else {
                        screenCmb.currentIndex = screenCmb.find(dock.currentScreen);
                    }

                    console.log(dock.currentScreen);
                }

                Connections{
                    target: dockConfig
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
                    //is occuppied
                    property bool acceptedIndex: true
                    property int previousIndex: -1

                    onCurrentIndexChanged: {
                        //it is used to restore the index when the screen edge
                        //is occuppied
                        if (!acceptedIndex) {
                            acceptedIndex = true;
                            currentIndex = previousIndex;
                        }
                    }

                    onActivated: {
                        previousIndex = currentIndex;
                        if (index === 0) {
                            var succeed = dock.setCurrentScreen("primary");

                            dock.onPrimary = true;
                            acceptedIndex = true;
                        } else if (index>0 && (index !== find(dock.currentScreen) || dock.onPrimary)) {
                            console.log("current index changed!!! :"+ index);
                            console.log("screen must be changed...");

                            var succeed = dock.setCurrentScreen(textAt(index));

                            if(succeed) {
                                dock.onPrimary = false;
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
                spacing: 1

                Connections{
                    target: dock
                    onDockLocationChanged: locationLayout.lockReservedEdges();
                    onDocksCountChanged: locationLayout.lockReservedEdges();
                }

                ExclusiveGroup {
                    id: locationGroup
                    property bool inStartup: true

                    onCurrentChanged: {
                        if (current.checked && !inStartup) {
                            dock.hideDockDuringLocationChange(current.edge);
                        }
                        inStartup = false;
                    }
                }

                function lockReservedEdges() {
                    var buttons = visibleChildren
                    var edges = dock.freeEdges()

                    for (var i = 0; i < buttons.length; i++) {
                        buttons[i].enabled = buttons[i].checked || freeEdge(buttons[i].edge, edges)
                    }
                }

                function freeEdge(edge, edges) {
                    for (var i = 0; i < edges.length; i++) {
                        if (edges[i] === edge)
                            return true
                    }
                    return false
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("bottom location", "Bottom")
                    iconSource: "arrow-down"
                    checked: dock.location === edge
                    checkable: true
                    enabled: checked || locationLayout.freeEdge(edge, dock.freeEdges())
                    exclusiveGroup: locationGroup

                    readonly property int edge: PlasmaCore.Types.BottomEdge
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("left location", "Left")
                    iconSource: "arrow-left"
                    checked: dock.location === edge
                    checkable: true
                    enabled: checked || locationLayout.freeEdge(edge, dock.freeEdges())
                    exclusiveGroup: locationGroup

                    readonly property int edge: PlasmaCore.Types.LeftEdge
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("top location", "Top")
                    iconSource: "arrow-up"
                    checked: dock.location === edge
                    checkable: true
                    enabled: checked || locationLayout.freeEdge(edge, dock.freeEdges())
                    exclusiveGroup: locationGroup

                    readonly property int edge: PlasmaCore.Types.TopEdge
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("right location", "Right")
                    iconSource: "arrow-right"
                    checked: dock.location === edge
                    checkable: true
                    enabled: checked || locationLayout.freeEdge(edge, dock.freeEdges())
                    exclusiveGroup: locationGroup

                    readonly property int edge: PlasmaCore.Types.RightEdge
                }
            }
        }
        //! END: Location

        //! BEGIN: Alignment
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            Header {
                text: i18n("Alignment")
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2
                LayoutMirroring.enabled: false
                spacing: 1

                property int panelPosition: plasmoid.configuration.panelPosition

                onPanelPositionChanged: {
                    if (panelPosition === Latte.Dock.Justify)
                        dock.addInternalViewSplitter()
                    else
                        dock.removeInternalViewSplitter()
                }

                Component.onCompleted: {
                    if (panelPosition === Latte.Dock.Justify)
                        dock.addInternalViewSplitter()
                    else
                        dock.removeInternalViewSplitter()
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

                    property int position: panelIsVertical ? Latte.Dock.Top : Latte.Dock.Left
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("center alignment", "Center")
                    iconSource: panelIsVertical ? "format-align-vertical-center" : "format-justify-center"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: Latte.Dock.Center
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: panelIsVertical ? i18nc("bottom alignment", "Bottom") : i18nc("right alignment", "Right")
                    iconSource: panelIsVertical ? "format-align-vertical-bottom" : "format-justify-right"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: panelIsVertical ? Latte.Dock.Bottom : Latte.Dock.Right
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18nc("justify alignment", "Justify")
                    iconSource: "format-justify-fill"
                    checked: parent.panelPosition === position
                    checkable: true
                    exclusiveGroup: alignmentGroup

                    property int position: Latte.Dock.Justify
                }
            }
        }
        //! END: Alignment

        //! BEGIN: Visibility
        ColumnLayout {
            Layout.fillWidth: true
            spacing: units.smallSpacing
            Layout.rightMargin: units.smallSpacing * 2

            Header {
                text: i18n("Visibility")
            }

            GridLayout {
                width: parent.width
                rowSpacing: 1
                columnSpacing: 1
                Layout.leftMargin: units.smallSpacing * 2
                Layout.rightMargin: units.smallSpacing * 2

                columns: 2

                property int mode: dock.visibility.mode

                ExclusiveGroup {
                    id: visibilityGroup
                    onCurrentChanged: {
                        if (current.checked)
                            dock.visibility.mode = current.mode
                    }
                }

                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Always Visible")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Dock.AlwaysVisible
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Auto Hide")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Dock.AutoHide
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Dodge Active")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Dock.DodgeActive
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Dodge Maximized")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Dock.DodgeMaximized
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Dodge All Windows")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Dock.DodgeAllWindows
                }
                PlasmaComponents.Button {
                    Layout.fillWidth: true
                    text: i18n("Windows Go Below")
                    checked: parent.mode === mode
                    checkable: true
                    exclusiveGroup: visibilityGroup

                    property int mode: Latte.Dock.WindowsGoBelow
                }
            }
        }
        //! END: Visibility

        //! BEGIN: Delay
        ColumnLayout {
            Layout.fillWidth: true
            Layout.rightMargin: units.smallSpacing * 2
            spacing: units.smallSpacing

            enabled: !(dock.visibility.mode === Latte.Dock.AlwaysVisible
                       || dock.visibility.mode === Latte.Dock.WindowsGoBelow)

            Header {
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
                    text: dock.visibility.timerShow

                    onValueChanged: {
                        dock.visibility.timerShow = value
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
                    text: dock.visibility.timerHide

                    onValueChanged: {
                        dock.visibility.timerHide = value
                    }
                }
            }
        }
        //! END: Delay

        PlasmaComponents.Label{
            id: bottomMarginSpacer
            text:" "
        }
    }
}
