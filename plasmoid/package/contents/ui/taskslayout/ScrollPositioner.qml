/*
 * Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Rectangle {
    id: listViewBase
    x: {
        if (!root.vertical) {
            if (root.userPanelPosition === Latte.Types.Center) {
                return icList.width / 2;
            } else if (root.userPanelPosition === Latte.Types.Left){
                return 0;
            } else if (root.userPanelPosition === Latte.Types.Right){
                return icList.width;
            }
        } else {
            return 0;
        }
    }

    y: {
        if (!root.vertical) {
            return 0;
        } else {
            if (root.userPanelPosition === Latte.Types.Center) {
                return icList.height / 2;
            } else if (root.userPanelPosition === Latte.Types.Top){
                return 0;
            } else if (root.userPanelPosition === Latte.Types.Bottom){
                return icList.height;
            }
        }
    }

    width: !root.vertical ? 1 : scrollableList.width
    height: !root.vertical ? scrollableList.height : 1

    color: "transparent"
    border.width: 1
    border.color: "transparent" //"purple"
}
