/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.core 0.2 as LatteCore

import "./privates" as Ability

Ability.MetricsPrivate {
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

    //! Mask
    mask.maxScreenEdge : root.behaveAsDockWithMask ? Math.max(0, plasmoid.configuration.screenEdgeMargin) : 0
    //! window geometry is updated after the local screen margin animation was zeroed*/
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
    mask.thickness.zoomedForItems: margin.screenEdge + (parabolic.factor.maxZoom * totals.thickness)

    mask.thickness.maxNormalForItemsWithoutScreenEdge: maxIconSize + (margin.maxThickness * 2)
    mask.thickness.maxZoomedForItemsWithoutScreenEdge: (maxIconSize + (margin.maxThickness * 2)) * parabolic.factor.maxZoom

    mask.thickness.maxNormalForItems: mask.maxScreenEdge + mask.thickness.maxNormalForItemsWithoutScreenEdge
    mask.thickness.maxZoomedForItems: mask.maxScreenEdge + mask.thickness.maxZoomedForItemsWithoutScreenEdge

    //! Padding
    padding.length: fraction.lengthPadding * iconSize
    padding.lengthApplet: fraction.lengthAppletPadding * iconSize
}
