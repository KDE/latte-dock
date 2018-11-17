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

import org.kde.latte 0.1 as Latte

import "../../code/ColorizerTools.js" as ColorizerTools

Loader{
    id: manager

    active: forceColorizer || forceSolidnessAndColorize

    readonly property bool forceSolidness: (root.solidPanel && !plasmoid.configuration.solidBackgroundForMaximized) || root.forceSolidPanel
                                           || !Latte.WindowSystem.compositingActive
    readonly property bool forceSolidnessAndColorize: forceSolidness && forceColorizeFromActiveWindowScheme

    readonly property real themeBackgroundColorLuma: ColorizerTools.colorLuminas(theme.backgroundColor)
    readonly property real themeTextColorLuma: ColorizerTools.colorLuminas(theme.textColor)
    readonly property color minimizedDotColor: themeTextColorLuma > 0.6 ? Qt.darker(theme.textColor, 1.7) : Qt.lighter(theme.textColor, 7)

    property bool mustBeShown: active && (!forceSolidPanel || forceSolidnessAndColorize)
    //! when forceSemiTransparentPanel is enabled because of snapped or maximized etc. windows
    //! then the colorizer could be enabled for low panel transparency levels (<40%)
                               && (!userShowPanelBackground || !forceSemiTransparentPanel || (forceSemiTransparentPanel && root.panelTransparency<40))
                               && !maximizedWindowTitleBarBehavesAsPanelBackground
                               && (plasmoid.configuration.solidBackgroundForMaximized || plasmoid.configuration.backgroundOnlyOnMaximized)
                               && !root.editMode && Latte.WindowSystem.compositingActive

    property real currentBackgroundLuminas: item ? item.currentBackgroundLuminas : -1000

    property QtObject applyTheme: {
        if (forceSolidnessAndColorize && dock.visibility.touchingWindowScheme) {
            return dock.visibility.touchingWindowScheme;
        }

        if (themeExtended) {
            if (currentBackgroundLuminas > 0.5) {
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

    readonly property color highlightColor: applyTheme.highlightColor
    readonly property color highlightedTextColor: applyTheme.highlightedTextColor
    readonly property color positiveTextColor: applyTheme.positiveTextColor
    readonly property color neutralTextColor: applyTheme.neutralTextColor
    readonly property color negativeTextColor: applyTheme.negativeTextColor

    readonly property color buttonTextColor: applyTheme.buttonTextColor
    readonly property color buttonBackgroundColor: applyTheme.buttonBackgroundColor
    readonly property color buttonHoverColor: applyTheme.buttonHoverColor
    readonly property color buttonFocusColor: applyTheme.buttonFocusColor

    readonly property string scheme: themeExtended ? applyTheme.schemeFile : "kdeglobals"

    sourceComponent: BackgroundTracker{}
}
