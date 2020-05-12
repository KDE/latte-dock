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

import "./privates" as Ability

Ability.MetricsPrivate {
    //! Signals
    signal iconSizeAnimationEnded();

    //! Public Properties
    iconSize: _iconSize
    maxIconSize: _maxIconSize
    backgroundThickness: background.thickness

    margin.length: fraction.lengthMargin * iconSize
    margin.thickness: marginMinThickness + fraction.thicknessMargin * Math.max(0, _iconSize - marginMinThickness)
    margin.maxThickness: marginMinThickness + fraction.thicknessMargin * Math.max(0, _maxIconSize - marginMinThickness)
    //margin.thickness: fraction.thicknessMargin * iconSize
   // margin.maxThickness: fraction.thicknessMargin * maxIconSize
    margin.screenEdge: (root.screenEdgeMarginEnabled && root.behaveAsPlasmaPanel && !root.editMode)
                       || !root.screenEdgeMarginEnabled
                       || root.hideThickScreenGap ?
                           0 : plasmoid.configuration.screenEdgeMargin

    padding.length: fraction.lengthPadding * iconSize
    padding.lengthApplet: fraction.lengthAppletPadding * iconSize
}
