/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
