/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

Item {
    property bool isEnabled: false
    property bool showIsBlocked: false

    property string currentText: ""
    property Item currentVisualParent: null

    function show(visualParent, text){}
    function hide(visualParent){}
}
