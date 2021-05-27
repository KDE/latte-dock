/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18n("Appearance")
         icon: "preferences-desktop-display-color"
         source: "config/ConfigAppearance.qml"
    }
    ConfigCategory {
         name: i18n("Panel")
         icon: "window-duplicate"
         source: "config/ConfigPanel.qml"
    }
    ConfigCategory {
         name: i18n("Interaction")
         icon: "preferences-system-windows-move"
         source: "config/ConfigInteraction.qml"
    }
}
