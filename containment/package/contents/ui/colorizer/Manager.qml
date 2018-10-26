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

Loader{
    id: manager

    active: forceColorizer || forceSolidnessAndColorize

    readonly property bool forceSolidness: (root.solidPanel && !plasmoid.configuration.solidBackgroundForMaximized) || root.forceSolidPanel
                                           || !Latte.WindowSystem.compositingActive
    readonly property bool forceSolidnessAndColorize: forceSolidness && forceColorizeFromActiveWindowScheme

    // formula for luminance according to:
    // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
    property real textColorRs: {
        var color = theme.textColor.r;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    property real textColorGs: {
        var color = theme.textColor.g;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    property real textColorBs: {
        var color = theme.textColor.b;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    //! -------
    property real backColorRs: {
        var color = theme.backgroundColor.r;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    property real backColorGs: {
        var color = theme.backgroundColor.g;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    property real backColorBs: {
        var color = theme.backgroundColor.b;
        if (color <= 0.03928) {
            return color / 12.92;
        } else {
            return Math.pow( ((color + 0.055) / 1.055), 2.4 );
        }
    }

    readonly property real themeBackgroundColorLuma: 0.2126*backColorRs + 0.7152*backColorGs + 0.0722*backColorBs
    readonly property real themeTextColorLuma: 0.2126*textColorRs + 0.7152*textColorGs + 0.0722*textColorBs
    readonly property color minimizedDotColor: themeTextColorLuma > 0.6 ? Qt.darker(theme.textColor, 1.7) : Qt.lighter(theme.textColor, 7)

    property bool mustBeShown: active && (!forceSolidPanel || forceSolidnessAndColorize)
    //! when forceSemiTransparentPanel is enabled because of snapped or maximized etc. windows
    //! then the colorizer could be enabled for low panel transparency levels (<40%)
                               && (!userShowPanelBackground || !forceSemiTransparentPanel || (forceSemiTransparentPanel && root.panelTransparency<40))
                               && !maximizedWindowTitleBarBehavesAsPanelBackground
                               && (plasmoid.configuration.solidBackgroundForMaximized || plasmoid.configuration.backgroundOnlyOnMaximized)
                               && !root.editMode && Latte.WindowSystem.compositingActive

    property real currentBackgroundLuminas: item ? item.currentBackgroundLuminas : -1000

    property color themeBrightColor: themeBackgroundColorLuma > themeTextColorLuma ? theme.backgroundColor : theme.textColor
    property color themeDarkColor: themeBackgroundColorLuma > themeTextColorLuma ? theme.textColor : theme.backgroundColor

    property color applyColor: {
        if (forceSolidnessAndColorize && dock.visibility.touchingWindowScheme) {
            return dock.visibility.touchingWindowScheme.textColor;
        }

        if (currentBackgroundLuminas>=0) {
            var textAbs = Math.abs(themeTextColorLuma - currentBackgroundLuminas);
            var backAbs = Math.abs(themeBackgroundColorLuma - currentBackgroundLuminas);

            if (textAbs > backAbs) {
                return Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b , 1); //remove any transparency
            }
        }

        return Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b, 1); //remove any transparency
    }

    //! new TEMPORARY options to pass palette to applets
    //! UNTIL Latte produces two different color schemes files (LIGHT / DARK)
    //! to be passed to applets etc...

    readonly property color textColor: applyColor
    readonly property color highlightColor: theme.highlightColor
    readonly property color highlightedTextColor: theme.highlightedTextColor
    readonly property color backgroundColor: {
        if (forceSolidnessAndColorize && dock.visibility.touchingWindowScheme) {
            return dock.visibility.touchingWindowScheme.backgroundColor;
        }

        if (currentBackgroundLuminas>=0) {
            var textAbs = Math.abs(themeTextColorLuma - currentBackgroundLuminas);
            var backAbs = Math.abs(themeBackgroundColorLuma - currentBackgroundLuminas);

            if (textAbs > backAbs) {
                return Qt.rgba(theme.backgroundColor.r, theme.backgroundColor.g, theme.backgroundColor.b , 1); //remove any transparency
            }
        }

        return Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 1); //remove any transparency
    }


    sourceComponent: BackgroundTracker{}
}
