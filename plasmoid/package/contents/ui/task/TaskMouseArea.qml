/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.tasks 0.1 as LatteTasks

MouseArea {
    id: taskMouseArea
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton | Qt.MidButton | Qt.RightButton
    hoverEnabled: taskItem.visible && (!inAnimation) && (!isStartup) && (!root.taskInAnimation)
                  &&(!inBouncingAnimation) && !isSeparator

    property bool pressed: false

    readonly property alias hoveredTimer: _hoveredTimer

    Connections {
        target: taskMouseArea
        onPressed: taskItem.mousePressed(mouse.x, mouse.y, mouse.button)
        onReleased: taskItem.mouseReleased(mouse.x, mouse.y, mouse.button)
    }

    onEntered: {
        if (isLauncher && windowsPreviewDlg.visible) {
            windowsPreviewDlg.hide(1);
        }

        //! show previews if enabled
        if(isAbleToShowPreview && !showPreviewsIsBlockedFromReleaseEvent && !isLauncher
                && (((root.showPreviews || (windowsPreviewDlg.visible && !isLauncher))
                     && windowsPreviewDlg.activeItem !== taskItem)
                    || root.highlightWindows)){

            if (!root.disableAllWindowsFunctionality) {
                //! don't delay showing preview in normal states,
                //! that is when the dock wasn't hidden
                if (!hoveredTimer.running && !windowsPreviewDlg.visible) {
                    //! first task with no previews shown can trigger the delay
                    hoveredTimer.start();
                } else if (windowsPreviewDlg.visible) {
                    //! when the previews are already shown, update them immediately
                    taskItem.showPreviewWindow();

                    if (taskItem.isWindow && root.highlightWindows) {
                        root.windowsHovered(model.WinIdList, taskItem.containsMouse);
                    }
                }
            }
        }

        taskItem.showPreviewsIsBlockedFromReleaseEvent = false;

        if (root.autoScrollTasksEnabled) {
            scrollableList.autoScrollFor(taskItem, false);
        }
    }

    onExited: {
        taskItem.isAbleToShowPreview = true;

        if (root.showPreviews) {
            root.hidePreview(17.5);
        }
    }

    // IMPORTANT: This must be improved ! even for small milliseconds  it reduces performance
    onPositionChanged: {
        if (taskItem.abilities.myView.isReady && !taskItem.abilities.myView.isShownFully) {
            return;
        }

        if((inAnimation == false)&&(!root.taskInAnimation)&&(!root.disableRestoreZoom) && hoverEnabled){
            // mouse.button is always 0 here, hence checking with mouse.buttons
            if (pressX != -1 && mouse.buttons == Qt.LeftButton
                    && isDragged
                    && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y) ) {
                root.dragSource = taskItem;
                dragHelper.startDrag(taskItem, model.MimeType, model.MimeData,
                                     model.LauncherUrlWithoutIcon, model.decoration);
                pressX = -1;
                pressY = -1;
            }
        }
    }

    onContainsMouseChanged:{
        if(!containsMouse && !inAnimation) {
            pressed=false;
        }

        ////disable hover effect///
        if (isWindow && root.highlightWindows && !containsMouse) {
            root.windowsHovered(model.WinIdList, false);
        }
    }

    onPressed: {
        //console.log("Pressed Task Delegate..");
        if (LatteCore.WindowSystem.compositingActive && !LatteCore.WindowSystem.isPlatformWayland) {
            if(root.leftClickAction !== LatteTasks.Types.PreviewWindows) {
                isAbleToShowPreview = false;
                windowsPreviewDlg.hide(2);
            }
        }

        slotPublishGeometries();

        var modAccepted = modifierAccepted(mouse);

        if ((mouse.button == Qt.LeftButton)||(mouse.button == Qt.MidButton) || modAccepted) {
            lastButtonClicked = mouse.button;
            pressed = true;
            pressX = mouse.x;
            pressY = mouse.y;

            if(!modAccepted){
                _resistanerTimer.start();
            }
        }
        else if (mouse.button == Qt.RightButton && !modAccepted){
            // When we're a launcher, there's no window controls, so we can show all
            // places without the menu getting super huge.
            if (model.IsLauncher === true && !isSeparator) {
                showContextMenu({showAllPlaces: true})
            } else {
                showContextMenu();
            }
        }
    }

    onReleased: {
        //console.log("Released Task Delegate...");
        _resistanerTimer.stop();

        if(pressed && (!inBlockingAnimation || inAttentionBuiltinAnimation) && !isSeparator){

            if (modifierAccepted(mouse) && !root.disableAllWindowsFunctionality){
                if( !taskItem.isLauncher ){
                    if (root.modifierClickAction == LatteTasks.Types.NewInstance) {
                        tasksModel.requestNewInstance(modelIndex());
                    } else if (root.modifierClickAction == LatteTasks.Types.Close) {
                        tasksModel.requestClose(modelIndex());
                    } else if (root.modifierClickAction == LatteTasks.Types.ToggleMinimized) {
                        tasksModel.requestToggleMinimized(modelIndex());
                    } else if ( root.modifierClickAction == LatteTasks.Types.CycleThroughTasks) {
                        if (isGroupParent)
                            subWindows.activateNextTask();
                        else
                            activateTask();
                    } else if (root.modifierClickAction == LatteTasks.Types.ToggleGrouping) {
                        tasksModel.requestToggleGrouping(modelIndex());
                    }
                } else {
                    activateTask();
                }
            } else if (mouse.button == Qt.MidButton && !root.disableAllWindowsFunctionality){
                if( !taskItem.isLauncher ){
                    if (root.middleClickAction == LatteTasks.Types.NewInstance) {
                        tasksModel.requestNewInstance(modelIndex());
                    } else if (root.middleClickAction == LatteTasks.Types.Close) {
                        tasksModel.requestClose(modelIndex());
                    } else if (root.middleClickAction == LatteTasks.Types.ToggleMinimized) {
                        tasksModel.requestToggleMinimized(modelIndex());
                    } else if ( root.middleClickAction == LatteTasks.Types.CycleThroughTasks) {
                        if (isGroupParent)
                            subWindows.activateNextTask();
                        else
                            activateTask();
                    } else if (root.middleClickAction == LatteTasks.Types.ToggleGrouping) {
                        tasksModel.requestToggleGrouping(modelIndex());
                    }
                } else {
                    activateTask();
                }
            } else if (mouse.button == Qt.LeftButton){
                var canPresentWindowsIsSupported = false;

                if (root.plasmaAtLeast525) {
                    //! At least Plasma 5.25 case
                    canPresentWindowsIsSupported = LatteCore.WindowSystem.compositingActive && backend.windowViewAvailable;
                } else if (root.plasmaGreaterThan522) {
                    //! At least Plasma 5.23 case
                    canPresentWindowsIsSupported = LatteCore.WindowSystem.compositingActive && backend.canPresentWindows;
                } else {
                    //! past Plasma versions
                    canPresentWindowsIsSupported = LatteCore.WindowSystem.compositingActive && backend.canPresentWindows();
                }

                if( !taskItem.isLauncher && !root.disableAllWindowsFunctionality ){
                    if ( (root.leftClickAction === LatteTasks.Types.PreviewWindows && isGroupParent)
                            || ( !canPresentWindowsIsSupported
                                && root.leftClickAction === LatteTasks.Types.PresentWindows
                                && isGroupParent) ) {
                        if(windowsPreviewDlg.activeItem !== taskItem || !windowsPreviewDlg.visible){
                            showPreviewWindow();
                        } else {
                            forceHidePreview(21.1);
                        }
                    } else if ( (root.leftClickAction === LatteTasks.Types.PresentWindows && !(isGroupParent && !LatteCore.WindowSystem.compositingActive))
                               || ((root.leftClickAction === LatteTasks.Types.PreviewWindows && !isGroupParent)) ) {
                        activateTask();
                    } else if (root.leftClickAction === LatteTasks.Types.CycleThroughTasks) {
                        if (isGroupParent) {
                            subWindows.activateNextTask();
                        } else {
                            activateTask();
                        }
                    }
                } else {
                    activateTask();
                }
            }

            backend.cancelHighlightWindows();
        }

        pressed = false;
    }

    onWheel: {
        var wheelActionsEnabled = (root.taskScrollAction !== LatteTasks.Types.ScrollNone || root.manualScrollTasksEnabled);

        if (isSeparator
                || wheelIsBlocked
                || !wheelActionsEnabled
                || inBouncingAnimation
                || !taskItem.abilities.myView.isShownFully){

            return;
        }

        var angleVertical = wheel.angleDelta.y / 8;
        var angleHorizontal = wheel.angleDelta.x / 8;

        wheelIsBlocked = true;
        scrollDelayer.start();

        var verticalDirection = (Math.abs(angleVertical) > Math.abs(angleHorizontal));
        var mainAngle = verticalDirection ? angleVertical : angleHorizontal;

        var positiveDirection = (mainAngle > 12);
        var negativeDirection = (mainAngle < -12);

        var parallelScrolling = (verticalDirection && plasmoid.formFactor === PlasmaCore.Types.Vertical)
                || (!verticalDirection && plasmoid.formFactor === PlasmaCore.Types.Horizontal);

        if (positiveDirection) {
            slotPublishGeometries();

            var overflowScrollingAccepted = (root.manualScrollTasksEnabled
                                             && scrollableList.contentsExceed
                                             && (root.manualScrollTasksType === LatteTasks.Types.ManualScrollVerticalHorizontal
                                                 || (root.manualScrollTasksType === LatteTasks.Types.ManualScrollOnlyParallel && parallelScrolling)) );


            if (overflowScrollingAccepted) {
                scrollableList.decreasePos();
            } else {
                if (isLauncher || root.disableAllWindowsFunctionality) {
                    taskItem.activateLauncher();
                } else if (isGroupParent) {
                    subWindows.activateNextTask();
                } else {
                    var taskIndex = modelIndex();

                    if (isMinimized) {
                        tasksModel.requestToggleMinimized(taskIndex);
                    }

                    tasksModel.requestActivate(taskIndex);
                }

                // hidePreviewWindow();
            }
        } else if (negativeDirection) {
            slotPublishGeometries();

            var overflowScrollingAccepted = (root.manualScrollTasksEnabled
                                             && scrollableList.contentsExceed
                                             && (root.manualScrollTasksType === LatteTasks.Types.ManualScrollVerticalHorizontal
                                                 || (root.manualScrollTasksType === LatteTasks.Types.ManualScrollOnlyParallel && parallelScrolling)) );


            if (overflowScrollingAccepted) {
                scrollableList.increasePos();
            } else {
                if (isLauncher || root.disableAllWindowsFunctionality) {
                    // do nothing
                } else if (isGroupParent) {
                    if (root.taskScrollAction === LatteTasks.Types.ScrollToggleMinimized) {
                        subWindows.minimizeTask();
                    } else {
                        subWindows.activatePreviousTask();
                    }
                } else {
                    var taskIndex = modelIndex();

                    var hidingTask = (!isMinimized && root.taskScrollAction === LatteTasks.Types.ScrollToggleMinimized);

                    if (isMinimized || hidingTask) {
                        tasksModel.requestToggleMinimized(taskIndex);
                    }

                    if (!hidingTask) {
                        tasksModel.requestActivate(taskIndex);
                    }
                }

                // hidePreviewWindow();
            }
        }
    }

    //A Timer to check how much time the task is hovered in order to check if we must
    //show window previews
    Timer {
        id: _hoveredTimer
        interval: Math.max(150,plasmoid.configuration.previewsDelay)
        repeat: false

        onTriggered: {
            if (root.disableAllWindowsFunctionality || !isAbleToShowPreview) {
                return;
            }

            if (taskItem.containsMouse) {
                if (root.showPreviews || (windowsPreviewDlg.visible && !isLauncher)) {
                    taskItem.showPreviewWindow();
                }

                if (taskItem.isWindow && root.highlightWindows) {
                    root.windowsHovered(model.WinIdList, taskItem.containsMouse);
                }
            }
        }
    }

    //A Timer to help in resist a bit to dragging, the user must try
    //to press a little first before dragging Started
    Timer {
        id: _resistanerTimer
        interval: taskItem.resistanceDelay
        repeat: false

        onTriggered: {
            if (!taskItem.inBlockingAnimation){
                taskItem.isDragged = true;
            }

            if (taskItem.abilities.debug.timersEnabled) {
                console.log("plasmoid timer: resistanerTimer called...");
            }
        }
    }

}
