/*
*  Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
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
    readonly property string styleName: "Latte"

    readonly property int activeStyle: 0 //LineIndicator
    readonly property real size: 0.10
    readonly property bool minimizedTaskColoredDifferently: false
    readonly property bool extraDotOnActive: false
    readonly property bool glowEnabled: false
    readonly property bool enabledForApplets: true
    readonly property int glowApplyTo: 2 /*All*/
    readonly property bool glow3D: true
    readonly property real glowOpacity: 0.35
    readonly property real lengthPadding: 0.08
    readonly property real thickMargin: 0.0
    readonly property bool reversed: false
}
