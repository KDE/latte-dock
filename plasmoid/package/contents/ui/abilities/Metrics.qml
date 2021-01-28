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

import org.kde.latte.abilities.client 0.1 as AbilityClient

AbilityClient.Metrics {
    //! Public Local Properties

    local.iconSize: {
        if (inPlasmaDesktop) {
            return maxIconSizeInPlasma;
        } else if (inPlasmaPanel) {
            return Math.max(16, panelThickness - 2*margin.thickness);
        }

        return maxIconSizeInPlasma;
    }

    local.backgroundThickness: totals.thickness

    local.margin.length: 0.1 * iconSize
    local.margin.thickness: {
        if (inPlasmaDesktop) {
            return 0.16 * iconSize;
        }

        return Math.max(2, (panelThickness - maxIconSizeInPlasma) / 2);
    }
    local.margin.screenEdge: 0
    local.padding.length: 0.04 * iconSize

    local.mask.thickness.normalForItems: local.totals.thickness
    local.mask.thickness.zoomedForItems: parabolic.factor.zoom * local.totals.thickness

    local.mask.thickness.maxNormalForItems: local.mask.thickness.normalForItems
    local.mask.thickness.maxZoomedForItems: local.mask.thickness.zoomedForItems

    readonly property int maxIconSizeInPlasma: Math.max(plasmoid.configuration.iconSize, 16)
    readonly property int panelThickness: (root.vertical ? root.width : root.height)
}
