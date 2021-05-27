/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Item{
    property bool activeIndicatorEnabled: true
    property bool latteSideColoringEnabled: true
    property bool lengthMarginsEnabled: true
    property bool parabolicEffectLocked: false
    property bool screenEdgeMarginSupported: false
    property bool windowsTrackingEnabled: false

    //   NAME: activeIndicatorEnabled
    //   EXPLANATION: when is TRUE, Latte can show its own Active Indicator
    //       when needed. For FALSE, the Latte Active Indicator is not drawn
    //       or used for that applet.
    //   SINCE: 0.9

    //   NAME: latteSideColoringEnabled
    //   EXPLANATION: when is FALSE, Latte is not painting/colorizing this applet
    //       in any case. In such case the applet can use 'palette'
    //       in order to access the color palette used at all cases from Latte
    //   USE CASE: when Latte is transparent and applets colors need to be adjusted in order
    //       to look consistent with the underlying desktop background OR the applet
    //       is not using monochromatic icons but rather colorful ones.
    //   SINCE: 0.9

    //   NAME: lengthMarginsEnabled
    //   EXPLANATION: when is TRUE, all length margins are drawn including
    //       indicator padding and applets distance. For FALSE, length margins
    //       are not drawn at all.
    //   SINCE: 0.9

    //   NAME: parabolicEffectLocked
    //   EXPLANATION: For TRUE, this applet is considered locked
    //       and can not be zoomed.
    //   SINCE: 0.9

    //   NAME: screenEdgeMarginSupported
    //   EXPLANATION: For TRUE, this applet can handle the screen edge margin on
    //       its own and Latte should not take any special actions to help it
    //       work properly.
    //   SINCE: 0.10

    //   NAME: windowsTrackingEnabled
    //   EXPLANATION: when is TRUE, Latte is informed that applet requests
    //       windows tracking functionality.
    //   SINCE: 0.9
}
