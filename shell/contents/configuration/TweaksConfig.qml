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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

PlasmaComponents.Page {
    Layout.maximumWidth: content.width + content.Layout.leftMargin * 2
    Layout.maximumHeight: content.height + units.smallSpacing * 2

    ColumnLayout {
        id: content

        width: dialog.maxWidth - Layout.leftMargin * 2
        spacing: units.largeSpacing
        anchors.centerIn: parent
        Layout.leftMargin: units.smallSpacing * 2

        //! BEGIN: Tasks Appearance
        ColumnLayout {
            spacing: units.smallSpacing

            Header {
                text: i18n("Behavior")
            }

            PlasmaComponents.CheckBox {
                id: showGlow
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Enable autostart during startup")

                onClicked: {
                }
            }

            PlasmaComponents.CheckBox {
                id: threeColorsWindows
                Layout.leftMargin: units.smallSpacing * 2
                text: i18n("Show dock on desktop or actitivy change")

                onClicked: {
                }
            }
        }
    }
}
