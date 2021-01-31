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

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

import "../../../code/AppletIdentifier.js" as AppletIdentifier

Item{
    id: mainCommunicator

    //             -------------------------------------
    //              BEGIN OF INTERNAL APPLET PROPERTIES
    //             LATTE<->APPLET COMMUNICATION PROTOCOL
    //             -------------------------------------
    //
    // NAME: latteBridge
    //   USAGE: property QtObject latteBridge: null
    //   EXPLANATION: The main shared object that Latte is using to communicate with the applet
    property bool appletContainsLatteBridge: appletRootItem && appletRootItem.hasOwnProperty("latteBridge") ? true : false
    //!              END OF INTERNAL APPLET PROPERTIES
    //             -------------------------------------

    //! BEGIN OF PROPERTIES
    //this is used for applets to identify their main icon shown in order to identify its main colors
    readonly property bool appletMainIconIsFound: (appletIconItem !== null) || (appletImageItem !== null)

    property bool inStartup: true

    property Item appletRootItem: appletDiscoveredRootItem ? appletDiscoveredRootItem : appletDefaultRootItem
    property Item appletDiscoveredRootItem: null
    property Item appletDefaultRootItem: applet && applet.children && applet.children.length>0 ? applet.children[0] : null

    property Item appletIconItem: null //first applet's IconItem to be used by Latte
    property Item appletImageItem: null //first applet's ImageItem to be used by Latte
    //! END OF PROPERTIES

    //! BEGIN OF PUBLIC PROPERTIES SET THROUGH LATTEBRIDGE.ACTIONS   
    readonly property Item requires: AbilityDefinition.AppletRequirements{}
    //! END OF PUBLIC PROPERTIES SET THROUGH LATTEBRIDGE.ACTIONS

    //! BEGIN OF ABILITIES SUPPORT
    readonly property bool animationsAreSupported: bridge && bridge.animations.client
    readonly property bool indexerIsSupported: bridge && bridge.indexer.client
    readonly property bool launchersAreSupported: bridge && bridge.launchers.client
    readonly property bool myViewIsSupported: bridge && bridge.myView.client
    readonly property bool parabolicEffectIsSupported: bridge && bridge.parabolic.client
    readonly property bool positionShortcutsAreSupported: bridge && bridge.shortcuts.client
    readonly property bool thinTooltipIsSupported: bridge && bridge.thinTooltip.client

    readonly property Item bridge: bridgeLoader.active ? bridgeLoader.item : null
    //! END OF ABILITIES SUPPORT

    onAppletIconItemChanged: {
        if (appletIconItem !== null) {
            communicator.appletIconItem.roundToIconSize = false;
        }
    }

    //! BEGIN OF FUNCTIONS

    function reconsiderAppletIconItem() {
        AppletIdentifier.reconsiderAppletIconItem();
    }

    //! END OF FUNCTIONS

    //! BEGIN OF CONNECTIONS
    Connections{
        target: appletItem
        onAppletChanged: {
            if (applet) {
                AppletIdentifier.checkAndUpdateAppletRootItem();
                AppletIdentifier.reconsiderAppletIconItem();
                overlayInitTimer.start();
            }
        }
    }

    //! END OF CONNECTIONS

    //! BEGIN OF LOADERS
    Loader{
        id: bridgeLoader
        active: appletContainsLatteBridge
        sourceComponent: LatteBridge{}
    }
    //! END OF LOADERS

    //! BEGIN OF TIMERS
    //a timer that is used in  order to init some Communicator values
    Timer {
        id: overlayInitTimer
        interval: 1000
        onTriggered: {
            AppletIdentifier.checkAndUpdateAppletRootItem();
            AppletIdentifier.reconsiderAppletIconItem();
            mainCommunicator.inStartup = false;

            if (appletItem.debug.timersEnabled) {
                console.log("containment timer: appletItem fakeInitTimer called...");
            }
        }
    }

    //! END OF TIMERS
}
