import QtQuick 2.0
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

Component {
    id: imageBufferingComponent
    Item {
        id: yourImageWithLoadedIconContainer
        anchors.fill: parent

        visible: false

        property QtObject imageTimer

        function updateImage(){
            if(!imageTimer)
                imageTimer = tttTimer.createObject(iconImage);
            else{
                imageTimer.loop = 1;
                imageTimer.restart();
            }
        }

        Item{
            id:fixedIcon

            width: panel.iconSize + 2*shadowImageNoActive.radius
            height: width

            visible:false

            KQuickControlAddons.QIconItem{
                id: iconImage

                width: panel.iconSize
                height: width
                anchors.centerIn: parent

                icon: decoration
                state: KQuickControlAddons.QIconItem.DefaultState

                enabled: true
                visible: true

                smooth: true
              //  onIconChanged: updateBuffers();

                property int counter: 0

                function updateBuffers(){
                    if((index !== -1) &&(!centralItem.toBeDestroyed) &&(!mainItemContainer.delayingRemove)){
                        if(panel.initializationStep){
                            panel.initializationStep = false;
                        }

                        centralItem.firstDrawed = true;
                  //      counter++;
                  //      console.log(counter);

                        if(shadowedImage && shadowedImage.source)
                            shadowedImage.source.destroy();


                        if(panel.enableShadows == true){
                            shadowImageNoActive.grabToImage(function(result) {
                                shadowedImage.source = result.url
                                result.destroy();
                            }, Qt.size(fixedIcon.width,fixedIcon.height) );
                        }
                        else{
                            fixedIcon.grabToImage(function(result) {
                                shadowedImage.source = result.url;
                                result.destroy();
                            }, Qt.size(fixedIcon.width,fixedIcon.height) );
                        }

                        mainItemContainer.buffersAreReady = true;
                        if(!panel.initializatedBuffers)
                            panel.noInitCreatedBuffers++;

                        iconImageBuffer.opacity = 1;
                    }
                }

                Component{
                    id:tttTimer

                    Timer{
                        id:ttt
                        repeat: true
                        interval: loop <= 1 ? centralItem.shadowInterval : 2500

                        property int loop: 0;

                        onTriggered: {
                            iconImage.updateBuffers();

                            loop++;

                           // console.log(loop+' - '+interval);
                            if(loop > 2)
                              ttt.destroy(300);
                        }

                        Component.onCompleted: ttt.start();
                    }// end of timer

                }//end of component of timer

                Component.onCompleted: {
                    yourImageWithLoadedIconContainer.updateImage();
                }

                Component.onDestruction: {
                    if(yourImageWithLoadedIconContainer.imageTimer)
                        yourImageWithLoadedIconContainer.imageTimer.destroy();
                }
            }
        }

        DropShadow {
            id:shadowImageNoActive
            visible: false
            width: fixedIcon.width
            height: fixedIcon.height
            anchors.centerIn: fixedIcon

            radius: centralItem.shadowSize
            samples: 2 * radius
            color: "#ff080808"
            source: fixedIcon

            verticalOffset: 2
        }

        Component.onDestruction: {
            if(yourImageWithLoadedIconContainer.imageTimer)
                yourImageWithLoadedIconContainer.imageTimer.destroy();
        }
    }
}

