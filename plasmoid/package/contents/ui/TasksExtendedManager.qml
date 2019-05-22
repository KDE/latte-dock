/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
*
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

import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte

//! Trying to WORKAROUND all the Plasma LibTaskManager limitations
//! concerning Tasks AND Launchers.
//!
//! Plasma LibTaskManager constantly creates ADDITIONS/REMOVALS when
//! a Task is changing its type from Launcher<->Startup<->Window.
//! This libtaskmanager behavior limits a lot the animations that
//! can be created in order to provide a moder user experience
//!
//! All the logic that is trying to improve the mentioned limits is provided
//! from this class

Item {
    id: tasksExtManager

    /// Launchers that are playing an ADD or REMOVAL animation
    /// and their Starups/Windows should be aware of
    property variant waitingLaunchers: []

    //! Launchers that must be shown IMMEDIATELY after a window removal
    //! because they are already present from a present libtaskmanager state
    property variant immediateLaunchers: []

    //! New launchers in order to be moved in correct place:
    //! launcher, pos)
    property variant launchersToBeMoved: []

    //! Tasks that change state (launcher,startup,window) and
    //! at the next state must look the same concerning the parabolic effect:
    //! (id, mScale)
    property variant frozenTasks: []


    signal waitingLauncherRemoved(string launch);


    /////////// FUNCTIONALITY ////////////////////


    /// WAITING LAUNCHERS
    function addWaitingLauncher(launch){
        if (waitingLauncherExists(launch)) {
            return;
        }

        waitingLaunchers.push(launch);
    }

    function removeWaitingLauncher(launch){
        for(var i=0; i<waitingLaunchers.length; ++i){
            if (equalsWaitingLauncher(waitingLaunchers[i], launch)) {
                waitingLaunchers.splice(i,1);
                waitingLauncherRemoved(launch);
                return;
            }
        }
    }

    function waitingLauncherExists(launch){
        for(var i=0; i<waitingLaunchers.length; ++i){
            if (equalsWaitingLauncher(waitingLaunchers[i], launch)) {
                return true;
            }
        }

        return false;
    }

    function equalsWaitingLauncher(waitingLauncher, launcher) {
        var equals = ( launcher !== ""
                      && waitingLauncher !== ""
                      && (launcher.indexOf(waitingLauncher) >= 0 || waitingLauncher.indexOf(launcher) >= 0));

        return equals;
    }

    function waitingLaunchersLength() {
        return waitingLaunchers.length;
    }

    //! IMMEDIATELAUNCHERS
    function addImmediateLauncher(launch){
        if (!immediateLauncherExists(launch)) {
            //console.log("Immediate Launcher Added::: "+launch);
            immediateLaunchers.push(launch);
        }
    }

    function removeImmediateLauncher(launch){
        for(var i=0; i<immediateLaunchers.length; ++i){
            if (immediateLaunchers[i]===launch) {
                immediateLaunchers.splice(i,1);
                //console.log("Immediate Launcher Removed::: "+launch);
                return;
            }
        }
    }

    function immediateLauncherExists(launch){
        for(var i=0; i<immediateLaunchers.length; ++i){
            if (immediateLaunchers[i]===launch) {
                return true;
            }
        }

        return false;
    }
    //!

    //! FROZENTASKS
    function getFrozenTask(identifier) {
        for(var i=0; i<frozenTasks.length; ++i) {
            if (frozenTasks[i].id === identifier) {
                return frozenTasks[i];
            }
        }
    }

    function removeFrozenTask(identifier) {
        var taskIndex = -1;
        for(var i=0; i<frozenTasks.length; ++i) {
            if (frozenTasks[i].id === identifier) {
                taskIndex = i;
            }
        }

        if (taskIndex > -1) {
            frozenTasks.splice(taskIndex, 1);
        }
    }

    function setFrozenTask(identifier, scale) {
        var frozenTaskExists = false;
        console.log("SET FROZEN :: "+identifier+" - "+scale);
        var frozenTask = getFrozenTask(identifier);

        if (frozenTask) {
            frozenTask.mScale = scale;
        } else {
            frozenTasks.push({id: identifier, mScale: scale});
        }
    }


    //! LAUNCHERSTOBEMOVED

    //! launchersToBeMoved, new launchers to have been added and must be repositioned
    function addLauncherToBeMoved(launcherUrl, toPos) {
        if (!isLauncherToBeMoved(launcherUrl)) {
            launchersToBeMoved.push({launcher: launcherUrl, pos: Math.max(0,toPos)});
            //console.log("-add launcher-");
            //printLaunchersToBeMoved()
        }
    }

    function printLaunchersToBeMoved(){
        for (var j=0; j<launchersToBeMoved.length; ++j){
            console.log(launchersToBeMoved[j].launcher+ " - "+launchersToBeMoved[j].pos);
        }
    }

    function moveLauncherToCorrectPos(launcherUrl, from) {
        if (isLauncherToBeMoved(launcherUrl)) {
            launchersToBeMovedTimer.from = from;
            launchersToBeMovedTimer.to = posOfLauncherToBeMoved(launcherUrl);
            launchersToBeMovedTimer.launcherUrl = launcherUrl

            removeLauncherToBeMoved(launcherUrl);
            launchersToBeMovedTimer.start();
        }
    }

    function removeLauncherToBeMoved(launcherUrl) {
        if (isLauncherToBeMoved(launcherUrl)) {
            var sLength = launchersToBeMoved.length;
            var index = -1;

            for (var i=0; i<sLength; ++i) {
                //!safety checker
                if (i>=launchersToBeMoved.length)
                    return -1;

                if (launchersToBeMoved[i].launcher === launcherUrl) {
                    index = i;
                    break;
                }
            }

            if (index > -1) {
                // console.log("removing launcher to be moved:: "+launcherUrl);
                launchersToBeMoved.splice(index, 1);
            }
        }
    }

    function posOfLauncherToBeMoved(launcherUrl) {
        var sLength = launchersToBeMoved.length;

        for (var i=0; i<sLength; ++i) {
            //!safety checker
            if (i>=launchersToBeMoved.length)
                return -1;

            if (launchersToBeMoved[i].launcher === launcherUrl)
                return launchersToBeMoved[i].pos;
        }

        return -1;
    }

    function isLauncherToBeMoved(launcher) {
        return (posOfLauncherToBeMoved(launcher) >= 0);
    }

    //!Trying to avoid a binding loop in TaskItem for modelLauncherUrl
    Timer {
        id: launchersToBeMovedTimer
        interval: 50
        property int from: -1
        property int to: -1

        property string launcherUrl: ""

        onTriggered: {
            //console.log("to be moved: "+launcherUrl + " - " + from +" -> "+to)

            tasksModel.move(from, to);
            /*if (latteView && latteView.launchersGroup >= Latte.Types.LayoutLaunchers) {
                latteView.layoutsManager.launchersSignals.moveTask(root.viewLayoutName,
                                                                   plasmoid.id,
                                                                   latteView.launchersGroup,
                                                                   from,
                                                                   to);
            }*/

            delayedLaynchersSyncTimer.start();
        }
    }

    //! delay a bit  the launchers syncing in order to avoid a crash
    //! the crash was caused from TasksExtendedManager when adding and moving a launcher (e.g. internal separator)
    //! and there were more than one synced docks
    Timer {
        id: delayedLaynchersSyncTimer
        interval: 450
        onTriggered: tasksModel.syncLaunchers();
    }
}
