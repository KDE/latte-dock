/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.abilities.host 0.1 as AbilityHost

import "./metrics" as MetricsPrivateTypes

AbilityHost.Metrics {
    id: mets
    property Item animations: null
    property Item autosize: null
    property Item background: null
    property Item indicators: null
    property Item parabolic: null

    //! private properties to avoid too many not needed animation calculations
    readonly property int _iconSize: autosizeEnabled && autosize.iconSize > 0 ? Math.min(autosize.iconSize, maxIconSize) : maxIconSize
    readonly property int _maxIconSize: {
        //! round to nearest odd number
        var storedIconSize = 2 * Math.round(plasmoid.configuration.iconSize / 2)

        return portionIconSize!==-1 ? portionIconSize : storedIconSize;
    }

    //! Private Properties
    readonly property int portionIconSize: { //icon size based on screen height
        if ((plasmoid.configuration.proportionIconSize===-1) || !latteView)
            return -1;

        var basedOnScreenHeight = Math.max(16,Math.round(latteView.screenGeometry.height * plasmoid.configuration.proportionIconSize/100))

        //! round to nearest odd number
        return 2 * Math.round(basedOnScreenHeight/2);
    }

    readonly property bool autosizeEnabled: autosize !== undefined && autosize.isActive

    readonly property MetricsPrivateTypes.Fraction fraction: MetricsPrivateTypes.Fraction{
        thicknessMargin: Math.max(indicators.info.minThicknessPadding, plasmoid.configuration.thickMargin / 100)
        lengthMargin: plasmoid.configuration.lengthExtMargin / 100
        lengthPadding: indicators.isEnabled ? indicators.padding : 0
        lengthAppletPadding: indicators.isEnabled ? indicators.info.appletLengthPadding : -1
    }

    //! based on background / plasma theme minimum thickness requirements; behaveAsPlasmaPanel and floating is a good scenario for this
    readonly property int marginMinThickness: Math.max(0, (background.totals.minThickness - _maxIconSize) / 2)

    readonly property int maxThicknessForView: Math.max(mask.maxScreenEdge + ((maxIconSize+2*margin.maxTailThickness)*2) + extraThicknessForZoomed,
                                                        384)

    //! Thickness Private Calculations

    readonly property int marginBetweenContentsAndEditRuler: 10
    readonly property int extraThicknessForNormal: Math.max(extraThicknessFromIndicators, extraThicknessFromShadows)
    readonly property int extraThicknessForZoomed: marginBetweenContentsAndEditRuler + extraThicknessForNormal

    readonly property int extraThicknessFromShadows: {
        if (LatteCore.WindowSystem.isPlatformWayland) {
            return 0;
        }

        //! 45% of max shadow size in px.
        var shadowMaxNeededMargin = 0.45 * root.myView.itemShadow.maxSize;
        var shadowOpacity = (plasmoid.configuration.shadowOpacity) / 100;
        //! +40% of shadow opacity in percentage
        shadowOpacity = shadowOpacity + shadowOpacity*0.4;

        //! This way we are trying to calculate how many pixels are needed in order for the shadow
        //! to be drawn correctly without being cut of from View::mask() under X11
        shadowMaxNeededMargin = (shadowMaxNeededMargin * shadowOpacity);

        //! give some more space when items shadows are enabled and extremely big
        if (root.myView.itemShadow.isEnabled && metrics.margin.maxHeadThickness < shadowMaxNeededMargin) {
            return shadowMaxNeededMargin - metrics.margin.maxHeadThickness;
        }

        return 0;
    }

    readonly property int extraThicknessFromIndicators: indicators.info.extraMaskThickness

    readonly property real mediumFactor: (1 + (0.65 * (parabolic.factor.maxZoom-1)))
    readonly property real mediumMarginsFactor: 1 + ((mediumFactor-1) * parabolic.factor.marginThicknessZoomInPercentage)
    readonly property real maxMarginsFactor: 1 + ((parabolic.factor.maxZoom-1) * parabolic.factor.marginThicknessZoomInPercentage)

    //! BEHAVIORS
    Behavior on iconSize {
        NumberAnimation {
            duration: 0.8 * animations.duration.proposed

            onRunningChanged: {
                if (!running) {
                    mets.iconSizeAnimationEnded();
                }
            }
        }
    }

    margin {
        Behavior on length {
            NumberAnimation {
                duration: 0.8 * animations.duration.proposed
                easing.type: Easing.OutCubic
            }
        }

        Behavior on screenEdge {
            enabled: !root.behaveAsPlasmaPanel
            NumberAnimation {
                duration: 0.8 * animations.duration.proposed
                easing.type: Easing.OutCubic
            }
        }

        Behavior on tailThickness {
            NumberAnimation {
                duration: 0.8 * animations.duration.proposed
                easing.type: Easing.OutCubic
            }
        }

        Behavior on headThickness {
            NumberAnimation {
                duration: 0.8 * animations.duration.proposed
                easing.type: Easing.OutCubic
            }
        }
    }

    padding {
        Behavior on length {
            NumberAnimation {
                duration: 0.8 * animations.duration.proposed
                easing.type: Easing.OutCubic
            }
        }
    }
}
