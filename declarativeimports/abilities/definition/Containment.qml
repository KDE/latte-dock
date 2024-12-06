/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.core 0.2 as LatteCore

Item {
    property bool isFirstAppletInContainment: false
    property bool isLastAppletInContainment: false

    property int alignment: LatteCore.Types.Center
}
