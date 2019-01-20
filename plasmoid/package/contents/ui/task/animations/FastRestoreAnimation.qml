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

///////Restore Zoom Animation/////
SequentialAnimation{
    id: fastRestoreAnimation

    property int speed: 5*mainItemContainer.animationTime

    ScriptAction {
        script: {
            mainItemContainer.inFastRestoreAnimation = true;
            mainItemContainer.inBlockingAnimation = true;
        }
    }

    ParallelAnimation {
        PropertyAnimation {
            target: wrapper
            property: "mScale"
            to: 1
            duration: fastRestoreAnimation.speed
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: wrapper
            property: "tempScaleWidth"
            to: 1
            duration: fastRestoreAnimation.speed
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: wrapper
            property: "tempScaleHeight"
            to: 1
            duration: fastRestoreAnimation.speed
            easing.type: Easing.InCubic
        }
    }

    onStopped: {
        if (!mainItemContainer.containsMouse && !parabolicManager.neighbourIsHovered(itemIndex)) {
            mainItemContainer.inMimicParabolicAnimation = false;
        } else {
            mainItemContainer.inMimicParabolicAnimation = true;
        }

        newWindowAnimation.stop();

        if (!mainItemContainer.containsMouse && !parabolicManager.neighbourIsHovered(itemIndex)) {
            mainItemContainer.inBlockingAnimation = false;
        }

        root.mimicEnterForParabolic();

        mainItemContainer.inFastRestoreAnimation = false;
        // console.log("Fast Restore Animation ended...");
    }

}
