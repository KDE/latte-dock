/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

SequentialAnimation{
    //Ghost animation that acts as a delayer
    PropertyAnimation {
        target: taskItem.parabolicItem
        property: "opacity"
        to: 1
        duration:  50
        easing.type: Easing.InQuad
    }
    //end of ghost animation

    ParallelAnimation{
        PropertyAnimation {
            target: taskItem.parabolicItem
            property: (icList.orientation == Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
            to: taskItem.containsMouse ? 1+2*(taskItem.abilities.parabolic.factor.maxZoom-1) : 1 + (1.5 * (taskItem.abilities.parabolic.factor.maxZoom-1))
            duration: launcherAnimation.speed
            easing.type: Easing.OutQuad
        }

        PropertyAnimation {
            target: taskItem.parabolicItem
            property: (icList.orientation == Qt.Horizontal) ? "tempScaleWidth" : "tempScaleHeight"
            to: 1
            duration: launcherAnimation.speed
            easing.type: Easing.OutQuad
        }
    }

    PropertyAnimation {
        target: taskItem.parabolicItem
        property: (icList.orientation === Qt.Vertical) ? "tempScaleWidth" : "tempScaleHeight"
        to: 1
        duration: 4*launcherAnimation.speed
        easing.type: Easing.OutBounce
    }

    ParallelAnimation{
        PropertyAnimation {
            target: taskItem.parabolicItem
            property: (icList.orientation === Qt.Vertical) ? "tempScaleHeight" : "tempScaleWidth"
            to: 1
            duration: taskItem.abilities.animations.speedFactor.current*launcherAnimation.speed
            easing.type: Easing.OutBounce
        }

        PropertyAnimation {
            target: taskItem.parabolicItem
            property: "mScale"
            to: 1
            duration: taskItem.abilities.animations.speedFactor.current*launcherAnimation.speed
            easing.type: Easing.OutQuad
        }
    }
}
