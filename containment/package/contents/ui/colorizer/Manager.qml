/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte 0.2 as Latte

import "../../code/ColorizerTools.js" as ColorizerTools

Loader{
    id: manager

    active: root.colorizerEnabled || forceSolidnessAndColorize

    readonly property bool forceSolidness: (root.solidStylePanel && !plasmoid.configuration.solidBackgroundForMaximized)
                                           || root.forceSolidPanel
                                           || !Latte.WindowSystem.compositingActive
    readonly property bool forceSolidnessAndColorize: forceSolidness && forceColorizeFromActiveWindowScheme

    readonly property bool backgroundIsBusy: item ? item.isBusy : false

    readonly property real themeBackgroundColorBrightness: ColorizerTools.colorBrightness(theme.backgroundColor)
    readonly property real themeTextColorBrightness: ColorizerTools.colorBrightness(theme.textColor)

    readonly property color minimizedDotColor: themeTextColorBrightness > 127.5 ? Qt.darker(theme.textColor, 1.7) : Qt.lighter(theme.textColor, 7)

    property bool mustBeShown: active && (!root.forceSolidPanel || forceSolidnessAndColorize)
    //! when forceSemiTransparentPanel is enabled because of touching or maximized etc. windows
    //! then the colorizer could be enabled for low panel transparency levels (<40%)
                               && (!userShowPanelBackground || !forceSemiTransparentPanel || (forceSemiTransparentPanel && root.panelTransparency<40))
                               && !maximizedWindowTitleBarBehavesAsPanelBackground
                               && (plasmoid.configuration.solidBackgroundForMaximized || plasmoid.configuration.backgroundOnlyOnMaximized)
                               && !root.editMode && Latte.WindowSystem.compositingActive

    readonly property real currentBackgroundBrightness: item ? item.currentBrightness : -1000

    property QtObject applyTheme: {
        if (forceSolidnessAndColorize && latteView.windowsTracker.touchingWindowScheme) {
            return latteView.windowsTracker.touchingWindowScheme;
        }

        if (themeExtended) {
            if (currentBackgroundBrightness > 127.5) {
                return themeExtended.lightTheme;
            } else {
                return themeExtended.darkTheme;
            }
        }

        return theme;
    }

    property color applyColor: applyTheme.textColor

    //! new TEMPORARY options to pass palette to applets
    //! UNTIL Latte produces two different color schemes files (LIGHT / DARK)
    //! to be passed to applets etc...
    readonly property color backgroundColor: applyTheme.backgroundColor
    readonly property color textColor: applyTheme.textColor
    readonly property color inactiveBackgroundColor: applyTheme === theme ? theme.backgroundColor : applyTheme.inactiveBackgroundColor
    readonly property color inactiveTextColor: applyTheme === theme ? theme.textColor : applyTheme.inactiveTextColor

    readonly property color highlightColor: theme.highlightColor
    readonly property color highlightedTextColor: theme.highlightedTextColor
    readonly property color positiveTextColor: applyTheme.positiveTextColor
    readonly property color neutralTextColor: applyTheme.neutralTextColor
    readonly property color negativeTextColor: applyTheme.negativeTextColor

    readonly property color buttonTextColor: theme.buttonTextColor
    readonly property color buttonBackgroundColor: theme.buttonBackgroundColor
    readonly property color buttonHoverColor: theme.buttonHoverColor
    readonly property color buttonFocusColor: theme.buttonFocusColor

    readonly property string scheme: {
        if (applyTheme===theme || !mustBeShown) {
            if (themeExtended) {
                return themeExtended.defaultTheme.schemeFile;
            } else {
                return "kdeglobals";
            }
        }

        return applyTheme.schemeFile;
    }

    sourceComponent: Latte.BackgroundTracker {
        activity: managedLayout ? managedLayout.lastUsedActivity : ""
        location: plasmoid.location
        screenName: latteView && latteView.positioner ? latteView.positioner.currentScreenName : ""
    }
}
