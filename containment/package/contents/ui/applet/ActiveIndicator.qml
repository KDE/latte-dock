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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item{
    id:glowFrame
    width: !root.isVertical ? parent.width : size
    height: root.isVertical ? parent.height : size

    property int size: root.statesLineSize

    /*Rectangle{
        anchors.fill: parent
        border.width: 1
        border.color: "yellow"
        color: "transparent"
        opacity:0.6
    }*/

    GlowPoint{
        id:activePoint
        anchors.centerIn: parent

        basicColor: theme.buttonFocusColor
        roundCorners: true

        opacity:{
            if ( (!vertical && width === glowFrame.size)
                    || (vertical && height === glowFrame.size))
                return 0;

            return 1;
        }

        property bool isActive: (applet.status >= PlasmaCore.Types.NeedsAttentionStatus && applet.status !== PlasmaCore.Types.HiddenStatus)

        property bool vertical: root.isVertical

        property int animationTime: root.durationTime * (2.7*units.longDuration)
        property int stateWidth: glowFrame.width
        property int stateHeight: glowFrame.height

        property real scaleFactor: wrapper.zoomScale

        function updateInitialSizes(){
            if(glowFrame){
                if(vertical)
                    width = glowFrame.size;
                else
                    height = glowFrame.size;

                if(vertical && isActive)
                    height = stateHeight;
                else
                    height = glowFrame.size;

                if(!vertical && isActive)
                    width = stateWidth;
                else
                    width = glowFrame.size;
            }
        }


        onIsActiveChanged: {
            // if(mainItemContainer.hasActive || windowsPreviewDlg.visible)
            activeAndReverseAnimation.start();
        }

        onScaleFactorChanged: {
            if(!activeAndReverseAnimation.running && !root.vertical && isActive){
                width = stateWidth;
            }
            else if (!activeAndReverseAnimation.running && root.vertical && isActive){
                height = stateHeight;
            }
        }

        onStateWidthChanged:{
            if(!activeAndReverseAnimation.running && !vertical && isActive)
                width = stateWidth;
        }

        onStateHeightChanged:{
            if(!activeAndReverseAnimation.running && vertical && isActive)
                height = stateHeight;
        }

        onVerticalChanged: updateInitialSizes();

        Component.onCompleted: {
            updateInitialSizes();

            root.onIconSizeChanged.connect(updateInitialSizes);
        }

        NumberAnimation{
            id: activeAndReverseAnimation
            target: activePoint
            property: root.isVertical ? "height" : "width"
            to: activePoint.isActive ? (root.isVertical ? activePoint.stateHeight : activePoint.stateWidth) : glowFrame.size
            duration: activePoint.animationTime
            easing.type: Easing.InQuad

            onStopped: activePoint.updateInitialSizes()
        }
    }

    anchors.rightMargin: plasmoid.location === PlasmaCore.Types.LeftEdge ? root.thickMarginBase : 0
    anchors.leftMargin: plasmoid.location === PlasmaCore.Types.RightEdge ? root.thickMarginBase : 0
    anchors.topMargin: plasmoid.location === PlasmaCore.Types.BottomEdge ? root.thickMarginBase : 0
    anchors.bottomMargin: plasmoid.location === PlasmaCore.Types.TopEdge ? root.thickMarginBase : 0

    states: [
        State {
            name: "left"
            when: ((plasmoid.location === PlasmaCore.Types.LeftEdge && !root.reverseLinesPosition) ||
                   (plasmoid.location === PlasmaCore.Types.RightEdge && root.reverseLinesPosition))

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:parent.verticalCenter; horizontalCenter:undefined;
                    top:undefined; bottom:undefined; left:undefined; right:parent.left;}
            }
        },
        State {
            name: "bottom"
            when: ((plasmoid.location === PlasmaCore.Types.BottomEdge && !root.reverseLinesPosition) ||
                   (plasmoid.location === PlasmaCore.Types.TopEdge && root.reverseLinesPosition))

            AnchorChanges {
                target: glowFrame
                anchors{ verticalCenter:undefined; horizontalCenter:parent.horizontalCenter;
                    top:parent.bottom; bottom:undefined; left:undefined; right:undefined;}
            }
        }
    ]
}


