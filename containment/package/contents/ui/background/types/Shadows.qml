/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

QtObject{
    property int left: 0
    property int right: 0
    property int top: 0
    property int bottom: 0

    property int fixedLeft: 0
    property int fixedRight: 0
    property int fixedTop: 0
    property int fixedBottom: 0

    readonly property int tailThickness: {
        if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            return left;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            return right;
        } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            return top;
        }

        return bottom;
    }

    readonly property int headThickness: {
        if (!backgroundShadowsInRegularStateEnabled || root.behaveAsPlasmaPanel) {
            return 0;
        }

        if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            return fixedRight;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            return fixedLeft;
        } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            return fixedBottom;
        }

        return fixedTop;
    }
}
