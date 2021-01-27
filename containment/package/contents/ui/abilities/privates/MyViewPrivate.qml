/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.core 0.2 as LatteCore

import org.kde.latte.abilities.host 0.1 as AbilityHost

import org.kde.latte.private.containment 0.1 as LatteContainment

AbilityHost.MyView {
    readonly property int itemShadowOpacity: (plasmoid.configuration.shadowOpacity/100) * 255
    readonly property int itemShadowMaxSize: itemShadow.isEnabled ? (0.5*metrics.maxIconSize) * (plasmoid.configuration.shadowSize/100) : 0

    readonly property string itemShadowCurrentColor: {
        if (plasmoid.configuration.shadowColorType === LatteContainment.Types.ThemeColorShadow) {
            var strC = String(theme.textColor);
            return strC.indexOf("#") === 0 ? strC.substr(1) : strC;
        } else if (plasmoid.configuration.shadowColorType === LatteContainment.Types.UserColorShadow) {
            return plasmoid.configuration.shadowColor;
        }

        return "080808"; // default
    }

    function decimalToHex(d, padding) {
        var hex = Number(d).toString(16);
        padding = typeof (padding) === "undefined" || padding === null ? padding = 2 : padding;

        while (hex.length < padding) {
            hex = "0" + hex;
        }

        return hex;
    }
}

