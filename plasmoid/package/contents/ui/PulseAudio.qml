/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2

import org.kde.plasma.private.volume 0.1

QtObject {
    id: pulseAudio

    signal streamsChanged

    // It's a JS object so we can do key lookup and don't need to take care of filtering duplicates.
    property var pidMatches: ({})

    property int maxVolumePercent: 125
    property int maxVolumeValue: Math.round(maxVolumePercent * PulseAudio.NormalVolume / 100.0)
    property int volumeStep: Math.round(5 * PulseAudio.NormalVolume / 100.0)

    function boundVolume(volume) {
        return Math.max(PulseAudio.MinimalVolume, Math.min(volume, maxVolumeValue));
    }

    // TODO Evict cache at some point, preferably if all instances of an application closed.
    function registerPidMatch(appName) {
        if (!hasPidMatch(appName)) {
            pidMatches[appName] = true;

            // In case this match is new, notify that streams might have changed.
            // This way we also catch the case when the non-playing instance
            // shows up first.
            // Only notify if we changed to avoid infinite recursion.
            streamsChanged();
        }
    }

    function hasPidMatch(appName) {
        return pidMatches[appName] === true;
    }

    function findStreams(key, value) {
        var streams = []
        for (var i = 0, length = instantiator.count; i < length; ++i) {
            var stream = instantiator.objectAt(i);
            if (stream[key] === value || (key==="appName" && stream[key].toLowerCase() === value.toLowerCase())) {
                streams.push(stream);
            }
        }
        return streams
    }

    function streamsForAppName(appName) {
        return findStreams("appName", appName);
    }

    function streamsForPid(pid) {
        var streams = findStreams("pid", pid);

        if (streams.length === 0) {
            for (var i = 0, length = instantiator.count; i < length; ++i) {
                var stream = instantiator.objectAt(i);

                if (stream.parentPid === -1) {
                    stream.parentPid = backend.parentPid(stream.pid);
                }

                if (stream.parentPid === pid) {
                    streams.push(stream);
                }
            }
        }

        return streams;
    }

    // QtObject has no default property, hence adding the Instantiator to one explicitly.
    property var instantiator: Instantiator {
        model: PulseObjectFilterModel {
            filters: [ { role: "VirtualStream", value: false } ]
            sourceModel: SinkInputModel {}
        }

        delegate: QtObject {
            readonly property int pid: Client ? Client.properties["application.process.id"] : 0
            // Determined on demand.
            property int parentPid: -1
            readonly property string appName: Client ? Client.properties["application.name"] : ""
            readonly property bool muted: Muted
            // whether there is nothing actually going on on that stream
            readonly property bool corked: Corked

            readonly property int volume: Math.round(pulseVolume / PulseAudio.NormalVolume * 100.0)
            readonly property int pulseVolume: Volume

            function mute() {
                Muted = true
            }
            function unmute() {
                Muted = false
            }

            function increaseVolume() {
                var bVolume = pulseAudio.boundVolume(Volume + pulseAudio.volumeStep);
                Volume = bVolume;
            }

            function decreaseVolume() {
                var bVolume = pulseAudio.boundVolume(Volume - pulseAudio.volumeStep);
                Volume = bVolume;
            }
        }

        onObjectAdded: pulseAudio.streamsChanged()
        onObjectRemoved: pulseAudio.streamsChanged()
    }

    Component.onCompleted: {
        console.log("PulseAudio Latte interface was loaded...");
    }
}
