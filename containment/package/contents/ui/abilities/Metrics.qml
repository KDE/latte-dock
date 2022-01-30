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
    margin.tailThickness: marginMinThickness + fraction.thicknessMargin * Math.max(0, _iconSize - marginMinThickness)
    margin.maxTailThickness: marginMinThickness + fraction.thicknessMargin * Math.max(0, _maxIconSize - marginMinThickness)
    margin.headThickness: (background.isGreaterThanItemThickness ? (background.totals.visualThickness - _iconSize - margin.tailThickness) : margin.tailThickness)
    margin.maxHeadThickness: (background.isGreaterThanItemThickness ? (background.totals.visualMaxThickness - _maxIconSize - margin.maxTailThickness) : margin.maxTailThickness)
    //margin.thickness: fraction.thicknessMargin * iconSize
   // margin.maxThickness: fraction.thicknessMargin * maxIconSize
    margin.screenEdge: (root.screenEdgeMarginEnabled && root.behaveAsPlasmaPanel)
                       || !root.screenEdgeMarginEnabled
                       || root.hideThickScreenGap ?
                           0 : plasmoid.configuration.screenEdgeMargin

    //! MarginsAra
    marginsArea.tailThickness: {
        if (!themeExtended) {
            return metrics.margin.tailThickness;
        }

        var areamargin = 0;
        if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            areamargin = themeExtended.marginsAreaTop;
        } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            areamargin = themeExtended.marginsAreaLeft;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            areamargin = themeExtended.marginsAreaRight;
        } else {
            areamargin = themeExtended.marginsAreaBottom;
        }

        return areamargin + metrics.margin.tailThickness;
    }

    marginsArea.headThickness: {
        if (!themeExtended) {
            return metrics.margin.headThickness;
        }

        var areamargin = 0;
        if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            areamargin = themeExtended.marginsAreaBottom;
        } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            areamargin = themeExtended.marginsAreaRight;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            areamargin = themeExtended.marginsAreaLeft;
        } else {
            areamargin = themeExtended.marginsAreaTop;
        }

        return areamargin + metrics.margin.headThickness;
    }

    //! Mask
    mask.maxScreenEdge : root.behaveAsDockWithMask ? Math.max(0, plasmoid.configuration.screenEdgeMargin) : 0
      // window geometry is updated after the local screen margin animation was zeroed*/
    mask.screenEdge: (!root.screenEdgeMarginEnabled || root.hideThickScreenGap) ? 0 : plasmoid.configuration.screenEdgeMargin

    mask.thickness.hidden: LatteCore.WindowSystem.compositingActive ?  2 : 1
    mask.thickness.normal: mask.screenEdge + Math.max(totals.thickness + extraThicknessForNormal, background.thickness + background.shadows.headThickness)
    mask.thickness.medium: mask.screenEdge + mediumFactor*(iconSize+extraThicknessForZoomed) + mediumMarginsFactor*totals.thicknessEdges
    mask.thickness.zoomed: mask.screenEdge + ((iconSize+extraThicknessForZoomed) * parabolic.factor.maxZoom) + maxMarginsFactor*totals.thicknessEdges + 2
    mask.thickness.maxNormal: mask.maxScreenEdge + maxIconSize + (margin.maxTailThickness + margin.maxHeadThickness) + extraThicknessForNormal
    mask.thickness.maxMedium: mask.maxScreenEdge + Math.max(mask.thickness.maxNormalForItems, extraThicknessForNormal + (mediumFactor*maxIconSize)+(mediumMarginsFactor*margin.tailThickness))
    mask.thickness.maxZoomed: mask.maxScreenEdge + Math.max( ((maxIconSize*parabolic.factor.maxZoom) + ((margin.maxTailThickness + margin.maxHeadThickness)*maxMarginsFactor)) + extraThicknessForZoomed,
                                                                    background.thickness + background.shadows.headThickness)

    mask.thickness.normalForItems: margin.screenEdge + totals.thickness
    mask.thickness.zoomedForItems: margin.screenEdge + (parabolic.factor.zoom * iconSize) + (parabolic.factor.marginThicknessZoom * totals.thicknessEdges)

    mask.thickness.maxNormalForItemsWithoutScreenEdge: maxIconSize + margin.maxTailThickness + margin.maxHeadThickness
    mask.thickness.maxZoomedForItemsWithoutScreenEdge: maxIconSize*parabolic.factor.maxZoom + (margin.maxTailThickness + margin.maxHeadThickness)*maxMarginsFactor

    mask.thickness.maxNormalForItems: mask.maxScreenEdge + mask.thickness.maxNormalForItemsWithoutScreenEdge
    mask.thickness.maxZoomedForItems: mask.maxScreenEdge + mask.thickness.maxZoomedForItemsWithoutScreenEdge

    //! Padding
    padding.length: fraction.lengthPadding * iconSize
    padding.lengthApplet: fraction.lengthAppletPadding * iconSize
}
