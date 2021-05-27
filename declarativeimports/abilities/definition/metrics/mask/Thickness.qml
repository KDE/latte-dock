/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Item {
    property int hidden: 1
    property int normal: 48
    property int medium: 48
    property int zoomed: 48

    property int maxNormal: 48
    property int maxMedium: 48
    property int maxZoomed: 48

    property int normalForItems: 48
    property int mediumForItems: 48
    property int zoomedForItems: 48

    property int maxNormalForItems: 48
    property int maxMediumForItems: 48
    property int maxZoomedForItems: 48

    property int maxNormalForItemsWithoutScreenEdge: 48
    property int maxZoomedForItemsWithoutScreenEdge: 48
}
