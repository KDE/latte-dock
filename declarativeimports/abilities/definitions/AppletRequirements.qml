/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0

Item{
    property bool activeIndicatorEnabled: true
    property bool latteSideColoringEnabled: true
    property bool latteIconOverlayEnabled: true
    property bool lengthMarginsEnabled: true
    property bool parabolicEffectLocked: false
    property bool screenEdgeMarginSupported: false
    property bool windowsTrackingEnabled: false

    property real innerZoomFactor: 1.0

    //   NAME: activeIndicatorEnabled
    //   EXPLANATION: when is TRUE, Latte can show its own Active Indicator
    //       when needed. For FALSE, the Latte Active Indicator is not drawn
    //       or used for that applet.
    //   SINCE: 0.9

    //   NAME: innerZoomFactor
    //   EXPLANATION: zoom factor that can be used from applets in order to draw
    //       some of their elements in specific cases, for example during animations.
    //       That zoom factor is irrelevant of parabolic effect and in used only
    //       occasionaly. It provides a hint for the container maximum zoom factor.
    //   SINCE: 0.10

    //   NAME: latteSideColoringEnabled
    //   EXPLANATION: when is FALSE, Latte is not painting/colorizing this applet
    //       in any case. In such case the applet can use 'palette'
    //       in order to access the color palette used at all cases from Latte
    //   USE CASE: when Latte is transparent and applets colors need to be adjusted in order
    //       to look consistent with the underlying desktop background OR the applet
    //       is not using monochromatic icons but rather colorful ones.
    //   SINCE: 0.9

    //   NAME: latteIconOverlayEnabled
    //   EXPLANATION: when is FALSE, Latte is not overlaying any icons above
    //       the applet or alters the applet visual in any sense.
    //       That means that the applet is responsible to provide a coherent
    //       parabolic effect experience.
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
