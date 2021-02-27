/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0

import "./metrics" as MetricsTypes

Item {
    id: _metrics
    property int iconSize: 48
    property int maxIconSize: 48
    property int backgroundThickness: 16

    property MetricsTypes.Margin margin: MetricsTypes.Margin{
        thickness: 4
        maxThickness: 4
        length: 4
        screenEdge: 0
    }

    property MetricsTypes.MarginsArea marginsArea: MetricsTypes.MarginsArea{
        marginThickness: 8 //margin.thickness + 4
        //readonly property int iconSize: _metrics.totals.thickness - thicknessEdges
        //readonly property int thicknessEdges: 2*marginThickness
    }

    property MetricsTypes.Padding padding: MetricsTypes.Padding{
        length: 0
        lengthApplet: 0
    }

    property MetricsTypes.Totals totals: MetricsTypes.Totals{
        //readonly property int length: iconSize + lengthEdges
        //readonly property int lengthEdge: margin.length + padding.length
        //readonly property int lengthEdges: 2 * lengthEdge
        //readonly property int lengthPaddings: 2 * padding.length
        //readonly property int marginsAreaThicknessEdges: 2 * margin.marginsAreaThickness
        //readonly property int thickness: iconSize + thicknessEdges
        //readonly property int thicknessEdges: 2 * margin.thickness
    }

    property MetricsTypes.Mask mask: MetricsTypes.Mask{
        screenEdge: 0
        maxScreenEdge: 0

        thickness {
            hidden: 1
            normal: 48
            medium: 48
            zoomed: 48

            maxNormal: 48
            maxMedium: 48
            maxZoomed: 48

            normalForItems: 48
            mediumForItems: 48
            zoomedForItems: 48

            maxNormalForItems: 48
            maxMediumForItems: 48
            maxZoomedForItems: 48

            maxNormalForItemsWithoutScreenEdge: 48
            maxZoomedForItemsWithoutScreenEdge: 48
        }
    }
}
