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

import QtQuick 2.0
import QtQml.Models 2.2

//trying to do a very simple thing to count how many windows does
//a task instance has...
//Workaround the mess with launchers, startups, windows etc.

Item{
    id: windowsContainer
    property int windowsCount: {
        if (isLauncher) {
            return 0;
        }

        if (isGroupParent) {
            return windowsRepeater.count;
        }

        return 1;
    }

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
                        windowsContainer.lastActiveWinInGroup = root.plasma515 ? (WinIdList!==undefined ? WinIdList[0] : 0) : (LegacyWinIdList!==undefined ? LegacyWinIdList[0] : 0);
                    }
                    windowsContainer.updateStates();
                }
            }

            Component.onCompleted: {
                rootIndex = mainItemContainer.modelIndex();
            }
        }
    }

    Connections{
        target: mainItemContainer
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
        windowsLocalModel.rootIndex = mainItemContainer.modelIndex();

        hasMinimized = false;
        hasShown = false;
        hasActive = false;

        if(IsGroupParent){
            checkInternalStates();
        }
        else{
            if(mainItemContainer.isActive)
                hasActive = true;

            if(mainItemContainer.isMinimized){
                hasMinimized = true;
            } else {
                hasShown = true;
            }
        }
    }

    function checkInternalStates(){
        var childs = windowsLocalModel.items;

        for(var i=0; i<childs.count; ++i){
            var kid = childs.get(i);

            if (kid.model.IsActive)
                hasActive = true;

            if(kid.model.IsMinimized)
                hasMinimized = true;
            else
                hasShown = true;
        }
    }

    function windowsTitles() {
        windowsLocalModel.rootIndex = mainItemContainer.modelIndex();
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
        windowsLocalModel.rootIndex = mainItemContainer.modelIndex();

        if (!mainItemContainer.isGroupParent) {
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
                var kidId = root.plasma515 ? (kid.model.WinIdList ? kid.model.WinIdList[0] : 0) : (kid.model.LegacyWinIdList ? kid.model.LegacyWinIdList[0] : 0);

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
        windowsLocalModel.rootIndex = mainItemContainer.modelIndex();

        if (!mainItemContainer.isGroupParent) {
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
                if ( (root.plasma515 ? kid.model.WinIdList[0] : kid.model.LegacyWinIdList[0]) === lastActiveWinInGroup) {
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

    Component.onCompleted: {
        mainItemContainer.checkWindowsStates.connect(initializeStates);
    }

    Component.onDestruction: {
        mainItemContainer.checkWindowsStates.disconnect(initializeStates);
    }
}
