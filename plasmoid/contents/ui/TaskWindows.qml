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
//a task instace has...
//Workaround the mess with launchers, startups, windows etc.

Item{
    id: windowsContainer
    property int windowsCount: 0

    property bool isLauncher: IsLauncher ? true : false
    property bool isStartup: IsStartup ? true : false
    property bool isWindow: IsWindow ? true : false

    onIsLauncherChanged: updateCounter();
    //  onIsStartupChanged: updateCounter();
    //    onIsWindowChanged: updateCounter();

    //states that exist in windows in a Group of windows
    property bool hasMinimized: false;
    property bool hasShown: false;
    property bool hasActive: false;

    //FIXME: For some reason the index is not updated correctly in some cases (e.g. window dragging, repositioning launchers)
    // and this way much beautiful information are lost, an activity change back and return,
    // it fixes this sometimes...
    DelegateModel {
        id: windowsLocalModel
        model: tasksModel //icList.model
        rootIndex: tasksModel.makeModelIndex(currentIndex >=0 ? currentIndex : index)

        property int currentIndex: -1

        delegate: Item{
            property string title: model.display
        }

        onCountChanged:{
            windowsContainer.updateCounter();
        }
    }

    function initializeStates(){
        hasMinimized = false;
        hasShown = false;
        hasActive = false;

        if(IsGroupParent){
            checkInternalStates();
        }
        else{
            if(mainItemContainer.isMinimized){
                hasMinimized = true;
            }
            else if(mainItemContainer.isActive)
                hasActive = true;
            else
                hasShown = true;
        }
    }

    function checkInternalStates(){
        windowsLocalModel.currentIndex = index;
        var childs = windowsLocalModel.items;

        for(var i=0; i<childs.count; ++i){
            var kid = childs.get(i);

            if(kid.model.IsMinimized)
                hasMinimized = true;
            else if (kid.model.IsActive)
                hasActive = true;
            else
                hasShown = true;
        }
    }

    function windowsTitles() {
        var result = new Array;

        windowsLocalModel.currentIndex = index;
        var childs = windowsLocalModel.items;

        for(var i=0; i<childs.count; ++i){
            var kid = childs.get(i);
            var title = kid.model.display

            //console.log(title);
            // FIXME: we may need a way to remove the app name from the end
            /*   var lst = title.lastIndexOf(" - ");
            if (lst > 0) {
                 title = title.substring(0, lst);
             }*/

            result.push(title);
        }

        return result;
    }

    //! function which is used to cycle activation into
    //! a group of windows
    function activateNextTask() {
        if (!mainItemContainer.isGroupParent) {
            return;
        }

        windowsLocalModel.currentIndex = index;
        var childs = windowsLocalModel.items;

        var nextAvailableWindow = 0;

        for(var i=0; i<childs.count; ++i){
            var kid = childs.get(i);
            if (kid.model.IsActive === true) {
                nextAvailableWindow = i + 1;
                break;
            }
        }

        if (nextAvailableWindow >= childs.count) {
            nextAvailableWindow = 0;
        }

        tasksModel.requestActivate(tasksModel.makeModelIndex(index,nextAvailableWindow));
    }


    Component.onCompleted: {
        mainItemContainer.checkWindowsStates.connect(initializeStates);
        updateCounter();
    }

    function updateCounter(){
        //    console.log("--------- "+ index+" -------");
        if(index>=0){
            if(IsGroupParent){
                windowsLocalModel.currentIndex = index;
                var tempC = windowsLocalModel.count;

                if (tempC == 0){
                    if(isLauncher){
                        windowsCount = 0;
                    }
                    else if(isWindow || isStartup){
                        windowsCount = 1;
                    }
                }
                else{
                    windowsCount = tempC;
                }
            }
            else{
                if(isLauncher){
                    windowsCount = 0;
                }
                else if(isWindow || isStartup){
                    windowsCount = 1;
                }
            }

            initializeStates();
        }

    }

}
