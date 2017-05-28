/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.1

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.taskmanager 0.1 as TaskManager

Item{
    id: winModel

    property bool maximizedWindowOnScreen: false

    //--- active-window-control start
    TaskManager.VirtualDesktopInfo { id: virtualDesktopInfo }
    TaskManager.ActivityInfo { id: activityInfo }
    TaskManager.TasksModel {
        id: tasksModel
        sortMode: TaskManager.TasksModel.SortVirtualDesktop
        groupMode: TaskManager.TasksModel.GroupDisabled

        virtualDesktop: virtualDesktopInfo.currentDesktop
        activity: activityInfo.currentActivity

        screenGeometry: root.screenGeometry

        filterByVirtualDesktop: true
        filterByScreen: true
        filterByActivity: true

        Component.onCompleted: {
            windowsRepeater.model = tasksModel;
           // winModel.reconsiderMaximized();
        }
    }

    Repeater{
        id: windowsRepeater

        delegate: Item{
            readonly property bool isMaximized: IsMaximized === true ? true : false
            readonly property bool isMinimized: IsMinimized === true ? true : false

            onIsMinimizedChanged: winModel.reconsiderMaximized();
            onIsMaximizedChanged: winModel.reconsiderMaximized();
        }

        onCountChanged: winModel.reconsiderMaximized();
    }

    function reconsiderMaximized() {
        var hasMaximized = false;

        for(var i=0; i<windowsRepeater.count; ++i){
            var kid = windowsRepeater.itemAt(i);

            if (kid && kid.isMaximized && !kid.isMinimized){
                hasMaximized = true;
                break;
            }
        }

        maximizedWindowOnScreen = hasMaximized;
    }
}
