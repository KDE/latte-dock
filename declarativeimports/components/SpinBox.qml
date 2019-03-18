/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Controls.Styles.Plasma 2.0 as Styles
import org.kde.plasma.core 2.0 as PlasmaCore

Controls.SpinBox {
    implicitWidth: theme.mSize(theme.defaultFont).width * 10

    style: Styles.SpinBoxStyle {
        PlasmaCore.Svg {
            id: arrowSvg
            imagePath: "widgets/arrows"
            colorGroup: PlasmaCore.Theme.ButtonColorGroup
        }
        incrementControl: PlasmaCore.SvgItem {
            implicitWidth: theme.mSize(theme.defaultFont).width * 1.8
            anchors {
                centerIn: parent
                margins: 1
                leftMargin: 0
                rightMargin: 3
            }
            svg: arrowSvg
            elementId: "up-arrow"
            opacity: control.enabled ? (styleData.upPressed ? 1 : 0.6) : 0.5
        }
        decrementControl: PlasmaCore.SvgItem {
            implicitWidth: theme.mSize(theme.defaultFont).width * 1.8
            anchors {
                centerIn: parent
                margins: 1
                leftMargin: 0
                rightMargin: 3
            }
            svg: arrowSvg
            elementId: "down-arrow"
            opacity: control.enabled ? (styleData.upPressed ? 1 : 0.6) : 0.5
        }
    }
}
