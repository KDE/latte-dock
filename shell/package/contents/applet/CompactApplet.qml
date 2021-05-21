/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

import org.kde.latte.core 0.2 as LatteCore

PlasmaCore.ToolTipArea {
    id: root
    objectName: "org.kde.desktop-CompactApplet"
    anchors.fill: parent

    mainText: plasmoid.toolTipMainText
    subText: plasmoid.toolTipSubText
    location: plasmoid.location
    active: !plasmoid.expanded
    textFormat: plasmoid.toolTipTextFormat
    mainItem: plasmoid.toolTipItem ? plasmoid.toolTipItem : null

    property Item fullRepresentation
    property Item compactRepresentation
    /*Discover real visual parent - the following code points to Applet::ItemWrapper*/
    property Item originalCompactRepresenationParent
    property Item compactRepresentationVisualParent: originalCompactRepresenationParent && originalCompactRepresenationParent.parent
                                                     ? originalCompactRepresenationParent.parent.parent : null

    onCompactRepresentationChanged: {
        if (compactRepresentation) {
            originalCompactRepresenationParent = compactRepresentation.parent;

            compactRepresentation.parent = root;
            compactRepresentation.anchors.fill = root;
            compactRepresentation.visible = true;
        }
        root.visible = true;
    }

    onFullRepresentationChanged: {

        if (!fullRepresentation) {
            return;
        }

        //if the fullRepresentation size was restored to a stored size, or if is dragged from the desktop, restore popup size
        if (fullRepresentation.Layout && fullRepresentation.Layout.preferredWidth > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.Layout.preferredWidth
            })
        } else if (fullRepresentation.implicitWidth > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.implicitWidth
            })
        } else if (fullRepresentation.width > 0) {
            popupWindow.mainItem.width = Qt.binding(function() {
                return fullRepresentation.width
            })
        } else {
            popupWindow.mainItem.width = Qt.binding(function() {
                return PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 35
            })
        }

        if (fullRepresentation.Layout && fullRepresentation.Layout.preferredHeight > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.Layout.preferredHeight
            })
        } else if (fullRepresentation.implicitHeight > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.implicitHeight
            })
        } else if (fullRepresentation.height > 0) {
            popupWindow.mainItem.height = Qt.binding(function() {
                return fullRepresentation.height
            })
        } else {
            popupWindow.mainItem.height = Qt.binding(function() {
                return PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 25
            })
        }

        fullRepresentation.parent = appletParent;
        fullRepresentation.anchors.fill = fullRepresentation.parent;
    }

   /* PlasmaCore.FrameSvgItem {
        id: expandedItem
        anchors.fill: parent
        imagePath: "widgets/tabbar"
        visible: fromCurrentTheme && opacity > 0
        prefix: {
            var prefix;
            switch (plasmoid.location) {
                case PlasmaCore.Types.LeftEdge:
                    prefix = "west-active-tab";
                    break;
                case PlasmaCore.Types.TopEdge:
                    prefix = "north-active-tab";
                    break;
                case PlasmaCore.Types.RightEdge:
                    prefix = "east-active-tab";
                    break;
                default:
                    prefix = "south-active-tab";
                }
                if (!hasElementPrefix(prefix)) {
                    prefix = "active-tab";
                }
                return prefix;
            }
        opacity: plasmoid.expanded ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }*/

    Timer {
        id: expandedSync
        interval: 100
        onTriggered: plasmoid.expanded = popupWindow.visible;
    }

    Connections {
        target: plasmoid.action("configure")
        function onTriggered() { plasmoid.expanded = false }
    }

    Connections {
        target: plasmoid
        function onContextualActionsAboutToShow() { root.hideToolTip() }
    }

    LatteCore.Dialog {
        id: popupWindow
        objectName: "popupWindow"
        flags: Qt.WindowStaysOnTopHint
        visible: plasmoid.expanded && fullRepresentation
        visualParent: compactRepresentationVisualParent ? compactRepresentationVisualParent : (compactRepresentation ? compactRepresentation : null)
       // location: PlasmaCore.Types.Floating //plasmoid.location
        edge: plasmoid.location /*this way dialog borders are not updated and it is used only for adjusting dialog position*/
        hideOnWindowDeactivate: plasmoid.hideOnWindowDeactivate
        backgroundHints: (plasmoid.containmentDisplayHints & PlasmaCore.Types.DesktopFullyCovered) ? PlasmaCore.Dialog.SolidBackground : PlasmaCore.Dialog.StandardBackground

        property var oldStatus: PlasmaCore.Types.UnknownStatus

        //It's a MouseEventListener to get all the events, so the eventfilter will be able to catch them
        mainItem: MouseEventListener {
            id: appletParent

            focus: true

            Keys.onEscapePressed: {
                plasmoid.expanded = false;
            }

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            Layout.minimumWidth: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.minimumWidth : 0
            Layout.minimumHeight: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.minimumHeight: 0

            Layout.preferredWidth: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.preferredWidth : -1
            Layout.preferredHeight: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.preferredHeight: -1

            Layout.maximumWidth: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.maximumWidth : Infinity
            Layout.maximumHeight: (fullRepresentation && fullRepresentation.Layout) ? fullRepresentation.Layout.maximumHeight: Infinity

            onActiveFocusChanged: {
                if (activeFocus && fullRepresentation) {
                    fullRepresentation.forceActiveFocus()
                }
            }
        }

        onVisibleChanged: {
            if (!visible) {
                expandedSync.restart();
                plasmoid.status = oldStatus;
            } else {
                oldStatus = plasmoid.status;
                plasmoid.status = PlasmaCore.Types.RequiresAttentionStatus;
                // This call currently fails and complains at runtime:
                // QWindow::setWindowState: QWindow::setWindowState does not accept Qt::WindowActive
                popupWindow.requestActivate();
            }
        }

    }
}
