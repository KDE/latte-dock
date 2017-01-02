import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

PlasmaComponents.Page{
    width: dialog.width - 2*dialog.windowSpace

    Column{
        spacing: 1.5*theme.defaultFont.pointSize
        width: parent.width
        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            Header{
                text: i18n("Appearance")
            }

            PlasmaComponents.CheckBox {
                id: showGlow
                text: i18n("Show glow around windows points")

                onCheckedChanged: {
                    plasmoid.configuration.showGlow = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.showGlow;
            }

            PlasmaComponents.CheckBox {
                id: threeColorsWindows
                text: i18n("Different color for minimized windows")

                onCheckedChanged: {
                    plasmoid.configuration.threeColorsWindows = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.threeColorsWindows;
            }

            PlasmaComponents.CheckBox {
                id: dotsOnActive
                text: i18n("Dots on active window")

                onCheckedChanged: {
                    plasmoid.configuration.dotsOnActive = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.dotsOnActive;
            }

            PlasmaComponents.CheckBox {
                id: reverseLinesPosition
                text: i18n("Reverse position for lines and dots")

                onCheckedChanged: {
                    plasmoid.configuration.reverseLinesPosition = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.reverseLinesPosition;
            }
        }
    }
}
