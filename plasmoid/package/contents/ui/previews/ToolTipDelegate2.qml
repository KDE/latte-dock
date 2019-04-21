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
    property var rootIndex

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
    property var virtualDesktopParent
    property bool isOnAllVirtualDesktopsParent
    property var activitiesParent
    //
    readonly property bool isVerticalPanel: plasmoid.formFactor === PlasmaCore.Types.Vertical

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

        //! DropArea
        DropArea {
            id: dropMainArea
            anchors.fill: parent
            enabled: isGroup
            preventStealing: true

            property QtObject currentWindow

            onDragLeave: {
                windowsPreviewDlg.hide();
            }

            onDragMove: {
                var current = mainToolTip.instanceAtPos(event.x, event.y);

                if (current && currentWindow !== current && current.submodelIndex) {
                    currentWindow = current;
                    tasksModel.requestActivate(current.submodelIndex);
                }
            }
        } //! DropArea

        //! Underneath MouseArea
        MouseArea {
            id: contentItemMouseArea
            anchors.fill: parent
            hoverEnabled: true

            onContainsMouseChanged: {
                mainToolTip.mouseIsInside();
            }
        }//! MouseArea

        Loader {
            id: contentItem
            active: mainToolTip.rootIndex !== undefined
            asynchronous: true
            sourceComponent: isGroup ? groupToolTip : singleTooltip

            Component {
                id: singleTooltip

                ToolTipInstance {
                    submodelIndex: mainToolTip.rootIndex
                }
            }

            Component {
                id: groupToolTip

                Grid {
                    rows: !isVerticalPanel
                    columns: isVerticalPanel
                    flow: isVerticalPanel ? Grid.TopToBottom : Grid.LeftToRight
                    spacing: units.largeSpacing

                    Repeater {
                        id: groupRepeater
                        model: DelegateModel {
                            model: mainToolTip.rootIndex ? tasksModel : null
                            rootIndex: mainToolTip.rootIndex

                            delegate: ToolTipInstance {
                                submodelIndex: tasksModel.makeModelIndex(mainToolTip.rootIndex.row, index)
                            }
                        }
                    }
                }
            }
        } //! Loader
    } //! Item

    //! Central Functionality
    function mouseIsInside(){
        var isInside = contentItemMouseArea.containsMouse || instancesContainMouse();

        if (isInside){
            mainToolTip.containsMouse = true;

            if(!root.latteView)
                checkListHovered.stop();
        } else {
            mainToolTip.containsMouse = false;

            if(!root.latteView)
                checkListHovered.startDuration(100);
            else
                root.latteView.startCheckRestoreZoomTimer();
        }
    }

    function instancesContainMouse() {
        var previewInstances = isGroup ? contentItem.children[0].children : contentItem.children;
        var instancesLength = previewInstances.length;

        for(var i=instancesLength-1; i>=0; --i) {
            if( (typeof(previewInstances[i].containsMouse) === "function") //ignore unrelevant objects
                    &&  previewInstances[i].containsMouse())
                return true;
        }

        return false;
    }

    function instanceAtPos(x, y){
        var previewInstances = isGroup ? contentItem.children[0].children : contentItem.children;
        var instancesLength = previewInstances.length;

        for(var i=0; i<instancesLength; ++i){
            var instance = previewInstances[i];
            var choords = contentItem.mapFromItem(instance,0, 0);

            if(choords.y < 0)
                choords.y = 0;
            if(choords.x < 0)
                choords.x = 0;

            if( (x>=choords.x) && (x<=choords.x+instance.width)
                    && (y>=choords.y) && (y<=choords.y+instance.height)){
                return instance;
            }
        }
        return null;
    }
}


