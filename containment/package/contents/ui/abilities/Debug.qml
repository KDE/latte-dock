/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.abilities.containers 0.1 as ContainerAbility
import org.kde.latte.abilities.definitions 0.1 as AbilityDefinition

ContainerAbility.Debug {
    graphicsEnabled: Qt.application.arguments.indexOf("--graphics")>=0
    inputMaskEnabled: Qt.application.arguments.indexOf("--input")>=0
    layouterEnabled: Qt.application.arguments.indexOf("--layouter")>=0
    localGeometryEnabled: Qt.application.arguments.indexOf("--localgeometry")>=0
    maskEnabled: Qt.application.arguments.indexOf("--mask") >= 0
    overloadedIconsEnabled: Qt.application.arguments.indexOf("--overloaded-icons")>=0
    spacersEnabled: Qt.application.arguments.indexOf("--spacers")>=0
    timersEnabled: Qt.application.arguments.indexOf("--timers")>=0
    windowEnabled: Qt.application.arguments.indexOf("--with-window")>=0
}
