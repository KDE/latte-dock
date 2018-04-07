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

import org.kde.plasma.plasmoid 2.0
import org.kde.draganddrop 2.0

import org.kde.taskmanager 0.1 as TaskManager

import org.kde.latte 0.1 as Latte

import "../code/tools.js" as TaskTools

Item {
    // signal urlDropped(url url)
    signal urlsDropped(var urls)

    property Item target
    property Item ignoredItem
    property bool moved: false

    property alias hoveredItem: dropHandler.hoveredItem
    property alias onlyLaunchers: dropHandler.onlyLaunchers


    Timer {
        id: ignoreItemTimer

        repeat: false
        interval: 200

        onTriggered: {
            ignoredItem = null;
        }
    }

    Connections {
        target: root
        onDragSourceChanged: {
            if (!dragSource) {
                ignoredItem = null;
                ignoreItemTimer.stop();
            }
        }
    }

    DropArea {
        id: dropHandler

        anchors.fill: parent

        preventStealing: true;

        property int droppedPosition: -1;
        property bool onlyLaunchers: false;
        property bool droppingSeparator: false;

        property Item hoveredItem

        function isDroppingSeparator(event) {
            return ((event.mimeData.formats.indexOf("text/x-plasmoidservicename") === 0)
                    && (String(event.mimeData.getDataAsByteArray("text/x-plasmoidservicename")) === "audoban.applet.separator"));
        }

        onDragEnter:{
            if(root.dragSource == null){
                onlyLaunchers = false;
                droppingSeparator = false;
                root.dropNewLauncher = false;
                var createLaunchers = false;

                if (event.mimeData.hasUrls || (event.mimeData.formats.indexOf("text/x-plasmoidservicename") !== 0)) {
                    root.dropNewLauncher = true;

                    if (event.mimeData.hasUrls){
                        createLaunchers = event.mimeData.urls.every(function (item) {
                            return backend.isApplication(item)
                        });
                    }
                } else if (isDroppingSeparator(event)) {
                    droppingSeparator = true;
                    root.dropNewLauncher = true;
                    return;
                } else {
                    event.ignore();
                }

                if (createLaunchers) {
                    onlyLaunchers = true;
                }
            }
        }

        onDragMove: {
            /* if(root.dragSource == null){
                root.dropNewLauncher = true;
            } */

            if (target.animating) {
                return;
            }

            var above = target.childAtPos(event.x, event.y);

            // If we're mixing launcher tasks with other tasks and are moving
            // a (small) launcher task across a non-launcher task, don't allow
            // the latter to be the move target twice in a row for a while, as
            // it will naturally be moved underneath the cursor as result of the
            // initial move, due to being far larger than the launcher delegate.
            // TODO: This restriction (minus the timer, which improves things)
            // has been proven out in the EITM fork, but could be improved later
            // by tracking the cursor movement vector and allowing the drag if
            // the movement direction has reversed, etablishing user intent to
            // move back.
            if (root.dragSource == null
                    && ignoredItem == above)
                return;

            if (root.dragSource != null
                    && root.dragSource.m.IsLauncher === true && above != null
                    && above.m != null
                    && above.m.IsLauncher !== true && above == ignoredItem) {
                return;
            } else {
                //ignoredItem = null;
            }

            //at some point it was needed the following  && above != ignoredItem
            //but know not... strange... && above != ignoredItem
            //I use the ignoredItem in order to reduce the move calls as much
            //as possible
            if (tasksModel.sortMode == TaskManager.TasksModel.SortManual && root.dragSource && ignoredItem == null) {
                var insertAt = TaskTools.insertIndexAt(above, event.x, event.y);

                if (root.dragSource != above && root.dragSource.itemIndex != insertAt) {
                    //      console.log(root.dragSource.itemIndex + " - "+insertAt);
                    root.dragSource.z = 100;
                    ignoredItem = above;

                    var pos = root.dragSource.itemIndex;
                    tasksModel.move(pos, insertAt);
                    if (latteDock && latteDock.launchersGroup >= Latte.Dock.LayoutLaunchers) {
                        latteDock.universalLayoutManager.launchersSignals.moveTask(root.managedLayoutName,
                                                                                   plasmoid.id,
                                                                                   latteDock.launchersGroup, pos, insertAt);
                    }
                    root.separatorsUpdated();
                    ignoreItemTimer.restart();
                }
            } else if (!root.dragSource && above && hoveredItem != above) {
                hoveredItem = above;
                //root.dropNewLauncher = true;
                activationTimer.restart();
            } else if (!above) {
                //root.dropNewLauncher = true;
                hoveredItem = null;
                activationTimer.stop();
            }

            if (hoveredItem && windowsPreviewDlg.visible && toolTipDelegate.parentIndex !== hoveredItem.itemIndex ) {
                windowsPreviewDlg.hide(6);
            }
        }

        onDragLeave: {
            hoveredItem = null;
            root.dropNewLauncher = false;
            onlyLaunchers = false;
            root.separatorsUpdated();
            activationTimer.stop();
        }

        onDrop: {
            // Reject internal drops.
            root.dropNewLauncher = false;
            onlyLaunchers = false;
            root.separatorsUpdated();

            if (event.mimeData.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") >= 0) {
                return;
            }

            if (droppingSeparator) {
                droppingSeparator = false;
                if (hoveredItem && hoveredItem.itemIndex >=0){
                    root.addInternalSeparatorAtPos(hoveredItem.itemIndex);
                } else {
                    root.addInternalSeparatorAtPos(0);
                }
                return;
            }

            if (event.mimeData.hasUrls) {
                parent.urlsDropped(event.mimeData.urls);
            }
        }

        Timer {
            id: activationTimer

            interval: 250
            repeat: false

            onTriggered: {
                if (dropHandler.onlyLaunchers || !root.dropNewLauncher || dropHandler.droppingSeparator) {
                    return;
                }

                if (parent.hoveredItem.m.IsGroupParent === true) {
                    root.showPreviewForTasks(parent.hoveredItem);
                    // groupDialog.visualParent = parent.hoveredItem;
                    // groupDialog.visible = true;
                } else if (parent.hoveredItem.m.IsLauncher !== true) {
                    if(windowsPreviewDlg.visible && toolTipDelegate.currentItem !==parent.hoveredItem.itemIndex ) {
                        windowsPreviewDlg.hide(5);
                        toolTipDelegate.currentItem=-1;
                    }

                    tasksModel.requestActivate(parent.hoveredItem.modelIndex());
                }
            }
        }
    }
    /*
    MouseArea {
        id: wheelHandler

        anchors.fill: parent
        property int wheelDelta: 0;
        enabled: plasmoid.configuration.wheelEnabled

        onWheel: wheelDelta = TaskTools.wheelActivateNextPrevTask(wheelDelta, wheel.angleDelta.y);
    } */
}
