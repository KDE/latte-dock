/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Loader {
    id: indicatorLoader
    anchors.fill: parent

    active: level.bridge && level.bridge.active && (level.isBackground || (level.isForeground && indicators.info.providesFrontLayer))
    sourceComponent: indicators.indicatorComponent

    //! Communications !//

    property Item level

    Connections {
        target: mainArea
        enabled: indicators.info.needsMouseEventCoordinates
        onPressed: level.mousePressed(mouse.x, mouse.y, mouse.button);
        onReleased: level.mouseReleased(mouse.x, mouse.y, mouse.button);
    }
}
