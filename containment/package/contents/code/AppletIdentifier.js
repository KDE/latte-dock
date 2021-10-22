/*
    SPDX-FileCopyrightText: 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

//! applets that haven't identified properly their Latte behavior and
//! create broken results when used in Latte
var blacklistedApplets = ["org.kde.redshiftControl",
                          "org.kde.plasma.notifications",
                          "menuZ"];

function typeOf(obj, className){
    var name = obj.toString();
    //if (applet.pluginName === "org.kde.plasma.kicker") //"set a plugin name to debug"
    //    console.log(name);
    return ((name.indexOf(className + "(") === 0) || (name.indexOf(className + "_QML") === 0));
}

function reconsiderAppletIconItem(){
    if (communicator.appletIconItem || !applet || communicator.indexerIsSupported)
        return;

    //! searching to find for that applet the first IconItem
    //! which is going to be used in order to deactivate its active
    //! from our MouseArea

    if (applet.pluginName === "org.kde.plasma.kickoff"
            || applet.pluginName === "org.kde.plasma.kickofflegacy") {
        identifyKickOff();
    } else if (applet.pluginName === "org.kde.plasma.kicker") {
        identifyKicker();
    } else if (applet.pluginName === "org.kde.plasma.notes") {
        identifyNotes();
    } else if (applet.pluginName === "org.kde.plasma.simplemenu") {
        identifySimpleMenu();
    } else if (applet.pluginName === "org.kde.plasma.userswitcher") {
        identifyUserSwitcher();
    } else if (applet.pluginName === "org.kde.comexpertise.plasma.kdeconnect.sms") {
        identifyKdeConnectSms();
    } else if (applet.pluginName === "org.kde.redshiftControl") {
        //blacklist
    } else {
        identifyGeneric();
    }
}

function checkAndUpdateAppletRootItem() {
    if (appletDiscoveredRootItem || communicator.indexerIsSupported) {
        return;
    }

    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        if (!appletDiscoveredRootItem && level0[i].hasOwnProperty("latteBridge")) {
            appletDiscoveredRootItem = level0[i];
        }
        if (appletDiscoveredRootItem) {
            break;
        }

        for(var j=0; j<level1.length; ++j){
            if (!appletDiscoveredRootItem && level1[j].hasOwnProperty("latteBridge")) {
                appletDiscoveredRootItem = level1[j];
            }
            if (appletDiscoveredRootItem) {
                break;
            }
        }
    }
}

function identifyGeneric() {
    if (blacklistedApplets.indexOf(applet.pluginName) >= 0) {
        return;
    }

    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        for(var j=0; j<level1.length; ++j){
            if (typeOf(level1[j], "CompactRepresentation")) {
                var level2 = level1[j].children;
                for(var k=0; k<level2.length; ++k){
                    if (typeOf(level2[k], "IconItem")) {
                        communicator.appletIconItem = level2[k];
                        return;
                    }
                }
            } else if (typeOf(level1[j], "IconItem")) {
                communicator.appletIconItem = level1[j];
                return;
            }
        }
    }
}

function identifyKdeConnectSms()
{
    if (applet.pluginName !== "org.kde.comexpertise.plasma.kdeconnect.sms")
        return;

    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        for(var j=0; j<level1.length; ++j){
            var level2 = level1[j].children;
            for(var k=0; k<level2.length; ++k){
                if (typeOf(level2[k], "IconItem")) {
                    communicator.appletIconItem = level2[k];
                    return;
                }
            }
        }

    }
}

function identifyKickOff() {
    if (applet.pluginName !== "org.kde.plasma.kickoff"
            && applet.pluginName !== "org.kde.plasma.kickofflegacy") {
        return;
    }

    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        for(var j=0; j<level1.length; ++j){
            if (typeOf(level1[j], "QQuickMouseArea")) {
                var level2 = level1[j].children;
                for(var k=0; k<level2.length; ++k){
                    if (typeOf(level2[k], "IconItem")) {
                        communicator.appletIconItem = level2[k];
                        return;
                    }
                }
            }
        }
    }
}

function findIconItem(currentChildren, steps) {
    var nextitems = currentChildren;
    var stepsFollowed = 0;

    for (var i=0; i<steps.length; ++i) {
        for (var j=0; j<nextitems.length; ++j) {
            var item = nextitems[j];
            if (typeOf(item, steps[i])) {
                stepsFollowed++;
                nextitems = item.children;
            }
        }

        if (stepsFollowed === steps.length) {
            break;
        }
    }

    if (stepsFollowed === steps.length) {
        for (var k=0; k<nextitems.length; ++k) {
            var lastItem = nextitems[k];
            if (typeOf(lastItem, "IconItem")) {
                return lastItem;
            }
        }
    }

    return null;
}

function identifyNotes() {
    var level0 = applet.parent.children;
    var stepsInCompactRepresentation = ["QQuickMouseArea"];

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        for(var j=0; j<level1.length; ++j){
            if (typeOf(level1[j], "CompactApplet")) {

                var icon = findIconItem(level1[j].compactRepresentation.children, stepsInCompactRepresentation)
                if (icon) {
                    communicator.appletIconItem = icon;
                    return;
                }
            }
        }
    }
}

function identifyUserSwitcher() {
    if (applet.pluginName !== "org.kde.plasma.userswitcher")
        return;

    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        for(var j=0; j<level1.length; ++j){
            if (applet.pluginName === "org.kde.plasma.userswitcher" && !root.behaveAsPlasmaPanel && !container.originalAppletBehavior) {
                if (typeOf(level1[j], "QQuickMouseArea")) {
                    var level2 = level1[j].children;
                    for(var k=0; k<level2.length; ++k){
                        if (typeOf(level2[k], "QQuickRow")) {
                            var iconIt;
                            var labelIt;
                            var level3 = level2[k].children;

                            for(var r=0; r<level3.length; ++r){
                                if (typeOf(level3[r], "IconItem")) {
                                    iconIt = level3[r];
                                } else if (typeOf(level3[r], "QQuickText")) {
                                    labelIt = level3[r];
                                }
                            }
                            if (iconIt && labelIt && !labelIt.visible){
                                communicator.appletIconItem = iconIt;
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

function identifyKicker() {
    if (applet.pluginName !== "org.kde.plasma.kicker")
        return;

    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        for(var j=0; j<level1.length; ++j){
            if (typeOf(level1[j], "CompactRepresentation")) {
                var level2 = level1[j].children;
                var iconIt;
                var imageIt;

                for(var k=0; k<level2.length; ++k){
                    if (typeOf(level2[k], "IconItem")) {
                        iconIt = level2[k];
                    } else if (typeOf(level2[k], "QQuickImage")) {
                        imageIt = level2[k];
                    }
                }

                if (iconIt) {
                    communicator.appletIconItem = iconIt;
                }

                if (imageIt){
                    communicator.appletImageItem = imageIt;
                }

                return;
            }
        }
    }
}

function identifySimpleMenu() {
    if (applet.pluginName !== "org.kde.plasma.simplemenu")
        return;

    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        var iconIt;
        var imageIt;

        for(var j=0; j<level1.length; ++j){
            if (typeOf(level1[j], "IconItem")) {
                iconIt = level1[j];
            } else if (typeOf(level1[j], "QQuickImage")) {
                imageIt = level1[j];
            }
        }

        if (iconIt) {
            communicator.appletIconItem = iconIt;
        }

        if (imageIt){
            communicator.appletImageItem = imageIt;
        }

        return;
    }
}
