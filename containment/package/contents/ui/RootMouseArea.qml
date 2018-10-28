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
            drawWindowTimer.start();
        }
    }

    onDoubleClicked: {
        drawWindowTimer.stop();
        restoreGrabberTimer.stop();
        tasksModel.requestToggleMaximized(tasksModel.activeTask);
    }

    Timer {
        id: drawWindowTimer
        interval: 350
        onTriggered: {
            if (mainArea.pressed && dock.visibility.activeWindowCanBeDragged()) {
                dock.disableGrabItemBehavior();
                dock.visibility.requestMoveActiveWindow(mainArea.mouseX, mainArea.mouseY);
                restoreGrabberTimer.start();
            }
        }
    }

    Timer {
        id: restoreGrabberTimer
        interval: 50
        onTriggered: {
            dock.restoreGrabItemBehavior();
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
