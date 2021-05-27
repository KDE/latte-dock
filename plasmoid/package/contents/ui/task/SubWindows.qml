/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQml.Models 2.2

//trying to do a very simple thing to count how many windows does
//a task instance has...
//Workaround the mess with launchers, startups, windows etc.

Item{
    id: windowsContainer
    property int windowsCount: {
        if (isLauncher || isStartup) {
            return 0;
        }

        if (isGroupParent) {
            return windowsRepeater.count;
        }

        return 1;
    }

    property int windowsMinimized: 0

    property bool isLauncher: IsLauncher ? true : false
    property bool isStartup: IsStartup ? true : false
    property bool isWindow: IsWindow ? true : false

    property int lastActiveWinInGroup: -1

    //states that exist in windows in a Group of windows
    property bool hasMinimized: false;
    property bool hasShown: false;
    property bool hasActive: false;

    Repeater{
        id: windowsRepeater
        model:DelegateModel {
            id: windowsLocalModel
            model: tasksModel

            delegate: Item{
                readonly property string title: display !== undefined ? display : ""
                readonly property bool isMinimized: IsMinimized === true ? true : false
                readonly property bool isActive: IsActive === true ? true : false

                onIsMinimizedChanged: windowsContainer.updateStates();
                onIsActiveChanged:  {
                    if (isActive) {
                        var winIdList = (root.plasma515 ? WinIdList : LegacyWinIdList);
                        windowsContainer.lastActiveWinInGroup = (winIdList!==undefined ? winIdList[0] : 0);
                    }
                    windowsContainer.updateStates();
                }
            }

            Component.onCompleted: {
                rootIndex = taskItem.modelIndex();
            }
        }
    }

    Connections{
        target: taskItem
        onItemIndexChanged: windowsContainer.updateStates();
    }

    Connections{
        target: root
        onInDraggingPhaseChanged: windowsContainer.updateStates();
    }

    //! try to give the time to the model to update its states in order to
    //! avoid any suspicious crashes during dragging grouped tasks that
    //! are synced between multiple panels/docks. At the same time in updateStates()
    //! function we block any DelegateModel updates when the user is dragging
    //! a task because this could create crashes
    Timer{
        id: initializeStatesTimer
        interval: 200
        onTriggered: windowsContainer.initializeStates();
    }

    function updateStates() {
        if (!root.inDraggingPhase) {
            initializeStatesTimer.start();
        }
    }

    function initializeStates(){
        windowsLocalModel.rootIndex = taskItem.modelIndex();

        hasMinimized = false;
        hasShown = false;
        hasActive = false;

        if(IsGroupParent){
            checkInternalStates();
        } else {
            var minimized = 0;

            if(taskItem.isActive)
                hasActive = true;

            if(taskItem.isMinimized){
                hasMinimized = true;
                minimized = minimized + 1;
            } else if (taskItem.isWindow) {
                hasShown = true;
            }

            windowsMinimized = minimized;
        }
    }

    function checkInternalStates(){
        var childs = windowsLocalModel.items;

        var minimized = 0;

        for(var i=0; i<childs.count; ++i){
            var kid = childs.get(i);

            if (kid.model.IsActive)
                hasActive = true;

            if(kid.model.IsMinimized) {
                hasMinimized = true;
                minimized = minimized + 1;
            } else if (kid.model.IsWindow) {
                hasShown = true;
            }
        }

        windowsMinimized = minimized;
    }

    function windowsTitles() {
        windowsLocalModel.rootIndex = taskItem.modelIndex();
        var result = new Array;
        var childs = windowsLocalModel.items;

        for(var i=0; i<childs.count; ++i){
            var kid = childs.get(i);
            var title = kid.model.display

            result.push(title);
        }

        return result;
    }

    //! function which is used to cycle activation into
    //! a group of windows
    function activateNextTask() {
        windowsLocalModel.rootIndex = taskItem.modelIndex();

        if (!taskItem.isGroupParent) {
            return;
        }

        var childs = windowsLocalModel.items;
        var nextAvailableWindow = -1;

        for(var i=0; i<childs.count; ++i){
            var kid = childs.get(i);
            if (kid.model.IsActive === true) {
                nextAvailableWindow = i + 1;
                break;
            }
        }

        //the active window is the last one
        if (nextAvailableWindow >= childs.count) {
            nextAvailableWindow = 0;
        }

        if (nextAvailableWindow === -1 && lastActiveWinInGroup !==-1){
            for(var i=0; i<childs.count; ++i){
                var kid = childs.get(i);
                var winIdList = (root.plasma515 ? kid.model.WinIdList : kid.model.LegacyWinIdList);
                var kidId = (winIdList !== undefined ? winIdList[0] : 0);

                if (kidId === lastActiveWinInGroup) {
                    nextAvailableWindow = i;
                    break;
                }
            }
        }

        if (nextAvailableWindow === -1)
            nextAvailableWindow = 0;

        tasksModel.requestActivate(tasksModel.makeModelIndex(index,nextAvailableWindow));
    }

    //! function which is used to cycle activation into
    //! a group of windows backwise
    function activatePreviousTask() {
        windowsLocalModel.rootIndex = taskItem.modelIndex();

        if (!taskItem.isGroupParent) {
            return;
        }

        var childs = windowsLocalModel.items;

        //indicates than nothing was found
        var prevAvailableWindow = -2;

        for(var i=childs.count-1; i>=0; --i){
            var kid = childs.get(i);
            if (kid.model.IsActive === true) {
                prevAvailableWindow = i - 1;
                break;
            }
        }

        //the active window is 0
        if (prevAvailableWindow == -1) {
            prevAvailableWindow = childs.count-1;
        }

        if (prevAvailableWindow === -2 && lastActiveWinInGroup !==-1){
            for(var i=childs.count-1; i>=0; --i){
                var kid = childs.get(i);
                var firstTask = (root.plasma515 ? kid.model.WinIdList[0] : kid.model.LegacyWinIdList[0]);
                if ( firstTask === lastActiveWinInGroup) {
                    prevAvailableWindow = i;
                    break;
                }
            }
        }

        //no window was found
        if (prevAvailableWindow === -2)
            prevAvailableWindow = 0;

        tasksModel.requestActivate(tasksModel.makeModelIndex(index,prevAvailableWindow));
    }

    //! function which is used to cycle activation into
    //! a group of windows backwise
    function minimizeTask() {
        windowsLocalModel.rootIndex = taskItem.modelIndex();

        if (!taskItem.isGroupParent) {
            return;
        }

        var childs = windowsLocalModel.items;

        //indicates than nothing was found
        var availableWindow = -1;

        for(var i=childs.count-1; i>=0; --i){
            var kid = childs.get(i);
            if (kid.model.IsActive === true) {
                availableWindow = i;
                break;
            }
        }

        if (availableWindow === -1 && lastActiveWinInGroup !==-1){
            for(var i=childs.count-1; i>=0; --i){
                var kid = childs.get(i);
                var firstTask = (root.plasma515 ? kid.model.WinIdList[0] : kid.model.LegacyWinIdList[0]);
                var isMinimized = kid.model.IsMinimized === true;

                if (firstTask === lastActiveWinInGroup && !isMinimized) {
                    availableWindow = i;
                    break;
                }
            }
        }

        //no window was found
        if (availableWindow === -1) {
            for(var i=childs.count-1; i>=0; --i){
                var kid = childs.get(i);
                if (kid.model.IsMinimized !== true) {
                    availableWindow = i;
                    break;
                }
            }
        }

        if (availableWindow !== -1) {
            tasksModel.requestToggleMinimized(tasksModel.makeModelIndex(index,availableWindow));
        }
    }

    Component.onCompleted: {
        taskItem.checkWindowsStates.connect(initializeStates);
    }

    Component.onDestruction: {
        taskItem.checkWindowsStates.disconnect(initializeStates);
    }
}
