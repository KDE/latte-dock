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
    // NAME: isInLatte
    //   USAGE: property bool isInLatte: false
    //   EXPLANATION: Latte sets it to true when this applet is in a Latte containment. This parameter
    //       is very important because it identifies the main element of the applet in which all latte
    //       parameters need to also placed. Be careful in case you are using CompactRepresentation then
    //       the main element for which you must place the Latte options is the CompactRepresentation.
    property bool appletContainsIsInLatte: appletRootItem && appletRootItem.hasOwnProperty("isInLatte") ? true : false

    // NAME: latteSettings
    //   USAGE: property QtObject latteSettings: null
    //   EXPLANATION: The main shared object that Latte is using to communicate with the applet
    //   USE CASE: when Latte is transparent and applets colors need to be adjusted in order
    //       to look consistent with the underlying desktop background
    property bool appletContainsLatteSettings: appletRootItem && appletRootItem.hasOwnProperty("latteSettings") ? true : false
    //!              END OF INTERNAL APPLET PROPERTIES
    //             -------------------------------------

    //! BEGIN OF PROPERTIES
    //this is used for folderView and icon widgets to fake their visual icons
    readonly property bool canShowOverlaiedLatteIcon: appletRootItem && communicator.appletIconItem
    readonly property bool overlayLatteIconIsActive: canShowOverlaiedLatteIcon
                                                     && settingsLoader.active
                                                     && !settingsLoader.item.disableLatteIconOverlay

    property Item appletRootItem: appletDiscoveredRootItem ? appletDiscoveredRootItem : appletDefaultRootItem
    property Item appletDiscoveredRootItem: null
    property Item appletDefaultRootItem: applet && applet.children && applet.children.length>0 ? applet.children[0] : null

    property Item appletIconItem; //first applet's IconItem to be used by Latte
    property Item appletImageItem; //first applet's ImageItem to be used by Latte
    //! END OF PROPERTIES

    //! BEGIN OF PROPERTY CHANGES
    onAppletContainsIsInLatteChanged: {
        if (appletContainsIsInLatte) {
            appletRootItem.isInLatte = true;
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
                AppletIdentifier.checkAndUpdateAppletRootItem();
                AppletIdentifier.reconsiderAppletIconItem();
                overlayInitTimer.start();
            }
        }

    }
    //! END OF CONNECTIONS

    //! BEGIN OF BINDINGS
    Binding{
        target: appletRootItem
        property: "latteSettings"
        when: settingsLoader.active
        value: settingsLoader.item
    }
    //! END OF BINDINGS

    Loader{
        id:settingsLoader
        active: appletContainsIsInLatte && appletContainsLatteSettings
        sourceComponent: LatteSettings{}
    }

    //! BEGIN OF TIMERS
    //a timer that is used in  order to init some Communicator values
    Timer {
        id: overlayInitTimer
        interval: 4000
        onTriggered: {
            AppletIdentifier.checkAndUpdateAppletRootItem();
            AppletIdentifier.reconsiderAppletIconItem();

            if (root.debugModeTimers) {
                console.log("containment timer: appletItem fakeInitTimer called...");
            }
        }
    }

    //! END OF TIMERS
}
