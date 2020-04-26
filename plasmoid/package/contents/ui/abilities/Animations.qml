/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.latte.abilities.definitions 0.1 as AbilityDefinition

import org.kde.latte.core 0.2 as LatteCore

AnimationsBase {
    //DEPRECATED
    //C1 A1 property bool animationsEnabled:  -> active
    //C11 A11 property int animationsNeedBothAxis:0 //animations need space in both axes, e.g zooming a task
    //C12 A12 property int animationsNeedLength: 0 // animations need length, e.g. adding a task
    //C13 A13 property int animationsNeedThickness: 0 // animations need thickness, e.g. bouncing animation

    //C8 A8 signal signalAnimationsNeedBothAxis(int value);
    //C9 A9 signal signalAnimationsNeedLength(int value);
    //C10 A10 signal signalAnimationsNeedThickness(int value);

    //C2 A2 property bool thickAnimated: -> hasThicknessAnimation
    //C5 A5 property int appliedDurationTime: -> speedFactor.normal
    //C7 A7 property int durationTime -> speedFactor.current
    //C6 A6 property int animationTime -> duration.proposed

    //C20 A20 shortDuration -> animations.duration.small
    //C21 A21 longDuration -> animations.duration.large

    //parabolic
    //C3 A3 property int animationStep -> hoverPixelSensitivity
    //C4 A4 property int animationsZoomFactor -> minZoomFactor

    readonly property AbilityDefinition.Animations local: AbilityDefinition.Animations{
        active: animations.speedFactor.current !== 0
        hoverPixelSensitivity: 1

        minZoomFactor: speedFactor.current === 0 ? 1 : 1.65

        speedFactor.normal: active ? speedFactor.current : 1.0
        speedFactor.current: plasmoid.configuration.durationTime

        duration.small: LatteCore.Environment.shortDuration
        duration.large: LatteCore.Environment.longDuration
    }
}

