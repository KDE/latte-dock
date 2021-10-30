/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.abilities.items 0.1 as AbilityItem

import "./privates" as Ability

Ability.IndicatorsPrivate {
    configuration: view && view.indicator && view.indicator.configuration ? view.indicator.configuration : null
    resources: view && view.indicator && view.indicator.resources ? view.indicator.resources : null

    isEnabled: view && view.indicator ? (view.indicator.enabled
                                                   && view.indicator.pluginIsReady
                                                   && view.indicator.configuration)
                                                : false

    type: view && view.indicator ? view.indicator.type : "org.kde.latte.default"

    indicatorComponent: view && view.indicator ? view.indicator.component : null

    info {
        enabledForApplets: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("enabledForApplets")
                           && indicatorLevel.item.enabledForApplets

        needsIconColors: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("needsIconColors")
                         && indicatorLevel.item.needsIconColors

        needsMouseEventCoordinates: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("needsMouseEventCoordinates")
                                    && indicatorLevel.item.needsMouseEventCoordinates

        providesFrontLayer: indicatorLevel.isLoaded &&  indicatorLevel.item.hasOwnProperty("providesFrontLayer")
                            && indicatorLevel.item.providesFrontLayer

        providesHoveredAnimation: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("providesHoveredAnimation")
                                  && indicatorLevel.item.providesHoveredAnimation

        providesClickedAnimation: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("providesClickedAnimation")
                                  && indicatorLevel.item.providesClickedAnimation

        providesInAttentionAnimation: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("providesInAttentionAnimation")
                                      && indicatorLevel.item.providesInAttentionAnimation

        providesGroupedWindowAddedAnimation: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("providesGroupedWindowAddedAnimation")
                                             && indicatorLevel.item.providesGroupedWindowAddedAnimation

        providesGroupedWindowRemovedAnimation: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("providesGroupedWindowRemovedAnimation")
                                               && indicatorLevel.item.providesGroupedWindowRemovedAnimation

        providesTaskLauncherAnimation: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("providesTaskLauncherAnimation")
                                       && indicatorLevel.item.providesTaskLauncherAnimation

        extraMaskThickness: {
            if (indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("extraMaskThickness")) {
                return indicatorLevel.item.extraMaskThickness;
            }

            return 0;
        }

        minThicknessPadding: {
            if (indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("minThicknessPadding")) {
                return indicatorLevel.item.minThicknessPadding;
            }

            return 0;
        }

        minLengthPadding: {
            if (indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("minLengthPadding")) {
                return indicatorLevel.item.minLengthPadding;
            }

            return 0;
        }

        lengthPadding: {
            if (indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("lengthPadding")) {
                return indicatorLevel.item.lengthPadding;
            }

            return 0.08;
        }

        appletLengthPadding: {
            if (indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("appletLengthPadding")) {
                return indicatorLevel.item.appletLengthPadding;
            }

            return -1;
        }

        backgroundCornerMargin: {
            if (indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("backgroundCornerMargin")) {
                return indicatorLevel.item.backgroundCornerMargin;
            }

            return 1.00;
        }

        svgPaths: indicatorLevel.isLoaded && indicatorLevel.item.hasOwnProperty("svgImagePaths") ?
                      indicatorLevel.item.svgImagePaths : []
    }

    readonly property Component plasmaStyleComponent: view && view.indicator ? view.indicator.plasmaComponent : null

    //! Metrics and values provided from an invisible indicator
    AbilityItem.IndicatorLevel{
        id: indicatorLevel
        opacity: 0
        level.isDrawn: root.indicators.isEnabled
        level.isBackground: true
        level.indicator: AbilityItem.IndicatorObject{
            animations: root.animations
            metrics: root.metrics
            host: root.indicators
        }

        readonly property bool isLoaded: active && item
    }
}
