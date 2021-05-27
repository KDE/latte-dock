/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.core 0.2 as LatteCore

Item{
    function setProperty(appletId, parameter, value) {
        if (parameter === "latteSideColoringEnabled") {
            mainCommunicator.requires.latteSideColoringEnabled = value;
        } else if (parameter === "activeIndicatorEnabled") {
            mainCommunicator.requires.activeIndicatorEnabled = value;
        } else if (parameter === "lengthMarginsEnabled") {
           mainCommunicator.requires.lengthMarginsEnabled = value;
        } else if (parameter === "parabolicEffectLocked") {
            mainCommunicator.requires.parabolicEffectLocked = value;
        } else if (parameter === "screenEdgeMarginSupported") {
            mainCommunicator.requires.screenEdgeMarginSupported = value;
        } else if (parameter === "windowsTrackingEnabled") {
            mainCommunicator.requires.windowsTrackingEnabled = value;
        }
    }

    function getProperty(appletId, parameter) {
        if (parameter === "latteSideColoringEnabled") {
            return mainCommunicator.requires.latteSideColoringEnabled;
        } else if (parameter === "activeIndicatorEnabled") {
            return mainCommunicator.requires.activeIndicatorEnabled;
        } else if (parameter === "lengthMarginsEnabled") {
            return mainCommunicator.requires.lengthMarginsEnabled;
        } else if (parameter === "parabolicEffectLocked") {
            return mainCommunicator.requires.parabolicEffectLocked;
        } else if (parameter === "screenEdgeMarginSupported") {
            return mainCommunicator.requires.screenEdgeMarginSupported;
        } else if (parameter === "windowsTrackingEnabled") {
            return mainCommunicator.requires.windowsTrackingEnabled;
        }

        return null;
    }


    //!   USAGE: send message to <pluginName>, for <action> command and set its property to <value>
    //!   EXPLANATION: applets can send messages/commands to other applets
    //!       in order to communicate with each other
    function broadcastToApplet(receiverPluginId, action, value) {
        root.broadcastedToApplet(receiverPluginId, action, value);
    }

    function version(major, minor, patch) {
        return LatteCore.Environment.makeVersion(major, minor, patch)
    }
}
