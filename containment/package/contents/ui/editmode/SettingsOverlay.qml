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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.2 as Latte

import "maxlength" as MaximumLength

Item{
    id: settingsRoot
    readonly property int thickness: ruler.thickness + rearrangeBtn.implicitHeight + spacing
    readonly property int spacing: 3

    MaximumLength.Ruler {
        id: ruler
    }

    PlasmaComponents.Button {
        id: rearrangeBtn
        text: i18n("Rearrange")
        iconSource: "document-edit"
        checkable: true
        tooltip: i18n("Rearrange and configure your widgets")

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
                return settingsRoot.width / 2 - implicitWidth / 2;
            }

            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                return visibilityManager.thicknessNormalOriginalValue - implicitWidth / 2 + implicitHeight / 2;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                return ruler.thickness - implicitWidth / 2 + implicitHeight / 2 + spacing;
            }
        }

        y: {
            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                return settingsRoot.height/2 - implicitHeight / 2;
            }

            if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                return ruler.y + ruler.thickness + spacing;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                return ruler.y - implicitHeight - spacing;
            }
        }

        onClicked: {
            if (plasmoid.configuration.inConfigureAppletsMode) {
                checked = false;
                plasmoid.configuration.inConfigureAppletsMode = false;
            } else {
                checked = true;
                plasmoid.configuration.inConfigureAppletsMode = true;
            }
        }

        Component.onCompleted: {
            if (plasmoid.configuration.inConfigureAppletsMode) {
                checked = true;
            } else {
                checked = false;
            }
        }
    }
}
