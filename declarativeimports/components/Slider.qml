/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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
import QtQuick.Templates 2.0 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

T.Slider {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
        Math.max(handle ? handle.implicitWidth : 0,
                 handle ? handle.implicitWidth : 0) + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
        Math.max(handle ? handle.implicitHeight : 0,
                 handle ? handle.implicitHeight : 0) + topPadding + bottomPadding)

    //padding: 3*units.smallSpacing //5//units.gridUnit
    topPadding: 3*units.smallSpacing
    bottomPadding: 3*units.smallSpacing
    leftPadding: units.smallSpacing
    rightPadding: units.smallSpacing

    snapMode: T.Slider.SnapOnRelease

    PlasmaCore.Svg {
        id: grooveSvg
        imagePath: "widgets/slider"
        colorGroup: PlasmaCore.ColorScope.colorGroup
    }

    handle: Item {
        property bool horizontal: control.orientation === Qt.Horizontal
        x: leftFixedPadding + (horizontal ? control.visualPosition * (control.availableWidth - width / 2) : (control.availableWidth - width) / 2)
        y: topFixedPadding + (horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height / 2))

        width: firstHandle.naturalSize.width
        height: firstHandle.naturalSize.height

        property int leftFixedPadding: horizontal && control.visualPosition === 0 ? 0 : control.leftPadding
        property int topFixedPadding: !horizontal && control.visualPosition === 0 ? 0 : control.topPadding

        Private.RoundShadow {
            anchors.fill: parent
            imagePath: "widgets/slider"
            focusElement: parent.horizontal ? "horizontal-slider-focus" : "vertical-slider-focus"
            hoverElement: parent.horizontal ? "horizontal-slider-hover" : "vertical-slider-hover"
            shadowElement: parent.horizontal ? "horizontal-slider-shadow" : "vertical-slider-shadow"
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "shadow")
        }
        PlasmaCore.SvgItem {
            id: firstHandle
            anchors.fill: parent
            svg: grooveSvg
            elementId: parent.horizontal ? "horizontal-slider-handle" : "vertical-slider-handle"
        }
    }

    background: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/slider"
        prefix: "groove"
        readonly property bool horizontal: control.orientation === Qt.Horizontal
        implicitWidth: horizontal ? units.gridUnit * 8 : margins.left + margins.right
        implicitHeight: horizontal ? margins.top + margins.bottom : units.gridUnit * 8
        width: horizontal ? control.availableWidth : implicitWidth
        height: horizontal ? implicitHeight : control.availableHeight
        anchors.centerIn: parent
        scale: horizontal && control.mirrored ? -1 : 1

        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            x: parent.horizontal ? 0 : (parent.width - width) / 2
            y: parent.horizontal ? (parent.height - height) / 2 : control.visualPosition * parent.height
            width: parent.horizontal ? control.position * parent.width + invisibleSpacer : parent.width
            height: parent.horizontal ? parent.height : control.position * parent.height + invisibleSpacer
            opacity: control.enabled ? 1 : 0.4

            property int invisibleSpacer: control.position === 0 ? 4 : 0
        }
    }
}
