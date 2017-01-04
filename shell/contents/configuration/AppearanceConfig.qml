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
    height: appearanceColumn.height

    Column{
        id: appearanceColumn
        spacing: 1.5*theme.defaultFont.pointSize
        width: parent.width

        //////////////// Applets Size
        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            Header{
                text: i18n("Applets Size")
            }

            RowLayout{
                width: parent.width

                property int step: 8

                PlasmaComponents.Slider{
                    id:appletsSizeSlider

                    valueIndicatorText: i18n("Applets Size")
                    valueIndicatorVisible: true

                    minimumValue: 16
                    maximumValue: 128

                    stepSize: parent.step

                    Layout.fillWidth:true

                    property bool inStartup:true

                    property int tempValue: value

                    Component.onCompleted: {
                        value = plasmoid.configuration.iconSize;
                        inStartup = false;
                    }

                    onPressedChanged: {
                        if (!pressed) {
                            plasmoid.configuration.iconSize = value;
                        }
                    }

                    onTempValueChanged:{
                        if(!inStartup && !pressed){
                            plasmoid.configuration.iconSize = value;
                        }
                    }
                }

                PlasmaComponents.Label{
                    text: appletsSizeSlider.value + " px."
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }
            }
        }

        /**********  Zoom On Hover ****************/
        Column{
            width: parent.width
            spacing: 0.8*theme.defaultFont.pointSize
            enabled: plasmoid.configuration.durationTime > 0
            Header{
                text: i18n("Zoom On Hover")
            }

            RowLayout{
                width: parent.width

                PlasmaComponents.Slider{
                    id:zoomSlider

                    valueIndicatorText: i18n("Zoom Factor")
                    valueIndicatorVisible: true

                    minimumValue: 1
                    maximumValue: 2

                    stepSize: 0.05

                    Layout.fillWidth:true

                    property bool inStartup:true

                    property real tempValue: value

                    Component.onCompleted: {
                        value = Number(1 + plasmoid.configuration.zoomLevel/20).toFixed(2)
                        inStartup = false;
                        //  console.log("Slider:"+value);
                    }

                    onPressedChanged: {
                        if (!pressed) {
                            var result = Math.round((value - 1)*20)
                            plasmoid.configuration.zoomLevel = result
                        }
                    }

                    onTempValueChanged:{
                        if(!inStartup && !pressed){
                            var result = Math.round((value - 1)*20)
                            plasmoid.configuration.zoomLevel = result
                        }
                    }

                }

                PlasmaComponents.Label{
                    enabled: showBackground.checked
                    //text: " "+Number(zoomSlider.value).toFixed(2)
                    text: " "+Number((zoomSlider.value*100)-100).toFixed(0)+"%"

                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }

            }
        }

        /**Animations Speed***/

        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            Header{
                text: i18n("Animations")
            }

            Flow{
                width: parent.width
                spacing: 2

                property bool inStartup: true
                property int duration: plasmoid.configuration.durationTime

                onDurationChanged: updateDurationVisual();

                Component.onCompleted: {
                    updateDurationVisual();
                    inStartup = false;
                }

                function updateDurationVisual(){
                    if(duration === 0){
                        firstDuration.checked = true;
                        secondDuration.checked = false;
                        thirdDuration.checked = false;
                        fourthDuration.checked = false;
                    }
                    else if(duration === 1){
                        firstDuration.checked = false;
                        secondDuration.checked = true;
                        thirdDuration.checked = false;
                        fourthDuration.checked = false;
                    }
                    else if(duration === 2){
                        firstDuration.checked = false;
                        secondDuration.checked = false;
                        thirdDuration.checked = true;
                        fourthDuration.checked = false;
                    }
                    else if(duration === 3){
                        firstDuration.checked = false;
                        secondDuration.checked = false;
                        thirdDuration.checked = false;
                        fourthDuration.checked = true;
                    }
                }


                PlasmaComponents.Button{
                    id: firstDuration
                    checkable: true
                    text: i18n("None")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.durationTime = 0;
                        }
                    }
                    onClicked: checked=true;
                }
                PlasmaComponents.Button{
                    id: secondDuration
                    checkable: true
                    text: i18n("x1")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.durationTime = 1;
                        }
                    }
                    onClicked: checked=true;
                }
                PlasmaComponents.Button{
                    id: thirdDuration
                    checkable: true
                    text: i18n("x2")
                    width: (parent.width / 4) - 2

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.durationTime = 2;
                        }
                    }
                    onClicked: checked=true;
                }

                PlasmaComponents.Button{
                    id: fourthDuration
                    checkable: true
                    text: i18n("x3")
                    width: (parent.width/4) - 1

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.durationTime = 3;
                        }
                    }
                    onClicked: checked=true;
                }
            }
        }


        Column{
            width: parent.width
            spacing: 0.8*theme.defaultFont.pointSize
            Header{
                text: i18n("Background")
            }

            PlasmaComponents.CheckBox{
                id: showBackground
                text: i18n("Show Panel Background")

                property bool inStartup: true
                onCheckedChanged:{
                    if(!inStartup)
                        plasmoid.configuration.useThemePanel = checked;
                }

                Component.onCompleted: {
                    checked = plasmoid.configuration.useThemePanel;
                    inStartup = false;
                }
            }

            RowLayout{
                width: parent.width

                PlasmaComponents.Slider{
                    id:panelSizeSlider
                    enabled: showBackground.checked
                    valueIndicatorText: i18n("Size")
                    valueIndicatorVisible: true

                    minimumValue: 0
                    maximumValue: 256

                    stepSize: 2

                    Layout.fillWidth:true

                    property bool inStartup: true
                    property int tempValue: value

                    Component.onCompleted: {
                        value = plasmoid.configuration.panelSize
                        inStartup = false;
                    }

                    onPressedChanged: {
                        if (!pressed) {
                            plasmoid.configuration.panelSize = value;
                        }
                    }

                    onTempValueChanged:{
                        if(!inStartup && !pressed){
                            plasmoid.configuration.panelSize = value;
                        }
                    }
                }

                PlasmaComponents.Label{
                    enabled: showBackground.checked
                    text: panelSizeSlider.value + " px."
                    Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
                }

            }
        }

        /******Shadows**********/
        Column{
            width: parent.width
            spacing: 0.8*theme.defaultFont.pointSize
            Header{
                text: i18n("Shadows")
            }

            RowLayout {
                width: parent.width

                ExclusiveGroup {
                    id: shadowsGroup
                    property bool inStartup: true

                    onCurrentChanged: {
                        if (!inStartup) {
                            if (current === noneShadow){
                                plasmoid.configuration.shadows = 0; /*No Shadows*/
                            } else if (current === lockedAppletsShadow){
                                plasmoid.configuration.shadows = 1; /*Locked Applets Shadows*/
                            } else if (current === allAppletsShadow){
                                plasmoid.configuration.shadows = 2; /*All Applets Shadows*/
                            }
                        }
                    }

                    Component.onCompleted: {
                        if (plasmoid.configuration.shadows === 0 /*No Shadows*/){
                            noneShadow.checked = true;
                        } else if (plasmoid.configuration.shadows === 1 /*Locked Applets*/) {
                            lockedAppletsShadow.checked = true;
                        } else if (plasmoid.configuration.shadows === 2 /*All Applets*/) {
                            allAppletsShadow.checked = true;
                        }

                        inStartup = false;
                    }
                }

                PlasmaComponents.RadioButton {
                    id: noneShadow
                    text: i18n("None")
                    exclusiveGroup: shadowsGroup
                }
                PlasmaComponents.RadioButton {
                    id: lockedAppletsShadow
                    text: i18n("Only for locked applets")
                    exclusiveGroup: shadowsGroup
                }
                PlasmaComponents.RadioButton {
                    id: allAppletsShadow
                    text: i18n("All applets")
                    exclusiveGroup: shadowsGroup
                }

            }
        }
    }
}
