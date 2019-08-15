/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    readonly property bool isCompiled: !colorizedAppletShadow.active
                                       || (colorizedAppletShadow.active && colorizedAppletShadow.item.status === ShaderEffect.Compiled)

    ColorOverlay {
        id: colorizer
        anchors.fill: parent
        color: colorizerManager.applyColor
        source: wrapper
    }

    ///Shadow in applets
    Loader{
        id: colorizedAppletShadow
        anchors.fill: colorizer

        active: (plasmoid.configuration.shadows >= 1) && (appletColorizer.opacity>0)

        sourceComponent: DropShadow{
            anchors.fill: parent
            color: root.appShadowColor
            fast: true
            samples: 2 * radius
            source: colorizer
            radius: shadowSize
            verticalOffset: forcedShadow ? 0 : 2

            readonly property int shadowSize : root.appShadowSize

            readonly property bool forcedShadow: root.forceTransparentPanel && plasmoid.configuration.shadows>0
                                                 && applet && applet.pluginName !== root.plasmoidName ? true : false
        }
    }
}
