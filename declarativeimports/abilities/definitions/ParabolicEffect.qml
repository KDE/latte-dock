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

import "./paraboliceffect" as ParabolicEffectTypes

Item {
    property ParabolicEffectTypes.Factor factor: ParabolicEffectTypes.Factor{}
    property bool restoreZoomIsBlocked: false

    readonly property ParabolicEffectTypes.PrivateProperties _privates: ParabolicEffectTypes.PrivateProperties {}

    signal sglClearZoom();
    signal sglUpdateLowerItemScale(int delegateIndex, real newScale, real step);
    signal sglUpdateHigherItemScale(int delegateIndex, real newScale, real step);
}
