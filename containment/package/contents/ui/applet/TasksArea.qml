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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

import "../" as RootElements

Rectangle{
    anchors.fill: parent
    radius: root.iconSize/10
    opacity: root.addLaunchersMessage ? 1 : 0

    color: Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b, 0.75)
    border.width: 1
    border.color: outlineColor

    readonly property color outlineColorBase: theme.backgroundColor
    readonly property real outlineColorBaseBrightness: ColorizerTools.colorBrightness(outlineColorBase)
    readonly property color outlineColor: {
        if (outlineColorBaseBrightness > 127.5) {
            return Qt.darker(outlineColorBase, 1.5);
        } else {
            return Qt.lighter(outlineColorBase, 2.2);
        }
    }

    Behavior on opacity{
        NumberAnimation { duration: 2*root.durationTime*appletItem.animationTime }
    }

    PlasmaExtras.Heading {
        id: heading
        width: parent.width
        height: parent.height * 0.4

        text: i18n("Tasks Area")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        fontSizeMode: Text.Fit
        color: theme.textColor

        rotation: {
            if (root.isHorizontal)
                return 0;
            else if (plasmoid.location === PlasmaCore.Types.LeftEdge)
                return -90;
            else if (plasmoid.location === PlasmaCore.Types.RightEdge)
                return 90;
        }
    }

    RootElements.AddWidgetVisual {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: heading.bottom
        anchors.topMargin: units.smallSpacing

        width: height
        height: (parent.height * 0.5) - 2 * units.smallSpacing
    }
}
