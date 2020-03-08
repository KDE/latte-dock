/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte

import "loaders" as Loaders
import "indicator" as Indicator
import "../applet/indicator" as AppletIndicator

Loader {
    id: environmentLoader

    width: active ? item.width : 0
    height: active ? item.height: 0

    property int alignment: Latte.Types.BottomEdgeCenterAlign

    sourceComponent: MouseArea{
        id: mainArea
        width: root.isHorizontal ? length : localThickness + root.localScreenEdgeMargin
        height: root.isVertical ? length :  localThickness + root.localScreenEdgeMargin

        acceptedButtons: Qt.LeftButton | Qt.MidButton

        readonly property int localThickness: (root.isHovered ? (root.iconSize + root.thickMargins)*root.zoomFactor : (root.iconSize + root.thickMargins))
        readonly property int length: {
            if (screenEdgeMarginEnabled && plasmoid.configuration.fittsLawIsRequested) {
                return root.isHorizontal ? root.width : root.height;
            }

            return useAllLayouts ? root.maxLength : root.realPanelLength;
        }

        property bool wheelIsBlocked: false

        hoverEnabled: true

        readonly property bool useAllLayouts: panelUserSetAlignment === Latte.Types.Justify && !root.inConfigureAppletsMode

        property int lastPressX: -1
        property int lastPressY: -1

        onContainsMouseChanged: {
            if (root.mouseInHoverableArea()) {
                root.stopCheckRestoreZoomTimer();
            } else {
                root.initializeHoveredIndexes();
                root.startCheckRestoreZoomTimer()
            }
        }

        onClicked: {
            if (root.closeActiveWindowEnabled && mouse.button === Qt.MidButton) {
                selectedWindowsTracker.lastActiveWindow.requestClose();
            }
        }

        onPressed: {
            if (!root.dragActiveWindowEnabled) {
                return;
            }

            if (mouse.button === Qt.LeftButton && selectedWindowsTracker.lastActiveWindow.canBeDragged()) {
                lastPressX = mouse.x;
                lastPressY = mouse.y;
                dragWindowTimer.start();
            }
        }

        onReleased: {
            lastPressX = -1;
            lastPressY = -1;
        }

        onPositionChanged: {
            if (!root.dragActiveWindowEnabled || !(mainArea.pressedButtons & Qt.LeftButton)) {
                return;
            }

            var stepX = Math.abs(lastPressX-mouse.x);
            var stepY = Math.abs(lastPressY-mouse.y);
            var threshold = 5;

            var tryDrag = mainArea.pressed && (stepX>threshold || stepY>threshold);

            if ( tryDrag && selectedWindowsTracker.lastActiveWindow.canBeDragged()) {
                dragWindowTimer.stop();
                activateDragging();
            }
        }

        onDoubleClicked: {
            if (!root.dragActiveWindowEnabled) {
                return;
            }

            dragWindowTimer.stop();
            selectedWindowsTracker.lastActiveWindow.requestToggleMaximized();
        }

        onWheel: {
            if (wheelIsBlocked) {
                return;
            }

            if (root.scrollAction === Latte.Types.ScrollNone) {
                root.emptyAreasWheel(wheel);
                return;
            }

            wheelIsBlocked = true;
            scrollDelayer.start();

            var delta = 0;

            if (wheel.angleDelta.y>=0 && wheel.angleDelta.x>=0) {
                delta = Math.max(wheel.angleDelta.y, wheel.angleDelta.x);
            } else {
                delta = Math.min(wheel.angleDelta.y, wheel.angleDelta.x);
            }

            var angle = delta / 8;

            var ctrlPressed = (wheel.modifiers & Qt.ControlModifier);

            if (angle>10) {
                //! upwards
                if (root.scrollAction === Latte.Types.ScrollDesktops) {
                    latteView.windowsTracker.switchToPreviousVirtualDesktop();
                } else if (root.scrollAction === Latte.Types.ScrollActivities) {
                    latteView.windowsTracker.switchToPreviousActivity();
                } else if (root.scrollAction === Latte.Types.ScrollToggleMinimized) {
                    if (!ctrlPressed) {
                        tasksLoader.item.activateNextPrevTask(true);
                    } else if (!selectedWindowsTracker.lastActiveWindow.isMaximized){
                        selectedWindowsTracker.lastActiveWindow.requestToggleMaximized();
                    }
                } else if (tasksLoader.active) {
                    tasksLoader.item.activateNextPrevTask(true);
                }
            } else if (angle<-10) {
                //! downwards
                if (root.scrollAction === Latte.Types.ScrollDesktops) {
                    latteView.windowsTracker.switchToNextVirtualDesktop();
                } else if (root.scrollAction === Latte.Types.ScrollActivities) {
                    latteView.windowsTracker.switchToNextActivity();
                } else if (root.scrollAction === Latte.Types.ScrollToggleMinimized) {
                    if (!ctrlPressed) {
                        if (selectedWindowsTracker.lastActiveWindow.isValid
                                && !selectedWindowsTracker.lastActiveWindow.isMinimized
                                && selectedWindowsTracker.lastActiveWindow.isMaximized){
                            //! maximized
                            selectedWindowsTracker.lastActiveWindow.requestToggleMaximized();
                        } else if (selectedWindowsTracker.lastActiveWindow.isValid
                                   && !selectedWindowsTracker.lastActiveWindow.isMinimized
                                   && !selectedWindowsTracker.lastActiveWindow.isMaximized) {
                            //! normal
                            selectedWindowsTracker.lastActiveWindow.requestToggleMinimized();
                        }
                    } else if (selectedWindowsTracker.lastActiveWindow.isMaximized) {
                        selectedWindowsTracker.lastActiveWindow.requestToggleMaximized();
                    }
                } else if (tasksLoader.active) {
                    tasksLoader.item.activateNextPrevTask(false);
                }
            }
        }

        Loaders.Tasks{
            id: tasksLoader
        }

        function activateDragging(){
            selectedWindowsTracker.requestMoveLastWindow(mainArea.mouseX, mainArea.mouseY);
            mainArea.lastPressX = -1;
            mainArea.lastPressY = -1;
        }

        //! Timers
        Timer {
            id: dragWindowTimer
            interval: 500
            onTriggered: {
                if (mainArea.pressed && selectedWindowsTracker.lastActiveWindow.canBeDragged()) {
                    mainArea.activateDragging();
                }
            }
        }

        //! A timer is needed in order to handle also touchpads that probably
        //! send too many signals very fast. This way the signals per sec are limited.
        //! The user needs to have a steady normal scroll in order to not
        //! notice a annoying delay
        Timer{
            id: scrollDelayer

            interval: 200
            onTriggered: mainArea.wheelIsBlocked = false;
        }

        //! Background Indicator
        Indicator.Bridge{
            id: indicatorBridge
        }

        //! Indicator Back Layer
        Indicator.Loader{
            id: indicatorBackLayer
            level: AppletIndicator.LevelOptions {
                id: backLevelOptions
                isBackground: true
                bridge: indicatorBridge
            }
        }
    }

    states:[
        State {
            name: "bottomCenter"
            when: (alignment === Latte.Types.BottomEdgeCenterAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:_mainLayout.bottom; left:undefined; right:undefined;
                    horizontalCenter: _mainLayout.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomLeft"
            when: (alignment === Latte.Types.BottomEdgeLeftAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:_mainLayout.bottom; left:parent.left; right:undefined;
                    horizontalCenter: undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomRight"
            when: (alignment === Latte.Types.BottomEdgeRightAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom: _mainLayout.bottom; left:undefined; right:parent.right;
                    horizontalCenter: undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "topCenter"
            when: (alignment === Latte.Types.TopEdgeCenterAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top: _mainLayout.top; bottom:undefined; left:undefined; right:undefined;
                    horizontalCenter: _mainLayout.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "topLeft"
            when: (alignment === Latte.Types.TopEdgeLeftAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top: _mainLayout.top; bottom:undefined; left: _mainLayout.left; right:undefined;
                    horizontalCenter: undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "topRight"
            when: (alignment === Latte.Types.TopEdgeRightAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top: _mainLayout.top; bottom:undefined; left:undefined; right: _mainLayout.right;
                    horizontalCenter: undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "leftCenter"
            when: (alignment === Latte.Types.LeftEdgeCenterAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:undefined; left: _mainLayout.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter: _mainLayout.verticalCenter}
            }
        },
        State {
            name: "leftTop"
            when: (alignment === Latte.Types.LeftEdgeTopAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:mainLayout.top; bottom:undefined; left: _mainLayout.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter: undefined}
            }
        },
        State {
            name: "leftBottom"
            when: (alignment === Latte.Types.LeftEdgeBottomAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:_mainLayout.bottom; left: _mainLayout.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter: undefined}
            }
        },
        State {
            name: "rightCenter"
            when: (alignment === Latte.Types.RightEdgeCenterAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:undefined; left:undefined; right: _mainLayout.right;
                    horizontalCenter:undefined; verticalCenter: _mainLayout.verticalCenter}
            }
        },
        State {
            name: "rightTop"
            when: (alignment === Latte.Types.RightEdgeTopAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:_mainLayout.top; bottom:undefined; left:undefined; right: _mainLayout.right;
                    horizontalCenter:undefined; verticalCenter: undefined}
            }
        },
        State {
            name: "rightBottom"
            when: (alignment === Latte.Types.RightEdgeBottomAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:_mainLayout.bottom; left:undefined; right: _mainLayout.right;
                    horizontalCenter:undefined; verticalCenter: undefined}
            }
        }
    ]
}
