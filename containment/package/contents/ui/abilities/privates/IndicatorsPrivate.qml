/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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
