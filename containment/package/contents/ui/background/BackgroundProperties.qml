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

import "./types" as BackgroundTypes
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item{
    property bool isShown: false

    property bool hasAllBorders: false
    property bool hasLeftBorder: false
    property bool hasRightBorder: false
    property bool hasTopBorder: false
    property bool hasBottomBorder: false
    readonly property bool hasBothLengthShadows:{
        if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            return (hasTopBorder && hasBottomBorder);
        }

        return (hasLeftBorder && hasRightBorder);
    }

    property int offset: 0
    property int length: 0
    property int thickness: 0
    property int screenEdgeMargin: 0
    property real currentOpacity: 0

    readonly property BackgroundTypes.Shadows shadows: BackgroundTypes.Shadows{}
    readonly property BackgroundTypes.Paddings paddings: BackgroundTypes.Paddings{}
    readonly property BackgroundTypes.Totals totals: BackgroundTypes.Totals{}
}
