/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2014 by Marco Martin <mart@kde.org>
*   Copyright (C) 2018 by Michail Vourlakos <mvourlakos@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.7
import org.kde.plasma.core 2.0 as PlasmaCore
import QtQuick.Controls 1.6 as QtControls
import QtQuick.Controls.Styles.Plasma 2.0 as Styles


/**
 * An interactive slider component with Plasma look and feel.
 *
 * @inherit QtQuick.Controls.Slider
 */
QtControls.Slider {
    id: slider

    /**
     * This property holds if a value indicator element will be shown while is
     * dragged or not.
     *
     * @warning The value indicator is not implemented in the Plasma Slider.
     *
     * The default value is false.
     */
    property bool valueIndicatorVisible: false

    /**
     * This property holds the text being displayed in the value indicator.
     *
     * @warning The value indicator is not implemented in the Plasma Slider.
     */
    property string valueIndicatorText: value

    /**
     * type:bool
     * This property holds if the slider visualizations has an inverted
     * direction.
     *
     * @warning: deprecated and not supported, here for retrocompatibility
     */
    property bool inverted: false

    width: slider.isVertical ? theme.mSize(theme.defaultFont).height*1.6 : 200
    height: slider.isVertical ? 200 : theme.mSize(theme.defaultFont).height*1.6
    // TODO: needs to define if there will be specific graphics for
    //     disabled sliders
    opacity: enabled ? 1.0 : 0.5

    activeFocusOnTab: true

    //FIXME: remove those 2 functions once we can depend from 5.4*/
    function accessibleIncreaseAction() { increase() }
    function accessibleDecreaseAction() { decrease() }

    function increase() {
        if (!enabled)
            return;
        if (inverted)
            value += stepSize;
        else
            value -= stepSize;
    }
    function decrease() {
        if (!enabled)
            return;
        if (inverted)
            value -= stepSize;
        else
            value += stepSize;
    }

    style: Styles.SliderStyle {}
}
