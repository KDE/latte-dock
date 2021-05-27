/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.core 2.0 as PlasmaCore

Rectangle {
    id: _corner
    width: 22
    height: width
    anchors.horizontalCenter: parent.right
    anchors.verticalCenter: parent.top
    rotation: 45
    color: resizeWindowMouseArea.isActive ? theme.buttonFocusColor : theme.textColor
    opacity: resizeWindowMouseArea.isActive ? 1 : 0.2

    readonly property alias isActive: resizeWindowMouseArea.isActive

    MouseArea {
        id: resizeWindowMouseArea
        anchors.fill: parent
        hoverEnabled: true

        cursorShape: plasmoid.location === PlasmaCore.Types.LeftEdge ? Qt.SizeBDiagCursor : Qt.SizeFDiagCursor

        readonly property bool isActive: containsMouse || pressed

        property bool initialized: false
        property int initGlobalX: 0
        property int initGlobalY: 0
        property int initWidth: 100
        property int initHeight: 100
        property real initScaleWidth: 1
        property real initScaleHeight: 1

        property int curGlobalX: 0
        property int curGlobalY: 0

        onPressed: {
            if (pressed) {
                var scenePos = mapToGlobal(mouse.x, mouse.y);

                if (plasmoid.location !== PlasmaCore.Types.LeftEdge) {
                    initGlobalX = viewConfig.x + mouse.x;
                    initGlobalY = viewConfig.y + mouse.y;
                } else {
                    initGlobalX = viewConfig.x + mouse.x;
                    initGlobalY = viewConfig.y + mouse.y;
                }

                initWidth = dialog.width;
                initHeight = dialog.height;
                initScaleWidth = dialog.userScaleWidth;
                initScaleHeight = dialog.userScaleHeight;
                initialized = true;
            }
        }

        onPositionChanged: {
            if (pressed && initialized) {
                var scenePos = mapToGlobal(mouse.x, mouse.y);
                if (plasmoid.location !== PlasmaCore.Types.LeftEsge) {
                    curGlobalX = viewConfig.x + mouse.x;
                    curGlobalY = viewConfig.y + mouse.y;
                } else {
                    curGlobalX = viewConfig.x + mouse.x;
                    curGlobalY = viewConfig.y + mouse.y;
                }

                var differX = 0;

                if (plasmoid.location !== PlasmaCore.Types.LeftEdge) {
                    differX = initGlobalX - curGlobalX;
                } else {
                    differX = curGlobalX - initGlobalX;
                }

                //! In normal settings mode for horizontal View when dragging the corner the length must be increased two times
                //! in order to be in the exact position when finished
                var percentXMultiplier = (!dialog.advancedLevel && plasmoid.formFactor===PlasmaCore.Types.Horizontal ? 2 : 1);

                var percentX = percentXMultiplier * (differX / initWidth);
                var newScaleWidth = Math.max(0.35, initScaleWidth + (percentX*initScaleWidth)).toFixed(3);

                var newScaleHeight = dialog.userScaleHeight;

                if (!dialog.advancedLevel) {
                    var differY = 0;

                    if (plasmoid.location !== PlasmaCore.Types.LeftEdge) {
                        if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                            differY = curGlobalY - initGlobalY;
                        } else {
                            differY = initGlobalY - curGlobalY;
                        }
                    } else {
                        differY = initGlobalY - curGlobalY;
                    }

                    var percentYMultiplier = (!dialog.advancedLevel && plasmoid.formFactor===PlasmaCore.Types.Vertical ? 2 : 1);
                    var percentY = percentYMultiplier * (differY / initHeight);
                    newScaleHeight = Math.max(0.5, initScaleHeight + (percentY*initScaleHeight)).toFixed(3);
                }

                universalSettings.setScreenScales(latteView.positioner.currentScreenName, newScaleWidth, newScaleHeight);
                dialog.userScaleWidth = newScaleWidth;
                dialog.userScaleHeight = newScaleHeight;
                viewConfig.syncGeometry();
            } else if (!pressed) {
                initialized = false;
            }
        }

        onReleased: {
            initialized = false;
        }

        onDoubleClicked: {
            dialog.userScaleWidth = 1;
            dialog.userScaleHeight = 1;
            universalSettings.setScreenScales(latteView.positioner.currentScreenName, 1, 1);
            viewConfig.syncGeometry();
        }
    }

    states:[
        State{
            name: "bottom"
            when: plasmoid.location === PlasmaCore.Types.BottomEdge

            AnchorChanges{
                target: _corner;
                anchors.horizontalCenter: parent.left; anchors.verticalCenter: parent.top;
            }
        },
        State{
            name: "top"
            when: plasmoid.location === PlasmaCore.Types.TopEdge

            AnchorChanges{
                target: _corner;
                anchors.horizontalCenter: parent.left; anchors.verticalCenter: parent.top;
            }
        },
        State{
            name: "left"
            when: plasmoid.location === PlasmaCore.Types.LeftEdge

            AnchorChanges{
                target: _corner;
                anchors.horizontalCenter: parent.right; anchors.verticalCenter: parent.top;
            }
        },
        State{
            name: "right"
            when: plasmoid.location === PlasmaCore.Types.RightEdge

            AnchorChanges{
                target: _corner;
                anchors.horizontalCenter: parent.left; anchors.verticalCenter: parent.top;
            }
        }
    ]
}
