/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0

import org.kde.latte.abilities.definitions 0.1 as AbilityDefinition

AbilityDefinition.AppletRequirements {
    id: requirements
    property Item bridge: null

    readonly property bool isActive: bridge !== null

    onIsActiveChanged: {
        if (isActive) {
            bridge.applet.activeIndicatorEnabled = requirements.activeIndicatorEnabled;
            bridge.applet.innerZoomFactor = requirements.innerZoomFactor;
            bridge.applet.latteSideColoringEnabled = requirements.latteSideColoringEnabled;
            bridge.applet.latteIconOverlayEnabled = requirements.latteIconOverlayEnabled;
            bridge.applet.lengthMarginsEnabled = requirements.lengthMarginsEnabled;
            bridge.applet.parabolicEffectLocked = requirements.parabolicEffectLocked;
            bridge.applet.screenEdgeMarginSupported = requirements.screenEdgeMarginSupported;
            bridge.applet.windowsTrackingEnabled = requirements.windowsTrackingEnabled;
        }
    }

    onActiveIndicatorEnabledChanged: {
        if (isActive) {
            bridge.applet.activeIndicatorEnabled = requirements.activeIndicatorEnabled;
        }
    }

    onInnerZoomFactorChanged: {
        if (isActive) {
            bridge.applet.innerZoomFactor = requirements.innerZoomFactor;
        }
    }

    onLatteSideColoringEnabledChanged: {
        if (isActive) {
            bridge.applet.latteSideColoringEnabled = requirements.latteSideColoringEnabled;
        }
    }

    onLatteIconOverlayEnabledChanged: {
        if (bridge) {
            bridge.applet.latteIconOverlayEnabled = requirements.latteIconOverlayEnabled;
        }
    }

    onLengthMarginsEnabledChanged: {
        if (isActive) {
            bridge.applet.lengthMarginsEnabled = requirements.lengthMarginsEnabled;
        }
    }

    onParabolicEffectLockedChanged: {
        if (isActive) {
            bridge.applet.parabolicEffectLocked = requirements.parabolicEffectLocked;
        }
    }

    onScreenEdgeMarginSupportedChanged: {
        if (isActive) {
            bridge.applet.screenEdgeMarginSupported = requirements.screenEdgeMarginSupported;
        }
    }

    onWindowsTrackingEnabledChanged: {
        if (isActive) {
            bridge.applet.windowsTrackingEnabled = requirements.windowsTrackingEnabled;
        }
    }
}
