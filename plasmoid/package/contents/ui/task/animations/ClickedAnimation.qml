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
    property bool pressed: taskItem.pressed
    property int speed: taskItem.abilities.animations.speedFactor.current * taskItem.abilities.animations.duration.large

    PropertyAnimation {
        target: brightnessTaskEffect
        property: "brightness"
        to: -0.5
        duration: clickedAnimation.speed
        easing.type: Easing.OutQuad
    }

    PropertyAnimation {
        target: brightnessTaskEffect
        property: "brightness"
        to: 0
        duration: clickedAnimation.speed
        easing.type: Easing.OutQuad
    }

    onPressedChanged: {
        if(!running && pressed && !taskItem.abilities.indicators.info.providesClickedAnimation &&
                ((taskItem.lastButtonClicked == Qt.LeftButton)||(taskItem.lastButtonClicked === Qt.MidButton)) ){
            start();
        }
    }
}
