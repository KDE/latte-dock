/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore

import org.kde.latte.private.containment 0.1 as LatteContainment

import org.kde.latte.abilities.items 0.1 as AbilityItem

import "loaders" as Loaders

Loader {
    id: environmentLoader

    width: root.isHorizontal ? length : localThickness
    height: root.isVertical ? length :  localThickness

    property int alignment: LatteCore.Types.BottomEdgeCenterAlign

    readonly property bool useAllLayouts: root.myView.alignment === LatteCore.Types.Justify

    readonly property int localThickness: active ? metrics.totals.thickness + metrics.margin.screenEdge : 0
    readonly property int length: {
        if (!active) {
            return 0;
        }

        if (screenEdgeMarginEnabled && root.floatingInternalGapIsForced) {
            return root.isHorizontal ? root.width : root.height;
        }

        return useAllLayouts ? root.maxLength : background.totals.visualLength;
    }

    sourceComponent: MouseArea{
        id: mainArea
        acceptedButtons: Qt.LeftButton | Qt.MidButton
        hoverEnabled: true

        property bool wheelIsBlocked: false

        property int lastPressX: -1
        property int lastPressY: -1

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

            if (root.scrollAction === LatteContainment.Types.ScrollNone) {
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
                if (root.scrollAction === LatteContainment.Types.ScrollDesktops) {
                    latteView.windowsTracker.switchToPreviousVirtualDesktop();
                } else if (root.scrollAction === LatteContainment.Types.ScrollActivities) {
                    latteView.windowsTracker.switchToPreviousActivity();
                } else if (root.scrollAction === LatteContainment.Types.ScrollToggleMinimized) {
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
                if (root.scrollAction === LatteContainment.Types.ScrollDesktops) {
                    latteView.windowsTracker.switchToNextVirtualDesktop();
                } else if (root.scrollAction === LatteContainment.Types.ScrollActivities) {
                    latteView.windowsTracker.switchToNextActivity();
                } else if (root.scrollAction === LatteContainment.Types.ScrollToggleMinimized) {
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
        AbilityItem.IndicatorLevel {
            id: indicatorBackLayer
            anchors.fill: parent

            level.isDrawn: root.indicators.isEnabled
            level.isBackground: true
            level.indicator: AbilityItem.IndicatorObject{
                animations: root.animations
                metrics: root.metrics
                host: root.indicators

                isEmptySpace: true
                isPressed: mainArea.pressed
                panelOpacity: root.background.currentOpacity
                shadowColor: root.myView.itemShadow.shadowSolidColor
                palette: colorizerManager.applyTheme

                iconBackgroundColor: "brown"
                iconGlowColor: "pink"
            }

            Connections {
                target: mainArea
                enabled: root.indicators.info.needsMouseEventCoordinates
                onPressed: indicatorBackLayer.level.mousePressed(mouse.x, mouse.y, mouse.button);
                onReleased: indicatorBackLayer.level.mouseReleased(mouse.x, mouse.y, mouse.button);
            }
        }
    }

    states:[
        State {
            name: "bottomCenter"
            when: (alignment === LatteCore.Types.BottomEdgeCenterAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:_mainLayout.bottom; left:undefined; right:undefined;
                    horizontalCenter: _mainLayout.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomLeft"
            when: (alignment === LatteCore.Types.BottomEdgeLeftAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:_mainLayout.bottom; left:_mainLayout.left; right:undefined;
                    horizontalCenter: undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomRight"
            when: (alignment === LatteCore.Types.BottomEdgeRightAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom: _mainLayout.bottom; left:undefined; right:_mainLayout.right;
                    horizontalCenter: undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "topCenter"
            when: (alignment === LatteCore.Types.TopEdgeCenterAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top: _mainLayout.top; bottom:undefined; left:undefined; right:undefined;
                    horizontalCenter: _mainLayout.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "topLeft"
            when: (alignment === LatteCore.Types.TopEdgeLeftAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top: _mainLayout.top; bottom:undefined; left: _mainLayout.left; right:undefined;
                    horizontalCenter: undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "topRight"
            when: (alignment === LatteCore.Types.TopEdgeRightAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top: _mainLayout.top; bottom:undefined; left:undefined; right: _mainLayout.right;
                    horizontalCenter: undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "leftCenter"
            when: (alignment === LatteCore.Types.LeftEdgeCenterAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:undefined; left: _mainLayout.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter: _mainLayout.verticalCenter}
            }
        },
        State {
            name: "leftTop"
            when: (alignment === LatteCore.Types.LeftEdgeTopAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:mainLayout.top; bottom:undefined; left: _mainLayout.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter: undefined}
            }
        },
        State {
            name: "leftBottom"
            when: (alignment === LatteCore.Types.LeftEdgeBottomAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:_mainLayout.bottom; left: _mainLayout.left; right:undefined;
                    horizontalCenter:undefined; verticalCenter: undefined}
            }
        },
        State {
            name: "rightCenter"
            when: (alignment === LatteCore.Types.RightEdgeCenterAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:undefined; left:undefined; right: _mainLayout.right;
                    horizontalCenter:undefined; verticalCenter: _mainLayout.verticalCenter}
            }
        },
        State {
            name: "rightTop"
            when: (alignment === LatteCore.Types.RightEdgeTopAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:_mainLayout.top; bottom:undefined; left:undefined; right: _mainLayout.right;
                    horizontalCenter:undefined; verticalCenter: undefined}
            }
        },
        State {
            name: "rightBottom"
            when: (alignment === LatteCore.Types.RightEdgeBottomAlign)

            AnchorChanges {
                target: environmentLoader
                anchors{ top:undefined; bottom:_mainLayout.bottom; left:undefined; right: _mainLayout.right;
                    horizontalCenter:undefined; verticalCenter: undefined}
            }
        }
    ]
}
