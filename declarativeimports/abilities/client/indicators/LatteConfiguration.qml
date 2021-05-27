/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Item{
    readonly property string styleName: "Latte"

    readonly property int activeStyle: 0 //LineIndicator
    readonly property real size: 0.10
    readonly property bool minimizedTaskColoredDifferently: false
    readonly property bool extraDotOnActive: false
    readonly property bool glowEnabled: false
    readonly property bool enabledForApplets: true
    readonly property int glowApplyTo: 2 /*All*/
    readonly property bool glow3D: true
    readonly property real glowOpacity: 0.35
    readonly property real lengthPadding: 0.08
    readonly property real thickMargin: 0.0
    readonly property bool reversed: false
}
