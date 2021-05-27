/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
