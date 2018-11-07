/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

Item{
    function setProperty(appletId, parameter, value) {
        if (parameter === "disableLatteSideColoring") {
            mainCommunicator.disableLatteSideColoring = value;
        } else if (parameter === "disableLatteIconOverlay") {
            mainCommunicator.disableLatteIconOverlay = value;
        }
    }

    function getProperty(appletId, parameter, value) {
        if (parameter === "disableLatteSideColoring") {
            return mainCommunicator.disableLatteSideColoring;
        } else if (parameter === "disableLatteIconOverlay") {
            return mainCommunicator.disableLatteIconOverlay;
        }

        return null;
    }

}
