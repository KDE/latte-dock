/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import org.kde.latte.abilities.containers 0.1 as ContainerAbility

import "./metrics" as MetricsPrivateTypes

ContainerAbility.Metrics {
    id: mets
    property Item animations: null
    property Item autosize: null
    property Item background: null
    property Item indicators: null

    //! private properties to avoid too many not needed animation calculations
    readonly property int _iconSize: autosizeEnabled && autosize.iconSize > 0 ? Math.min(autosize.iconSize, maxIconSize) : maxIconSize
    readonly property int _maxIconSize: portionIconSize!==-1 ? portionIconSize : plasmoid.configuration.iconSize

    //! Private Properties
    readonly property int portionIconSize: { //icon size based on screen height
        if ((plasmoid.configuration.proportionIconSize===-1) || !latteView)
            return -1;

        return Math.max(16,Math.round(latteView.screenGeometry.height * plasmoid.configuration.proportionIconSize/100/8)*8);
    }

    readonly property bool autosizeEnabled: autosize !== undefined && autosize.isActive

    readonly property MetricsPrivateTypes.Fraction fraction: MetricsPrivateTypes.Fraction{
        thicknessMargin: root.shrinkThickMargins ? indicators.info.minThicknessPadding :
                                                   Math.max(indicators.info.minThicknessPadding, plasmoid.configuration.thickMargin / 100)

        lengthMargin: plasmoid.configuration.lengthExtMargin / 100
        lengthPadding: indicators.isEnabled ? indicators.padding : 0
        lengthAppletPadding: indicators.infoLoaded ? indicators.info.appletLengthPadding : -1
    }

    //! based on background / plasma theme minimum thickness requirements; behaveAsPlasmaPanel and floating is a good scenario for this
    readonly property int marginMinThickness: Math.max(0, (background.totals.minThickness - _maxIconSize) / 2)

    //! BEHAVIORS
    Behavior on iconSize {
        enabled: !(root.editMode && root.behaveAsPlasmaPanel)
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
                     && !editModeVisual.editAnimationRunning /*avoid slide-out animation when from editMode we change to real floating*/

            NumberAnimation {
                duration: 0.8 * animations.duration.proposed
                easing.type: Easing.OutCubic
            }
        }

        Behavior on thickness {
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
