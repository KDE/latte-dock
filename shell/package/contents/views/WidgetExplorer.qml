/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwindowsystem 1.0

import QtQuick.Window 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.private.shell 2.0 as PlasmaShell

Item {
    id: main
    width: Math.max(heading.paintedWidth, units.iconSizes.enormous * 3 + units.smallSpacing * 4 + units.gridUnit * 2)
  //  height: 800//Screen.height
    opacity: draggingWidget ? 0.3 : 1
    visible: viewConfig.visible

    //property QtObject containment

    //external drop events can cause a raise event causing us to lose focus and
    //therefore get deleted whilst we are still in a drag exec()
    //this is a clue to the owning dialog that hideOnWindowDeactivate should be deleted
    //See https://bugs.kde.org/show_bug.cgi?id=332733
    property bool preventWindowHide: draggingWidget
                                     || categoriesDialog.status !== PlasmaComponents.DialogStatus.Closed
                                     || getWidgetsDialog.status !== PlasmaComponents.DialogStatus.Closed

    property bool outputOnly: draggingWidget

    property Item categoryButton
    property QtObject widgetExplorer: widgetExplorerLoader.active ? widgetExplorerLoader.item : null

    property bool draggingWidget: false

    signal closed();

    onClosed: {
        if (main.preventWindowHide) {
            return;
        }

        viewConfig.hideConfigWindow();
    }

    onVisibleChanged: {
        if (!visible) {
            kwindowsystem.showingDesktop = false;
            if (widgetExplorer) {
                widgetExplorer.widgetsModel.filterQuery = "";
                widgetExplorer.widgetsModel.filterType = "";
                widgetExplorer.widgetsModel.searchTerm = "";
            }
            searchInput.text = "";
        }
    }

    onWidgetExplorerChanged: {
        if (widgetExplorer) {
            setModelTimer.start();
        }
    }

    Component.onDestruction: {
        if (pendingUninstallTimer.running) {
            // we're not being destroyed so at least reset the filters
            widgetExplorer.widgetsModel.filterQuery = ""
            widgetExplorer.widgetsModel.filterType = ""
            widgetExplorer.widgetsModel.searchTerm = ""
        }
    }

    function addCurrentApplet() {
        var pluginName = list.currentItem ? list.currentItem.pluginName : ""
        if (pluginName) {
            widgetExplorer.addApplet(pluginName)
        }
    }

    KWindowSystem {
        id: kwindowsystem
    }

    QQC2.Action {
        shortcut: "Escape"
        onTriggered: {
            if (searchInput.length > 0) {
                searchInput.text = ""
            } else {
                main.closed();
            }
        }
    }

    QQC2.Action {
        shortcut: "Up"
        onTriggered: list.currentIndex = (list.count + list.currentIndex - 1) % list.count
    }

    QQC2.Action {
        shortcut: "Down"
        onTriggered: list.currentIndex = (list.currentIndex + 1) % list.count
    }

    QQC2.Action {
        shortcut: "Enter"
        onTriggered: addCurrentApplet()
    }

    QQC2.Action {
        shortcut: "Return"
        onTriggered: addCurrentApplet()
    }

    PlasmaCore.FrameSvgItem{
        id: backgroundFrameSvgItem
        anchors.fill: parent
        imagePath: "dialogs/background"
        enabledBorders: viewConfig.enabledBorders

        onEnabledBordersChanged: viewConfig.updateEffects()
        Component.onCompleted: viewConfig.updateEffects()
    }

    Loader {
        id: widgetExplorerLoader
        active: main.visible
        sourceComponent: PlasmaShell.WidgetExplorer {
            //id:widgetExplorer
             //view: desktop
            containment: containmentFromView
            onShouldClose: main.closed();
        }
    }


    PlasmaComponents.ModelContextMenu {
        id: categoriesDialog
        visualParent: categoryButton
        // model set on first invocation

        onClicked: {
            list.contentX = 0
            list.contentY = 0
            categoryButton.text = (model.filterData ? model.display : i18nd("plasma_shell_org.kde.plasma.desktop", "All Widgets"))
            widgetExplorer.widgetsModel.filterQuery = model.filterData
            widgetExplorer.widgetsModel.filterType = model.filterType
        }
    }

    PlasmaComponents.ModelContextMenu {
        id: getWidgetsDialog
        visualParent: getWidgetsButton
        placement: PlasmaCore.Types.TopPosedLeftAlignedPopup
        // model set on first invocation
        onClicked: model.trigger()
    }
    /*
    PlasmaCore.Dialog {
        id: tooltipDialog
        property Item appletDelegate
        location: PlasmaCore.Types.RightEdge //actually we want this to be the opposite location of the explorer itself
        type: PlasmaCore.Dialog.Tooltip
        flags:Qt.Window|Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint
        onAppletDelegateChanged: {
            if (!appletDelegate) {
                toolTipHideTimer.restart()
                toolTipShowTimer.running = false
            } else if (tooltipDialog.visible) {
                tooltipDialog.visualParent = appletDelegate
            } else {
                tooltipDialog.visualParent = appletDelegate
                toolTipShowTimer.restart()
                toolTipHideTimer.running = false
            }
        }
        mainItem: Tooltip { id: tooltipWidget }
        Behavior on y {
            NumberAnimation { duration: units.longDuration }
        }
    }
    Timer {
        id: toolTipShowTimer
        interval: 500
        repeat: false
        onTriggered: {
            tooltipDialog.visible = true
        }
    }
    Timer {
        id: toolTipHideTimer
        interval: 1000
        repeat: false
        onTriggered: tooltipDialog.visible = false
    }
    */


    PlasmaExtras.PlasmoidHeading {
        id: topArea
        implicitWidth: header.implicitWidth
        implicitHeight: header.implicitHeight
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        ColumnLayout {
            id: header
            anchors.fill: parent

            RowLayout {
                PlasmaExtras.Heading {
                    id: heading
                    level: 1
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Widgets")
                    elide: Text.ElideRight

                    Layout.fillWidth: true
                }
                PlasmaComponents.ToolButton {
                    id: getWidgetsButton
                    iconSource: "get-hot-new-stuff"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Get New Widgets...")
                    onClicked: {
                        getWidgetsDialog.model = widgetExplorer.widgetsMenuActions
                        getWidgetsDialog.openRelative()
                    }
                }
                PlasmaComponents.ToolButton {
                    id: closeButton
                    iconSource: "window-close"
                    onClicked: main.closed()
                }
            }

            RowLayout {
                PlasmaComponents.TextField {
                    id: searchInput
                    Layout.fillWidth: true
                    clearButtonShown: true
                    placeholderText: i18nd("plasma_shell_org.kde.plasma.desktop", "Search...")
                    onTextChanged: {
                        list.positionViewAtBeginning()
                        list.currentIndex = -1
                        widgetExplorer.widgetsModel.searchTerm = text
                    }

                    Component.onCompleted: forceActiveFocus()
                }
                PlasmaComponents.ToolButton {
                    id: categoryButton
                    tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Categories")
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "All Widgets")
                    iconSource: "view-filter"
                    onClicked: {
                        categoriesDialog.model = widgetExplorer.filterModel
                        categoriesDialog.open(0, categoryButton.height)
                    }
                }
            }

            Item {
                height: units.smallSpacing
            }
        }
    }

    Timer {
        id: setModelTimer
        interval: 20
        running: true
        onTriggered: {
            if (widgetExplorer) {
                list.model = widgetExplorer.widgetsModel
            }
        }
    }

    PlasmaExtras.ScrollArea {
        anchors {
            top: topArea.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

        // hide the flickering by fading in nicely
        opacity: setModelTimer.running ? 0 : 1
        Behavior on opacity {
            OpacityAnimator {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        GridView {
            id: list

            // model set delayed by Timer above

            activeFocusOnTab: true
            keyNavigationWraps: true
            cellWidth: Math.floor((width - units.smallSpacing) / 3)
            cellHeight: cellWidth + units.gridUnit * 4 + units.smallSpacing * 2

            delegate: AppletDelegate {}
            highlight: PlasmaComponents.Highlight {}
            highlightMoveDuration: 0
            //highlightResizeDuration: 0

            //slide in to view from the left
            add: Transition {
                NumberAnimation {
                    properties: "x"
                    from: -list.width
                    duration: units.shortDuration
                }
            }

            //slide out of view to the right
            remove: Transition {
                NumberAnimation {
                    properties: "x"
                    to: list.width
                    duration: units.shortDuration
                }
            }

            //if we are adding other items into the view use the same animation as normal adding
            //this makes everything slide in together
            //if we make it move everything ends up weird
            addDisplaced: list.add

            //moved due to filtering
            displaced: Transition {
                NumberAnimation {
                    properties: "x,y"
                    duration: units.shortDuration
                }
            }

            PlasmaExtras.Heading {
                anchors.fill: parent
                anchors.margins: units.largeSpacing
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                level: 2
                text: searchInput.text.length > 0 ? i18n("No widgets matched the search terms") : i18n("No widgets available")
                enabled: false
                visible: list.count == 0
            }
        }
    }

    //! Bindings
    Binding{
        target: viewConfig
        property: "hideOnWindowDeactivate"
        value: !main.preventWindowHide
    }

    //! Timers
    Timer {
        id: pendingUninstallTimer
        // keeps track of the applets the user wants to uninstall
        property var applets: []

        interval: 200
        onTriggered: {
            for (var i = 0, length = applets.length; i < length; ++i) {
                widgetExplorer.uninstall(applets[i])
            }
            applets = []

            /*if (sidePanelStack.state !== "widgetExplorer" && widgetExplorer) {
                widgetExplorer.destroy()
                widgetExplorer = null
            }*/
        }
    }
}
