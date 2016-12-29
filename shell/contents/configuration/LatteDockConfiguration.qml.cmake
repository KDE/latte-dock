import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

PlasmaCore.FrameSvgItem {
    imagePath: "dialogs/background"

    width: Math.max(420,noneShadow.width + lockedAppletsShadow.width + allAppletsShadow.width)
    height: mainColumn.height+2*windowSpace

    property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    property int windowSpace:8

    signal updateThickness();

    Column{
        id:mainColumn
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1.5*theme.defaultFont.pointSize
        width: parent.width - 2*windowSpace

        //////////// Location ////////////////

        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            RowLayout{
                width: parent.width
                PlasmaComponents.Label{
                    text: i18n("Location")
                    font.pointSize: 1.5 * theme.defaultFont.pointSize
                    Layout.alignment: Qt.AlignLeft
                }

                PlasmaComponents.Label{
                    font.pointSize: theme.defaultFont.pointSize
                    font.italic: true
                    opacity: 0.6

                    Layout.alignment: Qt.AlignRight
                    horizontalAlignment: Text.AlignRight

                    text: i18n("ver: ") +"@VERSION@"
                }
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
                    width: (parent.width/4) - 1

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

            PlasmaComponents.Label{
                text: i18n("Applets Alignment")
                font.pointSize: 1.5 * theme.defaultFont.pointSize
                Layout.alignment: Qt.AlignLeft
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
                    else if (panelPosition == Latte.Dock.Double){
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
                    width: (parent.width / 3) - 1

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
                    width: (parent.width / 3) - 1

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
                    width: (parent.width / 3) - 2

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
                    text: panelIsVertical ? i18n("Top")+ " | "+ i18n("Bottom") : i18n("Left") +" | "+ i18n("Right")
                    width: parent.width

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.panelPosition = Latte.Dock.Double;
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
            PlasmaComponents.Label{
                text: i18n("Visibility")
                font.pointSize: 1.5 * theme.defaultFont.pointSize
            }

            //user set Panel Visibility
            // 0-AlwaysVisible, 1-AutoHide, 2-DodgeActive, 3-DodgeMaximized, 4-DodgeAllWindows
            Flow{
                width: parent.width
                spacing: 2

                property bool inStartup: true
                property int panelVisibility: plasmoid.configuration.panelVisibility


                function updatePanelVisibilityVisual(){
                    if (panelVisibility === 0)
                        firstState.checked = true;
                    else
                        firstState.checked = false;

                    if (panelVisibility === 1)
                        secondState.checked = true;
                    else
                        secondState.checked = false;

                    if (panelVisibility === 2)
                        thirdState.checked = true;
                    else
                        thirdState.checked = false;

                    if (panelVisibility === 3)
                        fourthState.checked = true;
                    else
                        fourthState.checked = false;

                    if (panelVisibility === 4)
                        fifthState.checked = true;
                    else
                        fifthState.checked = false;

                    /*   if (panelVisibility === 5)
                        sixthState.checked = true;
                    else
                        sixthState.checked = false;*/
                }

                onPanelVisibilityChanged: updatePanelVisibilityVisual();

                Component.onCompleted: {
                    updatePanelVisibilityVisual();
                    inStartup = false;
                }

                PlasmaComponents.Button{
                    id: firstState
                    checkable: true
                    text: i18n("Always Visible")
                    width: (parent.width / 2) - 1

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.panelVisibility = 0
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
                            plasmoid.configuration.panelVisibility = 1
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
                            plasmoid.configuration.panelVisibility = 2
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
                            plasmoid.configuration.panelVisibility = 3
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
                            plasmoid.configuration.panelVisibility = 4
                        }
                    }
                    onClicked: checked=true;
                }
                /* PlasmaComponents.Button{
                    id: sixthState
                    checkable: true
                    text: i18n("Always Visible")
                    width: (parent.width/2) - 1

                    onCheckedChanged: {
                        if(checked && !parent.inStartup){
                            plasmoid.configuration.panelVisibility = 5
                        }
                    }
                    onClicked: checked=true;
                }*/
            }
        }

        //////////////// Applets Size

        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            PlasmaComponents.Label{
                text: i18n("Applets Size")
                font.pointSize: 1.5 * theme.defaultFont.pointSize
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
                            updateThickness();
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
            PlasmaComponents.Label{
                text: i18n("Zoom On Hover")
                font.pointSize: 1.5 * theme.defaultFont.pointSize
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
            PlasmaComponents.Label{
                text: i18n("Background")
                font.pointSize: 1.5 * theme.defaultFont.pointSize
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
            PlasmaComponents.Label{
                text: i18n("Shadows")
                font.pointSize: 1.5 * theme.defaultFont.pointSize
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

        Row{
            PlasmaComponents.Button{
                enabled: true
                text: i18n("Add New Dock")

                onClicked: dock.addNewDock();

                Component.onCompleted: {
                    var edges = dock.freeEdges();
                    if (edges.length === 0) {
                        enabled = false;
                    }
                }
            }
            PlasmaComponents.Button{
                enabled: true
                text: i18n("Remove Dock")

                onClicked: dock.removeDock();
            }
        }
    }
}


