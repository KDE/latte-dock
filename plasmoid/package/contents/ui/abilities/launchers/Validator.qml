/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore

//! Launchers Validate Timer
Timer{
    id:launchersOrderValidatorTimer
    interval: 400

    property var launchers: []

    function launchersAreInSync() {
        return arraysAreEqual(_launchers.currentShownLauncherList(), launchers);
    }

    function launcherValidPos(url) {
        for (var i=0; i<launchers.length; ++i) {
            if (launchers[i] === url) {
                return i;
            }
        }

        return -1;
    }

    function arraysAreEqual(list1, list2) {
        if (list1.length !== list2.length) {
            console.log("  arrays have different size...")
            return false;
        }

        for (var i=0; i<list1.length; ++i) {
            if (list1[i] !== list2[i]) {
                return false;
            }
        }

        return true;
    }

    //! true if upward is the best way to iterate through current
    //! in order to make it equal with goal
    function upwardIsBetter(current, goal)
    {
        var tCurrent = current.slice();

        if (!arraysAreEqual(tCurrent, goal)) {
            for (var i=0; i<tCurrent.length; ++i) {
                if (tCurrent[i] !== goal[i]) {
                    var val = tCurrent[i];
                    tCurrent.splice(i, 1);
                    tCurrent.splice(goal.indexOf(val), 0, val);

                    if (arraysAreEqual(tCurrent, goal)){
                        return true;
                    } else {
                        return false;
                    }
                }
            }
        }

        return false;
    }

    onTriggered: {
        if (launchersAreInSync()) {
            stop();
            console.log("launchers synced at:" + launchers);
            launchers.length = 0;
            _launchers.tasksModel.syncLaunchers();
        } else {
            var currentLaunchers = _launchers.currentShownLauncherList();

            if (upwardIsBetter(currentLaunchers, launchers)) {
                console.log("UPWARD....");
                for (var i=0; i<currentLaunchers.length; ++i) {
                    if (currentLaunchers[i] !== launchers[i]) {
                        var p = launcherValidPos(currentLaunchers[i]);
                        if (p === -1) {
                            console.log("No pos found for :"+currentLaunchers[i] + " at: "+launchers);
                            restart();
                            return;
                        }
                        var launcherLayoutIndex = _launchers.indexOfLayoutLauncher(currentLaunchers[i]);

                        if (launcherLayoutIndex === -1) {
                            console.log(" launcher was not found in model, syncing stopped...");
                            stop();
                            return;
                        }

                        console.log(" moving:" +launcherLayoutIndex + " _ " + p );
                        _launchers.tasksModel.move(launcherLayoutIndex, p);
                        restart();
                        return;
                    }
                }
            } else {
                console.log("DOWNWARD....");
                for (var i=currentLaunchers.length-1; i>=0; --i) {
                    if (currentLaunchers[i] !== launchers[i]) {
                        var p = launcherValidPos(currentLaunchers[i]);
                        if (p === -1) {
                            console.log("No pos found for :"+currentLaunchers[i] + " at: "+launchers);
                            restart();
                            return;
                        }
                        var launcherLayoutIndex = _launchers.indexOfLayoutLauncher(currentLaunchers[i]);

                        if (launcherLayoutIndex === -1) {
                            console.log(" launcher was not found in model, syncing stopped...");
                            stop();
                            return;
                        }

                        console.log(" moving:" +launcherLayoutIndex + " _ " + p );
                        _launchers.tasksModel.move(launcherLayoutIndex, p);
                        restart();
                        return;
                    }
                }
            }

            console.log("why we reached ??? ");
            console.log("CURRENT ::: " + currentLaunchers);
            console.log("VALID   ::: " + launchers);
        }
    }
}
