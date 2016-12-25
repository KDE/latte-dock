/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */


var layout;
var root;
var plasmoid;
var lastSpacer;


function restore() {
    var configString = String(plasmoid.configuration.appletOrder)

    //array, a cell for encoded item order
    var itemsArray = configString.split(";");

    //map applet id->order in panel
    var idsOrder = new Object();
    //map order in panel -> applet pointer
    var appletsOrder = new Object();

    for (var i = 0; i < itemsArray.length; i++) {
        //property name: applet id
        //property value: order
        idsOrder[itemsArray[i]] = i;
    }

    for (var i = 0; i < plasmoid.applets.length; ++i) {
        if (idsOrder[plasmoid.applets[i].id] !== undefined) {
            appletsOrder[idsOrder[plasmoid.applets[i].id]] = plasmoid.applets[i];
            //ones that weren't saved in AppletOrder go to the end
        } else {
            appletsOrder["unordered"+i] = plasmoid.applets[i];
        }
    }

    //finally, restore the applets in the correct order
    for (var i in appletsOrder) {
        root.addApplet(appletsOrder[i], -1, -1)
    }

    //add the splitter in the correct position if it exists
    if(plasmoid.configuration.splitterPosition !== -1){
        root.addInternalViewSplitter(plasmoid.configuration.splitterPosition);
    }

    //rewrite, so if in the orders there were now invalid ids or if some were missing creates a correct list instead
    save();
    restoreLocks();

    //update layouts in case there is a splitter in them
    root.updateLayouts();
}

function restoreLocks() {
    var configString = String(plasmoid.configuration.lockedZoomApplets)
    //array, a cell for encoded item order
    var itemsArray = configString.split(";");

    for (var i = 0; i < itemsArray.length; i++) {
        for (var j = 0; j < layout.children.length; ++j) {
            var child = layout.children[j];

            if (child.applet && (child.applet.id == itemsArray[i])) {
                child.lockZoom = true;
            }
        }
    }
}

function save() {
    var ids = new Array();
    var splitterExists = false;
    for (var i = 0; i < layout.children.length; ++i) {
        var child = layout.children[i];

        if (child.applet) {
            ids.push(child.applet.id);
        }
        else if(child.isInternalViewSplitter && plasmoid.configuration.panelPosition === 10){
            splitterExists = true;
            plasmoid.configuration.splitterPosition = i;
        }
    }

    if(!splitterExists)
        plasmoid.configuration.splitterPosition = -1;

    plasmoid.configuration.appletOrder = ids.join(';');
}

function saveLocks() {
    var ids = new Array();
    for (var i = 0; i < layout.children.length; ++i) {
        var child = layout.children[i];

        if (child.applet && child.lockZoom) {
            ids.push(child.applet.id);
        }
    }
    plasmoid.configuration.lockedZoomApplets = ids.join(';');
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
    if (item1 === item2) {
        return;
    }
    var removed = new Array();

    var child;

    var i;
    for (i = layout.children.length - 1; i >= 0; --i) {
        child = layout.children[i];
        removed.push(child);
        child.parent = root;

        if (child === item1) {
            break;
        }
    }

    item2.parent = layout;

    for (var j = removed.length - 1; j >= 0; --j) {
        removed[j].parent = layout;
    }
    return i;
}

//insert item2 after item1
function insertAfter(item1, item2) {
    if (item1 === item2) {
        return;
    }
    var removed = new Array();

    var child;

    var i;
    for (i = layout.children.length - 1; i >= 0; --i) {
        child = layout.children[i];
        //never ever insert after lastSpacer
        if (child === lastSpacer && item1 === lastSpacer) {
            removed.push(child);
            child.parent = root;
            break;
        } else if (child === item1) {
            //Already in position, do nothing
            if (layout.children[i+1] === item2) {
                return;
            }
            break;
        }

        removed.push(child);
        child.parent = root;
    }

    item2.parent = layout;

    for (var j = removed.length - 1; j >= 0; --j) {
        removed[j].parent = layout;
    }
    return i;
}

function insertAtIndex(item, position) {
    if (position < 0 || (position >= layout.children.length && position !== 0)) {
        return;
    }

    //never ever insert after lastSpacer
    var firstItem = (layout.children.length === 1) && (layout.children[0] === lastSpacer);

    //Important !!! , this is used to add the first item
    if(firstItem){
        lastSpacer.parent = root;
        position = 0;
    }

    if(layout.children.length > 0){
        if (layout.children[position] === lastSpacer) {
            --position;
        }
    }

    var removedItems = new Array();

    var totalChildren = layout.children.length;
    for (var i = position; i < totalChildren; ++i) {
        var child = layout.children[position];
        child.parent = root;
        removedItems.push(child);
    }

    item.parent = layout;
    for (var i in removedItems) {
        removedItems[i].parent = layout;
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
        child = layout.children[0];
    }
    item.parent = root;

    //PlasmaCore.Types.Vertical = 3
    if ((plasmoid.formFactor === 3 && y < child.y + child.height/2) ||
            (plasmoid.formFactor !== 3 && x < child.x + child.width/2)) {
        return insertBefore(child, item);
    } else {
        return insertAfter(child, item);
    }
}
