
function typeOf(obj, className){
    var name = obj.toString();
    //if (applet.pluginName === "org.kde.plasma.kicker") //"set a plugin name to debug"
    //    console.log(name);
    return ((name.indexOf(className + "(") === 0) || (name.indexOf(className + "_QML") === 0));
}

function reconsiderAppletIconItem(){
    if (container.appletIconItem || !applet)
        return;

    //! searching to find for that applet the first IconItem
    //! which is going to be used in order to deactivate its active
    //! from our MouseArea

    if (applet.pluginName === "org.kde.plasma.kickoff") {
        identifyKickOff();
    } else if (applet.pluginName === "org.kde.plasma.kicker") {
        identifyKicker();
    } else if (applet.pluginName === "org.kde.plasma.simplemenu") {
        identifySimpleMenu();
    } else if (applet.pluginName === "org.kde.plasma.userswitcher"&& !root.behaveAsPlasmaPanel && !container.lockZoom) {
        identifyUserSwitcher();
    } else {
        identifyGeneric();
    }
}

function identifyGeneric() {
    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        for(var j=0; j<level1.length; ++j){
            if (typeOf(level1[j], "CompactRepresentation")) {
                var level2 = level1[j].children;
                for(var k=0; k<level2.length; ++k){
                    if (typeOf(level2[k], "IconItem")) {
                        container.appletIconItem = level2[k];
                        return;
                    }
                }
            } else if (typeOf(level1[j], "IconItem")) {
                container.appletIconItem = level1[j];
                return;
            }
        }
    }
}

function identifyKickOff() {
    if (applet.pluginName !== "org.kde.plasma.kickoff")
        return;

    var level0 = applet.children;

    for(var i=0; i<level0.length; ++i){
        var level1 = level0[i].children;
        for(var j=0; j<level1.length; ++j){
            if (applet.pluginName === "org.kde.plasma.kickoff") {
                if (typeOf(level1[j], "QQuickMouseArea")) {
                    var level2 = level1[j].children;
                    for(var k=0; k<level2.length; ++k){
                        if (typeOf(level2[k], "IconItem")) {
                            container.appletIconItem = level2[k];
                            return;
                        }
                    }
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
            if (applet.pluginName === "org.kde.plasma.userswitcher" && !root.behaveAsPlasmaPanel && !container.lockZoom) {
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
                                container.appletIconItem = iconIt;
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
                    container.appletIconItem = iconIt;
                }

                if (imageIt){
                   container.appletImageItem = imageIt;
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
            container.appletIconItem = iconIt;
        }

        if (imageIt){
            container.appletImageItem = imageIt;
        }

        return;
    }
}
