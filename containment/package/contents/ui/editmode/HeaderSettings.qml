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

import "controls" as SettingsControls

Item {
    id: headerSettings
    width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? parent.width : parent.height
    height: thickness

    readonly property bool containsMouse: rearrangeBtn.containsMouse
    readonly property int thickness: rearrangeBtn.implicitHeight

    readonly property string tooltip: rearrangeBtn.tooltip

    rotation: {
        if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
            return  0;
        }

        if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            return 90;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            return -90;
        }
    }

    x: {
        if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
            return 0;
        }

        if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
            return visibilityManager.thicknessNormalOriginalValue + ruler.thickness + spacing * 2 - width/2 + height/2;
        } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
            return spacing - width/2 + height/2;
        }
    }

    y: {
        if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            return width/2 - height/2;
        }

        if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
            return spacing;
        } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
            return parent.height - rearrangeBtn.height - spacing;
        }
    }

    SettingsControls.Button{
        id: rearrangeBtn
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top

        text: i18n("Rearrange and configure your widgets")
        tooltip: i18n("Feel free to move around your widgets and configure them from their tooltips")
        reverseIcon: plasmoid.location === PlasmaCore.Types.RightEdge

        textColor: containsMouse ? colorizerManager.buttonTextColor : settingsRoot.textColor
        backgroundColor: containsMouse ? hoveredBackground : normalBackground// "transparent"
        checkedTextColor: colorizerManager.buttonTextColor
        checkedBackgroundColor: colorizerManager.buttonFocusColor

        checked: root.inConfigureAppletsMode

        property color normalBackground: Qt.rgba(colorizerManager.buttonHoverColor.r,
                                                 colorizerManager.buttonHoverColor.g,
                                                 colorizerManager.buttonHoverColor.b,
                                                 0.3)

        property color hoveredBackground: Qt.rgba(colorizerManager.buttonHoverColor.r,
                                                  colorizerManager.buttonHoverColor.g,
                                                  colorizerManager.buttonHoverColor.b,
                                                  0.7)

        onPressed: {
            if (Latte.WindowSystem.compositingActive) {
                plasmoid.configuration.inConfigureAppletsMode = !plasmoid.configuration.inConfigureAppletsMode;
            }
        }
    }

    PlasmaComponents.Button {
        anchors.fill: rearrangeBtn
        opacity: 0
        tooltip: headerSettings.tooltip

        onPressedChanged: {
            if (Latte.WindowSystem.compositingActive && pressed) {
                plasmoid.configuration.inConfigureAppletsMode = !plasmoid.configuration.inConfigureAppletsMode;
            }
        }
    }
}
