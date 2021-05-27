/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import QtQuick.Controls.Styles.Plasma 2.0 as PlasmaStyles

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaStyles.CheckBoxStyle{
    id: checkboxStyle

    label: PlasmaComponents.Label {
        id: labelStyleTxt
        text: control.text
        wrapMode: Text.WordWrap
        elide: Text.ElideNone

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.bottom
            height: 1 * units.devicePixelRatio
            color: theme.highlightColor
            visible: control.activeFocus
        }
    }

    panel: Item {
        implicitWidth: Math.round(row.implicitWidth + padding.left + padding.right)
        implicitHeight: Math.round(row.implicitHeight + padding.top + padding.bottom)

        RowLayout {
            id: row

            y: padding.top
            x: padding.left
            width: parent.width - padding.left - padding.right
            spacing: checkboxStyle.spacing

            Loader {
                id: indicatorLoader

                Layout.alignment: Qt.AlignTop
                sourceComponent: indicator
            }

            Loader {
                id: labelLoader

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true
                Layout.fillWidth: true
                sourceComponent: label
            }
        }
    }
}
