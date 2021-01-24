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

import org.kde.latte.abilities.items 0.1 as AbilityItem

AbilityItem.IndicatorObject{
    id: indicatorBridge
    animations: appletItem.animations
    metrics: appletItem.metrics
    indicatorsHost: indicators

    isApplet: true

    isActive: appletItem.isActive
    isHovered: appletItem.containsMouse
    isSquare: appletItem.isSquare

    hasActive: isActive

    scaleFactor: appletItem.wrapper.zoomScale
    panelOpacity: root.background.currentOpacity
    shadowColor: root.appShadowColorSolid

    palette: colorizerManager.applyTheme

    //!icon colors
    iconBackgroundColor: isSquare ? appletItem.wrapper.overlayIconLoader.backgroundColor : colorizerManager.buttonFocusColor
    iconGlowColor: isSquare ? appletItem.wrapper.overlayIconLoader.glowColor : colorizerManager.focusGlowColor
}
