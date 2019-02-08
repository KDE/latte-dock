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

import QtQuick 2.0
import QtQuick.Layouts 1.3

import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.Label {
    Layout.fillWidth: true
    Layout.topMargin: isFirstSubCategory ? 0 : units.smallSpacing * 2
    Layout.bottomMargin: units.smallSpacing
    horizontalAlignment: Text.AlignHCenter
    opacity: 0.3

    property bool isFirstSubCategory: false
}
