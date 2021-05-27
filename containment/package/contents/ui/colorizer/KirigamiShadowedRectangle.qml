/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.kirigami 2.12 as Kirigami

Kirigami.ShadowedRectangle {
    radius: painter.radius
    color: "transparent"
    shadow.size: main.shadowSize
    //! WORKAROUND, plasma theme shadow color compared to KirigamiShadowedRectangle drawn shadow has an alpha difference. This way
    //! we make sure that when the user uses the same shadow size with plasma theme original one we draw the same shadow compared visually
    shadow.color: Qt.rgba(main.shadowColor.r, main.shadowColor.g, main.shadowColor.b, Math.min(1, 0.336 + main.shadowColor.a))
}
