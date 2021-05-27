/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.abilities.items 0.1 as AbilityItem

AbilityItem.IndicatorLevel {
    id: indicatorLevel
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
                   && !appletItem.isMarginsAreaSeparator
                   && !appletItem.isHidden
                   && !(appletItem.isAutoFillApplet && !communicator.indexerIsSupported) //! simple Plasma AutoFill applets
                   && !communicator.indexerIsSupported
                   && level.indicator.host.isEnabled
                   && (appletItem.communicator.requires.activeIndicatorEnabled
                       || (!appletItem.communicator.requires.activeIndicatorEnabled && appletItem.debug.graphicsEnabled /*debug paddings*/)
                       && level.indicator.host.info.enabledForApplets)

    readonly property bool locked: appletItem.lockZoom || appletItem.parabolic.factor.zoom === 1

    //! Qt.min() is used to make sure that indicators always take into account the current applet length provided
    //! and as such always look centered even when applet are aligned to length screen edge
    property real visualLockedWidth: Math.min(appletItem.metrics.iconSize, appletWrapper.width) + appletItem.internalWidthMargins
    property real visualLockedHeight: Math.min(appletItem.metrics.iconSize, appletWrapper.height) + appletItem.internalHeightMargins

    Connections {
        target: appletItem
        enabled: indicatorLevel.level.indicator.host.isEnabled && indicatorLevel.level.indicator.host.info.needsMouseEventCoordinates
        onMousePressed: {
            var fixedPos = indicatorLevel.mapFromItem(appletItem, x, y);
            indicatorLevel.level.mousePressed(Math.round(fixedPos.x), Math.round(fixedPos.y), button);
        }

        onMouseReleased: {
            var fixedPos = indicatorLevel.mapFromItem(appletItem, x, y);
            indicatorLevel.level.mouseReleased(Math.round(fixedPos.x), Math.round(fixedPos.y), button);
        }
    }
}
