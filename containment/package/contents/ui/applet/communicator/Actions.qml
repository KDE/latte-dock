/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.core 0.2 as LatteCore

Item{
    function setProperty(appletId, parameter, value) {
        if (parameter === "latteSideColoringEnabled") {
            mainCommunicator.requires.latteSideColoringEnabled = value;
        } else if (parameter === "latteIconOverlayEnabled") {
            mainCommunicator.requires.latteIconOverlayEnabled = value;
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
        } else if (parameter === "latteIconOverlayEnabled") {
            return mainCommunicator.requires.latteIconOverlayEnabled;
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
