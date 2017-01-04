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

import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

PlasmaComponents.Page{
    width: dialog.width - 2*dialog.windowSpace
    Column{
        spacing: 1.5*theme.defaultFont.pointSize
        width: parent.width

        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            Header{
                text: i18n("Location")
            }

            Flow{
                width: parent.width
                spacing: 2

                property bool inStartup: true
                property int dockLocation: dock.location

                onDockLocationChanged: updateDockLocationVisual();

                Component.onCompleted: {
                    lockReservedEdges();
                    updateDockLocationVisual();
                    inStartup = false;
                }

                function lockReservedEdges() {
                    var edges = dock.freeEdges();

                    firstLocation.enabled = false;
                    secondLocation.enabled = false;
                    thirdLocation.enabled = false;
                    fourthLocation.enabled = false;

                    for (var i=0; i<edges.length; ++i){
                        if (edges[i] === PlasmaCore.Types.BottomEdge){
                            firstLocation.enabled = true;
                        } else if (edges[i] === PlasmaCore.Types.LeftEdge){
                            secondLocation.enabled = true;
                        } else if (edges[i] === PlasmaCore.Types.TopEdge){
                            thirdLocation.enabled = true;
                        } else if (edges[i] === PlasmaCore.Types.RightEdge){
                            fourthLocation.enabled = true;
                        }
                    }
                }

                function updateDockLocationVisual(){
                    if(dockLocation === PlasmaCore.Types.BottomEdge){
                        firstLocation.enabled = true;
                        firstLocation.checked = true;
                        secondLocation.checked = false;
                        thirdLocation.checked = false;
                        fourthLocation.checked = false;
                    }
                    else if(dockLocation === PlasmaCore.Types.LeftEdge){
                        firstLocation.checked = false;
                        secondLocation.enabled = true;
                        secondLocation.checked = true;
                        thirdLocation.checked = false;
                        fourthLocation.checked = false;
                    }
                    else if(dockLocation === PlasmaCore.Types.TopEdge){
                        firstLocation.checked = false;
                        secondLocation.checked = false;
                        thirdLocation.enabled = true;
                        thirdLocation.checked = true;
                        fourthLocation.checked = false;
                    }
                    else if(dockLocation === PlasmaCore.Types.RightEdge){
                        firstLocation.checked = false;
                        secondLocation.checked = false;
                        thirdLocation.checked = false;
                        fourthLocation.enabled = true;
                        fourthLocation.checked = true;
                    }
                }


                PlasmaComponents.Button{
                    id: firstLocation
                    checkable: true
                    text: i18n("Bottom")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.location = PlasmaCore.Types.BottomEdge
                        }
                    }
                    onClicked: checked=true;
                }
                PlasmaComponents.Button{
                    id: secondLocation
                    checkable: true
                    text: i18n("Left")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.location = PlasmaCore.Types.LeftEdge
                        }
                    }
                    onClicked: checked=true;
                }
                PlasmaComponents.Button{
                    id: thirdLocation
                    checkable: true
                    text: i18n("Top")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.location = PlasmaCore.Types.TopEdge
                        }
                    }
                    onClicked: checked=true;
                }

                PlasmaComponents.Button{
                    id: fourthLocation
                    checkable: true
                    text: i18n("Right")
                    width: (parent.width/4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.location = PlasmaCore.Types.RightEdge
                        }
                    }
                    onClicked: checked=true;
                }
            }
        }


        /////////// Applets Alignment //////////////////

        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            Header{
                text: i18n("Alignment")
            }

            //user set Panel Positions
            // 0-Center, 1-Left, 2-Right, 3-Top, 4-Bottom
            Flow{
                width: parent.width
                spacing: 2

                property bool inStartup: true
                property int panelPosition: plasmoid.configuration.panelPosition


                function updatePanelPositionVisual(){
                    if((panelPosition == Latte.Dock.Left)||(panelPosition == Latte.Dock.Top)){
                        firstPosition.checked = true;
                        centerPosition.checked = false;
                        lastPosition.checked = false;
                        splitTwoPosition.checked = false;
                        dock.removeInternalViewSplitter();
                    }
                    else if(panelPosition == Latte.Dock.Center){
                        firstPosition.checked = false;
                        centerPosition.checked = true;
                        lastPosition.checked = false;
                        splitTwoPosition.checked = false;
                        dock.removeInternalViewSplitter();
                    }
                    else if((panelPosition == Latte.Dock.Right)||(panelPosition == Latte.Dock.Bottom)){
                        firstPosition.checked = false;
                        centerPosition.checked = false;
                        lastPosition.checked = true;
                        splitTwoPosition.checked = false;
                        dock.removeInternalViewSplitter();
                    }
                    else if (panelPosition == Latte.Dock.Justify){
                        firstPosition.checked = false;
                        centerPosition.checked = false;
                        lastPosition.checked = false;
                        splitTwoPosition.checked = true;
                        //add the splitter visual
                        dock.addInternalViewSplitter();
                    }
                }

                onPanelPositionChanged: updatePanelPositionVisual();

                Component.onCompleted: {
                    updatePanelPositionVisual();
                    inStartup = false;
                }

                PlasmaComponents.Button{
                    id: firstPosition
                    checkable: true
                    text: panelIsVertical ? i18n("Top") : i18n("Left")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            if(panelIsVertical)
                                plasmoid.configuration.panelPosition = Latte.Dock.Top
                            else
                                plasmoid.configuration.panelPosition = Latte.Dock.Left
                        }
                    }
                    onClicked: checked=true;
                }
                PlasmaComponents.Button{
                    id: centerPosition
                    checkable: true
                    text: i18n("Center")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.panelPosition = Latte.Dock.Center
                        }
                    }
                    onClicked: checked=true;
                }
                PlasmaComponents.Button{
                    id: lastPosition
                    checkable: true
                    text: panelIsVertical ? i18n("Bottom") : i18n("Right")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            if(panelIsVertical)
                                plasmoid.configuration.panelPosition = Latte.Dock.Bottom
                            else
                                plasmoid.configuration.panelPosition = Latte.Dock.Right
                        }
                    }
                    onClicked: checked=true;
                }

                PlasmaComponents.Button{
                    id: splitTwoPosition
                    checkable: true
                    text: i18n("Justify")
                    width: (parent.width / 4)

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.panelPosition = Latte.Dock.Justify;
                        }
                    }
                    onClicked: checked=true;
                }
            }
        }

        //AlwaysVisible = 0,
        //AutoHide,
        //DodgeActive,
        //DodgeMaximized,
        //DodgeAllWindows
        /**********  Panel Visibility ****************/

        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize
            Header{
                text: i18n("Visibility")
            }

            //user set Panel Visibility
            // 0-AlwaysVisible, 1-AutoHide, 2-DodgeActive, 3-DodgeMaximized, 4-DodgeAllWindows
            Flow{
                width: parent.width
                spacing: 2

                property bool inStartup: true
                property int mode: dock.visibility.mode


                function updateModeVisual(){
                    if (mode === Latte.Dock.AlwaysVisible)
                        firstState.checked = true;
                    else
                        firstState.checked = false;

                    if (mode === Latte.Dock.AutoHide)
                        secondState.checked = true;
                    else
                        secondState.checked = false;

                    if (mode === Latte.Dock.DodgeActive)
                        thirdState.checked = true;
                    else
                        thirdState.checked = false;

                    if (mode === Latte.Dock.DodgeMaximized)
                        fourthState.checked = true;
                    else
                        fourthState.checked = false;

                    if (mode === Latte.Dock.DodgeAllWindows)
                        fifthState.checked = true;
                    else
                        fifthState.checked = false;
                }

                onModeChanged: updateModeVisual();

                Component.onCompleted: {
                    updateModeVisual();
                    inStartup = false;
                }

                PlasmaComponents.Button{
                    id: firstState
                    checkable: true
                    text: i18n("Always Visible")
                    width: (parent.width / 2) - 1

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.visibility.mode = Latte.Dock.AlwaysVisible
                        }
                    }
                    onClicked: checked=true;
                }
                PlasmaComponents.Button{
                    id: secondState
                    checkable: true
                    text: i18n("Auto Hide")
                    width: (parent.width / 2) - 1

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.visibility.mode = Latte.Dock.AutoHide
                        }
                    }
                    onClicked: checked=true;
                }
                PlasmaComponents.Button{
                    id: thirdState
                    checkable: true
                    text: i18n("Dodge Active")
                    width: (parent.width / 2) - 1

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.visibility.mode = Latte.Dock.DodgeActive
                        }
                    }
                    onClicked: checked=true;
                }

                PlasmaComponents.Button{
                    id: fourthState
                    checkable: true
                    text: i18n("Dodge Maximized")
                    width: (parent.width/2) - 1

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.visibility.mode = Latte.Dock.DodgeMaximized
                        }
                    }
                    onClicked: checked=true;
                }

                PlasmaComponents.Button{
                    id: fifthState
                    checkable: true
                    text: i18n("Dodge All Windows")
                    width: (parent.width/2) - 1

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            dock.visibility.mode = Latte.Dock.DodgeAllWindows
                        }
                    }
                    onClicked: checked=true;
                }
            }
        }

        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize
            Header{
                text: i18n("Delay")
            }

            GridLayout{
                columns: 2
                columnSpacing: 0.3*theme.defaultFont.pointSize
                width: parent.width

                Row{
                    Layout.alignment: Qt.AlignHCenter
                    PlasmaComponents.Label{
                        text: i18n("Hide:")
                    }

                    LatteTextField{
                        width: 9.5 * theme.defaultFont.pixelSize
                        property bool inStartup: true

                        Component.onCompleted: {
                            value = dock.visibility.timerHide
                            inStartup = false;
                        }

                        onValueChanged: {
                            if(!inStartup){
                                dock.visibility.timerHide = value;
                            }
                        }
                    }
                }

                Row{
                    Layout.alignment: Qt.AlignHCenter
                    PlasmaComponents.Label{
                        text: i18n("Show:")
                    }

                    LatteTextField{
                        width: 9.5 * theme.defaultFont.pixelSize
                        property bool inStartup: true

                        Component.onCompleted: {
                            value = dock.visibility.timerShow
                            inStartup = false;
                        }

                        onValueChanged: {
                            if(!inStartup){
                                dock.visibility.timerShow = value;
                            }
                        }
                    }
                }
            }
        }
    }
}
