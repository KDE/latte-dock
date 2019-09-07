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

    sourceComponent: MouseArea{
        id: mainArea

        width:  {
            if (root.isHorizontal) {
                return useAllLayouts ? root.maxLength : root.realPanelLength;
            } else {
                return root.isHovered ? (root.iconSize + root.thickMargins)*root.zoomFactor : (root.iconSize + root.thickMargins)
            }
        }

        height: {
            if (root.isVertical) {
                return useAllLayouts ? root.maxLength : root.realPanelLength;
            } else {
                return root.isHovered ? (root.iconSize + root.thickMargins)*root.zoomFactor : (root.iconSize + root.thickMargins)
            }
        }

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

        onPressed: {
            if (!root.dragActiveWindowEnabled) {
                return;
            }

            if (selectedWindowsTracker.lastActiveWindow.canBeDragged()) {
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
            if (!root.dragActiveWindowEnabled) {
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
            if (root.scrollAction === Latte.Types.ScrollNone) {
                root.emptyAreasWheel(wheel);
                return;
            }

            var delta = 0;

            if (wheel.angleDelta.y>=0 && wheel.angleDelta.x>=0) {
                delta = Math.max(wheel.angleDelta.y, wheel.angleDelta.x);
            } else {
                delta = Math.min(wheel.angleDelta.y, wheel.angleDelta.x);
            }

            var angle = delta / 8;

            if (angle>10) {
                if (root.scrollAction === Latte.Types.ScrollDesktops) {
                    latteView.windowsTracker.switchToPreviousVirtualDesktop();
                } else if (root.scrollAction === Latte.Types.ScrollActivities) {
                    latteView.windowsTracker.switchToPreviousActivity();
                } else if (tasksLoader.active) {
                    tasksLoader.item.activateNextPrevTask(true);
                }
            } else if (angle<-10) {
                if (root.scrollAction === Latte.Types.ScrollDesktops) {
                    latteView.windowsTracker.switchToNextVirtualDesktop();
                } else if (root.scrollAction === Latte.Types.ScrollActivities) {
                    latteView.windowsTracker.switchToNextActivity();
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

        states:[
            State {
                name: "bottom"
                when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

                AnchorChanges {
                    target: mainArea
                    anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined;
                        horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
                }
            },
            State {
                name: "top"
                when: (plasmoid.location === PlasmaCore.Types.TopEdge)

                AnchorChanges {
                    target: mainArea
                    anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined;
                        horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
                }
            },
            State {
                name: "left"
                when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

                AnchorChanges {
                    target: mainArea
                    anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined;
                        horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
                }
            },
            State {
                name: "right"
                when: (plasmoid.location === PlasmaCore.Types.RightEdge)

                AnchorChanges {
                    target: mainArea
                    anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right;
                        horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
                }
            }
        ]
    }

    states:[
        State {
            name: "bottom"
            when: (plasmoid.location === PlasmaCore.Types.BottomEdge)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom: _mainLayout.bottom; left:undefined; right:undefined;
                    horizontalCenter: _mainLayout.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "top"
            when: (plasmoid.location === PlasmaCore.Types.TopEdge)

            AnchorChanges {
                target: environmentLoader
                anchors{ top: _mainLayout.top; bottom:undefined; left:undefined; right:undefined;
                    horizontalCenter: _mainLayout.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "left"
            when: (plasmoid.location === PlasmaCore.Types.LeftEdge)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:undefined; left: _mainLayout.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter: _mainLayout.verticalCenter}
            }
        },
        State {
            name: "right"
            when: (plasmoid.location === PlasmaCore.Types.RightEdge)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:undefined; left:undefined; right: _mainLayout.right;
                    horizontalCenter:undefined; verticalCenter: _mainLayout.verticalCenter}
            }
        }
    ]
}
