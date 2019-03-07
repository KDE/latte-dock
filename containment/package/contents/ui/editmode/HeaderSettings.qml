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

    readonly property int thickness: rearrangeBtn.implicitHeight

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
        text: "Rearrange widgets"
        tooltip: i18n("Rearrange and configure your widgets")
        reverseIcon: plasmoid.location === PlasmaCore.Types.RightEdge

        textColor: settingsRoot.textColor
        backgroundColor: "transparent"
        checkedTextColor: colorizerManager.highlightedTextColor
        checkedBackgroundColor: colorizerManager.highlightColor

        checked: plasmoid.configuration.inConfigureAppletsMode

        onPressed: {
            plasmoid.configuration.inConfigureAppletsMode = !plasmoid.configuration.inConfigureAppletsMode;
        }

        states: [
            ///Left Edge
            State {
                name: "normal"
                when: (plasmoid.location !== PlasmaCore.Types.RightEdge)

                AnchorChanges {
                    target: rearrangeBtn
                    anchors{ top:parent.top; left:parent.left; right:undefined}
                }
                PropertyChanges{
                    target: rearrangeBtn;
                    anchors.leftMargin: units.largeSpacing; anchors.rightMargin:0
                }
            },
            State {
                name: "right"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)

                AnchorChanges {
                    target: rearrangeBtn
                    anchors{ top:parent.top; left:undefined; right:parent.right}
                }
                PropertyChanges{
                    target: rearrangeBtn;
                    anchors.leftMargin:0;    anchors.rightMargin:units.largeSpacing
                }
            }
        ]
    }
}
