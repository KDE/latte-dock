/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.abilities.host 0.1 as AbilityHost

AbilityHost.Debug {
    eventsSinkEnabled: Qt.application.arguments.indexOf("--events-sink")>=0
    graphicsEnabled: Qt.application.arguments.indexOf("--graphics")>=0
    inputMaskEnabled: Qt.application.arguments.indexOf("--input")>=0
    layouterEnabled: Qt.application.arguments.indexOf("--layouter")>=0
    localGeometryEnabled: Qt.application.arguments.indexOf("--localgeometry")>=0
    maskEnabled: Qt.application.arguments.indexOf("--mask") >= 0
    overloadedIconsEnabled: Qt.application.arguments.indexOf("--overloaded-icons")>=0
    spacersEnabled: Qt.application.arguments.indexOf("--spacers")>=0
    timersEnabled: Qt.application.arguments.indexOf("--timers")>=0
    windowEnabled: Qt.application.arguments.indexOf("--with-window")>=0
}
