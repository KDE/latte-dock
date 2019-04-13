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

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

Loader {
    id: indicatorLoader
    anchors.bottom: (plasmoid.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (plasmoid.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (plasmoid.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (plasmoid.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    anchors.horizontalCenter: root.isHorizontal ? parent.horizontalCenter : undefined
    anchors.verticalCenter: root.isVertical ? parent.verticalCenter : undefined

    active: level.bridge && level.bridge.active && (level.isBackground || (level.isForeground && indicators.info.providesFrontLayer))
    sourceComponent: {
        if (!indicators.enabledForApplets && appletItem.communicatorAlias.overlayLatteIconIsActive) {
            return indicators.plasmaStyleComponent;
        }

        return indicators.indicatorComponent;
    }

    width: root.isHorizontal ? appletItem.wrapperAlias.width - 2*appletItem.wrapperAlias.zoomScale*root.lengthExtMargin
                             : appletItem.wrapperAlias.width;

    height: root.vertical ? appletItem.wrapperAlias.height - 2*appletItem.wrapperAlias.zoomScale*root.lengthExtMargin :
                            appletItem.wrapperAlias.height;

    readonly property bool locked: appletItem.lockZoom || root.zoomFactor === 1

    property real visualLockedWidth: root.iconSize + root.internalWidthMargins
    property real visualLockedHeight: root.iconSize + root.internalHeightMargins

    //! Communications !//

    property Item level

    Connections {
        target: appletItem
        enabled: indicators.info.needsMouseEventCoordinates
        onMousePressed: {
            var fixedPos = indicatorLoader.mapFromItem(appletItem, x, y);
            level.mousePressed(Math.round(fixedPos.x), Math.round(fixedPos.y));
        }
    }
}
