/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.UserRequests {
    id: _userRequests
    property Item bridge: null
    readonly property bool isActive: bridge !== null

    function updateSignals() {
        if (isActive) {
            bridge.userRequests.sglViewType.connect(_userRequests.sglViewType);
        }
    }

    onIsActiveChanged: updateSignals()
    Component.onCompleted: updateSignals()
    Component.onDestruction: updateSignals()
}
