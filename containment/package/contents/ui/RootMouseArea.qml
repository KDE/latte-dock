/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.taskmanager 0.1 as TaskManager

MouseArea{
    id: mainArea

    anchors.fill: parent
    hoverEnabled: true

    property int lastPressX: -1
    property int lastPressY: -1

    onContainsMouseChanged: {
        if (mouseInHoverableArea()) {
            stopCheckRestoreZoomTimer();
        } else {
            initializeHoveredIndexes();
            startCheckRestoreZoomTimer()
        }
    }

    onPressed: {
        if (dock.visibility.activeWindowCanBeDragged()) {
            lastPressX = mouse.x;
            lastPressY = mouse.y;
            drawWindowTimer.start();
        }
    }

    onReleased: {
        lastPressX = -1;
        lastPressY = -1;
    }

    onPositionChanged: {
        var stepX = Math.abs(lastPressX-mouse.x);
        var stepY = Math.abs(lastPressY-mouse.y);
        var threshold = 5;

        var tryDrag = mainArea.pressed && (stepX>threshold || stepY>threshold);

        if ( tryDrag && dock.visibility.activeWindowCanBeDragged()) {
            drawWindowTimer.stop();
            activateDragging();
        }
    }

    onDoubleClicked: {
        drawWindowTimer.stop();
        restoreGrabberTimer.stop();
        tasksModel.requestToggleMaximized(tasksModel.activeTask);
    }

    function activateDragging(){
        dock.disableGrabItemBehavior();
        dock.visibility.requestMoveActiveWindow(mainArea.mouseX, mainArea.mouseY);
        restoreGrabberTimer.start();
    }

    Timer {
        id: drawWindowTimer
        interval: 500
        onTriggered: {
            if (mainArea.pressed && dock.visibility.activeWindowCanBeDragged()) {
                mainArea.activateDragging();
            }
        }
    }

    Timer {
        id: restoreGrabberTimer
        interval: 50
        onTriggered: {
            dock.restoreGrabItemBehavior();
            mainArea.lastPressX = -1;
            mainArea.lastPressY = -1;
        }
    }

    ////////// Dragging windows etc....
    TaskManager.TasksModel {
        id: tasksModel
        sortMode: TaskManager.TasksModel.SortVirtualDesktop
        groupMode: TaskManager.TasksModel.GroupDisabled

        virtualDesktop: virtualDesktopInfo.currentDesktop
        activity: activityInfo.currentActivity
        screenGeometry: plasmoid.screenGeometry

        filterByVirtualDesktop: true
        filterByScreen: true
        filterByActivity: true
    }
    TaskManager.VirtualDesktopInfo {
        id: virtualDesktopInfo
    }

    TaskManager.ActivityInfo {
        id: activityInfo
    }
    ////////// Dragging windows etc....
}
