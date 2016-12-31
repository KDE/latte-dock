import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import QtGraphicalEffects 1.0

Image{
    id: editVisual
    width: root.isHorizontal ? parent.width : visibilityManager.thicknessNormal
    height: root.isVertical ? parent.height : visibilityManager.thicknessNormal

    fillMode: Image.Tile
    source: "../icons/blueprint.jpg"
    opacity: 0

    property int location: plasmoid.location
    property int speed: root.durationTime*4*units.longDuration
    property int thickness: visibilityManager.thicknessNormal + shadowSize
    property int rootThickness: root.isHorizontal ? root.height : root.width
    property int shadowSize : Math.ceil(root.iconSize / 5)

    property bool animationSent: false
    property bool farEdge: (plasmoid.location===PlasmaCore.Types.BottomEdge) || (plasmoid.location===PlasmaCore.Types.RightEdge)
    property bool editAnimationEnded: false

    layer.enabled: true
    layer.effect: DropShadow {
        radius: editVisual.shadowSize
        samples: 2 * radius
        color: "#ee080808"
    }

    onLocationChanged:{
        initializeEditPosition();
    }

    function initializeNormalPosition() {
        if (location === PlasmaCore.Types.BottomEdge) {
            x = 0;
            y = editVisual.rootThickness;
        } else if (location === PlasmaCore.Types.RightEdge) {
            x = editVisual.rootThickness;
            y = 0;
        } else if (location === PlasmaCore.Types.LeftEdge) {
            x = -editVisual.thickness;
            y = 0;
        } else if (location === PlasmaCore.Types.TopEdge) {
            x = 0;
            y = -editVisual.thickness;
        }
    }

    function initializeEditPosition() {
        if (root.editMode) {
            if ((location === PlasmaCore.Types.LeftEdge) || (location === PlasmaCore.Types.TopEdge)){
                x = 0;
                y = 0;
            } else if (location === PlasmaCore.Types.BottomEdge) {
                x = 0;
                y = rootThickness - thickness + shadowSize;
            } else if (location === PlasmaCore.Types.RightEdge) {
                x = rootThickness - thickness + shadowSize;
                y = 0;
            }
        }
    }


    //////////// States ////////////////////

    states: [
        State{
            name: "*"
            when:  !root.editMode
        },

        State{
            name: "edit"
            when: root.editMode
        }
    ]

    transitions: [
        Transition{
            id: editTransition
            from: "*"
            to: "edit"

            SequentialAnimation{
                ScriptAction{
                    script:{
                        editVisual.opacity = 0

                        initializeNormalPosition();

                        if(!animationSent) {
                            animationSent = true;
                            var anim = root.animationsNeedLength+1;
                            root.slotAnimationsNeedLength(anim);
                        }
                    }
                }

                ParallelAnimation{
                    PropertyAnimation {
                        target: editVisual
                        property: "opacity"
                        to: 0.6
                        duration: editVisual.speed / 2
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: editVisual
                        property: root.isHorizontal ? "y" : "x"
                        to: editVisual.farEdge ? editVisual.rootThickness - editVisual.thickness + editVisual.shadowSize : 0
                        duration: editVisual.speed
                        easing.type: Easing.OutBounce


                    }
                }

                ScriptAction{
                    script:{
                        editVisual.editAnimationEnded = true;
                        visibilityManager.updateMaskArea();
                    }
                }
            }
        },
        Transition{
            from: "edit"
            to: "*"
            SequentialAnimation{
                ParallelAnimation{
                    PropertyAnimation {
                        target: editVisual
                        property: root.isHorizontal ? "y" : "x"
                        to: editVisual.farEdge ? editVisual.rootThickness : -editVisual.thickness
                        duration: editVisual.speed
                        easing.type: Easing.OutBounce
                    }
                    PropertyAnimation {
                        target: editVisual
                        property: "opacity"
                        to: 0
                        duration: editVisual.speed
                        easing.type: Easing.OutQuad
                    }
                }

                ScriptAction{
                    script:{
                        editVisual.editAnimationEnded = false;
                        if (editVisual.animationSent) {
                            var anim = root.animationsNeedLength-1;
                            root.slotAnimationsNeedLength(anim);
                            editVisual.animationSent = false;
                        }
                    }
                }
            }

        }
    ]
}
