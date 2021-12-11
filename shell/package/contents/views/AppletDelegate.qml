/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.4
import QtQuick.Layouts 1.1

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0

Item {
    id: delegate

    readonly property string pluginName: model.pluginName
    readonly property bool pendingUninstall: pendingUninstallTimer.applets.indexOf(pluginName) > -1

    width: list.cellWidth
    height: list.cellHeight

    DragArea {
        anchors.fill: parent
        supportedActions: Qt.MoveAction | Qt.LinkAction
        //onDragStarted: tooltipDialog.visible = false
        delegateImage: decoration
        enabled: !delegate.pendingUninstall
        mimeData {
            source: parent
        }
        Component.onCompleted: mimeData.setData("text/x-plasmoidservicename", pluginName)

        onDragStarted: {
            kwindowsystem.showingDesktop = true;
            main.draggingWidget = true;
        }
        onDrop: {
            main.draggingWidget = false;
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onDoubleClicked: {
                if (!delegate.pendingUninstall) {
                    widgetExplorer.addApplet(pluginName);
                    latteView.extendedInterface.appletCreated(pluginName);
                }
            }
            onEntered: delegate.GridView.view.currentIndex = index
            onExited: delegate.GridView.view.currentIndex = - 1
        }

        ColumnLayout {
            id: mainLayout
            spacing: units.smallSpacing
            anchors {
                left: parent.left
                right: parent.right
                //bottom: parent.bottom
                margins: units.smallSpacing * 2
                rightMargin: units.smallSpacing * 2 // don't cram the text to the border too much
                top: parent.top
            }

            Item {
                id: iconContainer
                width: units.iconSizes.enormous
                height: width
                Layout.alignment: Qt.AlignHCenter
                opacity: delegate.pendingUninstall ? 0.6 : 1
                Behavior on opacity {
                    OpacityAnimator {
                        duration: units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                Item {
                    id: iconWidget
                    anchors.fill: parent
                    PlasmaCore.IconItem {
                        anchors.fill: parent
                        source: model.decoration
                        visible: model.screenshot === ""
                    }
                    Image {
                        width: units.iconSizes.enormous
                        height: width
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        source: model.screenshot
                    }
                }

                Item {
                    id: badgeMask
                    anchors.fill: parent

                    Rectangle {
                        x: Math.round(-units.smallSpacing * 1.5 / 2)
                        y: x
                        width: runningBadge.width + Math.round(units.smallSpacing * 1.5)
                        height: width
                        radius: height
                        visible: running && delegate.GridView.isCurrentItem
                    }
                }

                Rectangle {
                    id: runningBadge
                    width: height
                    height: Math.round(theme.mSize(countLabel.font).height * 1.3)
                    radius: height
                    color: theme.highlightColor
                    visible: running && delegate.GridView.isCurrentItem
                    onVisibleChanged: maskShaderSource.scheduleUpdate()

                    PlasmaComponents.Label {
                        id: countLabel
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: running
                    }
                }

                ShaderEffect {
                    anchors.fill: parent
                    property var source: ShaderEffectSource {
                        sourceItem: iconWidget
                        hideSource: true
                        live: false
                    }
                    property var mask: ShaderEffectSource {
                        id: maskShaderSource
                        sourceItem: badgeMask
                        hideSource: true
                        live: false
                    }

                    supportsAtlasTextures: true

                    fragmentShader: "
                        varying highp vec2 qt_TexCoord0;
                        uniform highp float qt_Opacity;
                        uniform lowp sampler2D source;
                        uniform lowp sampler2D mask;
                        void main() {
                            gl_FragColor = texture2D(source, qt_TexCoord0.st) * (1.0 - (texture2D(mask, qt_TexCoord0.st).a)) * qt_Opacity;
                        }
                    "
                }

                PlasmaComponents.ToolButton {
                    id: uninstallButton
                    anchors {
                        top: parent.top
                        right: parent.right
                    }
                    iconSource: delegate.pendingUninstall ? "edit-undo" : "edit-delete"
                    // we don't really "undo" anything but we'll pretend to the user that we do
                    tooltip: delegate.pendingUninstall ? i18nd("plasma_shell_org.kde.plasma.desktop", "Undo uninstall")
                                                       : i18nd("plasma_shell_org.kde.plasma.desktop", "Uninstall widget")
                    flat: false
                    visible: model.local && delegate.GridView.isCurrentItem

                    onHoveredChanged: {
                        if (hovered) {
                            // hovering the uninstall button triggers onExited of the main mousearea
                            delegate.GridView.view.currentIndex = index
                        }
                    }

                    onClicked: {
                        var pending = pendingUninstallTimer.applets
                        if (delegate.pendingUninstall) {
                            var index = pending.indexOf(pluginName)
                            if (index > -1) {
                                pending.splice(index, 1)
                            }
                        } else {
                            pending.push(pluginName)
                        }
                        pendingUninstallTimer.applets = pending

                        if (pending.length) {
                            pendingUninstallTimer.restart()
                        } else {
                            pendingUninstallTimer.stop()
                        }
                    }
                }
            }
            PlasmaExtras.Heading {
                id: heading
                Layout.fillWidth: true
                level: 4
                text: model.name
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                lineHeight: 0.95
                horizontalAlignment: Text.AlignHCenter
            }
            PlasmaComponents.Label {
                Layout.fillWidth: true
                // otherwise causes binding loop due to the way the Plasma sets the height
                height: implicitHeight
                text: model.description
                font: theme.smallestFont
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                maximumLineCount: heading.lineCount === 1 ? 3 : 2
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
