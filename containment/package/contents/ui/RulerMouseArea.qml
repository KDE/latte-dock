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

MouseArea{
    enabled: root.editMode && !dock.visibility.isHidden

    cursorShape: {
        if (enabled) {
            return root.isHorizontal ? Qt.SizeHorCursor : Qt.SizeVerCursor
        } else {
            return Qt.ArrowCursor;
        }
    }

    hoverEnabled: enabled

    onWheel: {
        var angle = wheel.angleDelta.y / 8;

        if (angle > 12) {
            updateMaxLength(6);
        } else if (angle < -12) {
            updateMaxLength(-6);
        }
    }

    //! replica of updating maximum length from configuration tab
    function updateMaxLength(step) {
        var tempML = plasmoid.configuration.maxLength + step;

        var value = Math.max(Math.min(tempML,100),30);

        plasmoid.configuration.maxLength = value;

        var newTotal = Math.abs(plasmoid.configuration.offset) + value;

        //centered and justify alignments based on offset and get out of the screen in some cases
        var centeredCheck = ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                             || (plasmoid.configuration.panelPosition === Latte.Dock.Justify))
                && ((Math.abs(plasmoid.configuration.offset) + value/2) > 50);

        if (newTotal > 100 || centeredCheck) {
            if ((plasmoid.configuration.panelPosition === Latte.Dock.Center)
                    || (plasmoid.configuration.panelPosition === Latte.Dock.Justify)) {

                var suggestedValue = (plasmoid.configuration.offset<0) ? Math.min(0, -(100-value)): Math.max(0, 100-value);

                if ((Math.abs(suggestedValue) + value/2) > 50) {
                    if (suggestedValue < 0) {
                        suggestedValue = - (50 - value/2);
                    } else {
                        suggestedValue = 50 - value/2;
                    }
                }

                plasmoid.configuration.offset = suggestedValue;
            } else {
                plasmoid.configuration.offset = Math.max(0, 100-value);
            }
        }
    }
}
