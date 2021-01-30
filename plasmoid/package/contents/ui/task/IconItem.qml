/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

import "animations" as TaskAnimations

//I am using  KQuickControlAddons.QIconItem even though onExit it triggers the following error
//QObject::~QObject: Timers cannot be stopped from another thread
//but it increases performance almost to double during animation

Item{
    property Item contentItemContainer: iconImageBuffer
    property Item titleTooltipVisualParent: titleTooltipParent
    property Item previewsTootipVisualParent: previewsTooltipParent

    TitleTooltipParent{
        id: titleTooltipParent
        thickness: taskItem.abilities.parabolic.factor.zoom * taskItem.abilities.metrics.totals.thickness
    }

    TitleTooltipParent{
        id: previewsTooltipParent
        thickness: (taskItem.abilities.parabolic.factor.zoom * taskItem.abilities.metrics.totals.thickness)
    }

    //!

    // KQuickControlAddons.QIconItem{
    Item{
        id: iconGraphic
        width: parent.width
        height: parent.height

        //fix bug #478, when changing form factor sometimes the tasks are not positioned
        //correctly, in such case we make a fast reinitialization for the sizes
        Connections {
            target: plasmoid

            onFormFactorChanged:{
                taskItem.inAddRemoveAnimation = false;

                wrapper.mScale = 1.01;
                wrapper.tempScaleWidth = 1.01;
                wrapper.tempScaleHeight = 1.01;

                wrapper.mScale = 1;
                wrapper.tempScaleWidth = 1;
                wrapper.tempScaleHeight = 1;
            }
        }

        Item {
            id: iconImageBuffer
            anchors.centerIn: parent
            width: newTempSize
            height: width

            property int zoomedSize: taskItem.abilities.parabolic.factor.zoom * taskItem.abilities.metrics.iconSize

            property real basicScalingWidth : wrapper.inTempScaling ? (taskItem.abilities.metrics.iconSize * wrapper.scaleWidth) :
                                                                      taskItem.abilities.metrics.iconSize * wrapper.mScale
            property real basicScalingHeight : wrapper.inTempScaling ? (taskItem.abilities.metrics.iconSize * wrapper.scaleHeight) :
                                                                       taskItem.abilities.metrics.iconSize * wrapper.mScale

            property real newTempSize: {
                if (wrapper.opacity === 1 ) {
                    return Math.min(basicScalingWidth, basicScalingHeight);
                } else {
                    return Math.max(basicScalingWidth, basicScalingHeight);
                }
            }

            readonly property Item contentItem: children.length > 0 ? children[0] : null


            //! Latte Side Painting-style if the user chose it
            Loader{
                anchors.fill: iconImageBuffer
                active: plasmoid.configuration.forceMonochromaticIcons

                sourceComponent: ColorOverlay {
                    anchors.fill: parent
                    color: latteBridge ? latteBridge.palette.textColor : "transparent"
                    source: iconImageBuffer.contentItem
                }
            }
            //! Latte Side Painting-style if the user chose it
        } //IconImageBuffer

        ShortcutBadge{
            id: shortcutBadge
        }
    }
}// Icon Item
