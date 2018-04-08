/*
*   Copyright 2013 by Sebastian Kügler <sebas@kde.org>
*   Copyright 2014 by Martin Gräßlin <mgraesslin@kde.org>
*   Copyright 2016 by Kai Uwe Broulik <kde@privat.broulik.de>
*   Copyright 2017 by Roman Gilg <subdiff@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import QtQml.Models 2.2

import org.kde.draganddrop 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

import org.kde.taskmanager 0.1 as TaskManager


PlasmaExtras.ScrollArea {
    id: mainToolTip
    property Item parentTask
    property int parentIndex: -1

    property string appName
    property int pidParent
    property bool isGroup
    property bool hideCloseButtons

    property var windows
    readonly property bool isWin: windows !== undefined

    property variant icon
    property url launcherUrl
    property bool isLauncher
    property bool isMinimizedParent

    property bool containsMouse: false

    // Needed for generateSubtext()
    property string displayParent
    property string genericName
    property int virtualDesktopParent
    property bool isOnAllVirtualDesktopsParent
    property var activitiesParent
    //
    readonly property bool isVerticalPanel: plasmoid.formFactor == PlasmaCore.Types.Vertical

    Layout.minimumWidth: contentItem.width
    Layout.maximumWidth: Layout.minimumWidth

    Layout.minimumHeight: contentItem.height
    Layout.maximumHeight: Layout.minimumHeight

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property int textWidth: theme.mSize(theme.defaultFont).width * 20

    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

    Component.onCompleted: {
        flickableItem.interactive = Qt.binding(function() {
            return isVerticalPanel ? contentItem.height > viewport.height : contentItem.width > viewport.width
        });
    }
    Item{
        width: contentItem.width
        height: contentItem.height

        DropArea {
            id: dropMainArea
            anchors.fill: contentItem
            enabled: isGroup

            preventStealing: true

            property bool dragInside: false

            property QtObject currentWindow

            onDragLeave: {
                windowsPreviewDlg.hide();
            }

            onDragMove: {
                var current = groupTask.childAtPos(event.x, event.y);

                if (current && currentWindow !== current && current.submodelIndex) {
                    currentWindow = current;
                    tasksModel.requestActivate(current.submodelIndex);
                }
            }
        }

        MouseArea {
            id: contentItem
            width: childrenRect.width
            height: childrenRect.height

            hoverEnabled: true

            onContainsMouseChanged: {
                checkMouseInside();
            }

            function checkMouseInside(){
                var isInside = containsMouse || childrenContainMouse() || (parentTask && parentTask.containsMouse);
                if (isInside){
                    //root.disableRestoreZoom = true;
                    mainToolTip.containsMouse = true;

                    if(!root.latteDock)
                        checkListHovered.stop();
                }
                else{
                    //root.disableRestoreZoom = false;
                    mainToolTip.containsMouse = false;

                    if(!root.latteDock)
                        checkListHovered.startDuration(100);
                    else
                        root.latteDock.startCheckRestoreZoomTimer();
                }
            }

            function childrenContainMouse() {
                return singleTask.containsMouse() || groupTask.containsMouse();
            }

            ToolTipInstance {
                id: singleTask
                visible: !isGroup
            }

            Grid {
                id: groupTask
                rows: !isVerticalPanel
                columns: isVerticalPanel
                flow: isVerticalPanel ? Grid.TopToBottom : Grid.LeftToRight
                spacing: units.largeSpacing

                width: isGroup ? childrenRect.width : 0
                height: isGroup ? childrenRect.height : 0

                visible: isGroup && parentIndex !== -1

                Loader {
                    id: modelLoader
                    active: groupTask.visible

                    sourceComponent: Repeater {
                        id: groupRepeater

                        model: DelegateModel {
                            id: delegateModel

                            model: parentIndex !== -1 && isGroup ? tasksModel : undefined
                            rootIndex: tasksModel.makeModelIndex(parentIndex, -1)
                            delegate: ToolTipInstance {}
                        }

                        Component.onCompleted: {
                            parent = groupTask
                        }
                    }
                }


                function containsMouse(){
                    for(var i=1; i<children.length-1; ++i) {
                        if(children[i].containsMouse())
                            return true;
                    }

                    return false;
                }

                function childAtPos(x, y){
                    var tasks = groupTask.children;

                    for(var i=0; i<tasks.length; ++i){
                        var task = tasks[i];

                        var choords = contentItem.mapFromItem(task,0, 0);

                        if(choords.y < 0)
                            choords.y = 0;
                        if(choords.x < 0)
                            choords.x = 0;

                        if( (x>=choords.x) && (x<=choords.x+task.width)
                                && (y>=choords.y) && (y<=choords.y+task.height)){
                            return task;
                        }
                    }

                    return null;
                }
            }
        }
    }
}
