/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import org.kde.plasma.components 2.0 as PlasmaComponents
import "private" as Private

PlasmaComponents.CheckBox {
    property int value: 0

    onValueChanged: {
        if (partiallyCheckedEnabled) {
            checkedState = value;
        } else {
            checked = value;
        }
    }

    style: Private.CheckBoxStyle {}
}

