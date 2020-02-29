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

import QtQuick 2.7
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.2 as Latte


Item{
    id: button
    width: visibleButton.width
    height: visibleButton.height

    signal pressedChanged(bool pressed);

    property bool checked: false

    property bool iconPositionReversed: false
    property string text: "Default Text"
    property string tooltip: ""

    readonly property bool containsMouse: tooltipBtn.hovered
    readonly property int implicitHeight: visibleButton.height

    readonly property color appliedTextColor: checked ? checkedTextColor : textColor
    readonly property color appliedBackgroundColor: checked ? checkedBackgroundColor : backgroundColor

    readonly property color textColor: containsMouse ? colorizerManager.buttonTextColor : settingsRoot.textColor
    readonly property color backgroundColor: containsMouse ? hoveredBackground : normalBackground// "transparent"
    readonly property color checkedTextColor: colorizerManager.buttonTextColor
    readonly property color checkedBackgroundColor: colorizerManager.buttonFocusColor

    readonly property color normalBackground: Qt.rgba(colorizerManager.buttonHoverColor.r,
                                                      colorizerManager.buttonHoverColor.g,
                                                      colorizerManager.buttonHoverColor.b,
                                                      0.3)

    readonly property color hoveredBackground: Qt.rgba(colorizerManager.buttonHoverColor.r,
                                                       colorizerManager.buttonHoverColor.g,
                                                       colorizerManager.buttonHoverColor.b,
                                                       0.7)

    property Component icon

    Item{
        id: visibleButtonRoot
        width: visibleButton.width
        height: visibleButton.height

        Rectangle {
            id: visibleButton
            width: buttonRow.width + 4 * margin
            height: buttonRow.height + 2 * margin
            radius: 2
            color: appliedBackgroundColor
            //   border.width: 1
            //   border.color: checked ? appliedBackgroundColor : appliedTextColor

            readonly property int margin: units.smallSpacing

            RowLayout{
                id: buttonRow
                anchors.centerIn: parent
                spacing: units.smallSpacing
                layoutDirection: iconPositionReversed ? Qt.RightToLeft : Qt.LeftToRight

                Loader {
                    width: height
                    height: textLbl.implicitHeight
                    active: button.icon
                    sourceComponent: button.icon
                    visible: active

                    readonly property color iconColor: button.appliedTextColor
                }

                PlasmaComponents.Label{
                    id: textLbl
                    text: button.text
                    color: button.appliedTextColor
                }
            }
        }
    }

    PlasmaComponents.Button {
        id: tooltipBtn
        anchors.fill: visibleButtonRoot
        opacity: 0
        tooltip: button.tooltip

        onPressedChanged: button.pressedChanged(pressed)
    }
}
