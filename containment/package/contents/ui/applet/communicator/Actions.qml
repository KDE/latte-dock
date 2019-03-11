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

Item{
    // NAME: latteSideColoringEnabled
    // TYPE: bool
    //   USAGE: writable through actions.setProperty
    //   EXPLANATION: when is FALSE, Latte is not painting/colorizing this applet
    //       in any case. In such case the applet can use 'palette'
    //       in order to access the color palette used at all cases from Latte
    //   USE CASE: when Latte is transparent and applets colors need to be adjusted in order
    //       to look consistent with the underlying desktop background OR the applet
    //       is not using monochromatic icons but rather colorful ones.
    // @since: 0.9


    // NAME: latteIconOverlayEnabled
    // TYPE: bool
    //   USAGE: writable through actions.setProperty
    //   EXPLANATION: when is FALSE, Latte is not overlaying any icons above
    //       the applet or alters the applet visual in any sense.
    //       That means that the applet is responsible to provide a coherent
    //       parabolic effect experience.
    // @since: 0.9

    // NAME: activeIndicatorEnabled
    // TYPE: bool
    //   USAGE: writable through actions.setProperty
    //   EXPLANATION: when is TRUE, Latte can show its own Active Indicator
    //       when needed. For FALSE, the Latte Active Indicator is not drawn
    //       or used for that applet.
    // @since: 0.9

    // NAME: parabolicEffectEnabled
    // TYPE: bool
    //   USAGE: writable through actions.setProperty
    //   EXPLANATION: when is TRUE, Latte can use Parabolic Effect in order
    //       to draw that applet. For FALSE, this applet is considered locked
    //       and can not be zoomed.
    // @since: 0.9

    function setProperty(appletId, parameter, value) {
        if (parameter === "latteSideColoringEnabled") {
            mainCommunicator.latteSideColoringEnabled = value;
        } else if (parameter === "latteIconOverlayEnabled") {
            mainCommunicator.latteIconOverlayEnabled = value;
        } else if (parameter === "activeIndicatorEnabled") {
            mainCommunicator.activeIndicatorEnabled = value;
        } else if (parameter === "parabolicEffectLocked") {
            mainCommunicator.parabolicEffectLocked = value;
        }
    }

    function getProperty(appletId, parameter, value) {
        if (parameter === "latteSideColoringEnabled") {
            return mainCommunicator.latteSideColoringEnabled;
        } else if (parameter === "latteIconOverlayEnabled") {
            return mainCommunicator.latteIconOverlayEnabled;
        } else if (parameter === "activeIndicatorEnabled") {
            return mainCommunicator.activeIndicatorEnabled;
        } else if (parameter === "parabolicEffectLocked") {
            return mainCommunicator.parabolicEffectLocked;
        }

        return null;
    }


    //!   USAGE: send message to <pluginName>, for <action> command and set its property to <value>
    //!   EXPLANATION: applets can send messages/commands to other applets
    //!       in order to communicate with each other
    function broadcastToApplet(pluginName, action, value) {
        root.broadcastedToApplet(pluginName, action, value);
    }

}
