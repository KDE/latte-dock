/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
                ((taskItem.lastButtonClicked === Qt.LeftButton)||(taskItem.lastButtonClicked === Qt.MidButton)) ){
            start();
        }
    }
}
