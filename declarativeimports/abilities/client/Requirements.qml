/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.AppletRequirements {
    id: requirements
    property Item bridge: null

    readonly property bool isActive: bridge !== null

    onIsActiveChanged: {
        if (isActive) {
            bridge.applet.activeIndicatorEnabled = requirements.activeIndicatorEnabled;
            bridge.applet.latteSideColoringEnabled = requirements.latteSideColoringEnabled;
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

    onLatteSideColoringEnabledChanged: {
        if (isActive) {
            bridge.applet.latteSideColoringEnabled = requirements.latteSideColoringEnabled;
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
