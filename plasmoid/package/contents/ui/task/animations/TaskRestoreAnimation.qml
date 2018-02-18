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
    id: restoreAnimation

    ParallelAnimation{
        PropertyAnimation {
            target: wrapper
            property: "mScale"
            to: 1
            duration: 4 * mainItemContainer.animationTime
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: hiddenSpacerLeft
            property: "nScale"
            to: 0
            duration: 4 * mainItemContainer.animationTime
            easing.type: Easing.InCubic
        }

        PropertyAnimation {
            target: hiddenSpacerRight
            property: "nScale"
            to: 0
            duration: 4 * mainItemContainer.animationTime
            easing.type: Easing.InCubic
        }
    }
    //! debug code based on third task
    /*ScriptAction{
        script: {
            if (index===2) {
                console.log("restore script ended correctly...");
            }
        }
    }

    onStarted: {
        if (index === 2)
            console.log("restore animation started...");
    } */

    onStopped: {
        //! VERY IMPORTANT CODE, fixes a bug when cycling the mouse very fast inside
        //! and outside the dock, in some rare cases the restoreAnimation didnt end!!

        //if (latteDock && (!latteDock.dockContainsMouse || icList.hoveredIndex === -1)) {
        //    wrapper.mScale = 1;
        //}
        // if (index === 2)
        //    console.log("restore animation stopped... ind:"+index+" zoom:"+wrapper.mScale);
    }
}
