/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

Rectangle {
    id: listViewBase
    x: {
        if (!root.vertical) {
            if (root.alignment === LatteCore.Types.Center) {
                //! this way we get a perfectly centered offset value
                return (icList.width-1) / 2;
            } else if (root.alignment === LatteCore.Types.Left){
                return 0;
            } else if (root.alignment === LatteCore.Types.Right){
                //! this way we get a perfectly right aligned offset value
                return (icList.width-1);
            }
        }

        return 0;
    }

    y: {
        if (root.vertical) {
            if (root.alignment === LatteCore.Types.Center) {
                //! this way we get a perfectly centered offset value
                return (icList.height-1) / 2;
            } else if (root.alignment === LatteCore.Types.Top){
                return 0;
            } else if (root.alignment === LatteCore.Types.Bottom){
                //! this way we get a perfectly bottom aligned offset value
                return (icList.height-1);
            }
        }

        return 0;
    }

    width: !root.vertical ? 1 : scrollableList.width
    height: !root.vertical ? scrollableList.height : 1

    color: "transparent"
    border.width: 1
    border.color: "transparent" //"purple"
}
