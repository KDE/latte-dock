/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Item {
    readonly property int length: iconSize + lengthEdges
    readonly property int lengthEdge: margin.length + padding.length
    readonly property int lengthEdges: 2 * lengthEdge
    readonly property int lengthPaddings: 2 * padding.length
    readonly property int thickness: iconSize + thicknessEdges
    readonly property int thicknessEdges: margin.tailThickness + margin.headThickness
}
