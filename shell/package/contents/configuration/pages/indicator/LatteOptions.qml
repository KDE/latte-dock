/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte
import org.kde.latte.components 1.0 as LatteComponents

ColumnLayout {
    LatteComponents.SubHeader {
        text: i18nc("active indicator style","Style For Active")
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 2

        property int indicatorType: plasmoid.configuration.activeIndicatorType

        ExclusiveGroup {
            id: activeIndicatorTypeGroup
            onCurrentChanged: {
                if (current.checked) {
                    plasmoid.configuration.activeIndicatorType = current.indicatorType;
                }
            }
        }

        PlasmaComponents.Button {
            Layout.fillWidth: true

            text: i18nc("line indicator","Line")
            checked: parent.indicatorType === indicatorType
            checkable: true
            exclusiveGroup: activeIndicatorTypeGroup
            tooltip: i18n("Show a line indicator for active items")

            readonly property int indicatorType: Latte.Types.LineIndicator
        }

        PlasmaComponents.Button {
            Layout.fillWidth: true

            text: i18nc("dot indicator", "Dot")
            checked: parent.indicatorType === indicatorType
            checkable: true
            exclusiveGroup: activeIndicatorTypeGroup
            tooltip: i18n("Show a dot indicator for active items")

            readonly property int indicatorType: Latte.Types.DotIndicator
        }
    }

    LatteComponents.HeaderSwitch {
        id: showGlow
        Layout.fillWidth: true
        Layout.minimumHeight: implicitHeight
        Layout.bottomMargin: units.smallSpacing

        checked: plasmoid.configuration.showGlow
        level: 2
        text: i18n("Glow")
        tooltip: i18n("Enable/disable indicator glow")

        onPressed: {
            plasmoid.configuration.showGlow = !plasmoid.configuration.showGlow;
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 2
        enabled: plasmoid.configuration.showGlow

        property int option: plasmoid.configuration.glowOption

        ExclusiveGroup {
            id: glowGroup
            onCurrentChanged: {
                if (current.checked)
                    plasmoid.configuration.glowOption = current.option
            }
        }

        PlasmaComponents.Button {
            Layout.fillWidth: true
            text: i18nc("glow only to active task/applet indicators","On Active")
            checked: parent.option === option
            checkable: true
            exclusiveGroup:  glowGroup
            tooltip: i18n("Add glow only to active task/applet indicator")

            readonly property int option: Latte.Types.GlowOnlyOnActive
        }

        PlasmaComponents.Button {
            Layout.fillWidth: true
            text: i18nc("glow to all task/applet indicators","All")
            checked: parent.option === option
            checkable: true
            exclusiveGroup: glowGroup
            tooltip: i18n("Add glow to all task/applet indicators")

            readonly property int option: Latte.Types.GlowAll
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 2

        enabled: plasmoid.configuration.showGlow

        PlasmaComponents.Label {
            Layout.minimumWidth: implicitWidth
            horizontalAlignment: Text.AlignLeft
            Layout.rightMargin: units.smallSpacing
            text: i18n("Opacity")
        }

        LatteComponents.Slider {
            id: glowOpacitySlider
            Layout.fillWidth: true

            leftPadding: 0
            value: plasmoid.configuration.glowOpacity
            from: 0
            to: 100
            stepSize: 5
            wheelEnabled: false

            function updateGlowOpacity() {
                if (!pressed)
                    plasmoid.configuration.glowOpacity = value;
            }

            onPressedChanged: {
                updateGlowOpacity();
            }

            Component.onCompleted: {
                valueChanged.connect(updateGlowOpacity);
            }

            Component.onDestruction: {
                valueChanged.disconnect(updateGlowOpacity);
            }
        }

        PlasmaComponents.Label {
            text: glowOpacitySlider.value + " %"
            horizontalAlignment: Text.AlignRight
            Layout.minimumWidth: theme.mSize(theme.defaultFont).width * 4
            Layout.maximumWidth: theme.mSize(theme.defaultFont).width * 4
        }
    }

    ColumnLayout {
        spacing: 0
        visible: latteView.latteTasksPresent()

        LatteComponents.SubHeader {
            enabled: plasmoid.configuration.glowOption!==Latte.Types.GlowNone
            text: i18n("Tasks")
        }

        PlasmaComponents.CheckBox {
            id: threeColorsWindows
            text: i18n("Different color for minimized windows")
            checked: plasmoid.configuration.threeColorsWindows

            onClicked: {
                plasmoid.configuration.threeColorsWindows = checked
            }
        }

        PlasmaComponents.CheckBox {
            id: dotsOnActive
            text: i18n("Show an extra dot for grouped windows when active")
            checked: plasmoid.configuration.dotsOnActive
            tooltip: i18n("Grouped windows show both a line and a dot when one of them is active and the Line Active Indicator is enabled")
            enabled: plasmoid.configuration.activeIndicatorType === Latte.Types.LineIndicator

            onClicked: {
                plasmoid.configuration.dotsOnActive = checked
            }
        }
    }
}
