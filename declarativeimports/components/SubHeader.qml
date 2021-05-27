/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.3

import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.Label {
    Layout.fillWidth: true
    Layout.topMargin: isFirstSubCategory ? 0 : units.smallSpacing * 2
    Layout.bottomMargin: units.smallSpacing
    horizontalAlignment: Text.AlignHCenter
    opacity: 0.4

    property bool isFirstSubCategory: false
}
