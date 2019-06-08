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

    //! the loader loads the backgroundTracker component
    active: root.themeColors === Latte.Types.SmartThemeColors

    readonly property bool backgroundIsBusy: item ? item.isBusy : false

    readonly property real originalThemeTextColorBrightness: ColorizerTools.colorBrightness(theme.textColor)
    readonly property color originalLightTextColor: originalThemeTextColorBrightness > 127.5 ? theme.textColor : theme.backgroundColor

    readonly property real themeTextColorBrightness: ColorizerTools.colorBrightness(textColor)
    readonly property real backgroundColorBrightness: ColorizerTools.colorBrightness(backgroundColor)
    readonly property color minimizedDotColor: themeTextColorBrightness > 127.5 ? Qt.darker(textColor, 1.7) : Qt.lighter(textColor, 8)

    readonly property color focusGlowColor: Qt.hsva(buttonFocusColor.h, buttonFocusColor.s, 1.0, 1.0)

    readonly property color outlineColorBase: backgroundColor
    readonly property real outlineColorBaseBrightness: ColorizerTools.colorBrightness(outlineColorBase)
    readonly property color outlineColor: {
        if (!root.panelOutline) {
            return backgroundColor;
        }

        if (outlineColorBaseBrightness > 127.5) {
            return Qt.darker(outlineColorBase, 1.5);
        } else {
            return Qt.lighter(outlineColorBase, 2.2);
        }
    }

    readonly property bool editModeTextColorIsBright: ColorizerTools.colorBrightness(editModeTextColor) > 127.5
    readonly property color editModeTextColor: latteView && latteView.layout ? latteView.layout.textColor : "white"

    readonly property bool mustBeShown: (applyTheme && applyTheme !== theme) || (root.inConfigureAppletsMode && (root.themeColors === Latte.Types.SmartThemeColors))

    readonly property real currentBackgroundBrightness: item ? item.currentBrightness : -1000

    readonly property bool applyingWindowColors: (root.windowColors === Latte.Types.ActiveWindowColors && latteView && latteView.windowsTracker
                                                  && selectedWindowsTracker.activeWindowScheme)
                                                 || (root.windowColors === Latte.Types.TouchingWindowColors && latteView && latteView.windowsTracker
                                                     && latteView.windowsTracker.currentScreen.touchingWindowScheme)

    property QtObject applyTheme: {
        if (latteView && latteView.windowsTracker && !root.hasExpandedApplet) {
            if (root.windowColors === Latte.Types.ActiveWindowColors && selectedWindowsTracker.activeWindowScheme) {
                return selectedWindowsTracker.activeWindowScheme;
            }

            if (root.windowColors === Latte.Types.TouchingWindowColors && latteView.windowsTracker.currentScreen.touchingWindowScheme) {
                //! we must track touching windows and when they are not ative
                //! the active window scheme is used for convenience
                if (latteView.windowsTracker.currentScreen.existsWindowTouching
                        && !latteView.windowsTracker.currentScreen.activeWindowTouching
                        && latteView.windowsTracker.currentScreen.activeWindowScheme) {
                    return latteView.windowsTracker.currentScreen.activeWindowScheme;
                }

                return latteView.windowsTracker.currentScreen.touchingWindowScheme;
            }
        }

        if (themeExtended) {
            if (root.userShowPanelBackground && root.plasmaBackgroundForPopups && root.hasExpandedApplet /*for expanded popups when it is enabled*/
                    || (root.themeColors === Latte.Types.SmartThemeColors /*for Smart theming that Windows colors are not used and the user wants solidness at some cases*/
                        && root.windowColors === Latte.Types.NoneWindowColors
                        && root.forceSolidPanel) ) {
                /* plasma style*/
                return theme;
            }

            if (root.themeColors === Latte.Types.ReverseThemeColors) {
                return themeExtended.isLightTheme ? themeExtended.darkTheme : themeExtended.lightTheme;
            }

            if (root.themeColors === Latte.Types.SmartThemeColors) {
                if (currentBackgroundBrightness > 127.5) {
                    return themeExtended.lightTheme;
                } else {
                    return themeExtended.darkTheme;
                }
            }
        }

        return theme;
    }

    property color applyColor: textColor

    readonly property color backgroundColor:applyTheme.backgroundColor
    readonly property color textColor: {
        if (latteView && latteView.layout
                && root.inConfigureAppletsMode
                && Latte.WindowSystem.compositingActive
                && root.panelTransparency<40
                && (root.themeColors === Latte.Types.SmartThemeColors)) {
            return latteView.layout.textColor;
        }

        return applyTheme.textColor;
    }

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

    readonly property string scheme: {
        if (root.inConfigureAppletsMode && (root.themeColors === Latte.Types.SmartThemeColors)) {
            if (!Latte.WindowSystem.compositingActive && applyTheme !== theme) {
                return applyTheme.schemeFile;
            }

            //! in edit mode (that is shown the edit visual without opacity)
            //! take care the applets that need a proper color scheme to paint themselves
            if ((editModeTextColorIsBright && themeExtended.isLightTheme)
                    || (!editModeTextColorIsBright && !themeExtended.isLightTheme)) {
                if (themeExtended.darkTheme === themeExtended.defaultTheme) {
                    console.log("light theme..." + themeExtended.isLightTheme);
                    return themeExtended.lightTheme.schemeFile;
                } else {
                    console.log("dark theme..." + themeExtended.isLightTheme);
                    return themeExtended.darkTheme.schemeFile;
                }
            } else {
                console.log("default theme... : " + themeExtended.isLightTheme);
                return themeExtended.defaultTheme.schemeFile;
            }
        }

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
        activity: viewLayout ? viewLayout.lastUsedActivity : ""
        location: plasmoid.location
        screenName: latteView && latteView.positioner ? latteView.positioner.currentScreenName : ""
    }
}
