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

        /******Appearance******/
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

        /*********Behavior************/
        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            Header{
                text: i18n("Interaction")
            }

            PlasmaComponents.CheckBox {
                id: showPreviewsChk
                text: i18n("Preview windows on hovering")

                onCheckedChanged: {
                    plasmoid.configuration.showToolTips = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.showToolTips;
            }

            PlasmaComponents.CheckBox {
                id: highlightWindowsChk
                text: i18n("Highlight windows on hovering")

                onCheckedChanged: {
                    plasmoid.configuration.highlightWindows = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.highlightWindows;
            }

            PlasmaComponents.CheckBox {
                id: windowActionsChk
                text: i18n("Show window actions in the context menu")

                onCheckedChanged: {
                    plasmoid.configuration.showWindowActions = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.showWindowActions;
            }

            PlasmaComponents.CheckBox {
                id: smartLaunchersChk
                text: i18n("Show progress information in task buttons")

                onCheckedChanged: {
                    plasmoid.configuration.smartLaunchersEnabled = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.smartLaunchersEnabled;
            }
        }

        /*******Filters*******/
        Column{
            width:parent.width
            spacing: 0.8*theme.defaultFont.pointSize

            Header{
                text: i18n("Filters")
            }

            PlasmaComponents.CheckBox {
                id: showOnlyCurrentScreen
                text: i18n("Show only tasks from the current screen")
                onCheckedChanged: {
                    plasmoid.configuration.showOnlyCurrentScreen = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.showOnlyCurrentScreen;
            }

            PlasmaComponents.CheckBox {
                id: showOnlyCurrentDesktop
                text: i18n("Show only tasks from the current desktop")
                onCheckedChanged: {
                    plasmoid.configuration.showOnlyCurrentDesktop = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.showOnlyCurrentDesktop;
            }

            PlasmaComponents.CheckBox {
                id: showOnlyCurrentActivity
                text: i18n("Show only tasks from the current activity")
                onCheckedChanged: {
                    plasmoid.configuration.showOnlyCurrentActivity = checked;
                }

                Component.onCompleted: checked = plasmoid.configuration.showOnlyCurrentActivity;
            }

        }
    }
}
