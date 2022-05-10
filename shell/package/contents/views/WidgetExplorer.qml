/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2

import org.kde.plasma.components 2.0 as PC2 // for DialogStatus, ModelCOntextMenu, and Highlight
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwindowsystem 1.0
import org.kde.kirigami 2.19 as Kirigami

import QtQuick.Window 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.private.shell 2.0 as PlasmaShell

PC3.Page {
    id: main
    width: Math.max(heading.paintedWidth, PlasmaCore.Units.iconSizes.enormous * 3 + PlasmaCore.Units.smallSpacing * 4 + PlasmaCore.Units.gridUnit * 2)
  //  height: 800//Screen.height
    opacity: draggingWidget ? 0.3 : 1
    visible: viewConfig.visible

    //property QtObject containment

    property PlasmaCore.Dialog sidePanel

    //external drop events can cause a raise event causing us to lose focus and
    //therefore get deleted whilst we are still in a drag exec()
    //this is a clue to the owning dialog that hideOnWindowDeactivate should be deleted
    //See https://bugs.kde.org/show_bug.cgi?id=332733
    property bool preventWindowHide: draggingWidget || categoriesDialog.status !== PC2.DialogStatus.Closed
                                  || getWidgetsDialog.status !== PC2.DialogStatus.Closed

    property bool outputOnly: draggingWidget

    property Item categoryButton

    property bool draggingWidget: false

    property QtObject widgetExplorer: widgetExplorerLoader.active ? widgetExplorerLoader.item : null

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
            widgetExplorer.addApplet(pluginName);
            latteView.extendedInterface.appletCreated(pluginName);
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
        shortcut: "Enter"
        onTriggered: addCurrentApplet()
    }
    QQC2.Action {
        shortcut: "Return"
        onTriggered: addCurrentApplet()
    }

    PlasmaCore.FrameSvgItem{
        id: backgroundFrameSvgItem
        anchors.top: parent.top
        anchors.topMargin: -headerMargin
        width: parent.width
        height: parent.height + headerMargin
        imagePath: "dialogs/background"
        enabledBorders: viewConfig.enabledBorders

        readonly property int headerMargin: header.height + 50 /*magical number in order to fill the top gap*/

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

    PC2.ModelContextMenu {
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

    PC2.ModelContextMenu {
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
            NumberAnimation { duration: PlasmaCore.Units.longDuration }
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


    header: PlasmaExtras.PlasmoidHeading {
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
                PC3.ToolButton {
                    id: getWidgetsButton
                    icon.name: "get-hot-new-stuff"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Get New Widgets…")
                    onClicked: {
                        getWidgetsDialog.model = widgetExplorer.widgetsMenuActions
                        getWidgetsDialog.openRelative()
                    }
                }
                PC3.ToolButton {
                    id: closeButton
                    icon.name: "window-close"
                    onClicked: main.closed()
                }
            }

            RowLayout {
                PC3.TextField {
                    id: searchInput
                    Layout.fillWidth: true
                    clearButtonShown: true
                    placeholderText: i18nd("plasma_shell_org.kde.plasma.desktop", "Search…")

                    inputMethodHints: Qt.ImhNoPredictiveText

                    onTextChanged: {
                        list.positionViewAtBeginning()
                        list.currentIndex = -1
                        widgetExplorer.widgetsModel.searchTerm = text
                    }

                    Component.onCompleted: if (Kirigami.InputMethod && !Kirigami.InputMethod.willShowOnActive) { forceActiveFocus() }
                }
                PC3.ToolButton {
                    id: categoryButton
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "All Widgets")
                    icon.name: "view-filter"
                    onClicked: {
                        categoriesDialog.model = widgetExplorer.filterModel
                        categoriesDialog.open(0, categoryButton.height)
                    }

                    PC3.ToolTip {
                        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Categories")
                    }
                }
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

    PC3.ScrollView {
        anchors.fill: parent
        //anchors.rightMargin: - main.sidePanel.margins.right

        // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
        PC3.ScrollBar.horizontal.policy: PC3.ScrollBar.AlwaysOff

        // hide the flickering by fading in nicely
        opacity: setModelTimer.running ? 0 : 1
        Behavior on opacity {
            OpacityAnimator {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        GridView {
            id: list

            // model set delayed by Timer above

            activeFocusOnTab: true
            keyNavigationWraps: true
            cellWidth: Math.floor(width / 3)
            cellHeight: cellWidth + PlasmaCore.Units.gridUnit * 4 + PlasmaCore.Units.smallSpacing * 2

            delegate: AppletDelegate {}
            highlight: PC2.Highlight {}
            highlightMoveDuration: 0
            //highlightResizeDuration: 0

            //slide in to view from the left
            add: Transition {
                NumberAnimation {
                    properties: "x"
                    from: -list.width
                    duration: PlasmaCore.Units.shortDuration
                }
            }

            //slide out of view to the right
            remove: Transition {
                NumberAnimation {
                    properties: "x"
                    to: list.width
                    duration: PlasmaCore.Units.shortDuration
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
                    duration: PlasmaCore.Units.shortDuration
                }
            }
        }
    }

    PlasmaExtras.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
        text: searchInput.text.length > 0 ? i18n("No widgets matched the search terms") : i18n("No widgets available")
        visible: list.count == 0
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
