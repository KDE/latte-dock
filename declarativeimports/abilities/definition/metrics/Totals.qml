/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Item {
    readonly property int length: iconSize + lengthEdges
    readonly property int lengthEdge: margin.length + padding.length
    readonly property int lengthEdges: 2 * lengthEdge
    readonly property int lengthPaddings: 2 * padding.length
    readonly property int thickness: iconSize + thicknessEdges
    readonly property int thicknessEdges: 2 * margin.thickness
}
