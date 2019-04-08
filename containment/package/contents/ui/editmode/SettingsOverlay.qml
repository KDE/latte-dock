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
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.2 as Latte

import "controls" as SettingsControls
import "maxlength" as MaximumLength

import "../../code/ColorizerTools.js" as ColorizerTools

Item{
    id: settingsRoot
    readonly property bool containsMouse: headerSettings.containsMouse || ruler.containsMouse
                                          || tooltipMouseArea.containsMouse || editBackMouseArea.containsMouse
    readonly property int thickness: ruler.thickness + headerSettings.thickness + spacing * 3
    readonly property int spacing: 4

    property int textShadow: {
        if (textColorIsDark)  {
            return 1;
        } else {
            return 6;
        }
    }

    property string tooltip: ""

    readonly property real textColorBrightness: ColorizerTools.colorBrightness(textColor)
    readonly property bool textColorIsDark: textColorBrightness < 127.5
    readonly property color textColor: {
        if (imageTiler.opacity <= 0.4) {
            return colorizerManager.applyColor;
        } else {
            return latteView && latteView.managedLayout ? latteView.managedLayout.textColor : "#D7E3FF";
        }
    }

    layer.enabled: true
    layer.effect: DropShadow{
        radius: settingsRoot.textShadow
        fast: true
        samples: 2 * radius
        color: root.appShadowColorSolid
    }

    HeaderSettings{
        id: headerSettings
    }

    MaximumLength.Ruler {
        id: ruler
        thicknessMargin: headerSettings.thickness + 3 * spacing
        thickMargin: 3
    }

    //! Tooltip
    onContainsMouseChanged: {
        if (containsMouse) {
            hideTooltipTimer.stop();
            tooltip.visible = true;
        } else {
            hideTooltipTimer.restart();
        }
    }

    Binding{
        target: settingsRoot
        property: "tooltip"
        value: {
            if (ruler.containsMouse) {
                return ruler.tooltip;
            } else if (headerSettings.containsMouse) {
                return headerSettings.tooltip;
            } else if (editBackMouseArea.containsMouse) {
                return editBackMouseArea.tooltip;
            }
        }
    }

    Timer {
        id: hideTooltipTimer
        interval: units.longDuration * 2
        onTriggered: {
            if (!settingsRoot.containsMouse) {
                tooltip.visible = false;
            }
        }
    }

    PlasmaCore.Dialog {
        id: tooltip
        visualParent: titleTooltipParent

        flags: Qt.WindowStaysOnTopHint | Qt.ToolTip
        location: plasmoid.location

        mainItem: MouseArea {
            id: tooltipMouseArea
            Layout.minimumWidth: label.width + (2 * units.smallSpacing)
            Layout.preferredWidth: Layout.minimumWidth
            Layout.maximumWidth: Layout.minimumWidth

            Layout.minimumHeight: label.height
            Layout.preferredHeight: Layout.minimumHeight
            Layout.maximumHeight: Layout.minimumHeight
            hoverEnabled: true

            PlasmaComponents.Label {
                id: label
                anchors.centerIn: parent
                textFormat: Text.PlainText
                maximumLineCount: 1
                text: settingsRoot.tooltip
            }
        }
    }

}
