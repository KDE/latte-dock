import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.plasmoid 2.0

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

import org.kde.latte 0.1 as Latte

PlasmaCore.FrameSvgItem {
    id: dialog
    imagePath: "dialogs/background"

    //old way to count the dialog width
    //Math.max(420,appearancePage.noneShadow.width + appearancePage.lockedAppletsShadow.width + appearancePage.allAppletsShadow.width)
    width: 31*theme.defaultFont.pixelSize
    height: 37*theme.defaultFont.pixelSize

    property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    property int windowSpace:8

    FontLoader { id: tangerineFont; name: "Tangerine"; source: "../fonts/tangerine.ttf" }

    ColumnLayout{
        width: parent.width - 2*windowSpace
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: windowSpace

        spacing: 0

        RowLayout{
            id: header
            Layout.fillWidth: true
            spacing: 0

            KQuickControlAddons.QIconItem{
                id:logo

                width: 1.5*latteTxt.font.pixelSize
                height: width

                icon: "latte-dock"
            }

            PlasmaComponents.Label{
                id: latteTxt
                text: i18n("atte")
                font.family: tangerineFont.name
                font.pointSize: 2 * theme.defaultFont.pointSize
                font.italic: true
                Layout.alignment: Qt.AlignLeft
            }

            PlasmaComponents.Label{
                id: verLabel
                font.family: "serif"
                font.pointSize: theme.defaultFont.pointSize
                font.italic: true
                opacity: 0.6

                Layout.alignment: Qt.AlignRight
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true

                text: i18n("ver: ") +"@VERSION@"
            }
        }

        PlasmaComponents.TabBar{
            id:tabBar
            Layout.fillWidth: true

            PlasmaComponents.TabButton{
                text: i18n("Behavior")
                tab: behaviorPage
            }
            PlasmaComponents.TabButton{
                text: i18n("Appearance")
                tab: appearancePage
            }
            PlasmaComponents.TabButton{
                text: i18n("Tasks")
                tab: tasksPage
            }
        }

        Rectangle{
            Layout.fillWidth: true
            height: 28*theme.defaultFont.pixelSize

            property color bC: theme.backgroundColor
            property color transparentBack: Qt.rgba(bC.r,bC.g,bC.b,0.7)

            color: transparentBack

            border.width: 1
            border.color: theme.backgroundColor

            PlasmaComponents.TabGroup{
                anchors.fill: parent
                anchors.margins: 3

                privateContents: [
                    BehaviorConfig{
                        id: behaviorPage
                    },
                    AppearanceConfig{
                        id: appearancePage
                    },
                    TasksConfig{
                        id: tasksPage
                    }
                ]
            }
        }
    }

    Row{
        id: actionButtons
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: windowSpace

        spacing: windowSpace
        width: parent.width - 2*windowSpace

        PlasmaComponents.Button{
            enabled: true
            text: i18n("Add New Dock")

            onClicked: dock.addNewDock();

            Component.onCompleted: {
                var edges = dock.freeEdges();
                if (edges.length === 0) {
                    enabled = false;
                }
            }
        }
        PlasmaComponents.Button{
            enabled: true
            text: i18n("Remove Dock")

            onClicked: dock.removeDock();
        }
    }
}


