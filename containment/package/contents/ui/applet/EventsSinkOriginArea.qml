/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

MouseArea {
    id: __originArea
    enabled: visible
    hoverEnabled: true
    visible: root.latteView && root.latteView.sink.originParentItem !== originParentItem && width>0 && height>0

    onEntered: {
        root.latteView.sink.setSink(originParentItem, destination);
    }

    Loader{
        anchors.fill: parent
        active: appletItem.debug.eventsSinkEnabled && active

        sourceComponent: Rectangle {
            anchors.fill: parent
            color: "red"
            opacity: 0.2
        }
    }
}
