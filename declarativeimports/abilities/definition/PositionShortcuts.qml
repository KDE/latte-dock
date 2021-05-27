/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Item {
    property bool showPositionShortcutBadges: false
    property var badges: ['1','2','3','4','5','6','7','8','9','0', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.']

    signal sglActivateEntryAtIndex(int entryIndex);
    signal sglNewInstanceForEntryAtIndex(int entryIndex);
}
