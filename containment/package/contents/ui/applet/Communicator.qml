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

import "../../code/AppletIdentifier.js" as AppletIdentifier

Item{
    id: main

    //             -------------------------------------
    //              BEGIN OF INTERNAL APPLET PROPERTIES
    //             LATTE<->APPLET COMMUNICATION PROTOCOL
    //             -------------------------------------

    // USAGE: The applet can set the following properties in its main item in order to
    // establish communication with the Latte containment

    //             -------------------------------------
    //                  INGOING FROM LATTE TO APPLET
    //             -------------------------------------

    // USE: property bool isInLatte: false
    // EXPLANATION: Latte sets it to true when this applet is in a Latte containment
    property bool supportsIsInLatte: applet && applet.isInLatte !== undefined ? true : false

    //             -------------------------------------
    //                  OUTGOING FROM APPLET TO LATTE
    //             -------------------------------------

    // USE: property bool disableLatteParabolicIcon
    // DEPRECATED FROM: disableLatteOverlay
    // EXPLANATION: when is TRUE, Latte is not overlaying any icons above
    //     the applet or alters the applet visual in any sense.
    //     That means that the applet is responsible to provide a conherent
    //     parabolic effect experience.
    property bool disableLatteParabolicIconHeuristics: applet && applet.disableLatteParabolicIcon !== undefined ?
                                                           applet.disableLatteParabolicIcon : false
    // USE: property bool disableLatteOverlay
    // EXPLANATION: when is TRUE, Latte is not overlaying any icons above
    //     the applet or alters the applet visual in any sense.
    //     That means that the applet is responsible to provide a conherent
    //     parabolic effect experience.
    property bool disableLatteOverlay: applet && applet.disableLatteOverlay !== undefined ?
                                           applet.disableLatteOverlay : false

    //!              END OF INTERNAL APPLET PROPERTIES


    //! BEGIN OF PROPERTIES
    //this is used for folderView and icon widgets to fake their visual icons
    readonly property bool canShowOverlaiedLatteIcon: applet && communicator.appletIconItem //(applet.pluginName === "org.kde.plasma.folder" || applet.pluginName === "org.kde.plasma.icon")
    readonly property bool overlayLatteIconIsActive: canShowOverlaiedLatteIcon
                                                     && !(disableLatteParabolicIconHeuristics || disableLatteOverlay)

    property Item appletIconItem; //first applet's IconItem to be used by Latte
    property Item appletImageItem; //first applet's ImageItem to be used by Latte
    //! END OF PROPERTIES

    //! BEGIN OF PROPERTY CHANGES
    onSupportsIsInLatteChanged: {
        if (supportsIsInLatte) {
            applet.isInLatte = true;
        }
    }
    //! END OF PROPERTY CHANGES

    //! BEGIN OF FUNCTIONS
    function appletIconItemIsShown() {
        return appletIconItem && appletIconItem.visible;
    }

    function appletImageItemIsShown() {
        return appletImageItem && appletImageItem.visible;
    }

    function reconsiderAppletIconItem() {
        AppletIdentifier.reconsiderAppletIconItem();
    }

    function setAppletIconItemActive(isActive) {
        if (appletIconItem) {
            appletIconItem.active = isActive;
        }
    }
    //! END OF FUNCTIONS

    //! BEGIN OF CONNECTIONS
    Connections{
        target: container
        onAppletChanged: {
            if (applet) {
                main.reconsiderAppletIconItem();
                overlayInitTimer.start();
            }
        }

    }
    //! END OF CONNECTIONS

    //! BEGIN OF TIMERS
    //a timer that is used in  order to init some Communicator values
    Timer {
        id: overlayInitTimer
        interval: 4000
        onTriggered: {
            AppletIdentifier.reconsiderAppletIconItem();

            if (root.debugModeTimers) {
                console.log("containment timer: appletItem fakeInitTimer called...");
            }
        }
    }

    //! END OF TIMERS
}
