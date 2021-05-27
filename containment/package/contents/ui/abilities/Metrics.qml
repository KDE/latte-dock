/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

import "./privates" as Ability

Ability.MetricsPrivate {
    id: metrics
    //! Signals
    signal iconSizeAnimationEnded();

    //! Public Properties
    iconSize: _iconSize
    maxIconSize: _maxIconSize
    backgroundThickness: background.thickness

    //! Margin
    margin.length: fraction.lengthMargin * iconSize
    margin.thickness: marginMinThickness + fraction.thicknessMargin * Math.max(0, _iconSize - marginMinThickness)
    margin.maxThickness: marginMinThickness + fraction.thicknessMargin * Math.max(0, _maxIconSize - marginMinThickness)
    //margin.thickness: fraction.thicknessMargin * iconSize
   // margin.maxThickness: fraction.thicknessMargin * maxIconSize
    margin.screenEdge: (root.screenEdgeMarginEnabled && root.behaveAsPlasmaPanel)
                       || !root.screenEdgeMarginEnabled
                       || root.hideThickScreenGap ?
                           0 : plasmoid.configuration.screenEdgeMargin

    //! MarginsAra
    marginsArea.marginThickness: {
        if (!themeExtended) {
            return metrics.margin.thickness;
        }

        if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            return themeExtended.marginsAreaBottom + metrics.margin.thickness;
        } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            return themeExtended.marginsAreaRight + metrics.margin.thickness;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            return themeExtended.marginsAreaLeft + metrics.margin.thickness;
        }

        return themeExtended.marginsAreaTop + metrics.margin.thickness;
    }

    //! Mask
    mask.maxScreenEdge : root.behaveAsDockWithMask ? Math.max(0, plasmoid.configuration.screenEdgeMargin) : 0
      // window geometry is updated after the local screen margin animation was zeroed*/
    mask.screenEdge: (!root.screenEdgeMarginEnabled || root.hideThickScreenGap) ? 0 : plasmoid.configuration.screenEdgeMargin

    mask.thickness.hidden: LatteCore.WindowSystem.compositingActive ?  2 : 1
    mask.thickness.normal: mask.screenEdge + Math.max(totals.thickness + extraThicknessForNormal, background.thickness + background.shadows.headThickness)
    mask.thickness.medium: mask.screenEdge + (1 + (0.65 * (parabolic.factor.maxZoom-1)))*(totals.thickness+extraThicknessForZoomed)
    mask.thickness.zoomed: mask.screenEdge + ((totals.thickness+extraThicknessForZoomed) * parabolic.factor.maxZoom) + 2
    mask.thickness.maxNormal: mask.maxScreenEdge + maxIconSize + (margin.maxThickness * 2) + extraThicknessForNormal
    mask.thickness.maxMedium: mask.maxScreenEdge + Math.max(mask.thickness.maxNormalForItems, extraThicknessForNormal + (1 + (0.65 * (parabolic.factor.maxZoom-1)))*(maxIconSize+margin.maxThickness))
    mask.thickness.maxZoomed: mask.maxScreenEdge + Math.max( ((maxIconSize+(margin.maxThickness * 2)) * parabolic.factor.maxZoom) + extraThicknessForZoomed,
                                                                    background.thickness + background.shadows.headThickness)

    mask.thickness.normalForItems: margin.screenEdge + totals.thickness
    mask.thickness.zoomedForItems: margin.screenEdge + (parabolic.factor.zoom * totals.thickness)

    mask.thickness.maxNormalForItemsWithoutScreenEdge: maxIconSize + (margin.maxThickness * 2)
    mask.thickness.maxZoomedForItemsWithoutScreenEdge: (maxIconSize + (margin.maxThickness * 2)) * parabolic.factor.maxZoom

    mask.thickness.maxNormalForItems: mask.maxScreenEdge + mask.thickness.maxNormalForItemsWithoutScreenEdge
    mask.thickness.maxZoomedForItems: mask.maxScreenEdge + mask.thickness.maxZoomedForItemsWithoutScreenEdge

    //! Padding
    padding.length: fraction.lengthPadding * iconSize
    padding.lengthApplet: fraction.lengthAppletPadding * iconSize
}
