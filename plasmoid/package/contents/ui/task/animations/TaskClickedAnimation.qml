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

///////Activate animation/////
SequentialAnimation{
    id: clickedAnimation

    property bool pressed: mainItemContainer.pressed
    property int speed: root.durationTime*units.longDuration
    property real maxMScale: Math.max(1,root.zoomFactor - (root.zoomFactor - 1) / 2)

    ParallelAnimation{
        PropertyAnimation {
            target: brightnessTaskEffect
            property: "brightness"
            to: -0.5
            duration: clickedAnimation.speed
            easing.type: Easing.OutQuad
        }
       /* PropertyAnimation {
            target: wrapper
            property: "mScale"
            to: root.taskInAnimation ? 1 : Math.max(clickedAnimation.maxMScale, wrapper.mScale - (root.zoomFactor - 1) / 2)
            duration: clickedAnimation.speed
            easing.type: Easing.OutQuad
        }*/
    }

    ParallelAnimation{
        PropertyAnimation {
            target: brightnessTaskEffect
            property: "brightness"
            to: 0
            duration: clickedAnimation.speed
            easing.type: Easing.OutQuad
        }
      /*  PropertyAnimation {
            target: wrapper
            property: "mScale"
            to: root.taskInAnimation ? 1 : root.zoomFactor
            duration: clickedAnimation.speed
            easing.type: Easing.OutQuad
        }*/
    }


    onPressedChanged: {
        if( !running && pressed&&
                ((mainItemContainer.lastButtonClicked == Qt.LeftButton)||(mainItemContainer.lastButtonClicked == Qt.MidButton)) ){
            //mainItemContainer.animationStarted();
            start();
        }
    }

    onStopped: {
        if( !mainItemContainer.isDragged){
            //mainItemContainer.animationEnded();
            if(!root.latteDock)
                checkListHovered.startDuration(6*units.longDuration);
        }
    }
}
