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

import org.kde.latte 0.2 as Latte

Loader{
    id: indicatorLoader
    anchors.fill: parent

    active: bridge && bridge.active && (isBackground || (isForeground && indicators.info.providesFrontLayer))
    sourceComponent: {
        if (!indicators.enabledForApplets && appletItem.communicatorAlias.overlayLatteIconIsActive) {
            return indicators.plasmaStyleComponent;
        }

        return indicators.indicatorComponent;
    }

    property bool isBackground: true
    readonly property bool isForeground: !isBackground
    property Item bridge
}
