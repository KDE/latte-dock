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
    id: settings

    //! EXPOSED SIGNALS

    //!   USAGE: signals from other applets, <command>, <value>
    //!   EXPLANATION: applets can receive signals from other applets
    //!       to catch signals from other applets in order to communicate with
    //!       each other
    signal broadcasted(string action, variant value);

    //! EXPOSED PROPERTIES

    // NAME: version
    //   USAGE: read-only
    //   EXPLANATION: Latte communication version in order for the applet to use only properties
    //       and parameters that are valid
    // @since: 0.9
    readonly property int version: root.version

    // NAME: inEditMode
    //   USAGE: read-only
    //   EXPLANATION: Latte sets it to true when this applet is in a Latte containment and Latte
    //       is also in EditMode, that means when the user is altering applets and Latte latteView settings
    // @since: 0.9
    readonly property bool inEditMode: root.inConfigureAppletsMode

    // NAME: inPlasmaPanelStyle
    //   USAGE: read-only
    //   EXPLANATION: Latte sets it to true when this view behaves as Plasma panel concerning
    //       drawing background shadows externally and applets can take this into account in order
    //       to adjust their appearance and behavior
    // @since: 0.9
    readonly property bool inPlasmaPanelStyle: root.behaveAsPlasmaPanel

    // NAME: palette
    //   USAGE: read-only
    //   EXPLANATION: Latte updates it to its coloring palette in order for the applet
    //       to take responsibility of its coloring.
    //   USE CASE: when Latte is transparent and applets colors need to be adjusted in order
    //       to look consistent with the underlying desktop background
    // @since: 0.9
    readonly property QtObject palette: !latteSideColoringEnabled ? colorizerManager : null

    // NAME: applyPalette
    //   USAGE: read-only
    //   EXPLANATION: Latte updates it to TRUE when the applet must use the provided
    //       Latte "palette" and FALSE otherwise
    //   USE CASE: when Latte is transparent and applets colors need to be adjusted in order
    //       to look consistent with the underlying desktop background
    // @since: 0.9
    readonly property bool applyPalette: !latteSideColoringEnabled ? colorizerManager.mustBeShown : false

    // NAME: parabolicEffectEnabled
    //   USAGE: read-only
    //   EXPLANATION: Parabolic Effect is enabled or not for this Latte View
    //   USE CASE: it can be used from applets that want to be adjusted based
    //       on the parabolic Effect or not
    // @since: 0.9
    readonly property bool parabolicEffectEnabled: root.parabolicEffectEnabled && !appletItem.originalAppletBehavior

    // NAME: screenMarginEnabled
    //   USAGE: read-only
    //   EXPLANATION: Screen Edge Margin is enabled or not for this Latte View
    //   USE CASE: it can be used from applets that want to be adjusted based
    //       on the screen margin applied without losing any of their functionality
    // @since: 0.10
    readonly property bool screenEdgeMarginEnabled: root.screenEdgeMarginEnabled

    // NAME: iconSize
    //   USAGE: read-only
    //   EXPLANATION: The current icon size used in the Latte View
    //   USE CASE: it can be used from applets that want their size to be always
    //       relevant to the view icon size
    // @since: 0.9
    readonly property int iconSize: root.iconSize

    // NAME: screenEdgeMargin
    //   USAGE: read-only
    //   EXPLANATION: The screen edge margin applied in pixels
    //   USE CASE: it can be used from applets that want to be informed what is the screen edge margin
    //       currently applied
    // @since: 0.10
    readonly property int screenEdgeMargin: root.screenEdgeMargin

    // NAME: maxZoomFactor
    //   USAGE: read-only
    //   EXPLANATION: The maximum zoom factor currently used
    //   USE CASE: it can be used from applets that want to be informed what is the maximum
    //       zoom factor currently used
    // @since: 0.9
    readonly property real maxZoomFactor: root.zoomFactor

    // NAME: windowsTracker
    //   USAGE: read-only
    //   EXPLANATION: windows tracking based on the view this applet is present
    //   USE CASE: it can be used from applets that want windows tracking in order
    //       to update their appearance or their behavior accordingly
    // @since: 0.9
    readonly property QtObject windowsTracker: mainCommunicator.windowsTrackingEnabled && latteView && latteView.windowsTracker ?
                                                   latteView.windowsTracker : null

    property Item actions: Actions{}

    Connections {
        target: root
        onBroadcastedToApplet: {
            if (appletItem.applet && appletItem.applet.pluginName === pluginName) {
                settings.broadcasted(action, value);
            }
        }
    }

    //! Initialize
    Component.onCompleted: {
        appletRootItem.latteBridge = settings;
    }
}
