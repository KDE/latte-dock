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
import org.kde.plasma.core 2.0 as PlasmaCore

QtObject{
    property int visualThickness: 0
    property int visualLength: 0

    readonly property int shadowsLength: {
        if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
            return shadows.left+shadows.right;
        } else {
            return shadows.top+shadows.bottom;
        }
    }

    readonly property int shadowsThickness: {
        if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
            return shadows.top+shadows.bottom;
        } else {
            return shadows.left+shadows.right;
        }
    }

    readonly property int paddingsLength: {
        if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
            return paddings.left+paddings.right;
        } else {
            return paddings.top+paddings.bottom;
        }
    }

    readonly property int minThickness: paddings.headThickness + paddings.tailThickness;
}
