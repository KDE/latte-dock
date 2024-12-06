/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Item {
    property int headThickness: 8
    property int tailThickness: 8
    readonly property int iconSize: _metrics.totals.thickness - thicknessEdges
    readonly property int thicknessEdges: headThickness + tailThickness
}
