/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
