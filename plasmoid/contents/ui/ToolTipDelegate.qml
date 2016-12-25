/*
*   Copyright 2013 by Sebastian Kügler <sebas@kde.org>
*   Copyright 2014 by Martin Gräßlin <mgraesslin@kde.org>
*   Copyright 2016 by Kai Uwe Broulik <kde@privat.broulik.de>
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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

//Column {
MouseArea{
    id: tooltipContentItem
    hoverEnabled: true

    property Item toolTip
    property var parentIndex
    property var titles
    property var windows
    property string mainText
    property string subText
    property variant icon
    property url launcherUrl
    property bool group: windows ? (windows.length > 1) : false

    readonly property int thumbnailWidth: units.gridUnit * 15
    readonly property int thumbnailHeight: units.gridUnit * 10

    property int preferredTextWidth: theme.mSize(theme.defaultFont).width * 30
    property int _s: units.largeSpacing / 2

    Layout.minimumWidth: Math.max(thumbnailWidth, windowRow.width, appLabelRow.width) + _s
    Layout.minimumHeight: childrenRect.height
    Layout.maximumWidth: Layout.minimumWidth
    Layout.maximumHeight: Layout.minimumHeight

    onContainsMouseChanged: {
        checkMouseInside();
    }

    function checkMouseInside(){
        var isInside = containsMouse || windowRow.containsMouse();
        if (isInside){
            toolTipDelegate.currentItem = parentIndex;
        }
        else{
            toolTipDelegate.currentItem = -1;
            checkListHovered.restart();
        }
    }

    states: State {
        when: mpris2Source.hasPlayer

        PropertyChanges {
            target: thumbnailSourceItem
            opacity: 0 // cannot set visible to false or else WindowThumbnail won't provide thumbnail
        }
        PropertyChanges {
            target: playerControlsOpacityMask
            visible: true
            source: thumbnailSourceItem
            maskSource: playerControlsShadowMask
        }
        PropertyChanges {
            target: playerControlsRow
            visible: mpris2Source.hasPlayer
        }
    }

    PlasmaCore.DataSource {
        id: mpris2Source
        readonly property string current: {
            var desktopFileName = launcherUrl.toString().split('/').pop().replace(".desktop", "")

            for (var i = 0, length = sources.length; i < length; ++i) {
                var source = sources[i];
                var sourceData = data[source];

                if (sourceData && sourceData.DesktopEntry === desktopFileName) {
                    return source
                }
            }

            return ""
        }

        readonly property bool hasPlayer: !!current

        readonly property bool playing: hasPlayer && data[current].PlaybackStatus === "Playing"
        readonly property bool canControl: hasPlayer && data[current].CanControl
        readonly property bool canGoBack: hasPlayer && data[current].CanGoPrevious
        readonly property bool canGoNext: hasPlayer && data[current].CanGoNext
        readonly property bool canRaise: hasPlayer && data[current].CanRaise

        readonly property var currentMetadata: hasPlayer ? data[current].Metadata : ({})

        readonly property string track: {
            var xesamTitle = currentMetadata["xesam:title"]
            if (xesamTitle) {
                return xesamTitle
            }
            // if no track title is given, print out the file name
            var xesamUrl = currentMetadata["xesam:url"] ? currentMetadata["xesam:url"].toString() : ""
            if (!xesamUrl) {
                return ""
            }
            var lastSlashPos = xesamUrl.lastIndexOf('/')
            if (lastSlashPos < 0) {
                return ""
            }
            var lastUrlPart = xesamUrl.substring(lastSlashPos + 1)
            return decodeURIComponent(lastUrlPart)
        }
        readonly property string artist: currentMetadata["xesam:artist"] || ""
        readonly property string albumArt: currentMetadata["mpris:artUrl"] || ""

        function goPrevious() {
            startOperation("Previous")
        }

        function goNext() {
            startOperation("Next")
        }

        function playPause() {
            startOperation("PlayPause")
        }

        function raise() {
            startOperation("Raise")
        }

        function startOperation(op) {
            var service = mpris2Source.serviceForSource(current)
            var operation = service.operationDescription(op)
            return service.startOperationCall(operation)
        }

        engine: "mpris2"
        connectedSources: sources
    }

    Column{
        spacing: _s

        Item {
            id: thumbnailContainer
            width: Math.max(tooltipContentItem.width, windowRow.width)
            height: albumArtImage.available ? albumArtImage.height :
                                              raisePlayerArea.visible ? raisePlayerArea.height :
                                                                        windowRow.height

            Item {
                id: thumbnailSourceItem
                anchors.fill: parent

                PlasmaExtras.ScrollArea {
                    id: scrollArea
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.max(windowRow.width, thumbnailWidth)
                    height: parent.height

                    visible: !albumArtImage.available

                    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                    Component.onCompleted: {
                        flickableItem.interactive = Qt.binding(function() {
                            return contentItem.width > viewport.width;
                        });
                    }

                    Row {
                        id: windowRow
                        width: childrenRect.width
                        height: childrenRect.height
                        spacing: units.largeSpacing

                        Repeater {
                            model: (plasmoid.configuration.showToolTips && !albumArtImage.available)
                                   || !windowSystem.compositingActive ? windows : null

                            Column{
                                PlasmaCore.WindowThumbnail {
                                    id: windowThumbnail

                                    width: thumbnailWidth
                                    height: thumbnailHeight

                                    winId: modelData

                                    ToolTipWindowMouseArea {
                                        anchors.fill: parent
                                        modelIndex: tasksModel.makeModelIndex(parentIndex, group ? index : -1)
                                        winId: modelData
                                        thumbnailItem: parent
                                    }
                                }

                                PlasmaComponents.Label{
                                    text: titles && titles[index] ? titles[index] : ""
                                    wrapMode: Text.Wrap
                                    font.italic: true
                                    elide: Text.ElideRight
                                    opacity: 0.7
                                    textFormat: Text.PlainText
                                    verticalAlignment: Text.AlignVCenter

                                    width: thumbnailWidth
                                }
                            }

                        }

                        function containsMouse(){
                            for(var i=0; i<children.length-1; ++i){
                                if(children[i].children[0].containsMouse)
                                    return true;
                            }

                            return false;
                        }
                    }
                }

                Image {
                    id: albumArtImage
                    // also Image.Loading to prevent loading thumbnails just because the album art takes a split second to load
                    readonly property bool available: status === Image.Ready || status === Image.Loading

                    anchors.centerIn: parent
                    width: parent.width
                    height: thumbnailHeight
                    sourceSize: Qt.size(thumbnailWidth, thumbnailHeight)
                    asynchronous: true
                    source: mpris2Source.albumArt
                    fillMode: Image.PreserveAspectCrop
                    visible: available

                    ToolTipWindowMouseArea {
                        id: albumMouseArea

                        anchors.fill: parent
                        modelIndex: tasksModel.makeModelIndex(parentIndex)//, group ? (index ? index : -1) : -1)
                        winId: windows != undefined ? (windows[0] || 0) : 0
                    }
                }

                MouseArea {
                    id: raisePlayerArea
                    anchors.centerIn: parent
                    width: thumbnailWidth
                    height: thumbnailHeight

                    // if there's no window associated with this task, we might still be able to raise the player
                    visible: windows == undefined || !windows[0] && mpris2Source.canRaise
                    onClicked: mpris2Source.raise()

                    PlasmaCore.IconItem {
                        anchors.fill: parent
                        source: icon
                        animated: false
                        usesPlasmaTheme: false
                        visible: !albumArtImage.available
                    }
                }
            }

            Item {
                id: playerControlsShadowMask
                anchors.fill: thumbnailSourceItem
                visible: false // OpacityMask would render it

                Rectangle {
                    width: parent.width
                    height: parent.height - playerControlsRow.height
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: playerControlsRow.height
                    opacity: 0.2
                }
            }

            OpacityMask {
                id: playerControlsOpacityMask
                anchors.fill: thumbnailSourceItem
                visible: false
            }

            // prevent accidental click-through when a control is disabled
            MouseArea {
                anchors.fill: playerControlsRow
                enabled: playerControlsRow.visible
            }

            RowLayout {
                id: playerControlsRow
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: thumbnailSourceItem.bottom
                }
                width: thumbnailWidth
                spacing: 0
                enabled: mpris2Source.canControl
                visible: false

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    PlasmaExtras.Heading {
                        Layout.fillWidth: true
                        level: 4
                        wrapMode: Text.NoWrap
                        elide: Text.ElideRight
                        text: mpris2Source.track || ""
                    }

                    PlasmaExtras.Heading {
                        Layout.fillWidth: true
                        level: 5
                        wrapMode: Text.NoWrap
                        elide: Text.ElideRight
                        text: mpris2Source.artist || ""
                    }
                }

                PlasmaComponents.ToolButton {
                    enabled: mpris2Source.canGoBack
                    iconName: "media-skip-backward"
                    tooltip: i18nc("Go to previous song", "Previous")
                    Accessible.name: tooltip
                    onClicked: mpris2Source.goPrevious()
                }

                PlasmaComponents.ToolButton {
                    Layout.fillHeight: true
                    Layout.preferredWidth: height // make this button bigger
                    iconName: mpris2Source.playing ? "media-playback-pause" : "media-playback-start"
                    tooltip: mpris2Source.playing ? i18nc("Pause player", "Pause") : i18nc("Start player", "Play")
                    Accessible.name: tooltip
                    onClicked: mpris2Source.playPause()
                }

                PlasmaComponents.ToolButton {
                    enabled: mpris2Source.canGoNext
                    iconName: "media-skip-forward"
                    tooltip: i18nc("Go to next song", "Next")
                    Accessible.name: tooltip
                    onClicked: mpris2Source.goNext()
                }
            }
        }

        Row {
            id: appLabelRow
            width: childrenRect.width + _s
            height: childrenRect.height + units.largeSpacing
            spacing: units.largeSpacing

            Item {
                id: imageContainer
                width: tooltipIcon.width
                height: tooltipIcon.height
                y: _s

                PlasmaCore.IconItem {
                    id: tooltipIcon
                    x: _s
                    width: units.iconSizes.desktop
                    height: width
                    animated: false
                    usesPlasmaTheme: false
                    source: icon
                }
            }

            Column {
                id: mainColumn
                y: _s

                //This instance is purely for metrics
                PlasmaExtras.Heading {
                    id: tooltipMaintextPlaceholder
                    visible: false
                    level: 3
                    text: mainText
                    textFormat: Text.PlainText
                }
                PlasmaExtras.Heading {
                    id: tooltipMaintext
                    level: 3
                    width: Math.min(tooltipMaintextPlaceholder.width, preferredTextWidth)
                    //width: 400
                    elide: Text.ElideRight
                    text: mainText
                    textFormat: Text.PlainText
                }
                PlasmaComponents.Label {
                    id: tooltipSubtext
                    width: tooltipContentItem.preferredTextWidth
                    height: Math.min(theme.mSize(theme.defaultFont), contentHeight)
                    wrapMode: Text.WordWrap
                    text: subText
                    textFormat: Text.PlainText
                    opacity: 0.5
                    visible: text !== ""
                }
            }
        }

    }
}
