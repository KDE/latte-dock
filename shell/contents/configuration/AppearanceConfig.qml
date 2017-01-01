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

                PlasmaComponents.Button{
                    text:"-"

                    Layout.preferredWidth: parent.height
                    Layout.preferredHeight: parent.height

                    onClicked: appletsSizeSlider.value -= parent.step
                }

                PlasmaComponents.Slider{
                    id:appletsSizeSlider

                    valueIndicatorText: i18n("Applets Size")
                    valueIndicatorVisible: true

                    minimumValue: 16
                    maximumValue: 128

                    stepSize: parent.step

                    Layout.fillWidth:true

                    property bool inStartup:true

                    Component.onCompleted: {
                        value = plasmoid.configuration.iconSize;
                        inStartup = false;
                    }

                    onValueChanged:{
                        if(!inStartup){
                            plasmoid.configuration.iconSize = value;
                        }
                    }
                }

                PlasmaComponents.Button{
                    text:"+"

                    Layout.preferredWidth: parent.height
                    Layout.preferredHeight: parent.height

                    onClicked: appletsSizeSlider.value += parent.step;
                }

                PlasmaComponents.Label{
                    text: plasmoid.configuration.iconSize + " px."
                }
            }
        }



        /**********  Zoom On Hover ****************/
        Column{
            width: parent.width
            spacing: 0.8*theme.defaultFont.pointSize
            Header{
                text: i18n("Zoom On Hover")
            }

            RowLayout{
                width: parent.width

                PlasmaComponents.Button{
                    text:"-"

                    Layout.preferredWidth: parent.height
                    Layout.preferredHeight: parent.height

                    onClicked: zoomSlider.value -= 0.05
                }

                PlasmaComponents.Slider{
                    id:zoomSlider

                    valueIndicatorText: i18n("Zoom Factor")
                    valueIndicatorVisible: true

                    minimumValue: 1
                    maximumValue: 2

                    stepSize: 0.05

                    Layout.fillWidth:true

                    property bool inStartup:true

                    Component.onCompleted: {
                        value = Number(1 + plasmoid.configuration.zoomLevel/20).toFixed(2)
                        inStartup = false;
                        //  console.log("Slider:"+value);
                    }

                    onValueChanged:{
                        if(!inStartup){
                            var result = Math.round((value - 1)*20)
                            plasmoid.configuration.zoomLevel = result
                            //    console.log("Store:"+result);
                        }
                    }
                }

                PlasmaComponents.Button{
                    text:"+"

                    Layout.preferredWidth: parent.height
                    Layout.preferredHeight: parent.height

                    onClicked: zoomSlider.value += 0.05
                }

                PlasmaComponents.Label{
                    enabled: showBackground.checked
                    text: " "+Number(zoomSlider.value).toFixed(2)
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

                PlasmaComponents.Button{
                    enabled: showBackground.checked
                    text:"-"

                    Layout.preferredWidth: parent.height
                    Layout.preferredHeight: parent.height

                    onClicked: panelSizeSlider.value -= 2
                }

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

                    Component.onCompleted: {
                        value = plasmoid.configuration.panelSize
                        inStartup = false;
                    }

                    onValueChanged: {
                        if(!inStartup)
                            plasmoid.configuration.panelSize = value;
                    }
                }

                PlasmaComponents.Button{
                    enabled: showBackground.checked
                    text:"+"

                    Layout.preferredWidth: parent.height
                    Layout.preferredHeight: parent.height

                    onClicked: panelSizeSlider.value += 2
                }


                PlasmaComponents.Label{
                    enabled: showBackground.checked
                    text: panelSizeSlider.value + " px."
                }

            }
        }

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
