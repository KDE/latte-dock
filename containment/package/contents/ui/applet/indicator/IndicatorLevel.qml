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

import org.kde.latte.abilities.items 0.1 as AbilityItem

AbilityItem.IndicatorLevel {
    id: indicatorLoader
    anchors.bottom: (plasmoid.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : undefined
    anchors.top: (plasmoid.location === PlasmaCore.Types.TopEdge) ? parent.top : undefined
    anchors.left: (plasmoid.location === PlasmaCore.Types.LeftEdge) ? parent.left : undefined
    anchors.right: (plasmoid.location === PlasmaCore.Types.RightEdge) ? parent.right : undefined

    anchors.horizontalCenter: root.isHorizontal ? parent.horizontalCenter : undefined
    anchors.verticalCenter: root.isVertical ? parent.verticalCenter : undefined

    width: {
        if (root.isHorizontal) {
            if (appletItem.parabolicEffectIsSupported) {
                return appletItem.wrapper.zoomScale * visualLockedWidth;
            }

            return appletWrapper.width + appletItem.internalWidthMargins;
        } else {
            return appletItem.wrapper.width;
        }
    }

    height: {
        if (root.isVertical) {
            if (appletItem.parabolicEffectIsSupported) {
                return appletItem.wrapper.zoomScale * visualLockedHeight;
            }

            return appletWrapper.height + appletItem.internalHeightMargins;
        } else {
            return appletItem.wrapper.height;
        }
    }

    level.isDrawn: !appletItem.isSeparator
                   && !appletItem.isHidden
                   && !communicator.indexerIsSupported
                   && (indicatorsHost.isEnabled
                       && (appletItem.communicator.requires.activeIndicatorEnabled
                           || (!appletItem.communicator.requires.activeIndicatorEnabled && appletItem.debug.graphicsEnabled /*debug paddings*/))
                       && indicatorsHost.info.enabledForApplets)

    readonly property bool locked: appletItem.lockZoom || appletItem.parabolic.factor.zoom === 1

    //! Qt.min() is used to make sure that indicators always take into account the current applet length provided
    //! and as such always look centered even when applet are aligned to length screen edge
    property real visualLockedWidth: Math.min(appletItem.metrics.iconSize, appletWrapper.width) + appletItem.internalWidthMargins
    property real visualLockedHeight: Math.min(appletItem.metrics.iconSize, appletWrapper.height) + appletItem.internalHeightMargins
}
