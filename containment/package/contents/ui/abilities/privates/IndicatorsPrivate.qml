/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.latte.abilities.host 0.1 as AbilityHost

AbilityHost.Indicators {
    id: _indicators
    property QtObject view: null

    Connections {
        target: _indicators.info
        onSvgPathsChanged: {
            if (_indicators.isEnabled) {
                view.indicator.resources.setSvgImagePaths(_indicators.info.svgPaths);
            }
        }
    }

    Connections {
        target:_indicators
        onIsEnabledChanged: {
            if (_indicators.isEnabled) {
                view.indicator.resources.setSvgImagePaths(_indicators.info.svgPaths);
            }
        }
    }

    //! Bindings in order to inform View::Indicator
    Binding{
        target: view && view.indicator ? view.indicator : null
        property:"enabledForApplets"
        when: view && view.indicator
        value: _indicators.info.enabledForApplets
    }

    //! Bindings in order to inform View::Indicator::Info
    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"needsIconColors"
        when: view && view.indicator
        value: _indicators.info.needsIconColors
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"needsMouseEventCoordinates"
        when: view && view.indicator
        value: _indicators.info.needsMouseEventCoordinates
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"providesClickedAnimation"
        when: view && view.indicator
        value: _indicators.info.providesClickedAnimation
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"providesHoveredAnimation"
        when: view && view.indicator
        value: _indicators.info.providesHoveredAnimation
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"providesInAttentionAnimation"
        when: view && view.indicator
        value: _indicators.info.providesInAttentionAnimation
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"providesTaskLauncherAnimation"
        when: view && view.indicator
        value: _indicators.info.providesTaskLauncherAnimation
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"providesGroupedWindowAddedAnimation"
        when: view && view.indicator
        value: _indicators.info.providesGroupedWindowAddedAnimation
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"providesGroupedWindowRemovedAnimation"
        when: view && view.indicator
        value: _indicators.info.providesGroupedWindowRemovedAnimation
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"providesFrontLayer"
        when: view && view.indicator
        value: _indicators.info.providesFrontLayer
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"extraMaskThickness"
        when: view && view.indicator
        value: _indicators.info.extraMaskThickness
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"minLengthPadding"
        when: view && view.indicator
        value: _indicators.info.minLengthPadding
    }

    Binding{
        target: view && view.indicator ? view.indicator.info : null
        property:"minThicknessPadding"
        when: view && view.indicator
        value: _indicators.info.minThicknessPadding
    }
}
