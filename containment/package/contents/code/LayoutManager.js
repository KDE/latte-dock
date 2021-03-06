/*
*  Copyright 2016        Smith AR <audoban@openmailbox.org>
*            2016-2018   Michail Vourlakos <mvourlakos@gmail.com>
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

var layout;
var layoutS;
var layoutE;
var root;
var metrics;
var plasmoid;

var childFoundId = 11;
var inRestore=false;

function restore() {
    inRestore = true;
    //rewrite, so if in the orders there were now invalid ids or if some were missing creates a correct list instead
    save();
    restoreOptions();
    inRestore = false;
    root.updateIndexes();
}

function restoreOptions() {
    restoreOption("lockedZoomApplets");
    restoreOption("userBlocksColorizingApplets");
}

function restoreOption(option) {
    var configString;

    if (option === "lockedZoomApplets") {
        configString = String(plasmoid.configuration.lockedZoomApplets);
    } else if (option === "userBlocksColorizingApplets") {
        configString = String(plasmoid.configuration.userBlocksColorizingApplets);
    }

    //array, a cell for encoded item order
    var itemsArray = configString.split(";");

    for (var i = 0; i < itemsArray.length; i++) {
        for (var j = 0; j < layout.children.length; ++j) {
            var child = layout.children[j];

            if (child.applet && (child.applet.id == itemsArray[i])) {
                if (option === "lockedZoomApplets") {
                    child.lockZoom = true;
                } else if (option === "userBlocksColorizingApplets") {
                    child.userBlocksColorizing = true;
                }
            }
        }
    }
}

function save() {
}

function saveOptions() {
    saveOption("lockedZoomApplets");
    saveOption("userBlocksColorizingApplets");
}

function saveOption(option) {
    var ids = new Array();
    for (var i = 0; i < layoutS.children.length; ++i) {
        var child = layoutS.children[i];

        if (child.applet
                && (option === "lockedZoomApplets" && child.lockZoom)
                || (option === "userBlocksColorizingApplets" && child.userBlocksColorizing)) {
            ids.push(child.applet.id);
        }
    }

    for (var i = 0; i < layout.children.length; ++i) {
        var child = layout.children[i];

        if (child.applet
                && (option === "lockedZoomApplets" && child.lockZoom)
                || (option === "userBlocksColorizingApplets" && child.userBlocksColorizing)) {
            ids.push(child.applet.id);
        }
    }

    for (var i = 0; i < layoutE.children.length; ++i) {
        var child = layoutE.children[i];

        if (child.applet
                && (option === "lockedZoomApplets" && child.lockZoom)
                || (option === "userBlocksColorizingApplets" && child.userBlocksColorizing)) {
            ids.push(child.applet.id);
        }
    }

    if (option === "lockedZoomApplets") {
        plasmoid.configuration.lockedZoomApplets = ids.join(';');
    } else if (option === "userBlocksColorizingApplets") {
        plasmoid.configuration.userBlocksColorizingApplets = ids.join(';');
    }
}

function removeApplet (applet) {
    for (var i = layout.children.length - 1; i >= 0; --i) {
        var child = layout.children[i];
        if (child.applet === applet) {
            child.destroy();
        }
    }
}

//insert item2 before item1
function insertBefore(item1, item2) {
    return insertBeforeForLayout(item1.parent, item1, item2);;
}

//insert item2 before item1
function insertBeforeForLayout(tLayout, item1, item2) {
    if (item1 === item2) {
        return -1;
    }
    var removed = new Array();

    var child;

    var i;
    for (i = tLayout.children.length - 1; i >= 0; --i) {
        child = tLayout.children[i];
        removed.push(child);
        child.parent = root;

        if (child === item1) {
            break;
        }
    }

    item2.parent = tLayout;

    for (var j = removed.length - 1; j >= 0; --j) {
        removed[j].parent = tLayout;
    }

    return i;
}


//insert item2 after item1
function insertAfter(item1, item2) {
    return insertAfterForLayout(item1.parent, item1, item2);
}

//insert item2 after item1
function insertAfterForLayout(tLayout, item1, item2) {
    if (item1 === item2) {
        return -1;
    }
    var removed = new Array();

    var child;

    var i;
    for (i = tLayout.children.length - 1; i >= 0; --i) {
        child = tLayout.children[i];
        //never ever insert after lastSpacer
        if (child === lastSpacer && item1 === lastSpacer) {
            removed.push(child);
            child.parent = root;
            break;
        } else if (child === item1) {
            //Already in position, do nothing
            if (tLayout.children[i+1] === item2) {
                return -1;
            }
            break;
        }

        removed.push(child);
        child.parent = root;
    }

    item2.parent = tLayout;

    for (var j = removed.length - 1; j >= 0; --j) {
        removed[j].parent = tLayout;
    }

    return i;
}


function insertAtIndex(relevantLayout, item, position) {
    var addToEnd = false;
    if (position < 0 || (position > relevantLayout.children.length && !item.isInternalViewSplitter)) {
        return;
    } else if (position >= relevantLayout.children.length) {
        addToEnd = true;
    }

    //never ever insert after lastSpacer
    var firstItem = (relevantLayout.children.length === 1) && (relevantLayout.children[0] === lastSpacer);

    //Important !!! , this is used to add the first item
    if(firstItem){
        lastSpacer.parent = root;
        position = 0;
    }

    if (addToEnd){
        item.parent=relevantLayout;
        return;
    }

    if(relevantLayout.children.length > 0){
        if (relevantLayout.children[position] === lastSpacer) {
            --position;
        }
    }

    var removedItems = new Array();

    var totalChildren = relevantLayout.children.length;
    for (var i = position; i < totalChildren; ++i) {
        var child = relevantLayout.children[position];
        child.parent = root;
        removedItems.push(child);
    }

    item.parent = relevantLayout;
    for (var i in removedItems) {
        removedItems[i].parent = relevantLayout;
    }
}

function insertAtCoordinates(item, x, y) {
    if (root.isHorizontal) {
        y = layout.height / 2;
    } else {
        x = layout.width / 2;
    }
    var child = layout.childAt(x, y);

    //if we got a place inside the space between 2 applets, we have to find it manually
    if (!child) {
        if (root.isHorizontal) {
            for (var i = 0; i < layout.children.length; ++i) {
                var candidate = layout.children[i];
                if (x >= candidate.x && x < candidate.x + candidate.width + layout.rowSpacing) {
                    child = candidate;
                    break;
                }
            }
        } else {
            for (var i = 0; i < layout.children.length; ++i) {
                var candidate = layout.children[i];
                if (y >= candidate.x && y < candidate.y + candidate.height + layout.columnSpacing) {
                    child = candidate;
                    break;
                }
            }
        }
    }

    //already in position
    if (child === item) {
        return;
    }

    if (!child) {
        // check if dragging takes place after the end of the layout
        if ( ((root.isVertical && y > layout.height)||(root.isHorizontal && x > layout.width))
                && layout.children.length>0  ){
            child = layout.children[layout.children.length-1];
        } else {
            child = layout.children[0];
        }
    } else {
        item.parent = root;
    }

    if ((root.isVertical && y < child.y + child.height/2) ||
            (root.isHorizontal && x < child.x + child.width/2)) {
        return insertBefore(child, item);
    } else {
        return insertAfter(child, item);
    }
}

//this is used only for the start and end layouts
function insertAtLayoutCoordinates(tLayout, item, x, y) {
    if (root.isHorizontal) {
        y = tLayout.height / 2;
    } else {
        x = tLayout.width / 2;
    }
    var child = tLayout.childAt(x, y);

    //if we got a place inside the space between 2 applets, we have to find it manually
    if (!child) {
        if (root.isHorizontal) {
            for (var i = 0; i < tLayout.children.length; ++i) {
                var candidate = tLayout.children[i];
                if (x >= candidate.x && x < candidate.x + candidate.width + tLayout.rowSpacing) {
                    child = candidate;
                    break;
                }
            }
        } else {
            for (var i = 0; i < tLayout.children.length; ++i) {
                var candidate = tLayout.children[i];
                if (y >= candidate.x && y < candidate.y + candidate.height + tLayout.columnSpacing) {
                    child = candidate;
                    break;
                }
            }
        }
    }

    //already in position
    if (child === item) {
        //
        return childFoundId;
    }

    if (!child) {
        // check if dragging takes place after the end of the layout
        var neededSpace = 1.5 * (metrics.iconSize + metrics.totals.lengthEdge);
        if ( (((root.isVertical && (y - neededSpace) <= tLayout.height) && y>=0)
              ||(root.isHorizontal && (x - neededSpace) <= tLayout.width) && x>=0)
                && tLayout.children.length>0  ){
            child = tLayout.children[layout.children.length-1];
            // check if dragging takes place before the start of the layout
        } else if ( ((root.isVertical && (y >= -neededSpace) && (y<=neededSpace))
                     ||(root.isHorizontal && (x >= -neededSpace)  && (x<=neededSpace)))
                   && tLayout.children.length>0  ){
            child = tLayout.children[0];
        } else {
            return -1;
            //child = tLayout.children[0];
        }
    } else {
        item.parent = root;
    }

    if ((root.isVertical && y < child.y + child.height/2) ||
            (root.isHorizontal && x < child.x + child.width/2)) {
        return insertBeforeForLayout(tLayout, child, item);
    } else {
        return insertAfterForLayout(tLayout, child, item);
    }
}

function insertAtCoordinates2(item, x, y) {
    var pos = -1;

    var layoutPosS = layoutS.mapFromItem(root, x, y);
    pos = insertAtLayoutCoordinates(layoutS, item, layoutPosS.x, layoutPosS.y);

    if (pos === -1){
        var layoutPosE = layoutE.mapFromItem(root, x, y);
        pos = insertAtLayoutCoordinates(layoutE, item, layoutPosE.x, layoutPosE.y);
    }

    if (pos!==childFoundId && pos === -1) {
        var layoutPos = layout.mapFromItem(root, x, y);
        insertAtCoordinates(item, layoutPos.x, layoutPos.y);
    }
}
