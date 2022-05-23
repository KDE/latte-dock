/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte.core 0.2 as LatteCore

import "controls" as SettingsControls
import "maxlength" as MaximumLength

//import "../../code/ColorizerTools.js" as ColorizerTools

Item{
    id: settingsRoot
    readonly property bool containsMouse: false /*headerSettings.containsMouse || ruler.containsMouse
                                          || tooltipMouseArea.containsMouse || editBackMouseArea.containsMouse*/
    readonly property int thickness: ruler.thickness + headerSettings.thickness + spacing * 6
    readonly property int spacing: 4

    property int textShadow: {
        if (textColorIsDark)  {
            return 1;
        } else {
            return 6;
        }
    }

    property string tooltip: ""

    readonly property real textColorBrightness: LatteCore.Tools.colorBrightness(textColor)
    readonly property bool textColorIsDark: textColorBrightness < 127.5

    readonly property color bestContrastedTextColor: {
        if (imageTiler.opacity <= 0.4 && !universalSettings.inConfigureAppletsMode && themeExtended) {
            return latteView.colorizer.currentBackgroundBrightness > 127.5 ?
                        themeExtended.lightTheme.textColor :
                        themeExtended.darkTheme.textColor;
        }

        return latteView && latteView.layout ? latteView.layout.textColor : "#D7E3FF";
    }

    readonly property color textColor: bestContrastedTextColor

    layer.enabled: graphicsSystem.isAccelerated
    layer.effect: DropShadow{
        radius: settingsRoot.textShadow
        fast: true
        samples: 2 * radius
        color: root.appShadowColorSolid
    }

    HeaderSettings{
        id: headerSettings
    }

    MaximumLength.Ruler {
        id: ruler
        thicknessMargin: headerSettings.thickness + 3 * spacing
        thickMargin: 3
    }
}
