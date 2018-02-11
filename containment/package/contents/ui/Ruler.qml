/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.latte 0.1 as Latte

Item{
    id: rulerItem

    opacity: behaveAsPlasmaPanel ? 0.7 : 1

    property int length: root.isHorizontal ? (root.behaveAsPlasmaPanel ? root.width - root.maxIconSize/4 : root.maxLength):
                                             (root.behaveAsPlasmaPanel ? root.height - root.maxIconSize/4 : root.maxLength)

    property color foregroundColor: "#d7e3ff"

    Connections{
        target: plasmoid
        onLocationChanged: initializeEditPosition();
    }

    Connections{
        target: root
        onMaxIconSizeChanged: initializeEditPosition();
        onPanelAlignmentChanged: initializeEditPosition();
        onOffsetChanged: initializeEditPosition();
        onMaxLengthChanged: initializeEditPosition();
        onEditModeChanged: {
            if (editMode) {
                initializeEditPosition();
            }
        }
    }

    Connections{
        target: editModeVisual

        onRootThicknessChanged: {
            rulerItem.initializeEditPosition();
        }

        onThicknessChanged: {
            rulerItem.initializeEditPosition();
        }
    }

    RowLayout{
        width: parent.width

        layer.enabled: true
        layer.effect: DropShadow{
            radius: 6
            fast: true
            samples: 2 * radius
            color: "#040404"
        }

        Rectangle{
            width: 2
            height: theme.defaultFont.pixelSize

            color: foregroundColor
        }

        Item{
            width: 0.6 * theme.defaultFont.pixelSize
            height: theme.defaultFont.pixelSize
            clip:true

            Rectangle{
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.right
                width: 0.75*theme.defaultFont.pixelSize
                height: width
                rotation: 45

                color: foregroundColor
            }
        }

        Rectangle{
            Layout.fillWidth: true
            height: 2
            color: foregroundColor
        }

        PlasmaComponents.Label{
            id: maxLengthLbl
            text: i18n("Maximum Length")
            color: foregroundColor
        }

        Rectangle{
            Layout.fillWidth: true
            height: 2
            color: foregroundColor
        }

        Item{
            width: 0.6 * theme.defaultFont.pixelSize
            height: theme.defaultFont.pixelSize
            clip:true

            Rectangle{
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.left
                width: 0.75*theme.defaultFont.pixelSize
                height: width
                rotation: 45

                color: foregroundColor
            }
        }

        Rectangle{
            width: 2
            height: theme.defaultFont.pixelSize

            color: foregroundColor
        }
    }


    function initializeEditPosition() {
        if (root.editMode) {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge){
                x = 0;
            } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                y =editModeVisual.thickness - 1.5 *theme.defaultFont.pixelSize;
            } else if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                y = editModeVisual.rootThickness - editModeVisual.thickness + 0.5 * theme.defaultFont.pixelSize;
            } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                x = editModeVisual.rootThickness - editModeVisual.thickness + 0.5 * theme.defaultFont.pixelSize;
            }

            if (root.isHorizontal) {
                if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                    x = root.width/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Left) {
                    x = root.offset;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    x = root.width/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Right) {
                    x = root.width - rulerItem.length - root.offset;
                }
            } else if (root.isVertical) {
                if (plasmoid.configuration.panelPosition === Latte.Dock.Justify) {
                    y = root.height/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Top) {
                    y = root.offset;
                } else if (root.panelAlignment === Latte.Dock.Center) {
                    y = root.height/2 - rulerItem.length/2 + root.offset;
                } else if (root.panelAlignment === Latte.Dock.Bottom) {
                    y = root.height - rulerItem.length - root.offset;
                }
            }
        }
    }
}
