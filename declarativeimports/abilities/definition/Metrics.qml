/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import "./metrics" as MetricsTypes

Item {
    id: _metrics
    property int iconSize: 48
    property int maxIconSize: 48
    property int backgroundThickness: 16

    property MetricsTypes.Margin margin: MetricsTypes.Margin{
        tailThickness: 4
        maxTailThickness: 4
        headThickness: 4
        maxHeadThickness: 4
        length: 4
        screenEdge: 0
    }

    property MetricsTypes.MarginsArea marginsArea: MetricsTypes.MarginsArea{
        tailThickness: 8 //margin.thickness + 4
        headThickness: 8 //margin.thickness + 4
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
