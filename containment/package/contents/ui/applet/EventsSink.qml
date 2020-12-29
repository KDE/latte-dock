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

import QtQuick 2.7

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.latte.core 0.2 as LatteCore

Item {
    property Item destination: null

    readonly property int headThickness: appletItem.metrics.margin.thickness
    readonly property int tailThickness: destination ? destination.tailThicknessMargin : headThickness
    readonly property int lengthPadding: {
        if ( (root.panelAlignment === LatteCore.Types.Justify && appletItem.firstChildOfStartLayout)
            || (root.panelAlignment === LatteCore.Types.Justify && appletItem.lastChildOfEndLayout)
            || (root.panelAlignment !== LatteCore.Types.Justify && appletItem.firstChildOfMainLayout)
            || (root.panelAlignment !== LatteCore.Types.Justify && appletItem.lastChildOfMainLayout)) {
            //! Fitts Law on corners
            return appletItem.lengthAppletFullMargin;
        }

        return appletItem.lengthAppletPadding;
    }

    readonly property bool active: parent ? parent.active : false

    Loader{
        anchors.fill: parent
        active: appletItem.debug.eventsSinkEnabled && active

        sourceComponent: Rectangle {
            anchors.fill: parent
            color: "yellow"
            opacity: 0.2
            visible: root.latteView && root.latteView.sink.destinationItem === destination
        }
    }

    //! EventsSinkOriginArea gain "1px." margin from destination in order to not create
    //! endless loops between different EventsSinkOriginArea(s)

    EventsSinkOriginArea {
        id: topArea
        anchors.bottom: parent.top
        anchors.bottomMargin: 1
        anchors.horizontalCenter: parent.horizontalCenter

        width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? destination.width + 2 * lengthPadding : destination.width
        height: {
            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                return lengthPadding;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                return tailThickness;
            } else {
                return headThickness;
            }
        }
    }

    EventsSinkOriginArea {
        id: bottomArea
        anchors.top: parent.bottom
        anchors.topMargin: 1
        anchors.horizontalCenter: parent.horizontalCenter

        width: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? destination.width + 2 * lengthPadding : parent.width
        height: {
            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                return lengthPadding;
            } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                return tailThickness;
            } else {
                return headThickness;
            }
        }
    }

    EventsSinkOriginArea {
        id: leftArea
        anchors.right: parent.left
        anchors.rightMargin: 1
        anchors.verticalCenter: parent.verticalCenter

        height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? parent.height : destination.height + 2 * lengthPadding
        width: {
            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                return lengthPadding;
            } else if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                return tailThickness;
            } else {
                return headThickness;
            }
        }
    }

    EventsSinkOriginArea {
        id: rightArea
        anchors.left: parent.right
        anchors.leftMargin: 1
        anchors.verticalCenter: parent.verticalCenter

        height: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? parent.height : destination.height + 2 * lengthPadding
        width: {
            if (plasmoid.formFactor === PlasmaCore.Types.Horizontal) {
                return lengthPadding;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                return tailThickness;
            } else {
                return headThickness;
            }
        }
    }
}
