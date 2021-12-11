/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

MouseArea{
    id: rulerMouseArea
    hoverEnabled: true
    cursorShape: root.isHorizontal ? Qt.SizeHorCursor : Qt.SizeVerCursor

    onVisibleChanged: {
        if (!visible) {
            tooltip.visible = false;
        }
    }

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
        var updateminimumlength = (plasmoid.configuration.maxLength === plasmoid.configuration.minLength);

        var tempML = plasmoid.configuration.maxLength + step;

        var value = Math.max(Math.min(tempML,100), 30);

        if (updateminimumlength) {
            plasmoid.configuration.minLength = Math.max(30, value);
        }

        value = Math.max(plasmoid.configuration.minLength, value);
        plasmoid.configuration.maxLength = value;

        var newTotal = Math.abs(plasmoid.configuration.offset) + value;

        //centered and justify alignments based on offset and get out of the screen in some cases
        var centeredCheck = ((plasmoid.configuration.alignment === LatteCore.Types.Center)
                             || (plasmoid.configuration.alignment === LatteCore.Types.Justify))
                && ((Math.abs(plasmoid.configuration.offset) + value/2) > 50);

        if (newTotal > 100 || centeredCheck) {
            if ((plasmoid.configuration.alignment === LatteCore.Types.Center)
                    || (plasmoid.configuration.alignment === LatteCore.Types.Justify)) {

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
