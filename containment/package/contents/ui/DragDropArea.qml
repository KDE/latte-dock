/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
            var dndindex = fastLayoutManager.dndSpacerIndex();
            var eventx = event.x;
            var eventy = event.y;

            if (dndindex >= 0) {
                var masquearadedIndexFromPoint = fastLayoutManager.indexToMasquearadedPoint(fastLayoutManager.dndSpacerIndex());
                eventx = masquearadedIndexFromPoint.x;
                eventy = masquearadedIndexFromPoint.y;
            }

            plasmoid.processMimeData(event.mimeData, eventx, eventy);
            //! inform others what plasmoid was drag n' dropped to be added
            latteView.extendedInterface.appletDropped(event.mimeData, eventx, eventy);
            event.accept(event.proposedAction);
        }

        dndSpacer.opacity = 0;
    }
}
