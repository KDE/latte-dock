/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0 as DragDrop

import org.kde.latte.core 0.2 as LatteCore

DragDrop.DropArea {
    id: dragArea

    property bool containsDrag: false

    readonly property Item dragInfo: Item {
        readonly property bool entered: latteView && latteView.containsDrag
        property bool isTask: false
        property bool isPlasmoid: false
        property bool isSeparator: false
        property bool isLatteTasks: false
        property bool onlyLaunchers: false

        property bool computationsAreValid: false
    }

    Connections{
        target: root.dragInfo

        onEnteredChanged: {
            if(!root.dragInfo.entered) {
                dragArea.clearInfo();
            }
        }
    }

    Connections{
        target: latteView

        onContainsDragChanged: {
            if(!latteView.containsDrag) {
                dragArea.clearInfo();
            }
        }
    }

    function clearInfo() {
        clearInfoTimer.restart();
    }

    function isDroppingOnlyLaunchers(event) {
        if (!latteView) {
            return
        }

        if (event.mimeData.hasUrls || (event.mimeData.formats.indexOf("text/x-plasmoidservicename") !== 0)) {
            var onlyLaunchers = event.mimeData.urls.every(function (item) {
                return latteView.extendedInterface.isApplication(item);
            });

            return onlyLaunchers;
        }

        return false;
    }

    //! Give the time when an applet is dropped to be positioned properly
    Timer {
        id: clearInfoTimer
        interval: 100

        onTriggered: {
            dragArea.dragInfo.computationsAreValid = false;

            dragArea.dragInfo.isTask = false;
            dragArea.dragInfo.isPlasmoid = false;
            dragArea.dragInfo.isSeparator = false;
            dragArea.dragInfo.isLatteTasks = false;
            dragArea.dragInfo.onlyLaunchers = false;

            dndSpacer.parent = root;
            dndSpacer.opacity = 0;
        }
    }

    onDragEnter: {
        containsDrag = true;
        clearInfoTimer.stop();
        var isTask = event !== undefined
                && event.mimeData !== undefined
                && event.mimeData.formats !== undefined
                && event.mimeData.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") >= 0;

        var isSeparator = event !== undefined
                && event.mimeData !== undefined
                && ( latteView.mimeContainsPlasmoid(event.mimeData, "audoban.applet.separator")
                    || latteView.mimeContainsPlasmoid(event.mimeData, "org.kde.latte.separator") );

        var isLatteTasks = event !== undefined
                && event.mimeData !== undefined
                && latteView.mimeContainsPlasmoid(event.mimeData, "org.kde.latte.plasmoid");

        var isPlasmoid = event !== undefined
                && event.mimeData !== undefined
                && event.mimeData.formats !== undefined
                && !event.mimeData.hasUrls
                && event.mimeData.formats.indexOf("text/x-plasmoidservicename") === 0;

        dragInfo.isTask = isTask;
        dragInfo.isPlasmoid = isPlasmoid;
        dragInfo.isSeparator = isSeparator;
        dragInfo.isLatteTasks = isLatteTasks;
        dragInfo.onlyLaunchers = isDroppingOnlyLaunchers(event);
        dragInfo.computationsAreValid = true;

        if (dragInfo.isTask || plasmoid.immutable || !root.myView.isShownFully) {
            event.ignore();
            clearInfo();
            return;
        }

        //! Send signal AFTER the dragging is confirmed otherwise the restore mask signal from animations
        //! may not be triggered #408926
        animations.needLength.addEvent(dragArea);

        if (root.launchers.hasStealingApplet && dragInfo.onlyLaunchers) {
            root.launchers.showAddLaunchersMessageInStealingApplet();
            dndSpacer.opacity = 0;
            dndSpacer.parent = root;
            return;
        }

        fastLayoutManager.insertAtCoordinates(dndSpacer, event.x, event.y);
        dndSpacer.opacity = 1;
    }

    onDragMove: {
        containsDrag = true;
        clearInfoTimer.stop();
        if (dragInfo.isTask) {
            return;
        }

        if (root.launchers.hasStealingApplet && dragInfo.onlyLaunchers) {
            root.launchers.showAddLaunchersMessageInStealingApplet();
            dndSpacer.opacity = 0;
            dndSpacer.parent = root;
            return;
        }

        fastLayoutManager.insertAtCoordinates(dndSpacer, event.x, event.y);
        dndSpacer.opacity = 1;
    }

    onDragLeave: {
        containsDrag = false;
        animations.needLength.removeEvent(dragArea);

        if (root.launchers.hasStealingApplet) {
            root.launchers.hideAddLaunchersMessageInStealingApplet();
        }

        dndSpacer.opacity = 0;
        dndSpacer.parent = root;
    }

    onDrop: {
        containsDrag = false;
        animations.needLength.removeEvent(dragArea);

        if (root.launchers.hasStealingApplet) {
            root.launchers.hideAddLaunchersMessageInStealingApplet();
        }

        if (dragInfo.isTask || !root.myView.isShownFully) {
            return;
        }

        if (root.launchers.hasStealingApplet && dragInfo.onlyLaunchers) {
            root.launchers.addDroppedLaunchersInStealingApplet(event.mimeData.urls);
        } else {
            plasmoid.processMimeData(event.mimeData, event.x, event.y);
            event.accept(event.proposedAction);
        }

        dndSpacer.opacity = 0;

        if (dragInfo.isPlasmoid && root.myView.alignment === LatteCore.Types.Justify) {
            root.moveAppletsBasedOnJustifyAlignment();
        }
    }
}
