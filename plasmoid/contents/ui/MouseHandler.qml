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


import QtQuick 2.0

import org.kde.draganddrop 2.0

import org.kde.taskmanager 0.1 as TaskManager

//import "../code/layout.js" as LayoutManager
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
        interval: 120

        onTriggered: {
            ignoredItem = null;
        }
    }

    Connections {
        target: panel

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

        property Item hoveredItem

        onDragEnter:{
            if(panel.dragSource == null){
                panel.dropNewLauncher = true;

                var createLaunchers = event.mimeData.urls.every(function (item) {
                    return backend.isApplication(item)
                });


                if (createLaunchers)
                    onlyLaunchers = true;
            }
        }

        onDragMove: {
            if(panel.dragSource == null){
                panel.dropNewLauncher = true;
            }

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
          /*  if (panel.dragSource != null
                 && panel.dragSource.m.IsLauncher === true && above != null
                 && above.m != null
                 && above.m.IsLauncher !== true && above == ignoredItem) {
                return;
            } else {
                ignoredItem = null;
            }*/
            if (panel.dragSource == null
               && ignoredItem == above)
                return;

            //at some point it was needed the following  && above != ignoredItem
            //but know not... strange... && above != ignoredItem
            //I use the ignoredItem in order to reduce the move calls as much
            //as possible
            if (tasksModel.sortMode == TaskManager.TasksModel.SortManual && panel.dragSource && ignoredItem == null) {
                var insertAt = TaskTools.insertIndexAt(above, event.x, event.y);              

                if (panel.dragSource != above && panel.dragSource.itemIndex != insertAt) {
              //      console.log(panel.dragSource.itemIndex + " - "+insertAt);
                    tasksModel.move(panel.dragSource.itemIndex, insertAt);
                    ignoredItem = above;
                    ignoreItemTimer.restart();
                }
            } else if (!panel.dragSource && above && hoveredItem != above) {
                hoveredItem = above;
                panel.dropNewLauncher = true;
             //   activationTimer.restart();
            } else if (!above) {
                panel.dropNewLauncher = true;
                hoveredItem = null;
              //  activationTimer.stop();
            }
        }

        onDragLeave: {
            hoveredItem = null;
            panel.dropNewLauncher = false;
            onlyLaunchers = false;
            activationTimer.stop();
        }

        onDrop: {
            // Reject internal drops.
            panel.dropNewLauncher = false;
            onlyLaunchers = false;

            if (event.mimeData.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") >= 0) {
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
             /*   if (parent.hoveredItem.m.IsGroupParent === true) {
                    groupDialog.visualParent = parent.hoveredItem;
                    groupDialog.visible = true;
                } else if (parent.hoveredItem.m.IsLauncher !== true) {
                    tasksModel.requestActivate(parent.hoveredItem.modelIndex());
                }*/
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
