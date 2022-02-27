/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

import org.kde.latte.core 0.2 as LatteCore

MouseArea {
    id: configurationArea
    z: 1000

    width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? root.width : thickness
    height: plasmoid.formFactor === PlasmaCore.Types.Vertical ? root.height : thickness

    visible: root.inConfigureAppletsMode
    hoverEnabled: root.inConfigureAppletsMode

    focus: true
    cursorShape: {
        if (currentApplet && tooltip.visible && currentApplet.latteStyleApplet) {
            return root.isHorizontal ? Qt.SizeHorCursor : Qt.SizeVerCursor;
        }

        return Qt.ArrowCursor;
    }

    property bool isResizingLeft: false
    property bool isResizingRight: false
    property Item currentApplet
    property Item previousCurrentApplet
    readonly property alias draggedPlaceHolder: placeHolder

    property Item currentHoveredLayout: {
        if (placeHolder.parent !== configurationArea) {
            return placeHolder.parent;
        }

        return currentApplet ? currentApplet.parent : null
    }

    property int lastX
    property int lastY
    property int appletX
    property int appletY

    readonly property int thickness: metrics.mask.thickness.maxNormal - metrics.extraThicknessForNormal
    readonly property int spacerHandleSize: units.smallSpacing

    onHeightChanged: tooltip.visible = false;
    onWidthChanged: tooltip.visible = false;


    function hoveredItem(x, y) {
        //! main layout
        var relevantLayout = mapFromItem(layoutsContainer.mainLayout, 0, 0);
        var item = layoutsContainer.mainLayout.childAt(x-relevantLayout.x, y-relevantLayout.y);

        if (!item) {
            // start layout
            relevantLayout = mapFromItem(layoutsContainer.startLayout,0,0);
            item = layoutsContainer.startLayout.childAt(x-relevantLayout.x, y-relevantLayout.y);
        }

        if (!item) {
            // end layout
            relevantLayout = mapFromItem(layoutsContainer.endLayout,0,0);
            item = layoutsContainer.endLayout.childAt(x-relevantLayout.x, y-relevantLayout.y);
        }

        return item;
    }

    function relevantLayoutForApplet(curapplet) {
        var relevantLayout;

        if (curapplet.parent === layoutsContainer.mainLayout) {
            relevantLayout = mapFromItem(layoutsContainer.mainLayout, 0, 0);
        } else if (curapplet.parent === layoutsContainer.startLayout) {
            relevantLayout = mapFromItem(layoutsContainer.startLayout, 0, 0);
        } else if (curapplet.parent === layoutsContainer.endLayout) {
            relevantLayout = mapFromItem(layoutsContainer.endLayout, 0, 0);
        }

        return relevantLayout;
    }


    onPositionChanged: {
        if (pressed) {
            if(currentApplet){
                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    currentApplet.y += (mouse.y - lastY);
                } else {
                    currentApplet.x += (mouse.x - lastX);
                }
            }

            lastX = mouse.x;
            lastY = mouse.y;

            var mousesink = {x: mouse.x, y: mouse.y};

            //! ignore thicknes moving at all cases
            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                mousesink.y = configurationArea.height / 2;
            } else {
                mousesink.x = configurationArea.width / 2;
            }

            var item = hoveredItem(mousesink.x, mousesink.y);

            if (item && item !== placeHolder) {
                var posInItem = mapToItem(item, mousesink.x, mousesink.y);

                if ((plasmoid.formFactor === PlasmaCore.Types.Vertical && posInItem.y < item.height/2) ||
                        (plasmoid.formFactor !== PlasmaCore.Types.Vertical && posInItem.x < item.width/2)) {
                    fastLayoutManager.insertBefore(item, placeHolder);
                } else {
                    fastLayoutManager.insertAfter(item, placeHolder);
                }
            }

        } else {
            var item = hoveredItem(mouse.x, mouse.y);

            if (root.dragOverlay && item && !item.isParabolicEdgeSpacer) {
                root.dragOverlay.currentApplet = item;
            } else {
                currentApplet = null;
                root.dragOverlay.currentApplet = null;
            }
        }

        if (root.dragOverlay.currentApplet) {
            hideTimer.stop();

            tooltip.visible = true;
            tooltip.raise();
        }
    }

    onExited: hideTimer.restart();

    onCurrentAppletChanged: {
        previousCurrentApplet = currentApplet;

        if (!currentApplet || !root.dragOverlay.currentApplet) {
            hideTimer.restart();
            return;
        }

        var relevantLayout = relevantLayoutForApplet(currentApplet) ;

        if (!relevantLayout) {
            return;
        }

        lockButton.checked = currentApplet.lockZoom;
        colorizingButton.checked = !currentApplet.userBlocksColorizing;
    }

    onPressed: {
        if (!root.dragOverlay.currentApplet) {
            return;
        }

        var relevantApplet = mapFromItem(currentApplet, 0, 0);
        var rootArea = mapFromItem(root, 0, 0);

        appletX = mouse.x - relevantApplet.x + rootArea.x;
        appletY = mouse.y - relevantApplet.y + rootArea.y;

        lastX = mouse.x;
        lastY = mouse.y;
        fastLayoutManager.insertBefore(currentApplet, placeHolder);
        currentApplet.parent = root;
        currentApplet.x = root.isHorizontal ? lastX - currentApplet.width/2 : lastX-appletX;
        currentApplet.y = root.isVertical ? lastY - currentApplet.height/2 : lastY-appletY;
        currentApplet.z = 900;
    }

    onReleased: {
        if (!handle.visible) {
            tooltip.visible = false;
        }

        if (!root.dragOverlay.currentApplet) {
            return;
        }

        if(currentApplet && currentApplet.applet){
            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                currentApplet.applet.configuration.length = handle.height;
            } else {
                currentApplet.applet.configuration.length = handle.width;
            }
        }

        configurationArea.isResizingLeft = false;
        configurationArea.isResizingRight = false;

        fastLayoutManager.insertBefore(placeHolder, currentApplet);
        placeHolder.parent = configurationArea;
        currentApplet.z = 1;

        if (root.myView.alignment === LatteCore.Types.Justify) {
            fastLayoutManager.moveAppletsBasedOnJustifyAlignment();
        }

        fastLayoutManager.save();
        layouter.updateSizeForAppletsInFill();
    }

    onWheel: {
        if (!currentApplet || !currentApplet.latteStyleApplet) {
            return;
        }

        var angle = wheel.angleDelta.y / 8;

        if (angle > 12)
            currentApplet.latteStyleApplet.increaseLength();
        else if (angle < 12)
            currentApplet.latteStyleApplet.decreaseLength();
    }

    Connections {
        target: currentApplet
        onWidthChanged: {
            if (configurationArea.pressed && root.isHorizontal) {
                currentApplet.x = configurationArea.lastX - currentApplet.width/2;
            }
        }

        onHeightChanged: {
            if (configurationArea.pressed && root.isVertical) {
                currentApplet.y = configurationArea.lastY - currentApplet.height/2;
            }
        }
    }

    Item {
        id: placeHolder
        visible: configurationArea.pressed
        width: currentApplet !== null ? (root.isVertical ? currentApplet.width : Math.min(root.maxLength / 2, currentApplet.width)) : 0
        height: currentApplet !== null ? (!root.isVertical ? currentApplet.height : Math.min(root.maxLength / 2, currentApplet.height)) : 0

        readonly property bool isPlaceHolder: true
        readonly property int length: root.isVertical ? height : width
    }

    Timer {
        id: hideTimer
        interval: animations.duration.large * 2
        onTriggered: {
            if (!tooltipMouseArea.containsMouse) {
                tooltip.visible = false;
                currentApplet = null;
            }
        }
    }

    Item {
        id: handle
        parent: currentApplet ? currentApplet : configurationArea
        anchors.fill: parent
        visible: currentApplet && (configurationArea.containsMouse || tooltipMouseArea.containsMouse)

        Loader {
            anchors.fill: parent
            active: root.debug.graphicsEnabled
            sourceComponent: Rectangle {
                color: "transparent"
                border.width:1
                border.color: "yellow"
            }
        }

        //BEGIN functions
        //END functions

        Item {
            id: handleVisualItem
            width: root.isHorizontal ? parent.width : thickness
            height: root.isHorizontal ? thickness : parent.height

            readonly property int thickness: root.isHorizontal ? parent.height - metrics.margin.screenEdge : parent.width - metrics.margin.screenEdge

            Rectangle{
                anchors.fill: parent
                color: theme.backgroundColor
                radius: 3
                opacity: 0.35
            }

            PlasmaCore.IconItem {
                source: "transform-move"
                width: Math.min(144, root.metrics.iconSize)
                height: width
                anchors.centerIn: parent
                opacity: 0.9
                layer.enabled: root.environment.isGraphicsSystemAccelerated
                layer.effect: DropShadow {
                    radius: root.myView.itemShadow.size
                    fast: true
                    samples: 2 * radius
                    color: root.myView.itemShadow.shadowColor

                    verticalOffset: 2
                }
            }


            states:[
                State{
                    name: "bottom"
                    when: plasmoid.location === PlasmaCore.Types.BottomEdge

                    AnchorChanges{
                        target: handleVisualItem;
                        anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: undefined; anchors.top: undefined; anchors.bottom: parent.bottom;
                    }
                    PropertyChanges{
                        target: handleVisualItem;
                        anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: metrics.margin.screenEdge;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "top"
                    when: plasmoid.location === PlasmaCore.Types.TopEdge

                    AnchorChanges{
                        target: handleVisualItem;
                        anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: undefined;
                        anchors.right: undefined; anchors.left: undefined; anchors.top: parent.top; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: handleVisualItem;
                        anchors.leftMargin: 0;    anchors.rightMargin: 0;     anchors.topMargin: metrics.margin.screenEdge;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "left"
                    when: plasmoid.location === PlasmaCore.Types.LeftEdge

                    AnchorChanges{
                        target: handleVisualItem;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                        anchors.right: undefined; anchors.left: parent.left; anchors.top: undefined; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: handleVisualItem;
                        anchors.leftMargin: metrics.margin.screenEdge;    anchors.rightMargin: 0;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                },
                State{
                    name: "right"
                    when: plasmoid.location === PlasmaCore.Types.RightEdge

                    AnchorChanges{
                        target: handleVisualItem;
                        anchors.horizontalCenter: undefined; anchors.verticalCenter: parent.verticalCenter;
                        anchors.right: parent.right; anchors.left: undefined; anchors.top: undefined; anchors.bottom: undefined;
                    }
                    PropertyChanges{
                        target: handleVisualItem;
                        anchors.leftMargin: 0;    anchors.rightMargin: metrics.margin.screenEdge;     anchors.topMargin:0;    anchors.bottomMargin: 0;
                        anchors.horizontalCenterOffset: 0; anchors.verticalCenterOffset: 0;
                    }
                }
            ]

        }

        Behavior on opacity {
            NumberAnimation {
                duration: animations.duration.large
                easing.type: Easing.InOutQuad
            }
        }
    }
    PlasmaCore.Dialog {
        id: tooltip
        visualParent: currentApplet

        type: PlasmaCore.Dialog.Dock
        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.BypassWindowManagerHint | Qt.ToolTip
        location: plasmoid.location

        onVisualParentChanged: {
            if (visualParent && currentApplet
                    && (currentApplet.applet || currentApplet.isSeparator || currentApplet.isInternalViewSplitter)) {

                configureButton.visible = !currentApplet.isInternalViewSplitter
                        && (currentApplet.applet.pluginName !== "org.kde.latte.plasmoid")
                        && currentApplet.applet.action("configure")
                        && currentApplet.applet.action("configure").enabled;
                closeButton.visible = !currentApplet.isInternalViewSplitter && currentApplet.applet.action("remove") && currentApplet.applet.action("remove").enabled;
                lockButton.visible = !currentApplet.isInternalViewSplitter
                        && !currentApplet.communicator.indexerIsSupported
                        && !currentApplet.communicator.appletBlocksParabolicEffect
                        && !currentApplet.isSeparator;

                colorizingButton.visible = root.colorizerEnabled && !currentApplet.appletBlocksColorizing && !currentApplet.isInternalViewSplitter;

                label.text = currentApplet.isInternalViewSplitter ? i18n("Justify Splitter") : currentApplet.applet.title;
            }
        }

        mainItem: MouseArea {
            id: tooltipMouseArea
            enabled: currentApplet
            width: handleRow.childrenRect.width + (2 * handleRow.spacing)
            height: Math.max(configureButton.height, label.contentHeight, closeButton.height)
            hoverEnabled: true
            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            onEntered: hideTimer.stop();
            onExited: hideTimer.restart();

            Row {
                id: handleRow
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 2*units.smallSpacing

                Row{
                    spacing: units.smallSpacing
                    PlasmaComponents.ToolButton {
                        id: configureButton
                        anchors.verticalCenter: parent.verticalCenter
                        iconSource: "configure"
                        tooltip: i18n("Configure applet")
                        onClicked: {
                            tooltip.visible = false;
                            currentApplet.applet.action("configure").trigger();
                        }
                    }

                    PlasmaComponents.Label {
                        id: label
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: units.smallSpacing
                        textFormat: Text.PlainText
                        maximumLineCount: 1
                    }

                    Row{
                        spacing: units.smallSpacing/2

                        PlasmaComponents.ToolButton{
                            id: colorizingButton
                            checkable: true
                            iconSource: "color-picker"
                            tooltip: i18n("Enable painting  for this applet")

                            onClicked: {
                                fastLayoutManager.setOption(currentApplet.applet.id, "userBlocksColorizing", !checked);
                            }
                        }

                        PlasmaComponents.ToolButton{
                            id: lockButton
                            checkable: true
                            iconSource: checked ? "lock" : "unlock"
                            tooltip: i18n("Disable parabolic effect for this applet")

                            onClicked: {
                                fastLayoutManager.setOption(currentApplet.applet.id, "lockZoom", checked);
                            }
                        }

                        PlasmaComponents.ToolButton {
                            id: closeButton
                            anchors.verticalCenter: parent.verticalCenter
                            iconSource: "delete"
                            tooltip: i18n("Remove applet")
                            onClicked: {
                                tooltip.visible = false;
                                if(currentApplet && currentApplet.applet)
                                    currentApplet.applet.action("remove").trigger();
                            }
                        }
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "bottom"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

            AnchorChanges {
                target: configurationArea
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;
                    horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "top"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)

            AnchorChanges {
                target: configurationArea
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined;
                    horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "left"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

            AnchorChanges {
                target: configurationArea
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        State {
            name: "right"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)

            AnchorChanges {
                target: configurationArea
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;
                    horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        }
    ]
}
